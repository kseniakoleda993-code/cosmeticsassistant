#pragma once

#include "cosmetics/transport/transport.hpp"

namespace cosmetics {

class HttpsTransport final : public Transport {
public:
    explicit HttpsTransport(std::string token, long timeout_seconds = 35);
    Result<std::string> send(const std::string& method, const std::string& body) override;
    Result<std::string> upload(const std::string& method, const Fields& fields, const std::string& file) override;

private:
    std::string token;
    long timeout;
    Result<std::string> perform(const std::string& method, const std::string& body, const Fields& fields, const std::string& file);
};

}
