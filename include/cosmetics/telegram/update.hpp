#pragma once

#include "cosmetics/json/value.hpp"
#include "cosmetics/core/result.hpp"

namespace cosmetics {

struct Update {
    std::int64_t id;
    std::int64_t chat;
    std::string action;
    std::string callback_id;
};

struct Updates {
    std::vector<Update> actions;
    std::int64_t next_offset = 0;
};

Result<Updates> read_updates(const json::Array& array);

}
