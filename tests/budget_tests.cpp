#include "cosmetics/catalog/budget.hpp"

#include <gtest/gtest.h>

TEST(Budget, RoundTrip) {
    EXPECT_EQ(2u, cosmetics::budgets.size());
    EXPECT_EQ("средний", cosmetics::to_text(cosmetics::Budget::Average));
    EXPECT_EQ("выше среднего", cosmetics::to_text(cosmetics::Budget::AboveAverage));
    for (auto budget : cosmetics::budgets) {
        auto result = cosmetics::budget_from_text(cosmetics::to_text(budget));
        ASSERT_TRUE(result);
        EXPECT_EQ(budget, result.value());
    }
}

TEST(Budget, UnknownValue) {
    EXPECT_FALSE(cosmetics::budget_from_text(""));
    EXPECT_FALSE(cosmetics::budget_from_text("дешевый"));
    EXPECT_FALSE(cosmetics::budget_from_text("Средний"));
    EXPECT_THROW(cosmetics::to_text(static_cast<cosmetics::Budget>(99)), std::invalid_argument);
}
