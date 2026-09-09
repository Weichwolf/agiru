#include "runtime/Transaction.h"

#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Scopes.h"
#include "runtime/Session.h"
#include "type/CommitBehavior.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <optional>
#include <print>
#include <string>
#include <utility>

#include <execinfo.h>
#include <unistd.h>

namespace agiru {

namespace {
constexpr int kTraceFrames = 24;
}

namespace {

std::string NextName(std::size_t issued) {
  return "al_" + std::to_string(issued);
}

}

std::size_t Boundaries::Open(const Connection &connection) {
  if (!connection.InTransaction()) { connection.Run("BEGIN"); }
  ++issued_;
  std::string name = NextName(issued_);
  connection.Run("SAVEPOINT " + name);
  names_.push_back(std::move(name));
  return names_.size();
}

void Boundaries::Release(const Connection &connection, std::size_t depth) {
  if (depth == 0 || depth > names_.size()) { return; }
  try {
    connection.Run("RELEASE SAVEPOINT " + names_[depth - 1]);
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
  connection.Run("ROLLBACK TO SAVEPOINT " + names_[depth - 1]);
  connection.Run("RELEASE SAVEPOINT " + names_[depth - 1]);
  names_.resize(depth - 1);
  inconsistent_.clear();
}

void Boundaries::MarkConsistent(std::string_view table, bool consistent) {
  const auto held = std::ranges::find(inconsistent_, table);
  if (consistent && held != inconsistent_.end()) { inconsistent_.erase(held); }
  if (!consistent && held == inconsistent_.end()) { inconsistent_.emplace_back(table); }
}

void Boundaries::Commit(const Connection &connection) {
  if (!inconsistent_.empty()) {
    throw Error(
        "The transaction cannot be completed because it will cause inconsistencies in the " +
        inconsistent_.front() +
        " table. Check where and how the CONSISTENT function is used in the transaction "
        "to find the error.");
  }
  for (std::size_t i = names_.size(); i > 0; --i) {
    connection.Run("RELEASE SAVEPOINT " + names_[i - 1]);
  }
  for (std::string &name : names_) {
    ++issued_;
    name = NextName(issued_);
    connection.Run("SAVEPOINT " + name);
  }
}

}

namespace agiru::detail {

Scope::Scope() : depth_(Session::Current().Transaction().Open(Session::Current().Database())) {}

Scope::~Scope() {
  if (open_) { Session::Current().Transaction().Rollback(Session::Current().Database(), depth_); }
}

void Scope::Keep() {
  if (!open_) { return; }
  open_ = false;
  Session::Current().Transaction().Release(Session::Current().Database(), depth_);
}

void Scope::Discard(std::string_view why) {
  if (!open_) { return; }
  open_ = false;
  Session::Current().Transaction().SetLastError(std::string(why));
  Session::Current().Transaction().Rollback(Session::Current().Database(), depth_);
}

void Scope::Discard(const Error &error) {
  if (!open_) { return; }
  open_ = false;
  Session::Current().Transaction().SetLastError(error.what(), std::string(error.Code()));
  Session::Current().Transaction().Rollback(Session::Current().Database(), depth_);
}

}

namespace agiru {

namespace detail {

void RememberError(std::string_view text) {
  Session::Current().Transaction().SetLastError(std::string(text));
}

void RememberError(const Error &error) {
  static const bool traced = std::getenv("AGIRU_TRACE_ERRORS") != nullptr;
  if (traced) {
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
  if (const std::optional<CommitBehavior> standing = CommitScope::Standing()) {
    if (*standing == CommitBehavior::Ignore) { return; }
    throw Error("A Commit inside a [CommitBehavior(CommitBehavior::Error)] scope is refused");
  }
  Session::Current().Transaction().Commit(Session::Current().Database());
}

}
