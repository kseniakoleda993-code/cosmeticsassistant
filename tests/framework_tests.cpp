#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

TEST(Framework, BasicAssertions) {
    EXPECT_EQ(42, 42);
    EXPECT_EQ(std::string("привет"), std::string("привет"));
    EXPECT_NE(1, 2);
    EXPECT_TRUE(true);
    EXPECT_THROW(throw std::logic_error("ошибка"), std::logic_error);
}
