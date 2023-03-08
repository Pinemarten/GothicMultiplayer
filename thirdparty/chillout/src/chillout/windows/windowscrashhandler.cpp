#include "windowscrashhandler.h"
#include "../defines.h"

#include <tchar.h>
#include <stdio.h>
#include "StackWalker.h"

// minidump
#include <windows.h>
#include <TlHelp32.h>
//#include <tchar.h>
#include <dbghelp.h>
//#include <stdio.h>
#include <crtdbg.h>
#include <signal.h>

#include <locale>
#include <codecvt>
#include <ctime>
#include <sstream>
#include "../common/common.h"

#if _MSC_VER < 1400
#define strcpy_s(dst, len, src) strcpy(dst, src)
#define strncpy_s(dst, len, src, maxLen) strncpy(dst, len, src)
#define strcat_s(dst, len, src) strcat(dst, src)
#define _snprintf_s _snprintf
#define _tcscat_s _tcscat
#endif

#define CR_SEH_EXCEPTION                0    //!< SEH exception.
#define CR_CPP_TERMINATE_CALL           1    //!< C++ terminate() call.
#define CR_CPP_UNEXPECTED_CALL          2    //!< C++ unexpected() call.
#define CR_CPP_PURE_CALL                3    //!< C++ pure virtual function call (VS .NET and later).
#define CR_CPP_NEW_OPERATOR_ERROR       4    //!< C++ new operator fault (VS .NET and later).
#define CR_CPP_SECURITY_ERROR           5    //!< Buffer overrun error (VS .NET only).
#define CR_CPP_INVALID_PARAMETER        6    //!< Invalid parameter exception (VS 2005 and later).
#define CR_CPP_SIGABRT                  7    //!< C++ SIGABRT signal (abort).
#define CR_CPP_SIGFPE                   8    //!< C++ SIGFPE signal (flotating point exception).
#define CR_CPP_SIGILL                   9    //!< C++ SIGILL signal (illegal instruction).
#define CR_CPP_SIGINT                   10   //!< C++ SIGINT signal (CTRL+C).
#define CR_CPP_SIGSEGV                  11   //!< C++ SIGSEGV signal (invalid storage access).
#define CR_CPP_SIGTERM                  12   //!< C++ SIGTERM signal (termination request).

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#ifndef _AddressOfReturnAddress
    // Taken from: http://msdn.microsoft.com/en-us/library/s975zw7k(VS.71).aspx
    #ifdef __cplusplus
        #define EXTERNC extern "C"
    #else
        #define EXTERNC
    #endif

    // _ReturnAddress and _AddressOfReturnAddress should be prototyped before use
    EXTERNC void * _AddressOfReturnAddress(void);
    EXTERNC void * _ReturnAddress(void);
#endif

namespace Debug {
    class StackWalkerWithCallback : public StackWalker
    {
    public:
        StackWalkerWithCallback(const std::function<void(const char * const)> &callback):
            StackWalker(RetrieveVerbose | SymBuildPath),
            m_callback(callback)
        { }

    protected:
        virtual void OnOutput(LPCSTR szText) override {
            m_callback(szText);
        }

    private:
        std::function<void(const char * const)> m_callback;
    };

    BOOL PreventSetUnhandledExceptionFilter()
    {
        HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
        if (hKernel32 == NULL) return FALSE;
        void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
        if (pOrgEntry == NULL) return FALSE;

#ifdef _M_IX86
        // Code for x86:
        // 33 C0                xor         eax,eax
        // C2 04 00             ret         4
        unsigned char szExecute[] = { 0x33, 0xC0, 0xC2, 0x04, 0x00 };
#elif _M_X64
        // 33 C0                xor         eax,eax
        // C3                   ret
        unsigned char szExecute[] = { 0x33, 0xC0, 0xC3 };
#else
#error "The following code only works for x86 and x64!"
#endif

        SIZE_T bytesWritten = 0;
        BOOL bRet = WriteProcessMemory(GetCurrentProcess(),
                                       pOrgEntry, szExecute, sizeof(szExecute), &bytesWritten);
        return bRet;
    }

    BOOL CALLBACK MyMiniDumpCallback(
            PVOID                            pParam,
            const PMINIDUMP_CALLBACK_INPUT   pInput,
            PMINIDUMP_CALLBACK_OUTPUT        pOutput
            )
    {
        BOOL bRet = FALSE;

        // Check parameters

        if( pInput == 0 )
            return FALSE;

        if( pOutput == 0 )
            return FALSE;

        // Process the callbacks
        WindowsCrashHandler *handler = (WindowsCrashHandler*)pParam;

        switch( pInput->CallbackType )
        {
        case IncludeModuleCallback:
        {
            // Include the module into the dump
            bRet = TRUE;
        }
            break;

        case IncludeThreadCallback:
        {
            // Include the thread into the dump
            bRet = TRUE;
        }
            break;

        case ModuleCallback:
        {
            // Are data sections available for this module ?
            if( pOutput->ModuleWriteFlags & ModuleWriteDataSeg )
            {
                // Yes, they are, but do we need them?

                if( !handler->isDataSectionNeeded( pInput->Module.FullPath ) )
                {
                    pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);
                }
            }

            if( !(pOutput->ModuleWriteFlags & ModuleReferencedByMemory) )
            {
                // No, it does not - exclude it
                pOutput->ModuleWriteFlags &= (~ModuleWriteModule);
            }

            bRet = TRUE;
        }
            break;

        case ThreadCallback:
        {
            // Include all thread information into the minidump
            bRet = TRUE;
        }
            break;

        case ThreadExCallback:
        {
            // Include this information
            bRet = TRUE;
        }
            break;

        case MemoryCallback:
        {
            // We do not include any information here -> return FALSE
            bRet = FALSE;
        }
            break;

        case CancelCallback:
            break;
        }

        return bRet;

    }

    BOOL CreateMiniDump(LPCTSTR lpFileName, EXCEPTION_POINTERS * pep, MINIDUMP_TYPE mdt, WindowsCrashHandler * wch)
    {
        BOOL rv = FALSE;
        // Open the file

        HANDLE hFile = CreateFile( lpFileName, GENERIC_READ | GENERIC_WRITE,
                                   0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

        if( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) )
        {
            // Create the minidump

            MINIDUMP_EXCEPTION_INFORMATION mdei;

            mdei.ThreadId           = GetCurrentThreadId();
            mdei.ExceptionPointers  = pep;
            mdei.ClientPointers     = FALSE;

            MINIDUMP_CALLBACK_INFORMATION mci;

            mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback;
            mci.CallbackParam       = wch;

            rv = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(),
                                    hFile, mdt, (pep != 0) ? &mdei : 0, 0, &mci );

            // Close the file
            CloseHandle( hFile );
        }

        fflush(stdout);
        fflush(stderr);

        return rv;
    }

    // The following code gets exception pointers using a workaround found in CRT code.
    void GetExceptionPointers(DWORD dwExceptionCode,
                              EXCEPTION_POINTERS** ppExceptionPointers) {
        // The following code was taken from VC++ 8.0 CRT (invarg.c: line 104)

        EXCEPTION_RECORD ExceptionRecord;
        CONTEXT ContextRecord;
        memset(&ContextRecord, 0, sizeof(CONTEXT));

#ifdef _X86_

        __asm {
            mov dword ptr [ContextRecord.Eax], eax
                    mov dword ptr [ContextRecord.Ecx], ecx
                    mov dword ptr [ContextRecord.Edx], edx
                    mov dword ptr [ContextRecord.Ebx], ebx
                    mov dword ptr [ContextRecord.Esi], esi
                    mov dword ptr [ContextRecord.Edi], edi
                    mov word ptr [ContextRecord.SegSs], ss
                    mov word ptr [ContextRecord.SegCs], cs
                    mov word ptr [ContextRecord.SegDs], ds
                    mov word ptr [ContextRecord.SegEs], es
                    mov word ptr [ContextRecord.SegFs], fs
                    mov word ptr [ContextRecord.SegGs], gs
                    pushfd
                    pop [ContextRecord.EFlags]
        }

        ContextRecord.ContextFlags = CONTEXT_CONTROL;
#pragma warning(push)
#pragma warning(disable:4311)
        ContextRecord.Eip = (ULONG)_ReturnAddress();
        ContextRecord.Esp = (ULONG)_AddressOfReturnAddress();
#pragma warning(pop)
        ContextRecord.Ebp = *((ULONG *)_AddressOfReturnAddress()-1);


#elif defined (_IA64_) || defined (_AMD64_)

        /* Need to fill up the Context in IA64 and AMD64. */
        RtlCaptureContext(&ContextRecord);

#else  /* defined (_IA64_) || defined (_AMD64_) */

        ZeroMemory(&ContextRecord, sizeof(ContextRecord));

#endif  /* defined (_IA64_) || defined (_AMD64_) */

        ZeroMemory(&ExceptionRecord, sizeof(EXCEPTION_RECORD));

        ExceptionRecord.ExceptionCode = dwExceptionCode;
        ExceptionRecord.ExceptionAddress = _ReturnAddress();

        ///

        EXCEPTION_RECORD* pExceptionRecord = new EXCEPTION_RECORD;
        memcpy(pExceptionRecord, &ExceptionRecord, sizeof(EXCEPTION_RECORD));
        CONTEXT* pContextRecord = new CONTEXT;
        memcpy(pContextRecord, &ContextRecord, sizeof(CONTEXT));

        *ppExceptionPointers = new EXCEPTION_POINTERS;
        (*ppExceptionPointers)->ExceptionRecord = pExceptionRecord;
        (*ppExceptionPointers)->ContextRecord = pContextRecord;
    }

    void DoHandleCrash(EXCEPTION_POINTERS* pExPtrs) {
        WindowsCrashHandler &handler = WindowsCrashHandler::getInstance();
        handler.handleCrash(pExPtrs);
    }

    // http://groups.google.com/group/crashrpt/browse_thread/thread/a1dbcc56acb58b27/fbd0151dd8e26daf?lnk=gst&q=stack+overflow#fbd0151dd8e26daf
    // Thread procedure doing the dump for stack overflow.
    DWORD WINAPI StackOverflowThreadFunction(LPVOID lpParameter) {
        PEXCEPTION_POINTERS pExceptionPtrs =
                reinterpret_cast<PEXCEPTION_POINTERS>(lpParameter);

        if (pExceptionPtrs == NULL) {
            GetExceptionPointers(CR_SEH_EXCEPTION, &pExceptionPtrs);
        }

        DoHandleCrash(pExceptionPtrs);

        TerminateProcess(GetCurrentProcess(), CHILLOUT_EXIT_CODE);

        return 0;
    }

    static LONG WINAPI SehHandler(EXCEPTION_POINTERS* pExPtrs) {
#ifdef _DEBUG
        fprintf(stderr, "Chillout SehHandler");
#endif

#ifdef _M_IX86
        if (pExPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
        {
            // http://groups.google.com/group/crashrpt/browse_thread/thread/a1dbcc56acb58b27/fbd0151dd8e26daf?lnk=gst&q=stack+overflow#fbd0151dd8e26daf
            HANDLE thread = ::CreateThread(0, 0,
                                           &StackOverflowThreadFunction, pExPtrs, 0, 0);
            DWORD dwMilliseconds = 1000 * 30; // 30 seconds
            ::WaitForSingleObject(thread, dwMilliseconds);
            ::CloseHandle(thread);
            return EXCEPTION_EXECUTE_HANDLER;
        }
#endif

        if (pExPtrs == NULL) {
            GetExceptionPointers(CR_SEH_EXCEPTION, &pExPtrs);
        }

        DoHandleCrash(pExPtrs);

        TerminateProcess(GetCurrentProcess(), CHILLOUT_EXIT_CODE);

        // unreachable
        return EXCEPTION_EXECUTE_HANDLER;
    }

    // The following code is intended to fix the issue with 32-bit applications in 64-bit environment.
    // http://support.microsoft.com/kb/976038/en-us
    // http://code.google.com/p/crashrpt/issues/detail?id=104
    void EnableCrashingOnCrashes() {
        typedef BOOL (WINAPI *tGetPolicy)(LPDWORD lpFlags);
        typedef BOOL (WINAPI *tSetPolicy)(DWORD dwFlags);
        static const DWORD EXCEPTION_SWALLOWING = 0x1;

        const HMODULE kernel32 = LoadLibraryA("kernel32.dll");
        const tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(kernel32, "GetProcessUserModeExceptionPolicy");
        const tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(kernel32, "SetProcessUserModeExceptionPolicy");
        if(pGetPolicy && pSetPolicy)
        {
            DWORD dwFlags;
            if(pGetPolicy(&dwFlags))
            {
                // Turn off the filter
                pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING);
            }
        }
    }

    WindowsCrashHandler::WindowsCrashHandler() {
        m_lastExceptionPointers = NULL;

        m_oldSehHandler = NULL;

#if _MSC_VER>=1300
        m_prevPurec = NULL;
        m_prevNewHandler = NULL;
#endif

#if _MSC_VER>=1300 && _MSC_VER<1400    
        m_prevSec = NULL;
#endif

#if _MSC_VER>=1400
        m_prevInvpar = NULL;
#endif  

        m_prevSigABRT = NULL;
        m_prevSigINT = NULL;
        m_prevSigTERM = NULL;

        m_crtReportHook = NULL;
    }

    void WindowsCrashHandler::setup(const std::wstring &appName, const std::wstring &dumpsDir) {
        m_appName = appName;

        if (!appName.empty() && !dumpsDir.empty()) {
            std::wstring path = dumpsDir;
            while ((path.size() > 1) &&
                   (path[path.size() - 1] == L'\\')) {
                path.erase(path.size() - 1);
            }

            std::wstringstream s;
            s << L"\\\\?\\" << path << L'\\' << appName << L'_';
            formatDateTime(s, now(), WIDEN(CHILLOUT_DATETIME));
            s << ".dmp";
            m_pathToCrashDump = s.str();
        }

        EnableCrashingOnCrashes();
        setProcessExceptionHandlers();
        setThreadExceptionHandlers();
    }

    void WindowsCrashHandler::teardown() {
        unsetProcessExceptionHandlers();
        unsetThreadExceptionHandlers();
    }

    void WindowsCrashHandler::setCrashCallback(const std::function<void()> &callback) {
        m_crashCallback = callback;
    }

    void WindowsCrashHandler::setBacktraceCallback(const std::function<void(const char * const)> &callback) {
        m_backtraceCallback = callback;
    }

    void WindowsCrashHandler::handleCrash(EXCEPTION_POINTERS* pExPtrs) {
        std::lock_guard<std::mutex> guard(m_crashMutex);
        (void)guard;

        m_lastExceptionPointers = pExPtrs;

        if (m_crashCallback) {
            m_crashCallback();
        }

        // Terminate process
        TerminateProcess(GetCurrentProcess(), CHILLOUT_EXIT_CODE);
    }

    void WindowsCrashHandler::backtrace() {
        if (m_backtraceCallback && (m_lastExceptionPointers != NULL)) {
            StackWalkerWithCallback sw(m_backtraceCallback);
            sw.ShowCallstack(GetCurrentThread(), m_lastExceptionPointers->ContextRecord);
        }
    }

    void WindowsCrashHandler::createCrashDump(CrashDumpSize size) {
        if (m_lastExceptionPointers == NULL) { return; }
        if (m_pathToCrashDump.empty()) { return; }

        MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpNormal);

        switch (size)
        {
        case CrashDumpSmall:
            mdt = (MINIDUMP_TYPE)(MiniDumpScanMemory |
                                  MiniDumpWithThreadInfo);
            break;
        case CrashDumpNormal:
        {
            mdt = (MINIDUMP_TYPE)(MiniDumpScanMemory |
                                  MiniDumpWithHandleData |
                                  MiniDumpWithThreadInfo |
                                  MiniDumpWithIndirectlyReferencedMemory);
            break;
        }
        case CrashDumpFull:
        {
            mdt = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory |
                                  MiniDumpWithDataSegs |
                                  MiniDumpWithHandleData |
                                  MiniDumpWithFullMemoryInfo |
                                  MiniDumpScanMemory |
                                  MiniDumpWithThreadInfo |
                                  MiniDumpWithUnloadedModules);
            break;
        }
        }

        CreateMiniDump(m_pathToCrashDump.c_str(), m_lastExceptionPointers, mdt, this);
    }

    bool WindowsCrashHandler::isDataSectionNeeded(const WCHAR* pModuleName) {
        if( pModuleName == 0 ) {
            _ASSERTE( _T("Parameter is null.") );
            return false;
        }

        // Extract the module name

        WCHAR szFileName[_MAX_FNAME] = L"";
        _wsplitpath( pModuleName, NULL, NULL, szFileName, NULL );

        // Compare the name with the list of known names and decide
        // if contains app name in its path
        if( wcsstr( pModuleName, m_appName.c_str() ) != 0 ) {
            return true;
        } else if( _wcsicmp( szFileName, L"ntdll" ) == 0 ) {
            return true;
        } else if( wcsstr( szFileName, L"Qt5" ) != 0 ) {
            return true;
        }

        // Complete
        return false;
    }

    void WindowsCrashHandler::setProcessExceptionHandlers() {
        //SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS);
        m_oldSehHandler = SetUnhandledExceptionFilter(SehHandler);
#if defined _M_X64 || defined _M_IX86
        if (m_oldSehHandler)
            PreventSetUnhandledExceptionFilter();
#endif

#if _MSC_VER>=1300
        // Catch pure virtual function calls.
        // Because there is one _purecall_handler for the whole process,
        // calling this function immediately impacts all threads. The last
        // caller on any thread sets the handler.
        // http://msdn.microsoft.com/en-us/library/t296ys27.aspx
        m_prevPurec = _set_purecall_handler(PureCallHandler);

        // Catch new operator memory allocation exceptions
        //_set_new_mode(1); // Force malloc() to call new handler too
        m_prevNewHandler = _set_new_handler(NewHandler);
#endif

#if _MSC_VER>=1400
        // Catch invalid parameter exceptions.
        m_prevInvpar = _set_invalid_parameter_handler(InvalidParameterHandler);
#endif

#if _MSC_VER>=1300 && _MSC_VER<1400    
        // Catch buffer overrun exceptions
        // The _set_security_error_handler is deprecated in VC8 C++ run time library
        m_prevSec = _set_security_error_handler(SecurityHandler);
#endif

#if _MSC_VER>=1400
        _set_abort_behavior(0, _WRITE_ABORT_MSG);
        _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
#endif

#if defined(_MSC_VER)
        // Disable all of the possible ways Windows conspires to make automated
        // testing impossible.

        // ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
        // ::_set_error_mode(_OUT_TO_STDERR);

        // _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
        // _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
        // _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
        // _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
        // _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
        // _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

        m_crtReportHook = _CrtSetReportHook(CrtReportHook);
#endif

        // Set up C++ signal handlers

        // Catch an abnormal program termination
        m_prevSigABRT = signal(SIGABRT, SigabrtHandler);

        // Catch illegal instruction handler
        m_prevSigINT = signal(SIGINT, SigintHandler);

        // Catch a termination request
        m_prevSigINT = signal(SIGTERM, SigtermHandler);
    }

    void WindowsCrashHandler::unsetProcessExceptionHandlers() {
#if _MSC_VER>=1300
        if(m_prevPurec!=NULL) {
            _set_purecall_handler(m_prevPurec);
        }

        if(m_prevNewHandler!=NULL) {
            _set_new_handler(m_prevNewHandler);
        }
#endif

#if _MSC_VER>=1400
        if(m_prevInvpar!=NULL) {
            _set_invalid_parameter_handler(m_prevInvpar);
        }
#endif //_MSC_VER>=1400  

#if _MSC_VER>=1300 && _MSC_VER<1400    
        if(m_prevSec!=NULL)
            _set_security_error_handler(m_prevSec);
#endif //_MSC_VER<1400

        if(m_prevSigABRT!=NULL) {
            signal(SIGABRT, m_prevSigABRT);
        }

        if(m_prevSigINT!=NULL) {
            signal(SIGINT, m_prevSigINT);
        }

        if(m_prevSigTERM!=NULL) {
            signal(SIGTERM, m_prevSigTERM);
        }

        // Reset SEH exception filter
        if (m_oldSehHandler) {
            SetUnhandledExceptionFilter(m_oldSehHandler);
        }

        m_oldSehHandler = NULL;

        if (m_crtReportHook) {
            _CrtSetReportHook(m_crtReportHook);
        }
    }

    int WindowsCrashHandler::setThreadExceptionHandlers()
    {
        DWORD dwThreadId = GetCurrentThreadId();

        std::lock_guard<std::mutex> guard(m_threadHandlersMutex);

        auto it = m_threadExceptionHandlers.find(dwThreadId);
        if (it != m_threadExceptionHandlers.end()) {
            // handlers are already set for the thread
            return 1; // failed
        }

        ThreadExceptionHandlers handlers;

        // Catch terminate() calls.
        // In a multithreaded environment, terminate functions are maintained
        // separately for each thread. Each new thread needs to install its own
        // terminate function. Thus, each thread is in charge of its own termination handling.
        // http://msdn.microsoft.com/en-us/library/t6fk7h29.aspx
        handlers.m_prevTerm = set_terminate(TerminateHandler);

        // Catch unexpected() calls.
        // In a multithreaded environment, unexpected functions are maintained
        // separately for each thread. Each new thread needs to install its own
        // unexpected function. Thus, each thread is in charge of its own unexpected handling.
        // http://msdn.microsoft.com/en-us/library/h46t5b69.aspx
        handlers.m_prevUnexp = set_unexpected(UnexpectedHandler);

        // Catch a floating point error
        typedef void (*sigh)(int);
        handlers.m_prevSigFPE = signal(SIGFPE, (sigh)SigfpeHandler);

        // Catch an illegal instruction
        handlers.m_prevSigILL = signal(SIGILL, SigillHandler);

        // Catch illegal storage access errors
        handlers.m_prevSigSEGV = signal(SIGSEGV, SigsegvHandler);

        m_threadExceptionHandlers[dwThreadId] = handlers;

        return 0;
    }

    int WindowsCrashHandler::unsetThreadExceptionHandlers() {
        DWORD dwThreadId = GetCurrentThreadId();
        std::lock_guard<std::mutex> guard(m_threadHandlersMutex);
        (void)guard;

        auto it = m_threadExceptionHandlers.find(dwThreadId);
        if (it == m_threadExceptionHandlers.end()) {
            return 1;
        }

        ThreadExceptionHandlers* handlers = &(it->second);

        if(handlers->m_prevTerm!=NULL) {
            set_terminate(handlers->m_prevTerm);
        }

        if(handlers->m_prevUnexp!=NULL) {
            set_unexpected(handlers->m_prevUnexp);
        }

        if(handlers->m_prevSigFPE!=NULL) {
            signal(SIGFPE, handlers->m_prevSigFPE);
        }

        if(handlers->m_prevSigILL!=NULL) {
            signal(SIGINT, handlers->m_prevSigILL);
        }

        if(handlers->m_prevSigSEGV!=NULL) {
            signal(SIGSEGV, handlers->m_prevSigSEGV);
        }

        // Remove from the list
        m_threadExceptionHandlers.erase(it);

        return 0;
    }

    int __cdecl WindowsCrashHandler::CrtReportHook(int nReportType, char* szMsg, int* pnRet) {
        (void)szMsg;

        switch (nReportType) {
        case _CRT_WARN:
        case _CRT_ERROR:
        case _CRT_ASSERT:
            // Put some debug code here
            break;
        }

        if (pnRet) {
            *pnRet = 0;
        }

        return TRUE;
    }

    // CRT terminate() call handler
    void __cdecl WindowsCrashHandler::TerminateHandler() {
        // Abnormal program termination (terminate() function was called)

        // Retrieve exception information
        EXCEPTION_POINTERS* pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
        if (pExceptionPtrs == nullptr) {
            GetExceptionPointers(CR_CPP_TERMINATE_CALL, &pExceptionPtrs);
        }

        DoHandleCrash(pExceptionPtrs);
    }

    // CRT unexpected() call handler
    void __cdecl WindowsCrashHandler::UnexpectedHandler() {
        // Unexpected error (unexpected() function was called)

        // Retrieve exception information
        EXCEPTION_POINTERS* pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
        if (pExceptionPtrs == nullptr) {
            GetExceptionPointers(CR_CPP_UNEXPECTED_CALL, &pExceptionPtrs);
        }

        DoHandleCrash(pExceptionPtrs);
    }

    // CRT Pure virtual method call handler
    void __cdecl WindowsCrashHandler::PureCallHandler() {
        // Pure virtual function call

        // Retrieve exception information
        EXCEPTION_POINTERS* pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
        if (pExceptionPtrs == nullptr) {
            GetExceptionPointers(CR_CPP_PURE_CALL, &pExceptionPtrs);
        }

        DoHandleCrash(pExceptionPtrs);
    }

    // CRT buffer overrun handler. Available in CRT 7.1 only
#if _MSC_VER>=1300 && _MSC_VER<1400
    void __cdecl WindowsCrashHandler::SecurityHandler(int code, void *x)
    {
        // Security error (buffer overrun).

        code;
        x;

        EXCEPTION_POINTERS* pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
        if (pExceptionPtrs == nullptr) {
            GetExceptionPointers(CR_CPP_SECURITY_ERROR, &pExceptionPtrs);
        }

        DoHandleCrash(pExceptionPtrs);
    }
#endif

#if _MSC_VER>=1400
    // CRT invalid parameter handler
    void __cdecl WindowsCrashHandler::InvalidParameterHandler(
            const wchar_t* expression,
            const wchar_t* function,
            const wchar_t* file,
            unsigned int line,
            uintptr_t pReserved) {
        pReserved;
        expression;
        function;
        file;
        line;

        // fwprintf(stderr, L"Invalid parameter detected in function %s."
        //        L" File: %s Line: %d\n", function, file, line);

        // Retrieve exception information
        EXCEPTION_POINTERS* pExceptionPtrs = NULL;
        GetExceptionPointers(CR_CPP_INVALID_PARAMETER, &pExceptionPtrs);

        DoHandleCrash(pExceptionPtrs);
    }
#endif

    // CRT new operator fault handler
    int __cdecl WindowsCrashHandler::NewHandler(size_t) {
        // 'new' operator memory allocation exception

        // Retrieve exception information
        EXCEPTION_POINTERS* pExceptionPtrs = NULL;
        GetExceptionPointers(CR_CPP_NEW_OPERATOR_ERROR, &pExceptionPtrs);

        DoHandleCrash(pExceptionPtrs);

        // Unreacheable code
        return 0;
    }

    // CRT SIGABRT signal handler
    void WindowsCrashHandler::SigabrtHandler(int) {
        // Caught SIGABRT C++ signal

        // Retrieve exception information
        EXCEPTION_POINTERS* pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
        if (pExceptionPtrs == nullptr) {
            GetExceptionPointers(CR_CPP_SIGABRT, &pExceptionPtrs);
        }

        DoHandleCrash(pExceptionPtrs);
    }

    // CRT SIGFPE signal handler
    void WindowsCrashHandler::SigfpeHandler(int /*code*/, int subcode) {
        // Floating point exception (SIGFPE)
        (void)subcode;

        EXCEPTION_POINTERS* pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;

        DoHandleCrash(pExceptionPtrs);
    }

    // CRT sigill signal handler
    void WindowsCrashHandler::SigillHandler(int) {
        // Illegal instruction (SIGILL)

        // Retrieve exception information
        EXCEPTION_POINTERS* pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
        if (pExceptionPtrs == nullptr) {
            GetExceptionPointers(CR_CPP_SIGILL, &pExceptionPtrs);
        }

        DoHandleCrash(pExceptionPtrs);
    }

    // CRT sigint signal handler
    void WindowsCrashHandler::SigintHandler(int) {
        // Interruption (SIGINT)

        // Retrieve exception information
        EXCEPTION_POINTERS* pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
        if (pExceptionPtrs == nullptr) {
            GetExceptionPointers(CR_CPP_SIGINT, &pExceptionPtrs);
        }

        DoHandleCrash(pExceptionPtrs);
    }

    // CRT SIGSEGV signal handler
    void WindowsCrashHandler::SigsegvHandler(int) {
        // Invalid storage access (SIGSEGV)

        PEXCEPTION_POINTERS pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
        if (pExceptionPtrs == nullptr) {
            GetExceptionPointers(CR_CPP_SIGSEGV, &pExceptionPtrs);
        }

        DoHandleCrash(pExceptionPtrs);
    }

    // CRT SIGTERM signal handler
    void WindowsCrashHandler::SigtermHandler(int) {
        // Termination request (SIGTERM)

        // Retrieve exception information
        EXCEPTION_POINTERS* pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
        if (pExceptionPtrs == nullptr) {
            GetExceptionPointers(CR_CPP_SIGTERM, &pExceptionPtrs);
        }

        DoHandleCrash(pExceptionPtrs);
    }
}
