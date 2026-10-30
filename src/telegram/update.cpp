#include "cosmetics/telegram/update.hpp"
#include "cosmetics/json/field.hpp"

#include <algorithm>
#include <limits>

namespace cosmetics {
namespace {

const json::Object* object_field(const json::Object& object, const std::string& name) {
    return dynamic_cast<const json::Object*>(object.find(name));
}

}

Result<Updates> read_updates(const json::Array& array) {
    Updates updates;
    for (const auto& item : array.values) {
        const auto* object = dynamic_cast<const json::Object*>(item.get());
        if (!object) {
            return Result<Updates>::failure("Некорректный элемент getUpdates");
        }
        auto id = json::field<json::Number>(*object, "update_id");
        if (!id || id.value()->value < 0 || id.value()->value == std::numeric_limits<std::int64_t>::max()) {
            return Result<Updates>::failure("Некорректный update_id");
        }
        std::int64_t number = id.value()->value;
        updates.next_offset = std::max(updates.next_offset, number + 1);
        auto* callback = object_field(*object, "callback_query");
        auto* message = callback ? object_field(*callback, "message") : object_field(*object, "message");
        if (!message) {
            continue;
        }
        auto* chat = object_field(*message, "chat");
        if (!chat) {
            continue;
        }
        auto chat_id = json::field<json::Number>(*chat, "id");
        auto action = callback ? json::field<json::String>(*callback, "data") : json::field<json::String>(*message, "text");
        if (!chat_id || !action) {
            continue;
        }
        std::string callback_id;
        if (callback) {
            auto field = json::field<json::String>(*callback, "id");
            if (!field) {
                continue;
            }
            callback_id = field.value()->value;
        }
        updates.actions.push_back({number, chat_id.value()->value, action.value()->value, std::move(callback_id)});
    }
    std::stable_sort(updates.actions.begin(), updates.actions.end(), [](const Update& a, const Update& b) { return a.id < b.id; });
    return Result<Updates>::success(std::move(updates));
}

}
