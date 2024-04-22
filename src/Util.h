#pragma once

#include <source_location>
#include <stdexcept>

#include "ISA.h"

namespace remu {
inline void ThrowRuntimeError(
    const std::string& msg,
    const std::source_location location = std::source_location::current()) {
    throw std::runtime_error(std::string(location.file_name()) + ":" +
                             std::to_string(location.line()) + ": " + msg);
}

inline void Assert(bool cond, const std::source_location location =
                                  std::source_location::current()) {
    if (!cond) {
        ThrowRuntimeError("Assertion Failed", location);
    }
}

inline void InvalidInstruction(
    Word_t inst, Word_t addr,
    const std::source_location location = std::source_location::current()) {
    char buf[64];
    std::sprintf(buf, "0x%08x: 0x%08x", addr, inst);
    ThrowRuntimeError(buf, location);
}
// #include <cstdio>
// template <typename... T>
// inline std::string stringFormat(const char* format, T... args) {
//     constexpr int bufSize = 256;
//     char buf[bufSize];
//     std::snprintf(buf, bufSize, format, args...);
//     return buf;
// }
}  // namespace remu