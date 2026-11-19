#include "cosmetics/telegram/bot.hpp"

#include <algorithm>
#include <ostream>

namespace cosmetics {

Result<bool> Bot::poll() {
    auto updates = client.get_updates(offset);
    if (!updates) {
        return Result<bool>::failure(updates.error());
    }
    for (const auto& update : updates.value().actions) {
        if (update.id < offset) {
            continue;
        }
        offset = update.id + 1;
        try {
            handle(update);
        } catch (const std::logic_error&) {
            errors << "Некорректное состояние при обработке обновления " << update.id << '\n';
        } catch (const std::overflow_error&) {
            errors << "Переполнение суммы при обработке обновления " << update.id << '\n';
        }
    }
    offset = std::max(offset, updates.value().next_offset);
    return Result<bool>::success(true);
}

void Bot::handle(const Update& update) {
    if (!update.callback_id.empty()) {
        auto answer = client.answer_callback(update.callback_id);
        if (!answer) {
            errors << "Не удалось ответить на нажатие в обновлении " << update.id << '\n';
        }
    }
    auto transition = dialog.handle(users.get(update.chat), update.action);
    for (const auto& message : transition.messages) {
        auto sent = message.send(client, update.chat);
        if (!sent) {
            errors << "Не удалось отправить ответ на обновление " << update.id << ": " << sent.error() << '\n';
            return;
        }
    }
    users.set(update.chat, std::move(transition.user));
}

}
