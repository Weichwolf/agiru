#include "Cursor.h"

#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/Session.h"

#include <atomic>
#include <cstddef>
#include <cstdio>
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
      depth_(Session::HasCurrent() ? Session::Current().Transaction().Depth() : 0),
      epoch_(Session::HasCurrent() ? Session::Current().Transaction().CursorEpoch() : 0),
      block_(nullptr) {
  connection_->Run("DECLARE " + name_ + " NO SCROLL CURSOR FOR " + select, binds);
}

Cursor::~Cursor() {
  if (!Session::HasCurrent() || &Session::Current().Database() != connection_) { return; }
  if (Session::Current().Transaction().Depth() < depth_) { return; }
  if (Session::Current().Transaction().CursorEpoch() != epoch_) { return; }
  if (connection_->InFailedTransaction()) { return; }
  try {
    connection_->Run("CLOSE " + name_);
  } catch (const Error &e) {
    std::fputs(("agiru: the cursor " + name_ + " stayed open: " + e.what() + "\n").c_str(), stderr);
  }
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
