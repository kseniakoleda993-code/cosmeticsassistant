#pragma once

#include "cosmetics/core/result.hpp"

#include <array>
#include <string_view>

namespace cosmetics {

enum class Category { Hair, Body, Skin, Makeup };

inline constexpr std::array<Category, 4> categories{
    Category::Hair, Category::Body, Category::Skin, Category::Makeup
};

std::string_view to_text(Category category);
Result<Category> category_from_text(std::string_view text);

}
