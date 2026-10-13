#pragma once

#include "cosmetics/core/result.hpp"

#include <array>
#include <string_view>

namespace cosmetics {

enum class Budget { Average, AboveAverage };

inline constexpr std::array<Budget, 2> budgets{Budget::Average, Budget::AboveAverage};

std::string_view to_text(Budget budget);
Result<Budget> budget_from_text(std::string_view text);

}
