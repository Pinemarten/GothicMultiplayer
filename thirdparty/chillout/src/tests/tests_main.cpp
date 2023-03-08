#include <gtest/gtest.h>

int main(int argc, char **argv) {
    ::testing::GTEST_FLAG(catch_exceptions) = false;
    ::testing::GTEST_FLAG(death_test_style) = "threadsafe";
    //::testing::GTEST_FLAG(filter) = "RecoveryTest.AccessViolationTest";

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
