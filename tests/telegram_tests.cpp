#include "cosmetics/json/field.hpp"
#include "cosmetics/json/parse.hpp"
#include "cosmetics/telegram/client.hpp"
#include "cosmetics/dialog/message.hpp"
#include "cosmetics/transport/fake_transport.hpp"

#include <gtest/gtest.h>

using namespace cosmetics;

TEST(Telegram, EmptyAndMultipleUpdates) {
    FakeTransport transport;
    TelegramClient client(transport);
    transport.set_response(R"({"ok":true,"result":[]})");
    auto empty = client.get_updates(81);
    ASSERT_TRUE(empty);
    EXPECT_TRUE(empty.value().actions.empty());
    EXPECT_EQ(transport.requests().front().method, "getUpdates");
    auto request = json::parse(transport.requests().front().body);
    ASSERT_TRUE(request);
    auto& object = static_cast<const json::Object&>(*request.value());
    EXPECT_EQ(json::field<json::Number>(object, "offset").value()->value, 81);

    transport.set_response(R"({"ok":true,"result":[{"update_id":80,"message":{"chat":{"id":11},"text":"/start"}}]})");
    auto one = client.get_updates(80);
    ASSERT_TRUE(one);
    ASSERT_EQ(one.value().actions.size(), 1u);
    EXPECT_EQ(one.value().actions.front().chat, 11);
    EXPECT_EQ(one.value().actions.front().action, "/start");

    transport.set_response(R"({"ok":true,"result":[{"update_id":84},{"update_id":82,"callback_query":{"id":"cb1","data":"шампунь","message":{"chat":{"id":-100123456789}}}},{"update_id":81,"message":{"chat":{"id":5},"text":"привет"}},{"update_id":83,"message":{"chat":{"id":6},"location":{"latitude":53.9,"longitude":2.75e+1}}}]})");
    auto result = client.get_updates(81);
    ASSERT_TRUE(result);
    ASSERT_EQ(result.value().actions.size(), 2u);
    EXPECT_EQ(result.value().next_offset, 85);
    EXPECT_EQ(result.value().actions[0].action, "привет");
    EXPECT_EQ(result.value().actions[1].chat, -100123456789);
    EXPECT_EQ(result.value().actions[1].action, "шампунь");
    EXPECT_EQ(result.value().actions[1].callback_id, "cb1");
}

TEST(Telegram, ReportsApiAndTransportErrors) {
    FakeTransport transport;
    TelegramClient client(transport);
    for (const auto* response : {"{bad", "[]", R"({"ok":false})", R"({"ok":1})", R"({"ok":true})", R"({"ok":true,"result":{}})", R"({"ok":true,"result":[null]})", R"({"ok":true,"result":[{"update_id":-1}]})", R"({"ok":true,"result":[{"update_id":1.2}]})", R"({"ok":true,"result":[{"update_id":1e}]})", R"({"ok":true,"result":[{"update_id":9223372036854775807}]})"}) {
        transport.set_response(response);
        EXPECT_FALSE(client.get_updates(0)) << response;
    }
    transport.set_error("нет сети");
    auto error = client.get_updates(0);
    ASSERT_FALSE(error);
    EXPECT_EQ(error.error(), "нет сети");
    EXPECT_FALSE(client.send_message(1, "текст"));
}

TEST(Telegram, MessageAndCallbackRequests) {
    FakeTransport transport;
    TelegramClient client(transport);
    transport.set_response(R"({"ok":true,"result":{}})");
    ASSERT_TRUE(Message({"строка \"\n"}).send(client, 13));
    ASSERT_TRUE(Message({"выбери", {"а", "б", "в", "г", "д", "е"}}).send(client, 15));
    ASSERT_TRUE(client.answer_callback("cb\"1"));
    EXPECT_FALSE(client.send_photo(15, "подпись", "file.jpg"));
    const auto& requests = transport.requests();
    auto first = json::parse(requests[0].body);
    ASSERT_TRUE(first);
    auto& plain = static_cast<const json::Object&>(*first.value());
    EXPECT_EQ(plain.find("reply_markup"), nullptr);
    EXPECT_EQ(json::field<json::String>(plain, "text").value()->value, "строка \"\n");
    auto second = json::parse(requests[1].body);
    ASSERT_TRUE(second);
    auto& with_buttons = static_cast<const json::Object&>(*second.value());
    auto markup = json::field<json::Object>(with_buttons, "reply_markup");
    ASSERT_TRUE(markup);
    auto keyboard = json::field<json::Array>(*markup.value(), "inline_keyboard");
    ASSERT_TRUE(keyboard);
    ASSERT_EQ(keyboard.value()->values.size(), 6u);
    std::vector<std::string> labels;
    for (const auto& row : keyboard.value()->values) {
        auto& array = static_cast<const json::Array&>(*row);
        ASSERT_EQ(array.values.size(), 1u);
        auto& button = static_cast<const json::Object&>(*array.at(0));
        auto label = json::field<json::String>(button, "text").value()->value;
        EXPECT_EQ(json::field<json::String>(button, "callback_data").value()->value, label);
        labels.push_back(label);
    }
    EXPECT_EQ(labels, (std::vector<std::string>{"а", "б", "в", "г", "д", "е"}));
    EXPECT_EQ(requests[2].method, "answerCallbackQuery");
    EXPECT_EQ(requests[2].body, R"({"callback_query_id":"cb\"1"})");
}

TEST(Telegram, CartButtonsCarryTheirOwnActions) {
    FakeTransport transport;
    TelegramClient client(transport);
    transport.set_response(R"({"ok":true,"result":{}})");
    Message choice{"Корзина", {"удалить №1", "назад"}, {}, {"cart:3:0", "cart:back"}};
    ASSERT_TRUE(choice.send(client, 123));
    const auto& body = transport.requests().back().body;
    EXPECT_NE(body.find(R"("callback_data":"cart:3:0")"), std::string::npos);
    EXPECT_NE(body.find(R"("text":"удалить №1")"), std::string::npos);
    choice.callbacks.pop_back();
    EXPECT_THROW(choice.send(client, 123), std::invalid_argument);
}
