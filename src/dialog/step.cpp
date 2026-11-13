#include "cosmetics/dialog/step.hpp"

namespace cosmetics {

Transition Step::handle(const User& user, std::string_view action) const {
    auto result = advance(user, action);
    if (result) {
        return std::move(*result);
    }
    return {user, {{"Выбери ответ кнопкой ниже."}, prompt(user)}};
}

}
