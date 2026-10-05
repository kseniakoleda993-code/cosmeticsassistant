#pragma once

#include "cosmetics/transport/transport.hpp"

#include <vector>

namespace cosmetics {

struct Request {
    std::string method;
    std::string body;
};

class FakeTransport final : public Transport {
public:
    void set_response(std::string response);
    void set_error(std::string error);
    Result<std::string> send(const std::string& method, const std::string& body) override;
    const std::vector<Request>& requests() const noexcept;

private:
    Result<std::string> response_ = Result<std::string>::success("");
    std::vector<Request> requests_;
};

}
