#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/TestRunner.h"
#include "runtime/test/RunnerDatabase.h"

#include <algorithm>
#include <array>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#if __has_include(<execinfo.h>) && !defined(__MINGW32__)
#include <execinfo.h>
#endif
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
  std::string scratch = "agiru_test_0";
  bool fresh = false;
  bool list = false;
  bool isolate = false;
  std::string self;
};

constexpr int kUsage = 2;
constexpr std::size_t kLineBytes = 4096;

constexpr std::string_view kDatabase = AGIRU_DATABASE;

void Usage() {
  std::println("agiru -- Business Central, translated to C++");
  std::println("");
  std::println("  agiru run-tests [--suite <name>] [--codeunit <name>] [--scratch <db>]\n          "
               "       [--database <dsn>] [--fresh] [--list]");
  std::println("      Run the transpiled [Test] procedures through the AL test runner.");
  std::println("      With no filter, the whole installed test population.");
  std::println("      --list says which test codeunits this binary carries.");
  std::println("      --isolate runs every codeunit in its own process, so one that dies takes");
  std::println("      only itself: a segmentation fault is not an exception and nothing in the");
  std::println("      process can catch it (board:0612).");
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
    } else if (argument == "--scratch") {
      options.scratch = ValueOf(arguments, at);
    } else if (argument == "--fresh") {
      options.fresh = true;
    } else if (argument == "--list") {
      options.list = true;
    } else if (argument == "--isolate") {
      options.isolate = true;
    } else {
      throw agiru::Error("unknown option " + std::string(argument));
    }
  }
  return options;
}

std::string Quoted(std::string_view text) {
  std::string out = "'";
  for (const char c : text) {
    if (c == '\'') {
      out += "'\\''";
      continue;
    }
    out += c;
  }
  out += "'";
  return out;
}

int RunIsolated(const Options &options, std::span<const agiru::TestCatalogue *const> codeunits) {
  std::size_t passed = 0;
  std::size_t failed = 0;
  std::size_t died = 0;
  for (const agiru::TestCatalogue *codeunit : codeunits) {
    std::string command = Quoted(options.self) + " run-tests --codeunit " +
                          Quoted(codeunit->Name()) + " --scratch " + Quoted(options.scratch);
    if (!options.database.empty()) { command += " --database " + Quoted(options.database); }
    command += " 2>&1";
    std::FILE *child = popen(command.c_str(), "r");
    if (child == nullptr) {
      std::println("FAIL  {}  <no process>", codeunit->Name());
      failed += codeunit->Methods().size();
      continue;
    }
    std::string tail;
    std::array<char, kLineBytes> line{};
    while (std::fgets(line.data(), static_cast<int>(line.size()), child) != nullptr) {
      const std::string_view read(line.data());
      std::print("{}", read);
      if (read.find(" passed") != std::string_view::npos) { tail = read; }
    }
    const int status = pclose(child);
    std::fflush(stdout);
    std::size_t ran = 0;
    std::size_t of = 0;
    if (!tail.empty() && std::sscanf(tail.c_str(), "%zu of %zu", &ran, &of) == 2) {
      passed += ran;
      failed += of - ran;
      continue;
    }
    ++died;
    failed += codeunit->Methods().size();
    std::println("FAIL  {}  <the process died, status {}>", codeunit->Name(), status);
    std::fflush(stdout);
  }
  std::println("{} of {} passed, {} codeunit(s) died", passed, passed + failed, died);
  return failed == 0 ? 0 : 1;
}

int RunTests(const Options &options) {
  if (!options.suite.empty()) {
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
  if (options.isolate && options.codeunit.empty()) { return RunIsolated(options, codeunits); }
  const std::string master = options.database.empty() ? std::string(kDatabase) : options.database;
  const agiru::RunnerDatabase runner(master, options.scratch, options.fresh);
  const agiru::Session session(runner.Dsn());
  agiru::ProvisionInstalled(session.Database());
  const agiru::TestRun run =
      agiru::RunRegisteredTests(options.codeunit, [](const agiru::TestResult &result) {
        if (result.passed) { return; }
        std::println("FAIL  {}  {}\n      {}", result.codeunit, result.method, result.error);
        std::fflush(stdout);
      });
  std::println("{} of {} passed", run.passed, run.passed + run.failed);
  return run.failed == 0 ? 0 : 1;
}

int Version() {
  std::println("agiru -- an AL-to-C++ transpiler and runtime for Business Central");
  return 0;
}

}

namespace {

#if __has_include(<execinfo.h>) && !defined(__MINGW32__)
constexpr int kFrames = 64;
constexpr int kSignalBase = 128;
constexpr std::size_t kAltStack = 1U << 18U;

extern "C" void Fell(int signal) {
  std::array<void *, kFrames> frames{};
  const int depth = backtrace(frames.data(), kFrames);
  std::println(stderr, "agiru: signal {} -- {} frame(s) follow", signal, depth);
  std::fflush(stderr);
  backtrace_symbols_fd(frames.data(), depth, 2);
  std::_Exit(signal + kSignalBase);
}

void WatchForFalls() {
  static std::array<char, kAltStack> spare{};
  stack_t alt{};
  alt.ss_sp = spare.data();
  alt.ss_size = spare.size();
  alt.ss_flags = 0;
  static_cast<void>(sigaltstack(&alt, nullptr));
  struct sigaction how{};
  how.sa_handler = Fell;
  how.sa_flags = SA_ONSTACK | SA_RESETHAND;
  sigemptyset(&how.sa_mask);
  for (const int signal : {SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGABRT}) {
    static_cast<void>(sigaction(signal, &how, nullptr));
  }
}
#else
void WatchForFalls() {}
#endif

}

int main(int argc, char **argv) {
  WatchForFalls();
  try {
    std::vector<std::string_view> arguments;
    arguments.reserve(static_cast<std::size_t>(argc > 1 ? argc - 1 : 0));
    for (int i = 1; i < argc; ++i) { arguments.emplace_back(argv[i]); }
    Options options = Read(arguments);
    options.self = argv[0] == nullptr ? "agiru" : argv[0];
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
    } catch (...) { return 1; }
    return 1;
  } catch (...) { return 1; }
}
