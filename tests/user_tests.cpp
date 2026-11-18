#include "cosmetics/dialog/user.hpp"

#include <gtest/gtest.h>

using namespace cosmetics;

TEST(User, IndependentCopies) {
    User initial;
    auto updated = initial.with_category(Category::Hair).with_position(Position::Shampoo).with_budget(Budget::Average);
    updated = updated.with_cart(Cart().add(Product("товар", 100))).with_stage(Stage::Continue);
    EXPECT_EQ(initial.stage(), Stage::Start);
    EXPECT_FALSE(initial.category());
    EXPECT_FALSE(initial.position());
    EXPECT_FALSE(initial.budget());
    EXPECT_TRUE(initial.cart().items().empty());
    EXPECT_EQ(updated.category(), Category::Hair);
    EXPECT_EQ(updated.position(), Position::Shampoo);
    EXPECT_EQ(updated.budget(), Budget::Average);
    EXPECT_EQ(updated.cart().total(), 100);
    EXPECT_EQ(updated.stage(), Stage::Continue);
}

#include "cosmetics/storage/users.hpp"

TEST(Users, ChatIdsHaveSeparateState) {
    Users users;
    users.set(91, User().with_stage(Stage::Category));
    users.set(-100123456789, User().with_category(Category::Hair).with_stage(Stage::Position));
    EXPECT_EQ(users.get(91).stage(), Stage::Category);
    EXPECT_FALSE(users.get(91).category());
    EXPECT_EQ(users.get(-100123456789).stage(), Stage::Position);
    EXPECT_EQ(users.get(-100123456789).category(), Category::Hair);
    EXPECT_EQ(users.get(32).stage(), Stage::Start);
    users.set(91, User().with_stage(Stage::End));
    EXPECT_EQ(users.get(91).stage(), Stage::End);
}
