#include "cosmetics/dialog/dialog.hpp"

#include <gtest/gtest.h>

using namespace cosmetics;

TEST(Dialog, FullFlowAndButtonOrder) {
    SampleCatalog catalog;
    Dialog dialog(catalog);
    auto start = dialog.handle(User(), "/start");
    ASSERT_EQ(start.messages.size(), 3u);
    EXPECT_EQ(start.messages[0].text, "Привет!");
    EXPECT_EQ(start.messages[1].text, "Я твой помощник по подбору косметики");
    EXPECT_EQ(start.messages[2].text, "Что ты хочешь выбрать?");
    EXPECT_EQ(start.messages[2].buttons.size(), 4u);
    auto category = dialog.handle(start.user, "уход для волос");
    EXPECT_EQ(category.user.stage(), Stage::Position);
    auto old_button = dialog.handle(category.user, "помада");
    EXPECT_EQ(old_button.user.stage(), Stage::Position);
    auto position = dialog.handle(category.user, "шампунь");
    EXPECT_EQ(position.user.stage(), Stage::Budget);
    auto unrelated = dialog.handle(position.user, "уход для волос");
    EXPECT_EQ(unrelated.user.stage(), Stage::Budget);
    EXPECT_TRUE(unrelated.user.cart().items().empty());
    ASSERT_EQ(unrelated.messages.size(), 2u);
    EXPECT_EQ(unrelated.messages[1].buttons, (std::vector<std::string>{"средний", "выше среднего"}));
    auto offer = dialog.handle(position.user, "средний");
    EXPECT_EQ(offer.user.stage(), Stage::Continue);
    EXPECT_EQ(offer.user.cart().items().size(), 1u);
    EXPECT_EQ(offer.messages[1].buttons, (std::vector<std::string>{"да", "нет", "корзина"}));
    auto end = dialog.handle(offer.user, "нет");
    EXPECT_EQ(end.user.stage(), Stage::End);
    EXPECT_NE(end.messages[0].text.find("11.00 BYN"), std::string::npos);
    auto idle = dialog.handle(end.user, "да");
    EXPECT_EQ(idle.user.stage(), Stage::End);
    auto restart = dialog.handle(end.user, "/start");
    EXPECT_EQ(restart.user.stage(), Stage::Category);
    EXPECT_TRUE(restart.user.cart().items().empty());
}

TEST(Dialog, TwoPicksAndRepeatedPrompts) {
    SampleCatalog catalog;
    Dialog dialog(catalog);
    User user;
    std::vector<std::string> actions = {"/start", "уход для волос", "шампунь", "средний", "да", "уход для тела", "гель для душа", "выше среднего", "нет"};
    for (const auto& action : actions) {
        EXPECT_FALSE(dialog.step(user.stage()).prompt(user).buttons.empty());
        user = dialog.handle(user, action).user;
    }
    EXPECT_EQ(user.stage(), Stage::End);
    EXPECT_EQ(user.cart().total(), 3800);
    EXPECT_EQ(dialog.step(user.stage()).prompt(user).buttons, (std::vector<std::string>{"подобрать ещё", "корзина"}));
}

TEST(Dialog, InvalidStateAndCatalogFailure) {
    class EmptyCatalog : public Catalog {
        Result<Offer> find(Position, Budget) const override { return Result<Offer>::failure("нет товара"); }
    } catalog;
    Dialog dialog(catalog);
    EXPECT_EQ(dialog.handle(User(), "что-то").user.stage(), Stage::Start);
    EXPECT_EQ(dialog.handle(User().with_stage(Stage::Category), "шампунь").user.stage(), Stage::Category);
    EXPECT_THROW(dialog.step(Stage::Position).prompt(User()), std::logic_error);
    EXPECT_THROW(dialog.handle(User().with_stage(Stage::Position), "шампунь"), std::logic_error);
    EXPECT_THROW(dialog.handle(User().with_stage(Stage::Budget), "средний"), std::logic_error);
    EXPECT_EQ(dialog.handle(User().with_stage(Stage::Continue), "шампунь").user.stage(), Stage::Continue);
    auto user = User().with_stage(Stage::Budget).with_position(Position::Shampoo);
    auto result = dialog.handle(user, "средний");
    EXPECT_EQ(result.user.stage(), Stage::Budget);
    EXPECT_TRUE(result.user.cart().items().empty());
    EXPECT_EQ(result.messages.size(), 2u);
}

TEST(Dialog, RemovesItemFromFinishedCartAndResumesPicking) {
    SampleCatalog catalog;
    Dialog dialog(catalog);
    auto first = Cart().add(Product("Шампунь", 2800));
    auto first_view = dialog.handle(User().with_cart(first), "/cart");
    auto cart = first.add(Product("Маска", 6500)).add(Product("Шампунь", 2800));
    auto end = dialog.handle(User().with_cart(cart).with_stage(Stage::Continue), "нет");
    ASSERT_EQ(end.user.stage(), Stage::End);
    ASSERT_EQ(end.messages.front().buttons, (std::vector<std::string>{"подобрать ещё", "корзина"}));
    auto opened = dialog.handle(end.user, "корзина");
    ASSERT_EQ(opened.user.stage(), Stage::Cart);
    ASSERT_EQ(opened.messages.front().callbacks.size(), 4u);
    EXPECT_NE(opened.messages.front().text.find("121.00 BYN"), std::string::npos);
    const auto old_action = opened.messages.front().callbacks[1];
    auto removed = dialog.handle(opened.user, old_action);
    ASSERT_EQ(removed.user.cart().items().size(), 2u);
    EXPECT_EQ(removed.user.cart().total(), 5600);
    EXPECT_NE(removed.messages.front().text.find("56.00 BYN"), std::string::npos);
    auto stale = dialog.handle(removed.user, old_action);
    EXPECT_EQ(stale.user.cart().total(), 5600);
    auto back = dialog.handle(stale.user, "cart:back");
    EXPECT_EQ(back.user.stage(), Stage::End);
    EXPECT_NE(back.messages.front().text.find("56.00 BYN"), std::string::npos);
    auto resumed = dialog.handle(back.user, "подобрать ещё");
    EXPECT_EQ(resumed.user.stage(), Stage::Category);
    EXPECT_EQ(resumed.user.cart().total(), 5600);
    auto restarted = dialog.handle(resumed.user, "/start");
    EXPECT_TRUE(restarted.user.cart().items().empty());
    auto selected = dialog.handle(restarted.user, "уход для волос");
    auto shampoo = dialog.handle(selected.user, "шампунь");
    auto offer = dialog.handle(shampoo.user, "средний");
    auto reopened = dialog.handle(offer.user, "/cart");
    ASSERT_EQ(reopened.user.cart().items().size(), 1u);
    const auto first_action = first_view.messages.front().callbacks.front();
    EXPECT_NE(reopened.messages.front().callbacks.front(), first_action);
    auto repeated = dialog.handle(reopened.user, first_action);
    EXPECT_EQ(repeated.user.cart().items().size(), 1u);
}

TEST(Dialog, EmptyCartAndPaginationAfterRemoval) {
    SampleCatalog catalog;
    Dialog dialog(catalog);
    auto empty = dialog.handle(User(), "/cart");
    EXPECT_EQ(empty.user.stage(), Stage::Cart);
    EXPECT_NE(empty.messages.front().text.find("0.00 BYN"), std::string::npos);
    EXPECT_EQ(empty.messages.front().callbacks, (std::vector<std::string>{"cart:back"}));

    Cart cart;
    for (int i = 0; i < 9; ++i) {
        cart = cart.add(Product("Товар", 1000));
    }
    auto opened = dialog.handle(User().with_cart(cart), "/cart");
    auto next = dialog.handle(opened.user, "cart:next");
    EXPECT_EQ(next.user.cart_page(), 1u);
    EXPECT_EQ(dialog.handle(next.user, "cart:prev").user.cart_page(), 0u);
    ASSERT_EQ(next.messages.front().buttons.front(), "удалить №9");
    auto removed = dialog.handle(next.user, next.messages.front().callbacks.front());
    EXPECT_EQ(removed.user.cart_page(), 0u);
    EXPECT_EQ(removed.user.cart().total(), 8000);
}
