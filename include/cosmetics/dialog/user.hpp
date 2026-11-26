#pragma once

#include "cosmetics/catalog/cart.hpp"
#include "cosmetics/catalog/category.hpp"
#include "cosmetics/catalog/catalog.hpp"

#include <optional>

namespace cosmetics {

enum class Stage { Start, Category, Position, Budget, Continue, End, Cart };

class User {
public:
    Stage stage() const { return current; }
    std::optional<Category> category() const { return selected_category; }
    std::optional<Position> position() const { return selected_position; }
    std::optional<Budget> budget() const { return selected_budget; }
    const Cart& cart() const { return basket; }
    std::size_t cart_page() const { return page; }

    User open_cart() const {
        User copy = *this;
        copy.return_stage = current;
        copy.current = Stage::Cart;
        copy.page = 0;
        return copy;
    }

    User close_cart() const {
        return with_stage(return_stage);
    }

    User with_cart_page(std::size_t index) const {
        User copy = *this;
        copy.page = index;
        return copy;
    }

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
    Stage return_stage = Stage::Start;
    std::size_t page = 0;
};

}
