#include "cosmetics/http/response.hpp"

#include <algorithm>
#include <charconv>
#include <utility>

namespace cosmetics {
namespace {

std::string_view trim(std::string_view value) {
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
        value.remove_prefix(1);
    }
    while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
        value.remove_suffix(1);
    }
    return value;
}

bool valid_header_name(std::string_view name) {
    constexpr std::string_view punctuation = "!#$%&'*+-.^_`|~";
    return !name.empty() && std::all_of(name.begin(), name.end(), [&](unsigned char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') || punctuation.find(c) != std::string_view::npos;
    });
}

}

Result<HttpResponse> parse_http_response(std::string_view raw) {
    const auto separator = raw.find("\r\n\r\n");
    if (separator == std::string_view::npos) {
        return Result<HttpResponse>::failure("Незавершённые заголовки HTTP");
    }
    const auto line_end = raw.find("\r\n");
    const auto status_line = raw.substr(0, line_end);
    if (status_line.size() < 12 ||
        (status_line.substr(0, 9) != "HTTP/1.1 " && status_line.substr(0, 9) != "HTTP/1.0 ") ||
        (status_line.size() > 12 && status_line[12] != ' ')) {
        return Result<HttpResponse>::failure("Неверная строка статуса HTTP");
    }
    int status = 0;
    const auto code = status_line.substr(9, 3);
    const auto parsed = std::from_chars(code.data(), code.data() + code.size(), status);
    if (parsed.ec != std::errc{} || parsed.ptr != code.data() + code.size() || status < 100 || status > 599) {
        return Result<HttpResponse>::failure("Неверный код статуса HTTP");
    }

    HttpResponse response{status, {}, {}};
    auto position = line_end + 2;
    while (position < separator) {
        const auto end = raw.find("\r\n", position);
        const auto line = raw.substr(position, end - position);
        const auto colon = line.find(':');
        if (colon == std::string_view::npos || !valid_header_name(line.substr(0, colon))) {
            return Result<HttpResponse>::failure("Неверный заголовок HTTP");
        }
        auto name = std::string(line.substr(0, colon));
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) {
            return c >= 'A' && c <= 'Z' ? static_cast<char>(c + ('a' - 'A')) : static_cast<char>(c);
        });
        const auto value = trim(line.substr(colon + 1));
        if (std::any_of(value.begin(), value.end(), [](unsigned char c) {
                return (c < 32 && c != '\t') || c == 127;
            })) {
            return Result<HttpResponse>::failure("Недопустимый символ в заголовке HTTP");
        }
        const auto [entry, inserted] = response.headers.emplace(name, value);
        if (!inserted) {
            if (name == "content-length" || name == "transfer-encoding") {
                return Result<HttpResponse>::failure("Неоднозначная длина ответа HTTP");
            }
            entry->second += ", " + std::string(value);
        }
        position = end + 2;
    }
    if (response.headers.count("transfer-encoding") != 0) {
        return Result<HttpResponse>::failure("Transfer-Encoding не поддерживается: нужен Content-Length");
    }

    const auto body = raw.substr(separator + 4);
    std::size_t length = 0;
    const auto header = response.headers.find("content-length");
    if (header != response.headers.end()) {
        const auto& value = header->second;
        const auto number = std::from_chars(value.data(), value.data() + value.size(), length);
        if (value.empty() || number.ec != std::errc{} || number.ptr != value.data() + value.size()) {
            return Result<HttpResponse>::failure("Неверный Content-Length");
        }
    } else if (!body.empty()) {
        return Result<HttpResponse>::failure("Для непустого тела нужен Content-Length");
    }
    if (body.size() != length) {
        return Result<HttpResponse>::failure("Размер тела не совпадает с Content-Length");
    }
    response.body = std::string(body);
    return Result<HttpResponse>::success(std::move(response));
}

}
