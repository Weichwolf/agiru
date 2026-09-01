#include "runtime/Transaction.h"

#include "runtime/Database.h"
#include "runtime/Session.h"

#include <cstddef>
#include <string>
#include <utility>

namespace agiru {

namespace {

// A NAME PER BOUNDARY AND NEVER A REUSED ONE. PostgreSQL allows two savepoints to share a name and
// keeps both, so `ROLLBACK TO SAVEPOINT x` finds the innermost -- which is the right answer for a
// stack and the wrong one the moment Commit() retakes them, since the retaken outer would be
// shadowed by a stale inner. A counter that never goes backwards costs nothing and removes the
// question.
std::string NextName(std::size_t issued) {
  return "al_" + std::to_string(issued);
}

} // namespace

std::size_t Boundaries::Open(const Connection &connection) {
  // THE FIRST BOUNDARY OPENS THE TRANSACTION, because libpq does not. Outside a transaction block
  // every statement commits on its own and `SAVEPOINT` is an error -- a boundary that rolled back
  // nothing would be indistinguishable from one that worked.
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
  // ROLLBACK TO leaves the savepoint standing, so it is released afterwards: the boundary is gone
  // either way, and leaving it would let a later Release() at the same depth find a stale one.
  connection.Run("ROLLBACK TO SAVEPOINT " + names_[depth - 1]);
  connection.Run("RELEASE SAVEPOINT " + names_[depth - 1]);
  names_.resize(depth - 1);
}

void Boundaries::Commit(const Connection &connection) {
  // RELEASED FROM THE INSIDE OUT, THEN RETAKEN FROM THE OUTSIDE IN, all at this point. After this
  // every open boundary rolls back to HERE and no further, which is what makes an AL Commit durable
  // against a later error without leaving the enclosing block with nothing to roll back to.
  for (std::size_t i = names_.size(); i > 0; --i) {
    connection.Run("RELEASE SAVEPOINT " + names_[i - 1]);
  }
  for (std::string &name : names_) {
    ++issued_;
    name = NextName(issued_);
    connection.Run("SAVEPOINT " + name);
  }
}

} // namespace agiru

namespace agiru::detail {

Scope::Scope() : depth_(Session::Current().Transaction().Open(Session::Current().Database())) {}

Scope::~Scope() {
  // A BOUNDARY LEFT BY ANY OTHER ROUTE DISCARDS. An exception that is not an AL Error -- a
  // std::bad_alloc, a defect in the runtime -- must not leave half a write set behind for the next
  // statement to read as if it were whole.
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

} // namespace agiru::detail

namespace agiru {

std::string GetLastErrorText() {
  return std::string(Session::Current().Transaction().LastError());
}

void ClearLastError() {
  Session::Current().Transaction().ClearLastError();
}

void Commit() {
  Session::Current().Transaction().Commit(Session::Current().Database());
}

} // namespace agiru
