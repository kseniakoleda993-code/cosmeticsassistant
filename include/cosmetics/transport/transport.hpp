#pragma once

#include "cosmetics/core/result.hpp"

#include <string>

namespace cosmetics {

class Transport {
public:
    virtual ~Transport() = default;
    virtual Result<std::string> send(const std::string& method, const std::string& body) = 0;
};

}
