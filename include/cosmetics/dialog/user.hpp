#pragma once

#include "cosmetics/catalog/cart.hpp"
#include "cosmetics/catalog/category.hpp"
#include "cosmetics/catalog/catalog.hpp"

#include <optional>

namespace cosmetics {

enum class Stage { Start, Category, Position, Budget, Continue, End };

class User {
public:
    Stage stage() const { return current; }
    std::optional<Category> category() const { return selected_category; }
    std::optional<Position> position() const { return selected_position; }
    std::optional<Budget> budget() const { return selected_budget; }
    const Cart& cart() const { return basket; }

    User with_stage(Stage stage) const {
        User copy = *this;
        copy.current = stage;
        return copy;
    }

    User with_category(std::optional<Category> category) const {
        User copy = *this;
        copy.selected_category = category;
        return copy;
    }

    User with_position(std::optional<Position> position) const {
        User copy = *this;
        copy.selected_position = position;
        return copy;
    }

    User with_budget(std::optional<Budget> budget) const {
        User copy = *this;
        copy.selected_budget = budget;
        return copy;
    }

    User with_cart(const Cart& cart) const {
        User copy = *this;
        copy.basket = cart;
        return copy;
    }

private:
    Stage current = Stage::Start;
    std::optional<Category> selected_category;
    std::optional<Position> selected_position;
    std::optional<Budget> selected_budget;
    Cart basket;
};

}
