#include "cosmetics/catalog/category_positions.hpp"

#include <gtest/gtest.h>

#include <algorithm>

TEST(CategoryPositions, ExactOrderAndSizes) {
    using cosmetics::Category;
    using cosmetics::Position;
    std::vector<Position> hair = {Position::Shampoo, Position::Conditioner, Position::HairMask, Position::HairOil, Position::HeatProtection, Position::DryShampoo};
    std::vector<Position> body = {Position::ShowerGel, Position::BodyScrub, Position::BodyCream};
    std::vector<Position> skin = {Position::FaceScrub, Position::FaceWash, Position::Toner, Position::EnzymePowder};
    std::vector<Position> makeup = {Position::Foundation, Position::Blush, Position::Mascara, Position::Lipstick, Position::BrowGel, Position::Concealer};
    EXPECT_EQ(hair, cosmetics::positions_for(Category::Hair));
    EXPECT_EQ(body, cosmetics::positions_for(Category::Body));
    EXPECT_EQ(skin, cosmetics::positions_for(Category::Skin));
    EXPECT_EQ(makeup, cosmetics::positions_for(Category::Makeup));
}

TEST(CategoryPositions, AllPositionsOccurExactlyOnce) {
    std::vector<cosmetics::Position> combined;
    for (const auto category : cosmetics::categories) {
        const auto& items = cosmetics::positions_for(category);
        combined.insert(combined.end(), items.begin(), items.end());
    }
    EXPECT_EQ(19u, combined.size());
    std::sort(combined.begin(), combined.end());
    EXPECT_EQ(combined.end(), std::adjacent_find(combined.begin(), combined.end()));
    auto expected = cosmetics::positions;
    std::sort(expected.begin(), expected.end());
    EXPECT_TRUE(std::equal(combined.begin(), combined.end(), expected.begin(), expected.end()));
}

TEST(CategoryPositions, InvalidCategory) {
    EXPECT_THROW(cosmetics::positions_for(static_cast<cosmetics::Category>(99)), std::invalid_argument);
}
