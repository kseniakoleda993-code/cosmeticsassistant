#include "cosmetics/catalog/file_catalog.hpp"
#include "cosmetics/config/token.hpp"
#include "cosmetics/telegram/bot.hpp"
#include "cosmetics/transport/https_transport.hpp"

#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

namespace {

volatile std::sig_atomic_t stopped = 0;

void stop(int) {
    stopped = 1;
}

}

int main(int argc, char** argv) {
    std::signal(SIGINT, stop);
    std::signal(SIGTERM, stop);
    auto token = cosmetics::load_bot_token();
    if (!token) {
        std::cerr << token.error() << '\n';
        return 1;
    }
    cosmetics::HttpsTransport transport(std::move(token.value()));
    cosmetics::TelegramClient client(transport);
    if (argc > 2) {
        std::cerr << "Использование: cosmetics_bot [путь-к-products.tsv]\n";
        return 1;
    }
    auto catalog = cosmetics::FileCatalog::load(argc == 2 ? argv[1] : "data/products.tsv");
    if (!catalog) {
        std::cerr << catalog.error() << '\n';
        return 1;
    }
    cosmetics::Bot bot(client, catalog.value(), std::cerr);
    std::cerr << "Бот запущен\n";
    int delay = 1;
    while (!stopped) {
        auto result = bot.poll();
        if (result) {
            delay = 1;
            continue;
        }
        std::cerr << "Ошибка получения обновлений: " << result.error() << '\n';
        for (int i = 0; i < delay && !stopped; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        if (delay < 16) {
            delay *= 2;
        }
    }
    std::cerr << "Бот остановлен\n";
}
