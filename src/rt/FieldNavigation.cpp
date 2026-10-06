#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/Field.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"

#include "FieldMetadata.h"
#include "Filter.h"
#include "RecordFilter.h"
#include "RecordOrder.h"

#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::detail {

namespace {

using FieldRow = platform::Field;

struct LocatedField {
  const TableDef *table;
  const FieldDef *field;
};

auto Key(const LocatedField &row) {
  return std::pair{row.table->id.Value(), row.field->no.Value()};
}

auto Key(const FieldRow &row) {
  return std::pair{row.TableNo, row.No};
}

std::span<const LocatedField> InstalledFields() {
  static const auto fields = [] {
    std::vector<LocatedField> result;
    for (const auto *entry : InstalledTables()) {
      for (const auto &field : entry->table->fields) {
        if (field.no.Value() <= 0) { continue; }
        result.push_back({.table = entry->table, .field = &field});
      }
    }
    std::ranges::sort(result,
                      [](const auto &left, const auto &right) { return Key(left) < Key(right); });
    for (std::size_t i = 1; i < result.size(); ++i) {
      if (Key(result[i - 1]) == Key(result[i])) {
        throw Error("Field catalogue contains a duplicate installed field key");
      }
    }
    return result;
  }();
  return fields;
}

auto Boundary(std::span<const LocatedField> rows, const auto &before) {
  std::size_t low = 0;
  std::size_t high = rows.size();
  while (low < high) {
    const std::size_t middle = low + (high - low) / 2;
    if (before(rows[middle])) {
      low = middle + 1;
    } else {
      high = middle;
    }
  }
  return rows.begin() + static_cast<std::ptrdiff_t>(low);
}

const RecordState *Peek(const void *record) {
  return reinterpret_cast<const StateHandle *>(record)->Peek();
}

std::span<const FieldFilter> Filters(const RecordState *state) {
  return state == nullptr ? std::span<const FieldFilter>{} : state->filters;
}

bool KeyField(FieldNo no) {
  return no == FieldRow::Field_No::TableNo || no == FieldRow::Field_No::No;
}

std::vector<FieldFilter> KeyFilters(const RecordState *state) {
  std::vector<FieldFilter> result;
  for (const auto &filter : Filters(state)) {
    if (filter.group != kCrossColumnGroup && KeyField(filter.field)) { result.push_back(filter); }
  }
  return result;
}

void RequireProjected(FieldNo no) {
  using No = FieldRow::Field_No;
  if (no == No::DataClassification || no == No::SQLDataType || no == No::AppPackageID ||
      no == No::AppRuntimePackageID) {
    throw Error("Field catalogue filter/order requires an unqualified metadata attribute: " +
                std::to_string(no.Value()));
  }
}

std::span<const LocatedField> TableWindow(const RecordState *state) {
  const auto fields = InstalledFields();
  std::int64_t low = 1;
  std::int64_t high = std::numeric_limits<std::int32_t>::max();
  for (const auto &filter : Filters(state)) {
    if (filter.field != FieldRow::Field_No::TableNo || filter.group == kCrossColumnGroup) {
      continue;
    }
    const auto intervals = IntegerIntervals(ParseFilter(filter.text), {.low = low, .high = high});
    if (!intervals) { continue; }
    if (intervals->empty()) { return {}; }
    low = std::max(low, std::ranges::min(*intervals, {}, &Interval::low).low);
    high = std::min(high, std::ranges::max(*intervals, {}, &Interval::high).high);
  }
  const auto begin =
      Boundary(fields, [low](const auto &row) { return row.table->id.Value() < low; });
  const auto end =
      Boundary(fields, [high](const auto &row) { return row.table->id.Value() <= high; });
  return {begin, end};
}

struct PickedField {
  FieldRow row;
  std::size_t index;
};

class FieldSelection {
public:
  FieldSelection(const RecordState *state, const TableDef &table)
      : state_(state),
        rows_(TableWindow(state)),
        keys_(KeyFilters(state), table),
        filters_(Filters(state), table),
        order_(table,
               state == nullptr ? std::span<const SortField>{} : state->key,
               state == nullptr || state->ascending) {
    for (const auto &filter : Filters(state)) { RequireProjected(filter.field); }
    for (const auto &column : order_.Columns()) { RequireProjected(column.field->no); }
    keyOnly_ = std::ranges::all_of(Filters(state), [](const auto &filter) {
      return KeyField(filter.field) && filter.group != kCrossColumnGroup;
    });
  }

  bool Matches(const LocatedField &located, FieldRow &row, bool project) const {
    row.TableNo = located.table->id.Value();
    row.No = located.field->no.Value();
    if (!keys_.Matches(&row)) { return false; }
    if (state_ != nullptr && state_->markedOnly &&
        !state_->marks.contains(MarkKey(&row, platform::kFieldTable))) {
      return false;
    }
    if (!project && keyOnly_) { return true; }
    row = FieldRow{};
    LoadFieldMetadata(row, *located.table, *located.field);
    return filters_.Matches(&row);
  }

  std::int32_t Count(bool firstOnly) const {
    std::int64_t count = 0;
    FieldRow row;
    for (const auto &located : rows_) {
      if (!Matches(located, row, false)) { continue; }
      ++count;
      if (firstOnly) { break; }
    }
    if (count > std::numeric_limits<std::int32_t>::max()) {
      throw Error("Field catalogue count exceeds AL Integer");
    }
    return static_cast<std::int32_t>(count);
  }

  std::optional<PickedField> Pick(const FieldRow &anchor, char which) const {
    const auto columns = order_.Columns();
    if (columns.size() == 2 && columns[0].field->no == FieldRow::Field_No::TableNo &&
        columns[1].field->no == FieldRow::Field_No::No &&
        columns[0].ascending == columns[1].ascending) {
      return Indexed(anchor, which, columns[0].ascending);
    }
    return Unindexed(anchor, which);
  }

private:
  std::optional<PickedField> Indexed(const FieldRow &anchor, char which, bool ascending) const {
    const bool backwards = which == '+' || which == '<';
    const bool forward = ascending != backwards;
    auto begin = rows_.begin();
    auto end = rows_.end();
    if (which == '=') {
      begin = Boundary(rows_, [&](const auto &row) { return Key(row) < Key(anchor); });
      if (begin == end || Key(*begin) != Key(anchor)) { return std::nullopt; }
      end = begin + 1;
    } else if (which == '>' || which == '<') {
      if (forward) {
        begin = Boundary(rows_, [&](const auto &row) { return Key(row) <= Key(anchor); });
      } else {
        end = Boundary(rows_, [&](const auto &row) { return Key(row) < Key(anchor); });
      }
    }
    FieldRow row;
    while (begin != end) {
      const auto at = forward ? begin++ : --end;
      if (Matches(*at, row, true)) { return Located(row, *at); }
    }
    return std::nullopt;
  }

  std::optional<PickedField> Unindexed(const FieldRow &anchor, char which) const {
    std::optional<PickedField> best;
    FieldRow row;
    const bool backwards = which == '+' || which == '<';
    for (const auto &located : rows_) {
      if (!Matches(located, row, true)) { continue; }
      const auto compared = order_.Compare(&row, &anchor);
      if ((which == '=' && compared != 0) || (which == '>' && compared <= 0) ||
          (which == '<' && compared >= 0)) {
        continue;
      }
      if (!best || (backwards ? order_.Compare(&row, &best->row) > 0
                              : order_.Compare(&row, &best->row) < 0)) {
        best = Located(row, located);
      }
    }
    return best;
  }

  static PickedField Located(const FieldRow &row, const LocatedField &located) {
    return {.row = row, .index = static_cast<std::size_t>(&located - InstalledFields().data())};
  }

  const RecordState *state_;
  std::span<const LocatedField> rows_;
  RecordFilter keys_;
  RecordFilter filters_;
  RecordOrder order_;
  bool keyOnly_ = false;
};

void Land(void *record, const TableDef &table, RecordState &state, const PickedField &picked) {
  for (const auto &field : table.fields) {
    if (Stored(field)) { SetFieldText(record, field, StorageText(&picked.row, field)); }
  }
  state.at = picked.index;
  state.positioned = true;
  state.viewDirty = false;
}

void ValidateWhich(std::string_view which) {
  std::string seen;
  for (const char step : which) {
    if (std::string_view("-+=<>").find(step) == std::string_view::npos ||
        seen.find(step) != std::string::npos ||
        ((step == '-' || step == '+') && which.size() != 1)) {
      throw Error("Field.Find requires distinct '=<>', or '-'/'+' alone");
    }
    seen += step;
  }
}

}

std::optional<bool>
FindInstalledFields(void *record, const TableDef &table, std::string_view which) {
  if (!IsInstalledFieldProvider(table)) { return std::nullopt; }
  if (which.empty()) { which = "="; }
  ValidateWhich(which);
  auto &state = reinterpret_cast<StateHandle *>(record)->Ensure();
  state.open.Forget();
  state.positioned = false;
  state.stepped = 0;
  const FieldSelection selection(&state, table);
  for (const char step : which) {
    const auto picked = selection.Pick(*static_cast<FieldRow *>(record), step);
    if (!picked) { continue; }
    Land(record, table, state, *picked);
    return true;
  }
  return false;
}

std::optional<std::int32_t>
NextInstalledField(void *record, const TableDef &table, std::int32_t steps) {
  if (!IsInstalledFieldProvider(table)) { return std::nullopt; }
  auto &state = reinterpret_cast<StateHandle *>(record)->Ensure();
  if (!state.positioned || steps == 0) { return 0; }
  const FieldSelection selection(&state, table);
  FieldRow anchor = *static_cast<FieldRow *>(record);
  if (!state.viewDirty && state.at < InstalledFields().size()) {
    const auto &located = InstalledFields()[state.at];
    LoadFieldMetadata(anchor, *located.table, *located.field);
  }
  const bool backwards = steps < 0;
  const std::int64_t wanted = backwards ? -std::int64_t{steps} : steps;
  std::int64_t moved = 0;
  while (moved < wanted) {
    const auto picked = selection.Pick(anchor, backwards ? '<' : '>');
    if (!picked) { break; }
    Land(record, table, state, *picked);
    anchor = picked->row;
    ++moved;
    ++state.stepped;
  }
  return static_cast<std::int32_t>(backwards ? -moved : moved);
}

std::optional<std::int32_t>
CountInstalledFields(const void *record, const TableDef &table, bool firstOnly) {
  if (!IsInstalledFieldProvider(table)) { return std::nullopt; }
  return FieldSelection(Peek(record), table).Count(firstOnly);
}

}
