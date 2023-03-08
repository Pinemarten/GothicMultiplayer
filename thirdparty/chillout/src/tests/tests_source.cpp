#include "tests_source.h"
#include <cstdio>
#include <csignal>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#else
#include <stdexcept>
#endif

Base::~Base()
{
    m_pDerived -> function();
}

struct TestStruct {
    int m_Field;
};

void AccessViolation() {
    // Access violation
    std::shared_ptr<TestStruct> p;
//     int *p = nullptr;
#pragma warning(disable : 6011)   // warning C6011: Dereferencing NULL pointer 'p'
    //*p = 0;
    p->m_Field = 123;
#pragma warning(default : 6011)
}

void InvalidParameter() {
    char* formatString;
    // Call printf_s with invalid parameters.
    formatString = NULL;
#pragma warning(disable : 6387)   // warning C6387: 'argument 1' might be '0': this does not adhere to the specification for the function 'printf'
    printf(formatString);
#pragma warning(default : 6387)  
}

void RaiseSigill() {
    raise(SIGILL);
}

void RaiseSigsegv() {
    raise(SIGSEGV);
}

void RaiseSigterm() {
    raise(SIGTERM);
}

void ThrowException() {
    throw 123;
}

#pragma warning(disable: 4717) // avoid C4717 warning
#define BIG_NUMBER 0x1fffffff

void MemoryOverflow() {
    int *pi = new int[BIG_NUMBER];
    pi[BIG_NUMBER/4] = 123;
    pi[BIG_NUMBER/2] = 456;
    pi[3*BIG_NUMBER/4] = 789;

    MemoryOverflow();
}

#define SMALL_NUMBER 0x1fff
void StackOverflow() {
    int pi[SMALL_NUMBER];
    pi[SMALL_NUMBER/2] = 456;

    StackOverflow();
}

void RaiseSehException() {
#ifdef _WIN32
    RaiseException(42, 0, 0, NULL);
#else
    raise(SIGTERM);
#endif
}
