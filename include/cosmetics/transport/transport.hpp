#pragma once

#include "cosmetics/core/result.hpp"

#include <string>
#include <utility>
#include <vector>

namespace cosmetics {

using Fields = std::vector<std::pair<std::string, std::string>>;

class Transport {
public:
    virtual Result<std::string> upload(const std::string&, const Fields&, const std::string&) {
        return Result<std::string>::failure("Транспорт не поддерживает загрузку фото");
    }
    virtual ~Transport() = default;
    virtual Result<std::string> send(const std::string& method, const std::string& body) = 0;
};

}
