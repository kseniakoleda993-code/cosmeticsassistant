#include "cosmetics/transport/fake_transport.hpp"

#include <utility>

namespace cosmetics {

void FakeTransport::set_response(std::string response) {
    response_ = Result<std::string>::success(std::move(response));
}

void FakeTransport::set_error(std::string error) {
    response_ = Result<std::string>::failure(std::move(error));
}

Result<std::string> FakeTransport::send(const std::string& method, const std::string& body) {
    requests_.push_back({method, body});
    return response_;
}

const std::vector<Request>& FakeTransport::requests() const noexcept {
    return requests_;
}

}
