#include "cosmetics/catalog/catalog.hpp"

#include <gtest/gtest.h>

using namespace cosmetics;

TEST(Catalog, OffersInBothBudgets) {
    SampleCatalog catalog;
    for (auto position : positions) {
        auto cheap = catalog.find(position, Budget::Average);
        auto expensive = catalog.find(position, Budget::AboveAverage);
        ASSERT_TRUE(cheap);
        ASSERT_TRUE(expensive);
        EXPECT_FALSE(cheap.value().product.name().empty());
        EXPECT_FALSE(expensive.value().product.name().empty());
        EXPECT_GT(cheap.value().product.price(), 0);
        EXPECT_GT(expensive.value().product.price(), cheap.value().product.price());
    }
    EXPECT_FALSE(catalog.find(static_cast<Position>(-1), Budget::Average));
    EXPECT_FALSE(catalog.find(Position::Shampoo, static_cast<Budget>(-1)));
}
