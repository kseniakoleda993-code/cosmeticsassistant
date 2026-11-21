#include "cosmetics/transport/fake_transport.hpp"
#include "cosmetics/transport/https_transport.hpp"
#include <gtest/gtest.h>

#include <memory>
#include <type_traits>

static_assert(std::is_abstract_v<cosmetics::Transport>);
static_assert(std::has_virtual_destructor_v<cosmetics::Transport>);

TEST(Transport, PolymorphicResponse) {
    auto fake = std::make_unique<cosmetics::FakeTransport>();
    fake->set_response("{\"ok\":true}");
    std::unique_ptr<cosmetics::Transport> transport = std::move(fake);
    auto result = transport->send("getUpdates", "{}");
    ASSERT_TRUE(result);
    EXPECT_EQ(std::string("{\"ok\":true}"), result.value());
}

TEST(Transport, FailureAndRecovery) {
    cosmetics::FakeTransport transport;
    transport.set_error("нет соединения");
    auto failed = transport.send("sendMessage", "{}");
    ASSERT_TRUE(!failed);
    EXPECT_EQ(std::string("нет соединения"), failed.error());
    transport.set_response("готово");
    EXPECT_EQ(std::string("готово"), transport.send("sendMessage", "{}").value());
}

TEST(Transport, RecordsRequestsInOrder) {
    cosmetics::FakeTransport transport;
    ASSERT_TRUE(transport.requests().empty());
    transport.send("getUpdates", "{\"offset\":10}");
    transport.set_error("ошибка");
    transport.send("sendMessage", "{\"text\":\"привет\"}");
    EXPECT_EQ(std::size_t(2), transport.requests().size());
    EXPECT_EQ(std::string("getUpdates"), transport.requests()[0].method);
    EXPECT_EQ(std::string("{\"offset\":10}"), transport.requests()[0].body);
    EXPECT_EQ(std::string("sendMessage"), transport.requests()[1].method);
    EXPECT_EQ(std::string("{\"text\":\"привет\"}"), transport.requests()[1].body);
}

TEST(HttpsTransport, RejectsInvalidRequestsBeforeNetwork) {
    cosmetics::HttpsTransport transport("123:example");
    EXPECT_FALSE(transport.send("getMe/../", "{}"));
    EXPECT_FALSE(transport.send("", "{}"));
    EXPECT_FALSE(transport.upload("sendPhoto", {}, ""));
    cosmetics::HttpsTransport invalid("123:example?token");
    auto result = invalid.send("getMe", "{}");
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().find("123:example"), std::string::npos);
    cosmetics::HttpsTransport no_timeout("123:example", 0);
    EXPECT_FALSE(no_timeout.send("getMe", "{}"));
}
