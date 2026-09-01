#pragma once

#include <cstdio>
#include <string_view>

/// The gate, in fifty lines. No off-the-shelf framework: a dependency that only supplies macros for
/// `assert` is one too many -- and what the compiler can decide is a `static_assert` and not a case
/// at all.
namespace gate {

inline int failures = 0;
inline int checks = 0;

inline void Report(bool ok,
                   std::string_view claim,
                   std::string_view got,
                   std::string_view want,
                   const char *file,
                   int line) {
  ++checks;
  if (ok) { return; }
  ++failures;
  std::printf("FAIL  %s:%d  %.*s\n      is   %.*s\n      want %.*s\n",
              file,
              line,
              static_cast<int>(claim.size()),
              claim.data(),
              static_cast<int>(got.size()),
              got.data(),
              static_cast<int>(want.size()),
              want.data());
}

inline int Done(const char *suite) {
  std::printf("%s: %d check(s), %d red\n", suite, checks, failures);
  return failures == 0 ? 0 : 1;
}

} // namespace gate

/// A case states what it CLAIMS, not merely what it compares -- a red line without a claim forces
/// the reader into the source.
#define CHECK_TEXT(claim, got, want)                                                               \
  gate::Report((got) == (want), (claim), (got), (want), __FILE__, __LINE__)

#define CHECK_TRUE(claim, cond)                                                                    \
  gate::Report((cond), (claim), (cond) ? "true" : "false", "true", __FILE__, __LINE__)
