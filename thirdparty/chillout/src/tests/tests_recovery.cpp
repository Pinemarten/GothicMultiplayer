#include <float.h>
#include <gtest/gtest.h>
#include <chillout.h>
#include <defines.h>
#include <string>
#include <cstdio>
#include "tests_source.h"
#include "gtest_extensions.h"
#include <signal.h>
#include <defines.h>

void chilltrace(const char * const stackEntry) {
    if (stackEntry) {
        fprintf(stderr, "chilltrace:  %s", stackEntry);
    }
}

class RecoveryTest : public ::testing::Test
{
protected:
    virtual void SetUp() {
        auto &chillout = Debug::Chillout::getInstance();
        chillout.init(WIDEN("chillout_test"), WIDEN(STRINGIZE(CRASHDUMPSDIR)));
        chillout.setBacktraceCallback(chilltrace);
        chillout.setCrashCallback([&chillout](){
                chillout.backtrace();
#ifdef _WIN32
                chillout.createCrashDump();
#endif
            });
    }

    virtual void TearDown() {
        auto &chillout = Debug::Chillout::getInstance();
        chillout.deinit();
    }
};

class DISABLED_RecoveryTest : public ::testing::Test
{
};

TEST_F (RecoveryTest, PureVirtualMethodCallTest) {
#ifdef _WIN32
    const char regex[] = "::PureCallHandler";
#else
    const char regex[] = "PosixCrashHandler::handleCrash";
#endif
    
    ASSERT_EXIT(Derived(), ::extensions::ExitedOrKilled(CHILLOUT_EXIT_CODE, SIGSEGV), regex);
}

TEST_F (RecoveryTest, AccessViolationTest) {
    ASSERT_EXIT(AccessViolation(), ::extensions::ExitedOrKilled(CHILLOUT_EXIT_CODE, SIGSEGV), "AccessViolationTest");
}

#ifndef __linux__ 
TEST_F (RecoveryTest, InvalidParameterTest) {
#ifdef _WIN32
    const char regex[] = "::InvalidParameterHandler";
#else
    const char regex[] = "PosixCrashHandler::handleCrash";
#endif
        
    ASSERT_EXIT(InvalidParameter(), ::extensions::ExitedOrKilled(CHILLOUT_EXIT_CODE, SIGSEGV), regex);
}
#endif

TEST_F (RecoveryTest, SigillTest) {
#ifdef _WIN32
    const char regex[] = "::SigillHandler";
#else
    const char regex[] = "posixSignalHandler";
#endif

    ASSERT_EXIT(RaiseSigill(), ::extensions::ExitedOrKilled(CHILLOUT_EXIT_CODE, SIGILL), regex);
}

TEST_F (RecoveryTest, SigsegvTest) {
#ifdef _WIN32
    const char regex[] = "::SigsegvHandler";
#else
    const char regex[] = "posixSignalHandler";
#endif

    ASSERT_EXIT(RaiseSigsegv(), ::extensions::ExitedOrKilled(CHILLOUT_EXIT_CODE, SIGSEGV), regex);
}

TEST_F (RecoveryTest, SigtermTest) {
#ifdef _WIN32
    const char regex[] = "::SigtermHandler";
#else
    const char regex[] = "posixSignalHandler";
#endif

    ASSERT_EXIT(RaiseSigterm(), ::extensions::ExitedOrKilled(CHILLOUT_EXIT_CODE, SIGTERM), regex);
}

// this test is disabled because you can catch c++ exceptions
TEST_F (DISABLED_RecoveryTest, ThrowExceptionTest) {
    ASSERT_EXIT(ThrowException(), ::testing::ExitedWithCode(CHILLOUT_EXIT_CODE), "");
}

TEST_F (DISABLED_RecoveryTest, MemoryTest) {
    ASSERT_EXIT(MemoryOverflow(), ::testing::ExitedWithCode(CHILLOUT_EXIT_CODE), "::NewHandler");
}

TEST_F (DISABLED_RecoveryTest, StackOverflowTest) {
    ASSERT_EXIT(StackOverflow(), ::testing::ExitedWithCode(CHILLOUT_EXIT_CODE), "::adfadf");
}

TEST_F (RecoveryTest, RaiseExceptionTest) {
#ifdef _WIN32
    const char regex[] = "Chillout SehHandler";
#else
    const char regex[] = "posixSignalHandler";
#endif
    ASSERT_EXIT(RaiseSehException(), ::extensions::ExitedOrKilled(CHILLOUT_EXIT_CODE, SIGTERM), regex);
}
