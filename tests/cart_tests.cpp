#include "cosmetics/catalog/cart.hpp"
#include "cosmetics/dialog/summary.hpp"

#include <gtest/gtest.h>
#include <limits>

using namespace cosmetics;

TEST(Cart, CopiesKeepOrderAndSum) {
    Cart empty;
    auto one = empty.add(Product("a", 10));
    auto three = one.add(Product("b", 2147483648LL)).add(Product("c", 20));
    EXPECT_EQ(empty.total(), 0);
    EXPECT_TRUE(empty.items().empty());
    EXPECT_EQ(one.total(), 10);
    EXPECT_EQ(one.items().size(), 1u);
    EXPECT_EQ(three.total(), 2147483678LL);
    ASSERT_EQ(three.items().size(), 3u);
    EXPECT_EQ(three.items()[0].name(), "a");
    EXPECT_EQ(three.items()[1].name(), "b");
    EXPECT_EQ(three.items()[2].name(), "c");
    auto overflow = Cart().add(Product("a", std::numeric_limits<std::int64_t>::max())).add(Product("b", 1));
    EXPECT_THROW(overflow.total(), std::overflow_error);
}

TEST(Cart, SummaryIncludesZeroAndTotal) {
    EXPECT_EQ(cart_summary(Cart()), "Отлично! Суммарная стоимость продуктов:\n0.00 BYN\nБыл рад помочь! Если есть какие-либо вопросы, задавай");
    EXPECT_NE(cart_summary(Cart().add(Product("a", 4950))).find("49.50 BYN"), std::string::npos);
}

TEST(Cart, RemovesOnlySelectedItemAndRecalculatesTotal) {
    auto cart = Cart().add(Product("маска", 2500)).add(Product("маска", 2500)).add(Product("тонер", 3800));
    auto trimmed = cart.remove(1);
    EXPECT_EQ(cart.items().size(), 3u);
    ASSERT_EQ(trimmed.items().size(), 2u);
    EXPECT_EQ(trimmed.items()[0].name(), "маска");
    EXPECT_EQ(trimmed.items()[1].name(), "тонер");
    EXPECT_EQ(trimmed.total(), 6300);
    EXPECT_THROW(trimmed.remove(2), std::out_of_range);
}
