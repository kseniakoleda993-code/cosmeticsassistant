#include "cosmetics/json/field.hpp"
#include "cosmetics/json/parse.hpp"

#include <gtest/gtest.h>

using namespace cosmetics::json;

TEST(Json, ScalarsAndEscapes) {
    auto string = parse(R"("Привет \" \\ \/ \b \f \r \n \t \u263a \ud83d\ude00 \u0001")");
    ASSERT_TRUE(string);
    EXPECT_EQ(static_cast<String&>(*string.value()).value, "Привет \" \\ / \b \f \r \n \t ☺ 😀 \x01");
    auto again = parse(string.value()->text());
    ASSERT_TRUE(again);
    EXPECT_EQ(again.value()->text(), string.value()->text());
    for (auto input : {"0", "4", "-12", "9223372036854775807", "-9223372036854775808", "true", "false", "null"}) {
        auto result = parse(input);
        ASSERT_TRUE(result);
        EXPECT_EQ(result.value()->text(), input);
    }
}

TEST(Json, NestedValuesAndFields) {
    auto value = parse(" \n{\"list\": [{\"x\": 1}, {}, []], \"a\":\"b\", \"flag\":true, \"n\":null}");
    ASSERT_TRUE(value);
    const auto& object = static_cast<const Object&>(*value.value());
    auto list = field<Array>(object, "list");
    ASSERT_TRUE(list);
    auto* first = dynamic_cast<const Object*>(list.value()->at(0));
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(field<Number>(*first, "x").value()->value, 1);
    EXPECT_FALSE(field<Number>(object, "a"));
    EXPECT_FALSE(field<String>(object, "missing"));
    EXPECT_EQ(field<String>(object, "a").value()->value, "b");
    EXPECT_TRUE(field<Boolean>(object, "flag").value()->value);
    EXPECT_EQ(field<Null>(object, "n").value()->type(), Type::Null);
    EXPECT_EQ(list.value()->at(1)->text(), "{}");
    EXPECT_EQ(list.value()->at(2)->text(), "[]");
    EXPECT_EQ(list.value()->at(3), nullptr);
    auto roundtrip = parse(object.text());
    ASSERT_TRUE(roundtrip);
    EXPECT_EQ(roundtrip.value()->text(), object.text());
}

TEST(Json, RejectsInvalidInput) {
    for (auto input : {"", "\"bad", "\"\\u00xx\"", "\"\\ud800\"", "\"\\ud800\\u1234\"", "\"\\udc00\"", "\"\\q\"", "\"\\", "\"\n\"", "-", "01", "1.2", "1e1", "9223372036854775808", "[1,]", "[1", "{\"x\":1,}", "{\"x\" 1}", "{\"x\":1,\"x\":2}", "nul", "null 0"}) {
        EXPECT_FALSE(parse(input)) << input;
    }
    EXPECT_FALSE(parse(std::string(130, '[') + "0" + std::string(130, ']')));
}
