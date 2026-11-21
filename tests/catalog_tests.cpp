#include "cosmetics/catalog/catalog.hpp"
#include "cosmetics/catalog/file_catalog.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <set>

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

TEST(FileCatalog, AllPhotosAndPricesArePresent) {
    auto catalog = FileCatalog::load(std::filesystem::path(COSMETICS_DATA_DIR) / "products.tsv");
    ASSERT_TRUE(catalog) << (catalog ? "" : catalog.error());
    std::set<std::string> photos;
    for (auto position : positions) {
        auto cheap = catalog.value().find(position, Budget::Average);
        auto expensive = catalog.value().find(position, Budget::AboveAverage);
        ASSERT_TRUE(cheap);
        ASSERT_TRUE(expensive);
        EXPECT_GT(cheap.value().product.price(), 0);
        EXPECT_GT(expensive.value().product.price(), cheap.value().product.price());
        EXPECT_TRUE(photos.insert(cheap.value().photo).second);
        EXPECT_TRUE(photos.insert(expensive.value().photo).second);
    }
    EXPECT_EQ(photos.size(), 38u);
    auto oil = catalog.value().find(Position::HairOil, Budget::AboveAverage).value();
    EXPECT_EQ(oil.product.name(), "Olaplex No.7 Bonding Oil, 30 мл");
    EXPECT_EQ(oil.product.price(), 7500);
    EXPECT_EQ(std::filesystem::path(oil.photo).filename(), "hair-oil-above-average.jpg");
    EXPECT_FALSE(catalog.value().find(static_cast<Position>(-1), Budget::Average));
}

TEST(FileCatalog, MissingIncompleteAndInvalidData) {
    EXPECT_FALSE(FileCatalog::load(std::filesystem::path(COSMETICS_DATA_DIR) / "missing.tsv"));
    auto file = std::filesystem::path("invalid-catalog-test.tsv");
    for (const auto* contents : {"", "broken\n", "\nшампунь\tсредний\tтовар\t-1\ta.jpg\r\n", "шампунь\tсредний\tтовар\t22\t../x.jpg\n", "шампунь\tсредний\tтовар\t2200\tmissing.jpg\n"}) {
        std::ofstream(file) << contents;
        EXPECT_FALSE(FileCatalog::load(file));
    }
    std::filesystem::remove(file);
}
