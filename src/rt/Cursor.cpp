#include "Cursor.h"

#include "runtime/Database.h"
#include "runtime/Error.h"

#include <atomic>
#include <string>

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
    : connection_(&connection), name_(NextName()), block_(nullptr) {
  connection_->Run("DECLARE " + name_ + " NO SCROLL CURSOR FOR " + select, binds);
}

Cursor::~Cursor() {
  try {
    connection_->Run("CLOSE " + name_);
  } catch (const Error &) {
  }
}

bool Cursor::Fetch() {
  block_ = connection_->Execute("FETCH FORWARD " + std::to_string(kFetchBlock) + " FROM " + name_);
  row_ = 0;
  spent_ = block_.Rows() == 0;
  return !spent_;
}

bool Cursor::Step() {
  if (spent_) { return false; }
  if (block_.Rows() == 0) { return Fetch(); }
  ++row_;
  if (row_ < block_.Rows()) { return true; }
  return Fetch();
}

std::optional<std::string_view> Cursor::Value(std::size_t column) const {
  return block_.Value(row_, column);
}

}
