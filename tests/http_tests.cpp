#include "cosmetics/http/response.hpp"
#include <gtest/gtest.h>


TEST(Http, SuccessAndCaseInsensitiveHeaders) {
    auto result = cosmetics::parse_http_response("HTTP/1.1 200 OK\r\ncOnTeNt-LeNgTh: 5\r\nContent-Type: text/plain\r\n\r\nhello");
    ASSERT_TRUE(result);
    EXPECT_EQ(200, result.value().status);
    EXPECT_EQ(std::string("hello"), result.value().body);
    EXPECT_EQ(std::string("text/plain"), result.value().headers.at("content-type"));
}

TEST(Http, ErrorStatusAndBody) {
    auto result = cosmetics::parse_http_response("HTTP/1.0 404 Not Found\r\nContent-Length: 3\r\n\r\nno!");
    ASSERT_TRUE(result);
    EXPECT_EQ(404, result.value().status);
    EXPECT_EQ(std::string("no!"), result.value().body);
}

TEST(Http, EmptyBody) {
    for (const auto& raw : {"HTTP/1.1 204 No Content\r\n\r\n", "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n"}) {
        auto result = cosmetics::parse_http_response(raw);
        ASSERT_TRUE(result);
        ASSERT_TRUE(result.value().body.empty());
    }
}

TEST(Http, ByteLength) {
    const std::string body = "привет";
    auto result = cosmetics::parse_http_response("HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body);
    ASSERT_TRUE(result);
    EXPECT_EQ(body, result.value().body);
}

TEST(Http, DuplicateOrdinaryHeaders) {
    auto result = cosmetics::parse_http_response("HTTP/1.1 200 OK\r\nX-Test: one\r\nx-test: two\r\nContent-Length:\t0 \r\n\r\n");
    ASSERT_TRUE(result);
    EXPECT_EQ(std::string("one, two"), result.value().headers.at("x-test"));
}

TEST(Http, InvalidResponses) {
    for (const auto& raw : {
             "", "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n",
             "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nhi",
             "HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\nlong",
             "HTTP/1.1 200 OK\r\nContent-Length: -1\r\n\r\n",
             "HTTP/1.1 200 OK\r\nContent-Length: 1x\r\n\r\nx",
             "HTTP/1.1 200 OK\r\nContent-Length: 9999999999999999999999999\r\n\r\n",
             "HTTP/1.1 200 OK\r\nContent-Length:\r\n\r\n",
             "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nCONTENT-LENGTH: 0\r\n\r\n",
             "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n",
             "HTTP/1.1 200 OK\r\n\r\nbody",
             "HTTP/1.1 abc OK\r\n\r\n", "HTTP/1.1 099 Bad\r\n\r\n",
             "HTTP/1.1 2000 Bad\r\n\r\n", "HTTP/2.0 200 OK\r\n\r\n",
             "HTTP/1.1 200 OK\r\nBroken\r\n\r\n",
             "HTTP/1.1 200 OK\r\nBad Name: value\r\n\r\n"}) {
        ASSERT_TRUE(!cosmetics::parse_http_response(raw));
    }
}
