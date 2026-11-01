#include "cosmetics/telegram/request.hpp"
#include "cosmetics/json/value.hpp"

namespace cosmetics {

std::string message_request(std::int64_t chat, const std::string& text, const std::vector<std::string>& buttons) {
    json::Object request;
    request.fields["chat_id"] = std::make_unique<json::Number>(chat);
    request.fields["text"] = std::make_unique<json::String>(text);
    if (!buttons.empty()) {
        auto keyboard = std::make_unique<json::Array>();
        for (const auto& label : buttons) {
            auto button = std::make_unique<json::Object>();
            button->fields["text"] = std::make_unique<json::String>(label);
            button->fields["callback_data"] = std::make_unique<json::String>(label);
            auto row = std::make_unique<json::Array>();
            row->values.push_back(std::move(button));
            keyboard->values.push_back(std::move(row));
        }
        auto markup = std::make_unique<json::Object>();
        markup->fields["inline_keyboard"] = std::move(keyboard);
        request.fields["reply_markup"] = std::move(markup);
    }
    return request.text();
}

}
