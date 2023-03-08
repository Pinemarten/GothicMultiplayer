#include "gtest_extensions.h"

namespace extensions {
    ExitedOrKilled::ExitedOrKilled(int exit_code, int signal):
        _exitedWithCode(exit_code)
#ifndef _WIN32
        ,_killedBySignal(signal)
#endif
    { }

    bool ExitedOrKilled::operator()(int value) const {
        return _exitedWithCode(value) ||
#ifndef _WIN32
            _killedBySignal(value);
#else
        false;
#endif
    }
}


