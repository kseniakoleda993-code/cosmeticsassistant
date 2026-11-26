#include "cosmetics/dialog/dialog.hpp"
#include "cosmetics/catalog/category_positions.hpp"
#include "cosmetics/dialog/summary.hpp"

#include <algorithm>

namespace cosmetics {
namespace {

Message category_prompt(const User& user) {
    Message message{"Что ты хочешь выбрать?"};
    for (auto category : categories) {
        message.buttons.emplace_back(to_text(category));
    }
    if (!user.cart().items().empty()) {
        message.buttons.push_back("корзина");
    }
    return message;
}

Message position_prompt(Category category) {
    Message message{"Что именно с этого раздела ты хочешь найти?"};
    for (auto position : positions_for(category)) {
        message.buttons.emplace_back(to_text(position));
    }
    return message;
}

Message budget_prompt() {
    return {"Отлично, какой у тебя бюджет?", {std::string(to_text(Budget::Average)), std::string(to_text(Budget::AboveAverage))}};
}

Message continue_prompt() {
    return {"Нужно ли подобрать что-то ещё?", {"да", "нет", "корзина"}};
}

User new_selection(const Cart& cart) {
    return User().with_cart(cart).with_stage(Stage::Category);
}

const std::vector<std::string> end_buttons{"подобрать ещё", "корзина"};

Transition greeting(const User& user) {
    auto next = new_selection(user.cart().clear());
    return {next, {{"Привет!"}, {"Я твой помощник по подбору косметики"}, category_prompt(next)}};
}

class StartStep final : public Step {
public:
    Message prompt(const User&) const override {
        return {"Нажми /start, чтобы начать.", {"/start"}};
    }

    std::optional<Transition> advance(const User& user, std::string_view action) const override {
        if (action == "/start") {
            return greeting(user);
        }
        return std::nullopt;
    }
};

class CategoryStep final : public Step {
public:
    Message prompt(const User& user) const override {
        return category_prompt(user);
    }

    std::optional<Transition> advance(const User& user, std::string_view action) const override {
        auto category = category_from_text(action);
        if (!category) {
            return std::nullopt;
        }
        auto next = user.with_category(category.value()).with_stage(Stage::Position);
        return Transition{next, {position_prompt(category.value())}};
    }
};

class PositionStep final : public Step {
public:
    Message prompt(const User& user) const override {
        if (!user.category()) {
            throw std::logic_error("Не выбран раздел");
        }
        return position_prompt(*user.category());
    }

    std::optional<Transition> advance(const User& user, std::string_view action) const override {
        if (!user.category()) {
            throw std::logic_error("Не выбран раздел");
        }
        auto position = position_from_text(action);
        const auto& available = positions_for(*user.category());
        if (!position || std::find(available.begin(), available.end(), position.value()) == available.end()) {
            return std::nullopt;
        }
        auto next = user.with_position(position.value()).with_stage(Stage::Budget);
        return Transition{next, {budget_prompt()}};
    }
};

class BudgetStep final : public Step {
public:
    explicit BudgetStep(const Catalog& catalog) : catalog(catalog) {}

    Message prompt(const User&) const override {
        return budget_prompt();
    }

    std::optional<Transition> advance(const User& user, std::string_view action) const override {
        auto budget = budget_from_text(action);
        if (!budget) {
            return std::nullopt;
        }
        if (!user.position()) {
            throw std::logic_error("Не выбрана позиция");
        }
        auto offer = catalog.find(*user.position(), budget.value());
        if (!offer) {
            return Transition{user, {{"Не удалось подобрать товар. Попробуй ещё раз."}, prompt(user)}};
        }
        const auto& item = offer.value().product;
        std::string caption = "Хорошо, вот что я подобрал:\n" + item.name() + "\n" + format_price(item.price()) + " BYN";
        auto next = user.with_budget(budget.value()).with_cart(user.cart().add(item)).with_stage(Stage::Continue);
        return Transition{next, {{caption, {}, offer.value().photo}, continue_prompt()}};
    }

private:
    const Catalog& catalog;
};

class ContinueStep final : public Step {
public:
    Message prompt(const User&) const override {
        return continue_prompt();
    }

    std::optional<Transition> advance(const User& user, std::string_view action) const override {
        if (action == "да") {
            return Transition{new_selection(user.cart()), {category_prompt(user)}};
        }
        if (action == "нет") {
            return Transition{user.with_stage(Stage::End), {{cart_summary(user.cart()), end_buttons}}};
        }
        return std::nullopt;
    }
};

class EndStep final : public Step {
public:
    Message prompt(const User& user) const override {
        return {cart_summary(user.cart()), end_buttons};
    }

    std::optional<Transition> advance(const User& user, std::string_view action) const override {
        if (action == "/start") {
            return greeting(user);
        }
        if (action == "подобрать ещё") {
            return Transition{new_selection(user.cart()), {category_prompt(user)}};
        }
        return std::nullopt;
    }
};

constexpr std::size_t page_size = 8;

std::size_t last_page(const Cart& cart) {
    return cart.items().empty() ? 0 : (cart.items().size() - 1) / page_size;
}

std::string remove_action(const Cart& cart, std::size_t index) {
    return "cart:" + std::to_string(cart.revision()) + ":" + std::to_string(index);
}

std::string short_name(std::string name) {
    std::size_t length = 280;
    if (name.size() <= length) {
        return name;
    }
    while ((static_cast<unsigned char>(name[length]) & 0xc0) == 0x80) {
        --length;
    }
    return name.substr(0, length) + "…";
}

class CartStep final : public Step {
public:
    Message prompt(const User& user) const override {
        const auto& cart = user.cart();
        const auto& items = cart.items();
        std::size_t page = std::min(user.cart_page(), last_page(cart));
        std::size_t start = page * page_size;
        std::size_t end = std::min(start + page_size, items.size());
        Message message{items.empty() ? "Корзина пуста.\n" : "Корзина. Нажми кнопку, чтобы убрать товар.\n"};
        for (std::size_t i = start; i < end; ++i) {
            std::string number = std::to_string(i + 1);
            message.text += "\n" + number + ". " + short_name(items[i].name()) + " — " + format_price(items[i].price()) + " BYN";
            add_button(message, "удалить №" + number, remove_action(cart, i));
        }
        message.text += "\nИтого: " + format_price(cart.total()) + " BYN";
        if (page > 0) {
            add_button(message, "← предыдущая", "cart:prev");
        }
        if (page < last_page(cart)) {
            add_button(message, "следующая →", "cart:next");
        }
        add_button(message, "назад", "cart:back");
        return message;
    }

    std::optional<Transition> advance(const User& user, std::string_view action) const override {
        const auto& cart = user.cart();
        std::size_t page = std::min(user.cart_page(), last_page(cart));
        if (action == "cart:prev" && page > 0) {
            auto next = user.with_cart_page(page - 1);
            return Transition{next, {prompt(next)}};
        }
        if (action == "cart:next" && page < last_page(cart)) {
            auto next = user.with_cart_page(page + 1);
            return Transition{next, {prompt(next)}};
        }
        std::size_t start = page * page_size;
        std::size_t end = std::min(start + page_size, cart.items().size());
        for (std::size_t i = start; i < end; ++i) {
            if (action == remove_action(cart, i)) {
                auto trimmed = cart.remove(i);
                auto next = user.with_cart(trimmed).with_cart_page(std::min(page, last_page(trimmed)));
                auto message = prompt(next);
                message.text.insert(0, "Товар удалён.\n");
                return Transition{next, {std::move(message)}};
            }
        }
        return std::nullopt;
    }

private:
    static void add_button(Message& message, std::string label, std::string action) {
        message.buttons.push_back(std::move(label));
        message.callbacks.push_back(std::move(action));
    }
};
}

Dialog::Dialog(const Catalog& catalog) : steps{std::make_unique<StartStep>(), std::make_unique<CategoryStep>(), std::make_unique<PositionStep>(), std::make_unique<BudgetStep>(catalog), std::make_unique<ContinueStep>(), std::make_unique<EndStep>(), std::make_unique<CartStep>()} {}

const Step& Dialog::step(Stage stage) const {
    return *steps.at(static_cast<std::size_t>(stage));
}

Transition Dialog::handle(const User& user, std::string_view action) const {
    if (action == "/start") {
        return step(Stage::Start).handle(user, action);
    }
    if (action == "корзина" || action == "/cart") {
        auto next = user.stage() == Stage::Cart ? user : user.open_cart();
        return {next, {step(Stage::Cart).prompt(next)}};
    }
    if (user.stage() == Stage::Cart && action == "cart:back") {
        auto next = user.close_cart();
        return {next, {step(next.stage()).prompt(next)}};
    }
    return step(user.stage()).handle(user, action);
}

}
