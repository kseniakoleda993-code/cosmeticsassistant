#include "cosmetics/telegram/bot.hpp"
#include "cosmetics/transport/fake_transport.hpp"

#include <gtest/gtest.h>

#include <sstream>

using namespace cosmetics;

class ScenarioTransport : public Transport {
public:
    std::string updates = "[]";
    std::string offset_request;
    bool fail_poll = false;
    bool fail_send = false;
    std::vector<Request> sent;
    std::vector<std::string> photos;

    Result<std::string> send(const std::string& method, const std::string& body) override {
        if (method == "getUpdates") {
            offset_request = body;
            if (fail_poll) {
                return Result<std::string>::failure("нет сети");
            }
            return Result<std::string>::success("{\"ok\":true,\"result\":" + updates + "}");
        }
        if (fail_send) {
            fail_send = false;
            return Result<std::string>::failure("ошибка отправки");
        }
        sent.push_back({method, body});
        return Result<std::string>::success(R"({"ok":true,"result":{}})");
    }

    Result<std::string> upload(const std::string& method, const Fields& fields, const std::string& file) override {
        EXPECT_EQ(method, "sendPhoto");
        EXPECT_EQ(fields[0].first, "chat_id");
        EXPECT_EQ(fields[0].second, "11");
        EXPECT_EQ(fields[1].first, "caption");
        EXPECT_NE(fields[1].second.find("BYN"), std::string::npos);
        photos.push_back(file);
        return Result<std::string>::success(R"({"ok":true,"result":{}})");
    }
};

TEST(Bot, ProcessesBatchOnlyOnceAndKeepsChatsSeparate) {
    ScenarioTransport transport;
    TelegramClient client(transport);
    SampleCatalog catalog;
    std::ostringstream errors;
    Bot bot(client, catalog, errors);

    transport.updates = R"([{"update_id":12,"message":{"chat":{"id":11},"text":"/start"}}])";
    ASSERT_TRUE(bot.poll());
    ASSERT_EQ(transport.sent.size(), 3u);
    transport.updates = R"([{"update_id":15,"message":{"chat":{"id":22},"text":"/start"}},{"update_id":13,"callback_query":{"id":"x","data":"уход для волос","message":{"chat":{"id":11}}}},{"update_id":14,"message":{"chat":{"id":22},"text":"шампунь"}},{"update_id":16}])";
    ASSERT_TRUE(bot.poll());
    ASSERT_EQ(transport.sent.size(), 10u);
    EXPECT_EQ(transport.sent[3].method, "answerCallbackQuery");
    EXPECT_NE(transport.sent[4].body.find("шампунь"), std::string::npos);
    EXPECT_NE(transport.sent[6].body.find("/start"), std::string::npos);
    ASSERT_TRUE(bot.poll());
    EXPECT_EQ(transport.sent.size(), 10u);
    EXPECT_NE(transport.offset_request.find("\"offset\":17"), std::string::npos);
    EXPECT_TRUE(errors.str().empty());
}

TEST(Bot, PollErrorAndSendFailureDoNotBlockLaterUpdates) {
    ScenarioTransport transport;
    TelegramClient client(transport);
    SampleCatalog catalog;
    std::ostringstream errors;
    Bot bot(client, catalog, errors);
    transport.fail_poll = true;
    auto error = bot.poll();
    ASSERT_FALSE(error);
    EXPECT_EQ(error.error(), "нет сети");
    transport.fail_poll = false;
    transport.fail_send = true;
    transport.updates = R"([{"update_id":1,"message":{"chat":{"id":11},"text":"/start"}},{"update_id":2,"message":{"chat":{"id":22},"text":"/start"}}])";
    ASSERT_TRUE(bot.poll());
    ASSERT_EQ(transport.sent.size(), 3u);
    EXPECT_NE(transport.sent[0].body.find("\"chat_id\":22"), std::string::npos);
    EXPECT_NE(errors.str().find("ошибка отправки"), std::string::npos);
}

TEST(Bot, PhotoAndTotalThroughClient) {
    class PhotoCatalog : public Catalog {
        Result<Offer> find(Position, Budget) const override {
            return Result<Offer>::success({Product("Тестовое фото", 1234), "photo.jpg"});
        }
    } catalog;
    ScenarioTransport transport;
    TelegramClient client(transport);
    std::ostringstream errors;
    Bot bot(client, catalog, errors);
    transport.updates = R"([{"update_id":1,"message":{"chat":{"id":11},"text":"/start"}},{"update_id":2,"message":{"chat":{"id":11},"text":"уход для волос"}},{"update_id":3,"message":{"chat":{"id":11},"text":"шампунь"}},{"update_id":4,"message":{"chat":{"id":11},"text":"средний"}},{"update_id":5,"message":{"chat":{"id":11},"text":"нет"}}])";
    ASSERT_TRUE(bot.poll());
    EXPECT_EQ(transport.photos, (std::vector<std::string>{"photo.jpg"}));
    EXPECT_NE(transport.sent.back().body.find("12.34 BYN"), std::string::npos);
    EXPECT_TRUE(errors.str().empty());
}
