#pragma once

#include "cosmetics/core/result.hpp"

#include <map>
#include <string>
#include <string_view>

namespace cosmetics {

struct HttpResponse {
    int status;
    std::map<std::string, std::string> headers;
    std::string body;
};

Result<HttpResponse> parse_http_response(std::string_view raw);

}
