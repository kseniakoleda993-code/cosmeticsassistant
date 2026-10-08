#include "cosmetics/catalog/category.hpp"
#include <gtest/gtest.h>


TEST(Category, RoundTrip) {
    const std::array<std::string_view, 4> expected{
        "уход для волос", "уход для тела", "уходовая косметика", "декоративная косметика"
    };
    EXPECT_EQ(std::size_t(4), cosmetics::categories.size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
        const auto category = cosmetics::categories[i];
        EXPECT_EQ(expected[i], cosmetics::to_text(category));
        const auto parsed = cosmetics::category_from_text(expected[i]);
        ASSERT_TRUE(parsed);
        EXPECT_EQ(category, parsed.value());
    }
}

TEST(Category, UnknownValue) {
    ASSERT_TRUE(!cosmetics::category_from_text(""));
    ASSERT_TRUE(!cosmetics::category_from_text("шампунь"));
    ASSERT_TRUE(!cosmetics::category_from_text("Уход для волос"));
    EXPECT_THROW(([] { cosmetics::to_text(static_cast<cosmetics::Category>(99)); }()), std::invalid_argument);
}
