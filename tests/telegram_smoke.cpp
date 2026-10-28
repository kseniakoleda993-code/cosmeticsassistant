#include "cosmetics/config/token.hpp"
#include "cosmetics/transport/https_transport.hpp"

#include <iostream>

int main() {
    auto token = cosmetics::load_bot_token();
    if (!token) {
        std::cerr << token.error() << '\n';
        return 1;
    }
    cosmetics::HttpsTransport transport(token.value());
    auto response = transport.send("getMe", "{}");
    if (!response) {
        std::cerr << response.error() << '\n';
        return 1;
    }
    std::cout << "Telegram getMe: OK\n";
}
