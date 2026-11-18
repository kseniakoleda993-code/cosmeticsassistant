#pragma once

#include "cosmetics/dialog/user.hpp"
#include "cosmetics/storage/store.hpp"

namespace cosmetics {

class Users {
public:
    User get(std::int64_t chat) const { return records.get(chat); }
    void set(std::int64_t chat, User user) { records.set(chat, std::move(user)); }

private:
    Store<std::int64_t, User> records;
};

}
