#pragma once

#include "cosmetics/catalog/category.hpp"
#include "cosmetics/catalog/position.hpp"

#include <vector>

namespace cosmetics {

const std::vector<Position>& positions_for(Category category);

}
