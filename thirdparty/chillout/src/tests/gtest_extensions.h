#ifndef GTEST_EXTENSIONS_H
#define GTEST_EXTENSIONS_H

#include <gtest/gtest.h>

namespace extensions {
    // Tests that an exit code describes a normal exit with a given exit code.
    class GTEST_API_ ExitedOrKilled {
      public:
        explicit ExitedOrKilled(int exit_code, int signal);
        bool operator()(int value) const;

      private:
        // No implementation - assignment is unsupported.
        void operator=(const ExitedOrKilled& other);

#ifndef _WIN32
        ::testing::KilledBySignal _killedBySignal;
#endif
        ::testing::ExitedWithCode _exitedWithCode;
    };
}

#endif // GTEST_EXTENSIONS_H
