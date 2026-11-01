#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace cosmetics {

std::string message_request(std::int64_t chat, const std::string& text, const std::vector<std::string>& buttons);

}
