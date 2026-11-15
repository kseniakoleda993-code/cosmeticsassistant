#include "cosmetics/dialog/dialog.hpp"
#include "cosmetics/catalog/category_positions.hpp"
#include "cosmetics/dialog/summary.hpp"

#include <algorithm>

namespace cosmetics {
namespace {

Message category_prompt() {
    Message message{"Что ты хочешь выбрать?"};
    for (auto category : categories) {
        message.buttons.emplace_back(to_text(category));
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
    return {"Нужно ли подобрать что-то ещё?", {"да", "нет"}};
}

Transition greeting() {
    return {User().with_stage(Stage::Category), {{"Привет!"}, {"Я твой помощник по подбору косметики"}, category_prompt()}};
}

class StartStep final : public Step {
public:
    Message prompt(const User&) const override {
        return {"Нажми /start, чтобы начать.", {"/start"}};
    }

    std::optional<Transition> advance(const User&, std::string_view action) const override {
        if (action == "/start") {
            return greeting();
        }
        return std::nullopt;
    }
};

class CategoryStep final : public Step {
public:
    Message prompt(const User&) const override {
        return category_prompt();
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
            auto next = User().with_cart(user.cart()).with_stage(Stage::Category);
            return Transition{next, {category_prompt()}};
        }
        if (action == "нет") {
            return Transition{user.with_stage(Stage::End), {{cart_summary(user.cart())}}};
        }
        return std::nullopt;
    }
};

class EndStep final : public Step {
public:
    Message prompt(const User&) const override {
        return {"Чтобы начать новый подбор, нажми /start."};
    }

    std::optional<Transition> advance(const User&, std::string_view action) const override {
        if (action == "/start") {
            return greeting();
        }
        return std::nullopt;
    }
};

}

Dialog::Dialog(const Catalog& catalog) : steps{std::make_unique<StartStep>(), std::make_unique<CategoryStep>(), std::make_unique<PositionStep>(), std::make_unique<BudgetStep>(catalog), std::make_unique<ContinueStep>(), std::make_unique<EndStep>()} {}

const Step& Dialog::step(Stage stage) const {
    return *steps.at(static_cast<std::size_t>(stage));
}

Transition Dialog::handle(const User& user, std::string_view action) const {
    if (action == "/start") {
        return step(Stage::Start).handle(user, action);
    }
    return step(user.stage()).handle(user, action);
}

}
