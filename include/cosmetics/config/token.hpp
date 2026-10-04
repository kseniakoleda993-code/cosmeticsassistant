#pragma once

#include "cosmetics/core/result.hpp"

#include <filesystem>
#include <string>

namespace cosmetics {

Result<std::string> load_bot_token(const std::filesystem::path& file = ".bot-token");

}
