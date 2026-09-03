#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/TestRunner.h"

#include <algorithm>
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
  std::string database;
  bool list = false;
};

constexpr int kUsage = 2;

// THE CONNECTION STRING IS A BUILD SETTING AND NOT AN ENVIRONMENT VARIABLE, for the reason the gate
// gives: a run that reads its target from the shell passes or fails for reasons the tree cannot
// see.
// `--database` overrides it, because a client is allowed to be told where to connect.
constexpr std::string_view kDatabase = AGIRU_DATABASE;

void Usage() {
  std::println("agiru -- Business Central, translated to C++");
  std::println("");
  std::println("  agiru run-tests [--suite <name>] [--codeunit <name>] [--list]");
  std::println("      Run the transpiled [Test] procedures through the AL test runner.");
  std::println("      With no filter, the whole installed test population.");
  std::println("      --list says which test codeunits this binary carries.");
  std::println("      --database <url> connects somewhere other than the built-in default.");
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
    } else if (argument == "--database") {
      options.database = ValueOf(arguments, at);
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
/// \warning A RUN THAT FINDS NOTHING IS AN ABORT, NOT A PASS. Zero registered test codeunits means
///          the apps were not linked in, and "0 of 0 passed" would be the greenest possible lie.
int RunTests(const Options &options) {
  if (!options.suite.empty()) {
    // A SUITE IS A ROW IN `Test Suite`, not a name this binary knows. Guessing that a suite means
    // "everything" would report a pass over a population nobody asked for.
    throw agiru::Error("run-tests cannot select the suite " + options.suite +
                       " yet: a suite is data in the `Test Suite` table and needs the database "
                       "(board:0004). --codeunit works now. See board:0039.");
  }
  const std::vector<const agiru::TestCatalogue *> codeunits = agiru::RegisteredTestCodeunits();
  if (codeunits.empty()) {
    throw agiru::Error("no test codeunit is registered: nothing linked into this binary declares "
                       "`Subtype = Test`. See board:0038.");
  }
  if (options.list) {
    for (const agiru::TestCatalogue *codeunit : codeunits) {
      std::println("{:>7}  {}  ({} test(s))",
                   codeunit->Id().Value(),
                   codeunit->Name(),
                   codeunit->Methods().size());
    }
    return 0;
  }
  if (!options.codeunit.empty()) {
    const bool known = std::ranges::any_of(codeunits, [&](const agiru::TestCatalogue *codeunit) {
      return codeunit->Name() == options.codeunit;
    });
    if (!known) {
      throw agiru::Error("no test codeunit is called " + options.codeunit +
                         "; `run-tests --list` says which are registered");
    }
  }
  // A TEST METHOD IS A TRANSACTION, so the runner needs a session before it runs one -- and it
  // opens exactly one for the whole run, because a session is what holds the boundary stack.
  const agiru::Session session(options.database.empty() ? std::string(kDatabase)
                                                        : options.database);
  const agiru::TestRun run = agiru::RunRegisteredTests(options.codeunit);
  for (const agiru::TestResult &result : run.results) {
    if (result.passed) { continue; }
    std::println("FAIL  {}  {}\n      {}", result.codeunit, result.method, result.error);
  }
  std::println("{} of {} passed", run.passed, run.passed + run.failed);
  return run.failed == 0 ? 0 : 1;
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
