#include "cosmetics/core/result.hpp"
#include <gtest/gtest.h>

#include <memory>

TEST(Result, IntegerValue) {
    auto result = cosmetics::Result<int>::success(42);
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result);
    EXPECT_EQ(42, result.value());
    result.value() = 7;
    const auto& immutable = result;
    EXPECT_EQ(7, immutable.value());
    EXPECT_THROW(([&] { result.error(); }()), std::logic_error);
}

TEST(Result, StringValueAndError) {
    const auto result = cosmetics::Result<std::string>::success("строка");
    EXPECT_EQ(std::string("строка"), result.value());
    const auto error = cosmetics::Result<std::string>::failure("нет данных");
    ASSERT_TRUE(!error);
    EXPECT_EQ(std::string("нет данных"), error.error());
    EXPECT_THROW(([&] { error.value(); }()), std::logic_error);
}

TEST(Result, InvalidValueAccess) {
    auto result = cosmetics::Result<int>::failure("ошибка");
    ASSERT_TRUE(!result.has_value());
    EXPECT_EQ(std::string("ошибка"), result.error());
    EXPECT_THROW(([&] { result.value(); }()), std::logic_error);
}

TEST(Result, MoveOnlyValue) {
    auto result = cosmetics::Result<std::unique_ptr<int>>::success(std::make_unique<int>(15));
    auto moved = std::move(result);
    EXPECT_EQ(15, *moved.value());
}
