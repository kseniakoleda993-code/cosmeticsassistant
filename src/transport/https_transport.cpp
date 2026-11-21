#include "cosmetics/transport/https_transport.hpp"
#include "cosmetics/http/response.hpp"

#include <curl/curl.h>

#include <algorithm>
#include <memory>

namespace cosmetics {
namespace {

std::size_t write_body(char* bytes, std::size_t size, std::size_t count, void* output) noexcept {
    auto& body = *static_cast<std::string*>(output);
    std::size_t length = size * count;
    if (length > 4 * 1024 * 1024 - body.size()) {
        return 0;
    }
    try {
        body.append(bytes, length);
        return length;
    } catch (const std::exception&) {
        return 0;
    }
}

std::size_t write_headers(char* bytes, std::size_t size, std::size_t count, void* output) noexcept {
    auto& headers = *static_cast<std::string*>(output);
    std::size_t length = size * count;
    std::string_view line(bytes, length);
    if (line.substr(0, 5) == "HTTP/") {
        headers.clear();
    } else if (headers.size() >= 4 && headers.compare(headers.size() - 4, 4, "\r\n\r\n") == 0) {
        return length;
    }
    if (length > 65536 - headers.size()) {
        return 0;
    }
    try {
        headers.append(bytes, length);
        return length;
    } catch (const std::exception&) {
        return 0;
    }
}

bool add_field(curl_mime* mime, const std::string& name, const std::string& value) {
    auto* part = curl_mime_addpart(mime);
    return part && curl_mime_name(part, name.c_str()) == CURLE_OK && curl_mime_data(part, value.data(), value.size()) == CURLE_OK;
}

bool safe_character(unsigned char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9');
}

struct CurlLibrary {
    CURLcode status = curl_global_init(CURL_GLOBAL_DEFAULT);
    ~CurlLibrary() {
        if (status == CURLE_OK) {
            curl_global_cleanup();
        }
    }
};

}

HttpsTransport::HttpsTransport(std::string token, long timeout_seconds) : token(std::move(token)), timeout(timeout_seconds) {}

Result<std::string> HttpsTransport::send(const std::string& method, const std::string& body) {
    return perform(method, body, {}, {});
}

Result<std::string> HttpsTransport::upload(const std::string& method, const Fields& fields, const std::string& file) {
    if (file.empty()) {
        return Result<std::string>::failure("Не указан файл фотографии");
    }
    return perform(method, {}, fields, file);
}

Result<std::string> HttpsTransport::perform(const std::string& method, const std::string& body, const Fields& fields, const std::string& file) {
    using Response = Result<std::string>;
    if (method.empty() || !std::all_of(method.begin(), method.end(), safe_character)) {
        return Response::failure("Некорректное имя метода Telegram");
    }
    auto token_char = [](unsigned char ch) { return safe_character(ch) || ch == ':' || ch == '_' || ch == '-'; };
    if (timeout <= 0 || token.empty() || !std::all_of(token.begin(), token.end(), token_char)) {
        return Response::failure("Некорректные настройки HTTPS");
    }
    static CurlLibrary library;
    if (library.status != CURLE_OK) {
        return Response::failure("Не удалось запустить HTTPS транспорт");
    }
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> handle(curl_easy_init(), curl_easy_cleanup);
    if (!handle) {
        return Response::failure("Не удалось создать HTTPS запрос");
    }

    std::string url = "https://api.telegram.org/bot" + token + "/" + method;
    std::string response;
    std::string response_headers;
    auto* curl = handle.get();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_body);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_headers);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response_headers);

    std::unique_ptr<curl_mime, decltype(&curl_mime_free)> mime(nullptr, curl_mime_free);
    std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> headers(nullptr, curl_slist_free_all);
    if (file.empty()) {
        headers.reset(curl_slist_append(nullptr, "Content-Type: application/json"));
        if (!headers) {
            return Response::failure("Не удалось подготовить заголовки HTTPS запроса");
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers.get());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(body.size()));
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    } else {
        mime.reset(curl_mime_init(curl));
        if (!mime) {
            return Response::failure("Не удалось подготовить загрузку фото");
        }
        for (const auto& field : fields) {
            if (!add_field(mime.get(), field.first, field.second)) {
                return Response::failure("Не удалось подготовить поля загрузки фото");
            }
        }
        auto* photo = curl_mime_addpart(mime.get());
        if (!photo || curl_mime_name(photo, "photo") != CURLE_OK || curl_mime_filedata(photo, file.c_str()) != CURLE_OK) {
            return Response::failure("Не удалось открыть файл фото");
        }
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime.get());
    }

    CURLcode result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
        return Response::failure(std::string("Ошибка HTTPS: ") + curl_easy_strerror(result));
    }
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    if (status < 200 || status >= 300) {
        return Response::failure("Telegram вернул HTTP " + std::to_string(status));
    }
    curl_off_t length = -1;
    curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &length);
    if (length >= 0) {
        auto parsed = parse_http_response(response_headers + response);
        if (!parsed) {
            return Response::failure(parsed.error());
        }
        return Response::success(std::move(parsed.value().body));
    }
    return Response::success(std::move(response));
}

}
