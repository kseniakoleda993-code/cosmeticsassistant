#include <gtest/gtest.h>

#include <iostream>

TEST(FailureProbe, FailsWithDiagnostics) {
    EXPECT_EQ(1, 2);
}

TEST(FailureProbe, ContinuesAfterFailure) {
    std::cout << "probe-continued\n";
    SUCCEED();
}
