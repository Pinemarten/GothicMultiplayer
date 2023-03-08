#include "posixcrashhandler.h"

#include <execinfo.h>
#include <errno.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <memory>

#include "../defines.h"
#include "../common/common.h"

#define KILOBYTE 1024
#define DEMANGLE_MEMORY_SIZE (10*(KILOBYTE))
#define STACK_MEMORY_SIZE (90*(KILOBYTE))

namespace Debug {
    struct FreeDeleter {
        void operator()(void* ptr) const {
            free(ptr);
        }
    };

    char *fake_alloc(char **memory, size_t size) {
        char *allocated = *memory;
        char *last = allocated + size;
        *last = '\0';
        *memory += size + 1;
        return allocated;
    }

    void chilltrace(const char * const stackEntry) {
        if (stackEntry) {
            fputs(stackEntry,stderr);
        }
    }

    void posixSignalHandler( int signum, siginfo_t* si, void* ucontext ) {
        (void)si;
        (void)ucontext;

        auto &handler = PosixCrashHandler::getInstance();
        handler.handleCrash();

        // If you caught one of the above signals, it is likely you just
        // want to quit your program right now.
        //exit( signum );
        std::_Exit(CHILLOUT_EXIT_CODE);
    }

    PosixCrashHandler::PosixCrashHandler():
        m_stackMemory(nullptr),
        m_demangleMemory(nullptr)
    {
        m_stackMemory = (char*)malloc(STACK_MEMORY_SIZE);
        memset(&m_stackMemory[0], 0, STACK_MEMORY_SIZE);

        m_demangleMemory = (char*)malloc(DEMANGLE_MEMORY_SIZE);
        memset(&m_demangleMemory[0], 0, DEMANGLE_MEMORY_SIZE);

        m_backtraceCallback = chilltrace;
    }

    PosixCrashHandler::~PosixCrashHandler() {
        free(m_stackMemory);
        free(m_demangleMemory);
    }

    void PosixCrashHandler::setup(const std::string &appName, const std::string &crashDirPath) {
        struct sigaction sa;
        sa.sa_sigaction = posixSignalHandler;
        sigemptyset( &sa.sa_mask );

#ifdef __APPLE__
        /* for some reason we backtrace() doesn't work on osx
       when we use an alternate stack */
        sa.sa_flags = SA_SIGINFO;
#else
        sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
#endif

        sigaction( SIGABRT, &sa, NULL );
        sigaction( SIGSEGV, &sa, NULL );
        sigaction( SIGBUS,  &sa, NULL );
        sigaction( SIGILL,  &sa, NULL );
        sigaction( SIGFPE,  &sa, NULL );
        sigaction( SIGPIPE, &sa, NULL );
        sigaction( SIGTERM, &sa, NULL );

        if (!crashDirPath.empty()) {
            std::string path = crashDirPath;
            while ((path.size() > 1) &&
                   (path[path.size() - 1] == '/')) {
                path.erase(path.size() - 1);
            }

            std::stringstream s;
            s << path << "/" << appName << "_";
            formatDateTime(s, now(), CHILLOUT_DATETIME);
            s << ".bktr";
            m_backtraceFilePath = s.str();
        }
    }

    void PosixCrashHandler::teardown() {
        struct sigaction sa;
        sigset_t mysigset;
        
        sigemptyset(&mysigset);
        
        sa.sa_handler = SIG_DFL;
        sa.sa_mask = mysigset;
        sa.sa_flags = 0;

        sigaction( SIGABRT, &sa, NULL );
        sigaction( SIGSEGV, &sa, NULL );
        sigaction( SIGBUS,  &sa, NULL );
        sigaction( SIGILL,  &sa, NULL );
        sigaction( SIGFPE,  &sa, NULL );
        sigaction( SIGPIPE, &sa, NULL );
        sigaction( SIGTERM, &sa, NULL );
    }

    void PosixCrashHandler::handleCrash() {
        if (m_crashCallback) {
            m_crashCallback();
        }
    }
    
    void PosixCrashHandler::setCrashCallback(const std::function<void()> &callback) {
        m_crashCallback = callback;
    }
    
    void PosixCrashHandler::setBacktraceCallback(const std::function<void(const char * const)> &callback) {
        m_backtraceCallback = callback;
    }

    void PosixCrashHandler::backtrace() {
        if (m_backtraceCallback) {
            walkStackTrace(m_stackMemory, STACK_MEMORY_SIZE);
        }
    }

    void PosixCrashHandler::walkStackTrace(char *memory, size_t memorySize, int maxFrames) {
        const size_t framesSize = maxFrames * sizeof(void*);
        void **callstack = reinterpret_cast<void**>(fake_alloc(&memory, framesSize));
        int frames = ::backtrace(callstack, maxFrames);

        const int stackOffset = callstack[2] == callstack[1] ? 2 : 1;

        if (!m_backtraceFilePath.empty()) {
            if (FILE *fp = fopen(m_backtraceFilePath.c_str(), "a")) {
                fseek(fp, 0, SEEK_END);
                int fd = fileno(fp);
                backtrace_symbols_fd(callstack, frames, fd);
                fclose(fp);
            }
        }

        std::unique_ptr<char*, FreeDeleter> symbolsPtr(backtrace_symbols(callstack, frames));
        if (!symbolsPtr) { return; }

        char **symbols = symbolsPtr.get();

        for (int i = stackOffset; i < frames; ++i) {
            memset(memory, 0, memorySize - framesSize - 1);

            char *stackFrame = dlDemangle(callstack[i], symbols[i], i, memory);
            if (stackFrame) {
                m_backtraceCallback(stackFrame);
            }
        }

        if (frames == maxFrames) {
            m_backtraceCallback("[truncated]\n");
        }
    }

    char *PosixCrashHandler::dlDemangle(void *addr, char *symbol, int frameIndex, char *stackMemory) {
        Dl_info info;
        if (dladdr(addr, &info) != 0) {
            const int stackFrameSize = 4096;
            char *stackFrame = fake_alloc(&stackMemory, stackFrameSize);

            if ((info.dli_sname != NULL) && (info.dli_sname[0] == '_')) {
                int status = -1;
                size_t length = DEMANGLE_MEMORY_SIZE;
                char *demangled = abi::__cxa_demangle(info.dli_sname, m_demangleMemory, &length, &status);
                if (status == 0) { m_demangleMemory = demangled; }

                snprintf(stackFrame, stackFrameSize, "%-3d %*p %s + %zd\n",
                         frameIndex, int(2 + sizeof(void*) * 2), addr,
                         status == 0 ? demangled :
                                       info.dli_sname == 0 ? symbol : info.dli_sname,
                         (char *)addr - (char *)info.dli_saddr);

            } else {
                snprintf(stackFrame, stackFrameSize, "%-3d %*p %s\n",
                         frameIndex, int(2 + sizeof(void*) * 2), addr, symbol);
            }

            return stackFrame;
        }

        return NULL;
    }
}
