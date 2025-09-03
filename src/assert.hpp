/* TombRaiderLinuxLauncher
 * Martin Bångens Copyright (C) 2025
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef SRC_ASSERT_HPP_
#define SRC_ASSERT_HPP_

#include <QtGlobal>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cxxabi.h>
#include <backtrace.h>
#include <backtrace-supported.h>
#include <signal.h>
#include <sys/resource.h>

// --- libbacktrace helpers ---
static backtrace_state* g_bt_state = nullptr;

static void bt_error_callback(void*, const char* msg, int errnum) {
    fprintf(stderr, "libbacktrace error: %s (err=%d)\n", msg, errnum);
}

static int bt_full_callback(void* data,
                            uintptr_t /*pc*/,
                            const char* filename,
                            int lineno,
                            const char* function) {
    bool* first_frame = static_cast<bool*>(data);

    // Skip our assert wrapper and ?? frames
    if (!filename || !function)
        return 0;
    if (std::string(filename).find("assert.hpp") != std::string::npos)
        return 0;

    const char* pretty = function ? function : "??";
    char* demangled = nullptr;
    int status = 0;
    if (function) {
        demangled = abi::__cxa_demangle(function, nullptr, nullptr, &status);
        if (status == 0 && demangled)
            pretty = demangled;
    }

    fprintf(stderr, "%s %s:%d: %s\n",
            (*first_frame ? "-->" : "   "),
            filename,
            lineno,
            pretty);

    *first_frame = false;
    free(demangled);
    return 0;
}

inline void print_stacktrace() {
#ifdef BACKTRACE_SUPPORTED
    if (!g_bt_state) {
        g_bt_state = backtrace_create_state(nullptr, 1, bt_error_callback, nullptr);
    }
    bool first_frame = true;
    backtrace_full(g_bt_state, 0, bt_full_callback, bt_error_callback, &first_frame);
#else
    fprintf(stderr, "Backtrace not supported on this build.\n");
#endif
}

// --- Qt-safe assert with trace ---
#define Q_ASSERT_WITH_TRACE(cond)                                           \
    do {                                                                    \
        if (!(cond)) {                                                      \
            fprintf(stderr, "ASSERT: \"%s\" in file %s, line %d\n",         \
                    #cond, __FILE__, __LINE__);                             \
            print_stacktrace();                                             \
            fflush(stderr);                                                 \
            struct rlimit rl{};                                             \
            rl.rlim_cur = 0;                                                \
            rl.rlim_max = 0;                                                \
            setrlimit(RLIMIT_CORE, &rl);                                    \
            signal(SIGABRT, SIG_DFL);                                       \
            abort();                                                        \
        }                                                                   \
    } while (0)

#endif  // SRC_ASSERT_HPP_
