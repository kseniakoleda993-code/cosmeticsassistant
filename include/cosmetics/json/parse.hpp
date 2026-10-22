#pragma once

#include "cosmetics/core/result.hpp"
#include "cosmetics/json/value.hpp"

#include <string_view>

namespace cosmetics::json {

Result<Ptr> parse(std::string_view input);

}
