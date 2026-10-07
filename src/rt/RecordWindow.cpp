#include "runtime/RecordWindow.h"

#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordState.h"
#include "runtime/Session.h"
#include "runtime/Table.h"
#include "runtime/TablePermissions.h"

#include "RecordOrder.h"
#include "RecordSeek.h"
#include "Selection.h"
#include "Temporary.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace agiru {
namespace {

[[noreturn]] void Refuse(std::string_view code) {
  throw Error("Record window refused this operation", std::string(code));
}

void RequireSource(const void *record, const TableDef &table) {
  if (record == nullptr) { Refuse("RecordWindowInput"); }
  detail::RequireRecordPermission(record, table, TableOperation::Read);
  if (detail::TempOf(record) != nullptr || table.tableType != TableType::Normal ||
      table.sequenceField.Value() != 0 || table.keys.empty() || table.keys.front().fields.empty()) {
    Refuse("RecordWindowProvider");
  }
}

}

struct RecordWindow::Impl {
  const TableDef *table;
  Result rows;
  std::size_t limit;
  bool backwards;

  [[nodiscard]] std::size_t Size() const { return std::min(limit, rows.Rows()); }
};

RecordWindow::RecordWindow(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}

RecordWindow::~RecordWindow() = default;
RecordWindow::RecordWindow(RecordWindow &&other) noexcept = default;
RecordWindow &RecordWindow::operator=(RecordWindow &&other) noexcept = default;

std::size_t RecordWindow::Size() const {
  if (impl_ == nullptr) { Refuse("RecordWindowState"); }
  return impl_->Size();
}

bool RecordWindow::HasMore() const {
  if (impl_ == nullptr) { Refuse("RecordWindowState"); }
  return impl_->rows.Rows() > impl_->limit;
}

std::size_t RecordWindow::RowsRead() const {
  if (impl_ == nullptr) { Refuse("RecordWindowState"); }
  return impl_->rows.Rows();
}

void RecordWindow::Load(std::size_t index, void *record) const {
  if (index >= Size()) { Refuse("RecordWindowIndex"); }
  const auto &table = *impl_->table;
  RequireSource(record, table);
  const std::size_t row = impl_->backwards ? Size() - 1 - index : index;
  std::size_t column = 0;
  for (const auto &field : table.fields) {
    if (!Stored(field)) { continue; }
    if (!impl_->rows.Value(row, column).has_value()) { Refuse("RecordWindowNull"); }
    ++column;
  }
  auto &state = reinterpret_cast<detail::StateHandle *>(record)->Ensure();
  state.open.Forget();
  state.positioned = false;
  column = 0;
  for (const auto &field : table.fields) {
    if (!Stored(field)) { continue; }
    const auto value = impl_->rows.Value(row, column);
    if (!value.has_value()) { Refuse("RecordWindowNull"); }
    detail::SetFieldText(record, field, *value);
    ++column;
  }
  state.positioned = true;
}

void ValidateRecordWindowLimit(std::size_t limit) {
  if (limit == 0 || limit >= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
    Refuse("RecordWindowLimit");
  }
}

RecordWindow ReadRecordWindow(const void *record,
                              const TableDef &table,
                              RecordWindowPosition position,
                              std::size_t limit) {
  ValidateRecordWindowLimit(limit);
  if (position != RecordWindowPosition::First && position != RecordWindowPosition::Last &&
      position != RecordWindowPosition::After && position != RecordWindowPosition::Before) {
    Refuse("RecordWindowPosition");
  }
  RequireSource(record, table);
  const auto *state = reinterpret_cast<const detail::StateHandle *>(record)->Peek();
  if ((position == RecordWindowPosition::After || position == RecordWindowPosition::Before) &&
      (state == nullptr || !state->positioned)) {
    Refuse("RecordWindowAnchor");
  }
  auto selected = detail::Select(state, table);
  const detail::RecordOrder order(table,
                                  state == nullptr ? std::span<const detail::SortField>{}
                                                   : state->key,
                                  state == nullptr || state->ascending);
  const bool backwards =
      position == RecordWindowPosition::Before || position == RecordWindowPosition::Last;
  if (position == RecordWindowPosition::After || position == RecordWindowPosition::Before) {
    detail::SeekRecord(selected, order, record, backwards ? "<" : ">");
  }
  std::string sql = "SELECT " + detail::Columns(table) + " FROM " + selected.from;
  if (!selected.where.empty()) { sql += " WHERE " + selected.where; }
  sql += " ORDER BY " + detail::SqlRecordOrder(order, backwards);
  sql += " LIMIT " + std::to_string(limit + 1);
  return RecordWindow(std::make_unique<RecordWindow::Impl>(
      &table, Session::Current().Database().Execute(sql, selected.binds), limit, backwards));
}

}
