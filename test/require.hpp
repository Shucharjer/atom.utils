#pragma once
#include <iostream>
#include <source_location>

inline void report_failure(const char* expr, const std::source_location& location) {
    std::cerr << "REQUIRE failed: " << expr << '\n'
              << "File: " << location.file_name() << '\n'
              << "Function: " << location.function_name() << '\n'
              << "Line: " << location.line() << '\n'
              << "Column: " << location.column() << '\n';
}

#define REQUIRES(expr)                                                                             \
    for (auto flag = true; flag; flag = false) {                                                   \
        if (!(expr)) {                                                                             \
            report_failure(#expr, std::source_location::current());                                \
        }                                                                                          \
    }                                                                                              \
    //

#define REQUIRES_FALSE(expr)                                                                       \
    for (auto flag = true; flag; flag = false) {                                                   \
        if ((expr)) {                                                                              \
            report_failure(#expr, std::source_location::current());                                \
        }                                                                                          \
    }                                                                                              \
    //
