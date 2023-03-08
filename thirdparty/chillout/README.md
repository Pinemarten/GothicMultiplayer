# chillout

[![Build status](https://ci.appveyor.com/api/projects/status/b2mfobskucq936hc/branch/master?svg=true)](https://ci.appveyor.com/project/Ribtoks/chillout/branch/master)
[![Build Status](https://travis-ci.org/ribtoks/chillout.svg?branch=master)](https://travis-ci.org/ribtoks/chillout)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/986469b7d3b54cdca5349193ec0210ca)](https://www.codacy.com/app/ribtoks/chillout)

![os windows](https://img.shields.io/badge/OS-Windows-yellowgreen.svg)
![os mac](https://img.shields.io/badge/OS-macOS-yellowgreen.svg)
![os linux](https://img.shields.io/badge/OS-Linux-yellowgreen.svg)

![license](https://img.shields.io/badge/license-MIT-blue.svg)
![copyright](https://img.shields.io/badge/%C2%A9-Taras_Kushnir-blue.svg)
![language](https://img.shields.io/badge/language-c++-blue.svg)
![c++](https://img.shields.io/badge/std-c++11-blue.svg) 

Chillout is a simple cross-platform crash handling library.

This library can be used on Windows, OS X and Linux to catch various unhandled exceptions (access violation, signals, alloc errors etc.), generate backtraces for logs and possibly crash dump (the latter currently only for Windows).

The API is very simple - you provide your callbacks for crash and/or backtracing. Chillout makes sure your handlers will be executed in the event of a crash.

### Basic usage

    auto &chillout = Debug::Chillout::getInstance();
    // install various crash handlers
    chillout.init("my_app_name", "/path/to/crash/or/backtraces/dir");
    
    chillout.setBacktraceCallback([](const char * const stackEntry) {
        fprintf(stderr, "my trace:  %s", stackEntry);
    });
    
    chillout.setCrashCallback([&chillout]() {
        chillout.backtrace();
        chillout.createCrashDump();
    });

### Disclaimer

Yes, ~Breakpad~ Crashpad exists, but sometimes it is too big or too complicated to setup (e.g. for small project). This library is definitely less reliable and featured than age-tested Breakpad/Crashpad, but it is simple, small and covered by tests.

### Limitations

* Linux/Mac - no core dumps and any other type of crash dumps
* stacktrace only of the thread which crashed

### References

This project won't be possible without:

* [CrashRpt](http://crashrpt.sourceforge.net/) - most full collection of information about Windows crash handlers
* [DeathHandler](https://github.com/vmarkovtsev/DeathHandler) - ideas to preallocate buffer in case of broken heap on 'nix systems
* [gist by @fmela](https://gist.github.com/fmela/591333) - idea to use `dladdr()` instead of parsing raw/mangled line myself
* [stacktrace in Chromium](https://chromium.googlesource.com/chromium/src/base/+/master/debug/stack_trace_posix.cc) - just useful information how demangling works in Chromium
