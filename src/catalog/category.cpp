#include "cosmetics/catalog/category.hpp"

#include <stdexcept>

namespace cosmetics {
namespace {

constexpr std::array<std::string_view, 4> labels{
    "уход для волос", "уход для тела", "уходовая косметика", "декоративная косметика"
};

}

std::string_view to_text(Category category) {
    for (std::size_t i = 0; i < categories.size(); ++i) {
        if (categories[i] == category) {
            return labels[i];
        }
    }
    throw std::invalid_argument("Неизвестный раздел каталога");
}

Result<Category> category_from_text(std::string_view text) {
    for (std::size_t i = 0; i < categories.size(); ++i) {
        if (labels[i] == text) {
            return Result<Category>::success(categories[i]);
        }
    }
    return Result<Category>::failure("Неизвестный раздел каталога");
}

}
