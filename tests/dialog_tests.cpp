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
    EXPECT_EQ(offer.messages[1].buttons, (std::vector<std::string>{"да", "нет"}));
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
    EXPECT_TRUE(dialog.step(user.stage()).prompt(user).buttons.empty());
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
