#include "cosmetics/catalog/product.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

static_assert(std::is_same_v<decltype(std::declval<cosmetics::Product&>().name()), const std::string&>);
static_assert(std::is_same_v<decltype(std::declval<cosmetics::Product&>().price()), std::int64_t>);

TEST(Product, StoresNameAndPrice) {
    const cosmetics::Product product("Шампунь", 123450);
    EXPECT_EQ("Шампунь", product.name());
    EXPECT_EQ(123450, product.price());
    EXPECT_EQ("1234.50", cosmetics::format_price(product.price()));
}

TEST(Product, FormatsBoundaryPrices) {
    EXPECT_EQ("0.00", cosmetics::format_price(0));
    EXPECT_EQ("0.01", cosmetics::format_price(1));
    EXPECT_EQ("0.09", cosmetics::format_price(9));
    EXPECT_EQ("0.10", cosmetics::format_price(10));
    EXPECT_EQ("0.99", cosmetics::format_price(99));
    EXPECT_EQ("1.00", cosmetics::format_price(100));
    EXPECT_EQ("1.01", cosmetics::format_price(101));
    EXPECT_EQ("21474836.48", cosmetics::format_price(2147483648LL));
    EXPECT_EQ("92233720368547758.07", cosmetics::format_price(std::numeric_limits<std::int64_t>::max()));
}

TEST(Product, AllowsZeroPrice) {
    const cosmetics::Product product("Пробник", 0);
    EXPECT_EQ(0, product.price());
}

TEST(Product, RejectsInvalidValues) {
    EXPECT_THROW(cosmetics::Product("", 100), std::invalid_argument);
    EXPECT_THROW(cosmetics::Product("Шампунь", -1), std::invalid_argument);
    EXPECT_THROW(cosmetics::format_price(-1), std::invalid_argument);
    EXPECT_THROW(cosmetics::format_price(std::numeric_limits<std::int64_t>::min()), std::invalid_argument);
}
