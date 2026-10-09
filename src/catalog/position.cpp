#include "cosmetics/catalog/position.hpp"

#include <stdexcept>

namespace cosmetics {
namespace {

constexpr std::array<std::string_view, 19> labels{
    "шампунь", "кондиционер", "маска", "масло", "термозащита", "сухой шампунь",
    "гель для душа", "скраб для тела", "крем/лосьон для тела",
    "скраб", "гель для умывания", "тонер", "энзимная пудра",
    "тональная основа", "румяна", "тушь", "помада", "гель для бровей", "консилер"
};

}

std::string_view to_text(Position position) {
    for (std::size_t i = 0; i < positions.size(); ++i) {
        if (positions[i] == position) {
            return labels[i];
        }
    }
    throw std::invalid_argument("Неизвестная позиция каталога");
}

Result<Position> position_from_text(std::string_view text) {
    for (std::size_t i = 0; i < positions.size(); ++i) {
        if (labels[i] == text) {
            return Result<Position>::success(positions[i]);
        }
    }
    return Result<Position>::failure("Неизвестная позиция каталога");
}

}
