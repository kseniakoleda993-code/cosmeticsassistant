#include "cosmetics/catalog/position.hpp"
#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

TEST(Position, RoundTrip) {
    const std::array<std::string_view, 19> expected{
        "шампунь", "кондиционер", "маска", "масло", "термозащита", "сухой шампунь",
        "гель для душа", "скраб для тела", "крем/лосьон для тела",
        "скраб", "гель для умывания", "тонер", "энзимная пудра",
        "тональная основа", "румяна", "тушь", "помада", "гель для бровей", "консилер"
    };
    EXPECT_EQ(std::size_t(19), cosmetics::positions.size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
        const auto position = cosmetics::positions[i];
        EXPECT_EQ(expected[i], cosmetics::to_text(position));
        const auto parsed = cosmetics::position_from_text(expected[i]);
        ASSERT_TRUE(parsed);
        EXPECT_EQ(position, parsed.value());
    }
}

TEST(Position, UniqueValuesAndLabels) {
    auto values = cosmetics::positions;
    std::sort(values.begin(), values.end());
    ASSERT_TRUE(std::adjacent_find(values.begin(), values.end()) == values.end());
    std::vector<std::string_view> labels;
    for (const auto value : values) {
        labels.push_back(cosmetics::to_text(value));
    }
    std::sort(labels.begin(), labels.end());
    ASSERT_TRUE(std::adjacent_find(labels.begin(), labels.end()) == labels.end());
}

TEST(Position, UnknownValue) {
    ASSERT_TRUE(!cosmetics::position_from_text(""));
    ASSERT_TRUE(!cosmetics::position_from_text("крем"));
    ASSERT_TRUE(!cosmetics::position_from_text("уход для тела"));
    EXPECT_THROW(([] { cosmetics::to_text(static_cast<cosmetics::Position>(99)); }()), std::invalid_argument);
}
