#include "cosmetics/storage/store.hpp"

#include <gtest/gtest.h>
#include <string>

using namespace cosmetics;

TEST(Store, ValuesAreIndependentAndReplaceable) {
    Store<std::string, int> numbers;
    EXPECT_EQ(numbers.get("missing"), 0);
    EXPECT_FALSE(numbers.contains("missing"));
    EXPECT_EQ(numbers.size(), 0u);
    numbers.set("a", 1);
    numbers.set("b", 2);
    numbers.set("a", 3);
    EXPECT_TRUE(numbers.contains("a"));
    EXPECT_EQ(numbers.get("a"), 3);
    EXPECT_EQ(numbers.get("b"), 2);
    EXPECT_EQ(numbers.size(), 2u);
    Store<int, std::string> strings;
    strings.set(1, "строка");
    EXPECT_EQ(strings.get(1), "строка");
    EXPECT_TRUE(strings.get(2).empty());
}
