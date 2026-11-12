#pragma once

#include "cosmetics/telegram/client.hpp"

namespace cosmetics {

struct Message {
    std::string text;
    std::vector<std::string> buttons = {};
    std::string photo = {};

    Result<bool> send(TelegramClient& client, std::int64_t chat) const {
        if (!photo.empty()) {
            return client.send_photo(chat, text, photo);
        }
        return client.send_message(chat, text, buttons);
    }
};

}
