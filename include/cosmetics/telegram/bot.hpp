#pragma once

#include "cosmetics/dialog/dialog.hpp"
#include "cosmetics/storage/users.hpp"

#include <iosfwd>

namespace cosmetics {

class Bot {
public:
    Bot(TelegramClient& client, const Catalog& catalog, std::ostream& errors) : client(client), dialog(catalog), errors(errors) {}
    Result<bool> poll();

private:
    TelegramClient& client;
    Dialog dialog;
    std::ostream& errors;
    Users users;
    std::int64_t offset = 0;

    void handle(const Update& update);
};

}
