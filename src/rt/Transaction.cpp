#include "runtime/Transaction.h"

#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/ProcessDiagnostics.h"
#include "runtime/Scopes.h"
#include "runtime/Session.h"
#include "type/CommitBehavior.h"
#include "type/TransactionType.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <execinfo.h>
#include <unistd.h>

namespace agiru {

namespace {
constexpr int kTraceFrames = 24;
constexpr std::string_view kFailedCommit =
    "the transaction cannot be committed because a statement inside it failed";
constexpr std::string_view kLostIsolation =
    "the test isolation transaction was lost; execution cannot continue inside this runner";
}

namespace {

std::string NextName(std::size_t issued) {
  return "al_" + std::to_string(issued);
}

}

std::size_t Boundaries::Open(const Connection &connection) {
  if (isolationFloor_ > names_.size()) { throw Error(kLostIsolation); }
  ++issued_;
  Boundary next{.name = NextName(issued_), .inconsistentBefore = inconsistent_};
  names_.reserve(names_.size() + 1);
  if (!connection.InTransaction()) { connection.Run("BEGIN"); }
  connection.Run("SAVEPOINT " + next.name);
  names_.push_back(std::move(next));
  return names_.size();
}

void Boundaries::ClearCommand() noexcept {
  names_.clear();
  inconsistent_.clear();
  isolationFloor_ = 0;
  autoRollbackTest_ = false;
  type_ = TransactionType::UpdateNoLocks;
  ++cursorEpoch_;
}

void Boundaries::Release(const Connection &connection, std::size_t depth) {
  if (depth == 0 || depth > names_.size()) { return; }
  try {
    connection.Run("RELEASE SAVEPOINT " + names_[depth - 1].name);
  } catch (const DatabaseError &refused) {
    Rollback(connection, depth);
    throw Error(std::string("the transaction cannot be kept, a statement inside it failed and "
                            "the failure was swallowed: ") +
                refused.what());
  }
  names_.resize(depth - 1);
}

void Boundaries::Rollback(const Connection &connection, std::size_t depth) {
  if (depth == 0 || depth > names_.size()) { return; }
  connection.Run("ROLLBACK TO SAVEPOINT " + names_[depth - 1].name);
  connection.Run("RELEASE SAVEPOINT " + names_[depth - 1].name);
  std::vector<std::string> restored = std::move(names_[depth - 1].inconsistentBefore);
  names_.resize(depth - 1);
  inconsistent_.swap(restored);
  ++cursorEpoch_;
}

void Boundaries::MarkConsistent(std::string_view table, bool consistent) {
  const auto held = std::ranges::find(inconsistent_, table);
  if (consistent && held != inconsistent_.end()) { inconsistent_.erase(held); }
  if (!consistent && held == inconsistent_.end()) { inconsistent_.emplace_back(table); }
}

void Boundaries::Commit(const Connection &connection) {
  if (isolationFloor_ > names_.size()) { throw Error(kLostIsolation); }
  if (connection.InFailedTransaction()) { throw Error(kFailedCommit); }
  if (!inconsistent_.empty()) {
    throw Error(
        "The transaction cannot be completed because it will cause inconsistencies in the " +
        inconsistent_.front() +
        " table. Check where and how the CONSISTENT function is used in the transaction "
        "to find the error.");
  }
  std::vector<Boundary> renewed;
  renewed.reserve(names_.size() - isolationFloor_);
  for (std::size_t i = isolationFloor_; i < names_.size(); ++i) {
    ++issued_;
    renewed.push_back(Boundary{.name = NextName(issued_), .inconsistentBefore = inconsistent_});
  }
  try {
    if (isolationFloor_ == 0) {
      if (connection.InTransaction()) {
        connection.Run("COMMIT");
        ++cursorEpoch_;
      }
      names_.clear();
      if (!renewed.empty()) { connection.Run("BEGIN"); }
    } else if (!renewed.empty()) {
      connection.Run("RELEASE SAVEPOINT " + names_[isolationFloor_].name);
      names_.resize(isolationFloor_);
    }
    for (Boundary &boundary : renewed) {
      connection.Run("SAVEPOINT " + boundary.name);
      names_.push_back(std::move(boundary));
    }
  } catch (const DatabaseError &) {
    if (!connection.InTransaction()) {
      names_.clear();
      inconsistent_.clear();
      ++cursorEpoch_;
    }
    throw;
  }
}

}

namespace agiru::detail {

Scope::Scope() : depth_(Session::Current().Transaction().Open(Session::Current().Database())) {}

Scope::~Scope() noexcept(false) {
  if (!open_) { return; }
  const auto rollback = [this] {
    Session::Current().Transaction().Rollback(Session::Current().Database(), depth_);
  };
  if (std::uncaught_exceptions() == 0) {
    rollback();
    return;
  }
  try {
    rollback();
  } catch (const std::exception &error) {
    std::fputs("agiru: scope rollback failed while unwinding: ", stderr);
    std::fputs(error.what(), stderr);
    std::fputc('\n', stderr);
  }
}

void Scope::Keep() {
  if (!open_) { return; }
  open_ = false;
  Session::Current().Transaction().Release(Session::Current().Database(), depth_);
}

void Scope::Discard(std::string_view why) {
  if (!open_) { return; }
  Session::Current().Transaction().SetLastError(std::string(why));
  open_ = false;
  Session::Current().Transaction().Rollback(Session::Current().Database(), depth_);
}

void Scope::Discard(const Error &error) {
  if (!open_) { return; }
  Session::Current().Transaction().SetLastError(error.what(), std::string(error.Code()));
  open_ = false;
  Session::Current().Transaction().Rollback(Session::Current().Database(), depth_);
}

IsolationFloor::IsolationFloor(std::size_t depth)
    : previous_(Session::Current().Transaction().IsolationFloor(depth)) {}

IsolationFloor::~IsolationFloor() {
  Session::Current().Transaction().IsolationFloor(previous_);
}

AutoRollbackTest::AutoRollbackTest(bool active)
    : previous_(Session::Current().Transaction().AutoRollbackTest(active)) {}

AutoRollbackTest::~AutoRollbackTest() {
  Session::Current().Transaction().AutoRollbackTest(previous_);
}

}

namespace agiru {

namespace detail {

void RememberError(std::string_view text) {
  Session::Current().Transaction().SetLastError(std::string(text));
}

void RememberError(const Error &error) {
  if (TraceErrors()) {
    std::println(stderr, "caught: {}", error.what());
    std::array<void *, kTraceFrames> frames{};
    const int depth = backtrace(frames.data(), static_cast<int>(frames.size()));
    backtrace_symbols_fd(frames.data(), depth, STDERR_FILENO);
  }
  Session::Current().Transaction().SetLastError(error.what(), std::string(error.Code()));
}

}

std::string GetLastErrorText() {
  return std::string(Session::Current().Transaction().LastError());
}

void ClearLastError() {
  Session::Current().Transaction().ClearLastError();
}

void Commit() {
  if (Session::Current().Transaction().IsAutoRollbackTest()) {
    throw Error("A Commit cannot be called from a test method with TransactionModel AutoRollback");
  }
  if (const std::optional<CommitBehavior> standing = CommitScope::Standing()) {
    if (*standing == CommitBehavior::Ignore) { return; }
    throw Error("A Commit inside a [CommitBehavior(CommitBehavior::Error)] scope is refused");
  }
  Session::Current().Transaction().Commit(Session::Current().Database());
}

}
