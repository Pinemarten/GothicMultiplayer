#include <float.h>
#include <gtest/gtest.h>
#include "tests_source.h"

TEST (CrashTest, PureVirtualMethodCallTest) {
    ASSERT_DEATH(Derived(), "");
}

TEST (CrashTest, AccessViolationTest) {
    ASSERT_DEATH(AccessViolation(), "");
}

// This test is disabled because of CRT assert window.
// This window could be disabled for a process but
// gtest starts new process for DEATH tests so it
// makes no sense to run it.
TEST (CrashTest, DISABLED_InvalidParameterTest) {
    ASSERT_DEATH(InvalidParameter(), "");
}

TEST (CrashTest, SigillTest) {
    ASSERT_DEATH(RaiseSigill(), "");
}

TEST (CrashTest, SigsegvTest) {
    ASSERT_DEATH(RaiseSigsegv(), "");
}

TEST (CrashTest, SigtermTest) {
    ASSERT_DEATH(RaiseSigterm(), "");
}

TEST (CrashTest, ThrowExceptionTest) {
    ASSERT_ANY_THROW(ThrowException());
}

TEST (CrashTest, DISABLED_MemoryTest) {
    ASSERT_ANY_THROW(MemoryOverflow());
}

TEST (CrashTest, DISABLED_StackOverflowTest) {
    ASSERT_DEATH(StackOverflow(), "");
}

TEST (CrashTest, RaiseExceptionTest) {
    ASSERT_DEATH(RaiseSehException(), "");
}
