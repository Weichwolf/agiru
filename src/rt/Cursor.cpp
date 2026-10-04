#include "Cursor.h"

#include "runtime/Database.h"
#include "runtime/Session.h"
#include "runtime/Transaction.h"

#include <atomic>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::detail {

namespace {

std::string NextName() {
  static std::atomic<unsigned long long> counter{0};
  return "agiru_" + std::to_string(counter.fetch_add(1, std::memory_order_relaxed));
}

}

Cursor::Cursor(const Connection &connection,
               const std::string &select,
               std::vector<std::optional<std::string>> binds)
    : connection_(&connection),
      name_(NextName()),
      epoch_(Session::HasCurrent() ? Session::Current().Transaction().CursorEpoch() : 0),
      block_(nullptr) {
  connection_->Run("DECLARE " + name_ + " NO SCROLL CURSOR FOR " + select, binds);
}

Cursor::~Cursor() {
  if (!Session::HasCurrent() || &Session::Current().Database() != connection_) { return; }
  if (!connection_->InTransaction() || connection_->InFailedTransaction()) { return; }
  try {
    if (Session::Current().Transaction().CursorEpoch() != epoch_) {
      const std::vector<std::optional<std::string>> bindings{name_};
      if (connection_->Execute("SELECT 1 FROM pg_catalog.pg_cursors WHERE name = $1", bindings)
              .Rows() == 0) {
        return;
      }
    }
    connection_->Run("CLOSE " + name_);
  } catch (const std::exception &e) {
    std::fputs("agiru: the cursor ", stderr);
    std::fputs(name_.c_str(), stderr);
    std::fputs(" stayed open: ", stderr);
    std::fputs(e.what(), stderr);
    std::fputc('\n', stderr);
  }
}

bool Cursor::Current() const {
  return Session::HasCurrent() && &Session::Current().Database() == connection_ &&
         Session::Current().Transaction().CursorEpoch() == epoch_ && connection_->InTransaction();
}

bool Cursor::Fetch() {
  Result next =
      connection_->Execute("FETCH FORWARD " + std::to_string(kFetchBlock) + " FROM " + name_);
  spent_ = next.Rows() == 0;
  if (spent_) { return false; }
  block_ = std::move(next);
  row_ = 0;
  return true;
}

bool Cursor::Step() {
  if (spent_) { return false; }
  if (block_.Rows() == 0) { return Fetch(); }
  if (row_ + 1 < block_.Rows()) {
    ++row_;
    return true;
  }
  return Fetch();
}

std::optional<std::string_view> Cursor::Value(std::size_t column) const {
  return block_.Value(row_, column);
}

}
