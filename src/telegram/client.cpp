#include "cosmetics/telegram/client.hpp"
#include "cosmetics/json/field.hpp"
#include "cosmetics/json/parse.hpp"
#include "cosmetics/telegram/request.hpp"

namespace cosmetics {
namespace {

Result<json::Ptr> check_reply(Result<std::string> reply) {
    if (!reply) {
        return Result<json::Ptr>::failure(reply.error());
    }
    auto parsed = json::parse(reply.value(), true);
    if (!parsed) {
        return Result<json::Ptr>::failure(parsed.error());
    }
    auto* object = dynamic_cast<const json::Object*>(parsed.value().get());
    if (!object) {
        return Result<json::Ptr>::failure("Ответ Telegram должен быть объектом");
    }
    auto ok = json::field<json::Boolean>(*object, "ok");
    if (!ok || !ok.value()->value) {
        return Result<json::Ptr>::failure("Telegram отклонил запрос");
    }
    return parsed;
}

Result<bool> check_sent(Result<std::string> reply) {
    auto result = check_reply(std::move(reply));
    if (!result) {
        return Result<bool>::failure(result.error());
    }
    return Result<bool>::success(true);
}

}

Result<Updates> TelegramClient::get_updates(std::int64_t offset) {
    auto body = "{\"offset\":" + std::to_string(offset) + ",\"timeout\":25,\"allowed_updates\":[\"message\",\"callback_query\"]}";
    auto reply = check_reply(transport.send("getUpdates", body));
    if (!reply) {
        return Result<Updates>::failure(reply.error());
    }
    const auto& object = static_cast<const json::Object&>(*reply.value());
    auto array = json::field<json::Array>(object, "result");
    if (!array) {
        return Result<Updates>::failure(array.error());
    }
    return read_updates(*array.value());
}

Result<bool> TelegramClient::send_message(std::int64_t chat, const std::string& text, const std::vector<std::string>& buttons, const std::vector<std::string>& callbacks) {
    return check_sent(transport.send("sendMessage", message_request(chat, text, buttons, callbacks)));
}

Result<bool> TelegramClient::send_photo(std::int64_t chat, const std::string& caption, const std::string& file) {
    return check_sent(transport.upload("sendPhoto", {{"chat_id", std::to_string(chat)}, {"caption", caption}}, file));
}

Result<bool> TelegramClient::answer_callback(const std::string& id) {
    std::string body = "{\"callback_query_id\":" + json::String(id).text() + "}";
    return check_sent(transport.send("answerCallbackQuery", body));
}

}
