#include "cosmetics/config/token.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>

namespace cosmetics {
namespace {

Result<std::string> validate_token(std::string token) {
    if (token.empty() || token.size() > 4096 ||
        std::any_of(token.begin(), token.end(), [](unsigned char c) { return std::isspace(c) != 0; })) {
        return Result<std::string>::failure("Токен пустой или имеет неверный формат");
    }
    return Result<std::string>::success(std::move(token));
}

}

Result<std::string> load_bot_token(const std::filesystem::path& file) {
    const char* environment = std::getenv("TELEGRAM_BOT_TOKEN");
    if (environment != nullptr && *environment != '\0') {
        return validate_token(environment);
    }

    std::ifstream input(file);
    if (!input) {
        return Result<std::string>::failure("Не задан TELEGRAM_BOT_TOKEN и не удалось открыть файл настроек");
    }
    std::string token;
    std::getline(input, token);
    if (input.bad()) {
        return Result<std::string>::failure("Не удалось прочитать файл настроек");
    }
    if (!token.empty() && token.back() == '\r') {
        token.pop_back();
    }
    std::string extra;
    if (std::getline(input, extra) || input.bad()) {
        return Result<std::string>::failure("Файл настроек должен содержать одну строку с токеном");
    }
    return validate_token(std::move(token));
}

}
