#include "cosmetics/catalog/budget.hpp"

#include <stdexcept>

namespace cosmetics {

std::string_view to_text(Budget budget) {
    switch (budget) {
        case Budget::Average: return "средний";
        case Budget::AboveAverage: return "выше среднего";
    }
    throw std::invalid_argument("Неизвестный бюджет");
}

Result<Budget> budget_from_text(std::string_view text) {
    for (auto budget : budgets) {
        if (to_text(budget) == text) {
            return Result<Budget>::success(budget);
        }
    }
    return Result<Budget>::failure("Неизвестный бюджет");
}

}
