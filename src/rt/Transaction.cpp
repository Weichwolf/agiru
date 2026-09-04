#include "runtime/Transaction.h"

#include "runtime/Database.h"
#include "runtime/Session.h"

#include <cstddef>
#include <string>
#include <utility>

namespace agiru {

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
  connection.Run("RELEASE SAVEPOINT " + names_[depth - 1]);
  names_.resize(depth - 1);
}

void Boundaries::Rollback(const Connection &connection, std::size_t depth) {
  if (depth == 0 || depth > names_.size()) { return; }
  connection.Run("ROLLBACK TO SAVEPOINT " + names_[depth - 1]);
  connection.Run("RELEASE SAVEPOINT " + names_[depth - 1]);
  names_.resize(depth - 1);
}

void Boundaries::Commit(const Connection &connection) {
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

}

namespace agiru {

namespace detail {

void RememberError(std::string_view text) {
  Session::Current().Transaction().SetLastError(std::string(text));
}

}

std::string GetLastErrorText() {
  return std::string(Session::Current().Transaction().LastError());
}

void ClearLastError() {
  Session::Current().Transaction().ClearLastError();
}

void Commit() {
  Session::Current().Transaction().Commit(Session::Current().Database());
}

}
