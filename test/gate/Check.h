#pragma once

#include <cstdio>
#include <exception>
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

/// Runs a suite and reports it.
///
/// EVERY GATE MAIN NEEDS THE SAME TWO HANDLERS, because `bugprone-exception-escape` will not let an
/// exception leave `main` and because a handler that itself throws is no handler. Written once here
/// rather than copied into every case -- a copied `catch (...)` is a silent place per file, and the
/// baseline counts those for a reason.
template <typename Body> int Run(const char *suite, Body body) {
  try {
    body();
  } catch (const std::exception &e) {
    std::fputs("FAIL  an exception left ", stderr);
    std::fputs(suite, stderr);
    std::fputs("\n      ", stderr);
    std::fputs(e.what(), stderr);
    std::fputs("\n", stderr);
    return 1;
  } catch (...) {
    std::fputs("FAIL  an unknown exception left the gate\n", stderr);
    return 1;
  }
  return Done(suite);
}

} // namespace gate

/// A case states what it CLAIMS, not merely what it compares -- a red line without a claim forces
/// the reader into the source.
#define CHECK_TEXT(claim, got, want)                                                               \
  gate::Report((got) == (want), (claim), (got), (want), __FILE__, __LINE__)

#define CHECK_TRUE(claim, cond)                                                                    \
  gate::Report((cond), (claim), (cond) ? "true" : "false", "true", __FILE__, __LINE__)

/// For a claim that something raises NOTHING. It keeps the message in the failure output, which a
/// plain truth check would throw away exactly when it is needed.
#define CHECK_SILENT(claim, got)                                                                   \
  gate::Report((got).empty(), (claim), (got), "no error", __FILE__, __LINE__)
