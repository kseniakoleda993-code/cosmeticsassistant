#pragma once

#include "cosmetics/telegram/update.hpp"
#include "cosmetics/transport/transport.hpp"

namespace cosmetics {

class TelegramClient {
public:
    explicit TelegramClient(Transport& transport) : transport(transport) {}
    Result<Updates> get_updates(std::int64_t offset);
    Result<bool> send_message(std::int64_t chat, const std::string& text, const std::vector<std::string>& buttons = {});
    Result<bool> send_photo(std::int64_t chat, const std::string& caption, const std::string& file);
    Result<bool> answer_callback(const std::string& id);

private:
    Transport& transport;
};

}
