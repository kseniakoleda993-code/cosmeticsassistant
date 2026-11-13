#pragma once

#include "cosmetics/dialog/message.hpp"
#include "cosmetics/dialog/user.hpp"

#include <string_view>

namespace cosmetics {

struct Transition {
    User user;
    std::vector<Message> messages;
};

class Step {
public:
    virtual ~Step() = default;
    Transition handle(const User& user, std::string_view action) const;
    virtual Message prompt(const User& user) const = 0;

protected:
    virtual std::optional<Transition> advance(const User& user, std::string_view action) const = 0;
};

}
