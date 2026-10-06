#include "CatalogueNavigation.h"

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"

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

std::size_t Boundary(std::size_t low, std::size_t high, const auto &before) {
  while (low < high) {
    const std::size_t middle = low + (high - low) / 2;
    if (before(middle)) {
      low = middle + 1;
    } else {
      high = middle;
    }
  }
  return low;
}

const RecordState *Peek(const void *record) {
  return reinterpret_cast<const StateHandle *>(record)->Peek();
}

std::span<const FieldFilter> Filters(const RecordState *state) {
  return state == nullptr ? std::span<const FieldFilter>{} : state->filters;
}

bool KeyField(const TableDef &table, FieldNo no) {
  return std::ranges::find(table.keys[0].fields, no) != table.keys[0].fields.end();
}

std::vector<FieldFilter> KeyFilters(const RecordState *state, const TableDef &table) {
  std::vector<FieldFilter> result;
  for (const auto &filter : Filters(state)) {
    if (filter.group != kCrossColumnGroup && KeyField(table, filter.field)) {
      result.push_back(filter);
    }
  }
  return result;
}

void CopyValues(void *into, const void *from, const TableDef &table) {
  for (const auto &field : table.fields) {
    if (Stored(field)) { SetFieldText(into, field, StorageText(from, field)); }
  }
}

std::pair<std::size_t, std::size_t>
Window(const TableDef &table, const CatalogueReader &reader, const auto &visit) {
  if (table.keys.empty() || table.keys[0].fields.empty()) {
    throw Error("Catalogue navigation requires a declared primary key");
  }
  const auto *field = Field(table, table.keys[0].fields[0]);
  if (field == nullptr || field->type != FieldType::Integer) {
    throw Error("Catalogue navigation requires a declared Integer leading key");
  }
  std::int64_t low = 1;
  std::int64_t high = std::numeric_limits<std::int32_t>::max();
  visit([&](FieldNo no, const Expression &expression) {
    if (no != field->no || low > high) { return; }
    const auto intervals = IntegerIntervals(expression, {.low = low, .high = high});
    if (!intervals) { return; }
    if (intervals->empty()) {
      low = 1;
      high = 0;
      return;
    }
    low = std::max(low, std::ranges::min(*intervals, {}, &Interval::low).low);
    high = std::min(high, std::ranges::max(*intervals, {}, &Interval::high).high);
  });
  if (low > high) { return {0, 0}; }
  const auto key = [&](std::size_t index) {
    reader.key(reader.candidate, index);
    return *reinterpret_cast<const std::int32_t *>(
        static_cast<const std::byte *>(reader.candidate) + field->offset);
  };
  return {Boundary(0, reader.size, [&](std::size_t index) { return key(index) < low; }),
          Boundary(0, reader.size, [&](std::size_t index) { return key(index) <= high; })};
}

std::pair<std::size_t, std::size_t>
Window(const RecordState *state, const TableDef &table, const CatalogueReader &reader) {
  return Window(table, reader, [&](const auto &consume) {
    for (const auto &filter : Filters(state)) {
      if (filter.group != kCrossColumnGroup) { consume(filter.field, ParseFilter(filter.text)); }
    }
  });
}

class CatalogueSelection {
public:
  CatalogueSelection(const RecordState *state, const TableDef &table, const CatalogueReader &reader)
      : state_(state),
        table_(table),
        reader_(reader),
        window_(Window(state, table, reader)),
        keys_(KeyFilters(state, table), table),
        filters_(Filters(state), table),
        order_(table,
               state == nullptr ? std::span<const SortField>{} : state->key,
               state == nullptr || state->ascending),
        primary_(table, {}, true) {
    if (reader.require != nullptr) {
      for (const auto &filter : Filters(state)) { reader.require(filter.field); }
      for (const auto &column : order_.Columns()) { reader.require(column.field->no); }
    }
    keyOnly_ = std::ranges::all_of(Filters(state), [&](const auto &filter) {
      return KeyField(table, filter.field) && filter.group != kCrossColumnGroup;
    });
  }

  bool Matches(std::size_t index, bool project) const {
    reader_.key(reader_.candidate, index);
    if (!keys_.Matches(reader_.candidate)) { return false; }
    if (state_ != nullptr && state_->markedOnly &&
        !state_->marks.contains(MarkKey(reader_.candidate, table_))) {
      return false;
    }
    if (!project && keyOnly_) { return true; }
    reader_.project(reader_.candidate, index);
    return filters_.Matches(reader_.candidate);
  }

  std::int32_t Count(bool firstOnly) const {
    std::int64_t count = 0;
    for (std::size_t index = window_.first; index < window_.second; ++index) {
      if (!Matches(index, false)) { continue; }
      ++count;
      if (firstOnly) { break; }
    }
    if (count > std::numeric_limits<std::int32_t>::max()) {
      throw Error(std::string(table_.name) + " catalogue count exceeds AL Integer");
    }
    return static_cast<std::int32_t>(count);
  }

  std::optional<std::size_t> Pick(const void *anchor, char which) const {
    const auto columns = order_.Columns();
    const auto keys = table_.keys[0].fields;
    bool indexed = columns.size() == keys.size();
    for (std::size_t index = 0; indexed && index < columns.size(); ++index) {
      indexed = columns[index].field->no == keys[index] &&
                columns[index].ascending == columns[0].ascending;
    }
    return indexed ? Indexed(anchor, which, columns[0].ascending) : Unindexed(anchor, which);
  }

private:
  std::optional<std::size_t> Indexed(const void *anchor, char which, bool ascending) const {
    const bool backwards = which == '+' || which == '<';
    const bool forward = ascending != backwards;
    auto [begin, end] = window_;
    const auto compared = [&](std::size_t index) {
      reader_.key(reader_.candidate, index);
      return primary_.Compare(reader_.candidate, anchor);
    };
    if (which == '=') {
      begin = Boundary(begin, end, [&](std::size_t index) { return compared(index) < 0; });
      if (begin == end || compared(begin) != 0) { return std::nullopt; }
      end = begin + 1;
    } else if (which == '>' || which == '<') {
      if (forward) {
        begin = Boundary(begin, end, [&](std::size_t index) { return compared(index) <= 0; });
      } else {
        end = Boundary(begin, end, [&](std::size_t index) { return compared(index) < 0; });
      }
    }
    while (begin != end) {
      const auto index = forward ? begin++ : --end;
      if (!Matches(index, true)) { continue; }
      CopyValues(reader_.selected, reader_.candidate, table_);
      return index;
    }
    return std::nullopt;
  }

  std::optional<std::size_t> Unindexed(const void *anchor, char which) const {
    std::optional<std::size_t> best;
    const bool backwards = which == '+' || which == '<';
    for (std::size_t index = window_.first; index < window_.second; ++index) {
      if (!Matches(index, true)) { continue; }
      const auto compared = order_.Compare(reader_.candidate, anchor);
      if ((which == '=' && compared != 0) || (which == '>' && compared <= 0) ||
          (which == '<' && compared >= 0)) {
        continue;
      }
      if (!best || (backwards ? order_.Compare(reader_.candidate, reader_.selected) > 0
                              : order_.Compare(reader_.candidate, reader_.selected) < 0)) {
        CopyValues(reader_.selected, reader_.candidate, table_);
        best = index;
      }
    }
    return best;
  }

  const RecordState *state_;
  const TableDef &table_;
  const CatalogueReader &reader_;
  std::pair<std::size_t, std::size_t> window_;
  RecordFilter keys_;
  RecordFilter filters_;
  RecordOrder order_;
  RecordOrder primary_;
  bool keyOnly_ = false;
};

void Land(void *record,
          const TableDef &table,
          RecordState &state,
          const CatalogueReader &reader,
          std::size_t index) {
  CopyValues(record, reader.selected, table);
  state.at = index;
  state.positioned = true;
  state.viewDirty = false;
}

void ValidateWhich(std::string_view which, const TableDef &table) {
  std::string seen;
  for (const char step : which) {
    if (std::string_view("-+=<>").find(step) == std::string_view::npos ||
        seen.find(step) != std::string::npos ||
        ((step == '-' || step == '+') && which.size() != 1)) {
      throw Error(std::string(table.name) + ".Find requires distinct '=<>', or '-'/'+' alone");
    }
    seen += step;
  }
}

}

bool FindCatalogue(void *record,
                   const TableDef &table,
                   std::string_view which,
                   const CatalogueReader &reader) {
  if (which.empty()) { which = "="; }
  ValidateWhich(which, table);
  auto &state = reinterpret_cast<StateHandle *>(record)->Ensure();
  state.open.Forget();
  state.positioned = false;
  state.stepped = 0;
  const CatalogueSelection selection(&state, table, reader);
  for (const char step : which) {
    const auto picked = selection.Pick(record, step);
    if (!picked) { continue; }
    Land(record, table, state, reader, *picked);
    return true;
  }
  return false;
}

std::int32_t NextCatalogue(void *record,
                           const TableDef &table,
                           std::int32_t steps,
                           const CatalogueReader &reader) {
  auto &state = reinterpret_cast<StateHandle *>(record)->Ensure();
  if (!state.positioned || steps == 0) { return 0; }
  const CatalogueSelection selection(&state, table, reader);
  if (!state.viewDirty && state.at < reader.size) {
    reader.project(reader.anchor, state.at);
  } else {
    CopyValues(reader.anchor, record, table);
  }
  const bool backwards = steps < 0;
  const std::int64_t wanted = backwards ? -std::int64_t{steps} : steps;
  std::int64_t moved = 0;
  while (moved < wanted) {
    const auto picked = selection.Pick(reader.anchor, backwards ? '<' : '>');
    if (!picked) { break; }
    Land(record, table, state, reader, *picked);
    CopyValues(reader.anchor, reader.selected, table);
    ++moved;
    ++state.stepped;
  }
  return static_cast<std::int32_t>(backwards ? -moved : moved);
}

void ScanCatalogue(const TableDef &table,
                   const CatalogueReader &reader,
                   const CatalogueScan &scan) {
  const auto window = Window(table, reader, [&](const auto &consume) {
    for (const auto &filter : scan.filters) { consume(filter.field, filter.expression); }
  });
  std::vector<ColumnPredicate> keyFilters;
  for (const auto &filter : scan.filters) {
    if (reader.require != nullptr) { reader.require(filter.field); }
    if (KeyField(table, filter.field)) { keyFilters.push_back(filter); }
  }
  const auto keys = RecordFilter::FromPredicates(keyFilters, table);
  const auto filters = RecordFilter::FromPredicates(scan.filters, table);
  const bool project = scan.project || keyFilters.size() != scan.filters.size();
  for (auto index = window.first; index < window.second; ++index) {
    reader.key(reader.candidate, index);
    if (!keys.Matches(reader.candidate)) { continue; }
    if (project) {
      reader.project(reader.candidate, index);
      if (!filters.Matches(reader.candidate)) { continue; }
    }
    if (!scan.visit(scan.context, reader.candidate)) { break; }
  }
}

std::int32_t CountCatalogue(const void *record,
                            const TableDef &table,
                            bool firstOnly,
                            const CatalogueReader &reader) {
  return CatalogueSelection(Peek(record), table, reader).Count(firstOnly);
}

}
