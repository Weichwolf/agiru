#include "runtime/Error.h"

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <print>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct Options {
  std::string_view command;
  std::string suite;
  std::string codeunit;
  bool list = false;
};

constexpr int kUsage = 2;

void Usage() {
  std::println("agiru -- Business Central, translated to C++");
  std::println("");
  std::println("  agiru run-tests [--suite <name>] [--codeunit <name>]");
  std::println("      Run the transpiled [Test] procedures through the AL test runner.");
  std::println("      With no filter, the whole installed test population.");
  std::println("");
  std::println("  agiru version");
  std::println("      What this binary is.");
}

std::string ValueOf(std::span<const std::string_view> arguments, std::size_t &at) {
  if (at + 1 >= arguments.size()) {
    throw agiru::Error("the option " + std::string(arguments[at]) + " wants a value");
  }
  ++at;
  return std::string(arguments[at]);
}

Options Read(std::span<const std::string_view> arguments) {
  Options options;
  if (arguments.empty()) { return options; }
  options.command = arguments.front();
  for (std::size_t at = 1; at < arguments.size(); ++at) {
    const std::string_view argument = arguments[at];
    if (argument == "--suite") {
      options.suite = ValueOf(arguments, at);
    } else if (argument == "--codeunit") {
      options.codeunit = ValueOf(arguments, at);
    } else if (argument == "--list") {
      options.list = true;
    } else {
      throw agiru::Error("unknown option " + std::string(argument));
    }
  }
  return options;
}

/// AL's own test runner walks the `[Test]` procedures; this is the door in front of it, the way
/// BC's `Run-TestsInBcContainer` is a door in front of `Invoke-NavCodeunit` (board:0039).
///
/// \warning REFUSED, AND IT NAMES WHAT IS MISSING RATHER THAN REPORTING ZERO TESTS. A runner needs
///          `Codeunit.Run` as a transaction boundary (board:0040), the error builtins that
///          `asserterror` reads, and a database with the CRONUS data behind it (board:0004). A run
///          that found nothing and said "0 of 0 passed" would be green for the worst possible
///          reason.
int RunTests(const Options &options) {
  std::string what = "the whole installed test population";
  if (!options.suite.empty()) { what = "the suite " + options.suite; }
  if (!options.codeunit.empty()) { what = "the codeunit " + options.codeunit; }
  throw agiru::Error("run-tests cannot run " + what +
                     " yet: the AL test runner needs Codeunit.Run as a transaction boundary "
                     "(board:0040), the error builtins asserterror reads, and a database "
                     "(board:0004). See board:0039.");
}

int Version() {
  std::println("agiru -- an AL-to-C++ transpiler and runtime for Business Central");
  return 0;
}

} // namespace

/// A FAILURE IS LOUD AND IT IS ALSO A RETURN CODE. The handler prints and returns; what the checker
/// sees is that PRINTING can itself throw, which would leave `main` -- so the handler is wrapped in
/// one of its own, and the last resort says nothing and returns 1.
int main(int argc, char **argv) {
  try {
    std::vector<std::string_view> arguments;
    arguments.reserve(static_cast<std::size_t>(argc > 1 ? argc - 1 : 0));
    for (int i = 1; i < argc; ++i) { arguments.emplace_back(argv[i]); }
    const Options options = Read(arguments);
    if (options.command.empty() || options.command == "help" || options.command == "--help") {
      Usage();
      return options.command.empty() ? kUsage : 0;
    }
    if (options.command == "run-tests") { return RunTests(options); }
    if (options.command == "version") { return Version(); }
    std::println(stderr, "agiru: no such command: {}", options.command);
    Usage();
    return kUsage;
  } catch (const std::exception &e) {
    try {
      std::println(stderr, "agiru: {}", e.what());
    } catch (...) {
      // Reporting itself failed; the return code is the report.
      return 1;
    }
    return 1;
  } catch (...) { return 1; }
}
