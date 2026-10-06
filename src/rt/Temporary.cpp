#include "Temporary.h"

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "runtime/TemporaryRecord.h"
#include "type/BigInteger.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Integer.h"

#include "RecordFilter.h"
#include "RecordOrder.h"

#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::detail {

namespace {

RecordState *StateOf(void *record) {
  return &reinterpret_cast<StateHandle *>(record)->Ensure();
}

const RecordState *PeekOf(const void *record) {
  return reinterpret_cast<const StateHandle *>(record)->Peek();
}

const FieldDef &FieldOf(const TableDef &table, FieldNo no) {
  for (const FieldDef &def : table.fields) {
    if (def.no == no) { return def; }
  }
  throw Error("the table carries no field " + std::to_string(no.Value()));
}

std::span<const FieldNo> PrimaryKey(const TableDef &table) {
  return table.keys.empty() ? std::span<const FieldNo>{} : table.keys[0].fields;
}

std::strong_ordering
Ordered(const void *a, const void *b, const TableDef &table, std::span<const FieldNo> by) {
  for (const FieldNo no : by) {
    const std::strong_ordering order = CompareField(a, b, FieldOf(table, no));
    if (order != std::strong_ordering::equal) { return order; }
  }
  return std::strong_ordering::equal;
}

struct Held {
  TempTable *temp;
  RecordState *state;
};

Held Reach(void *record) {
  RecordState *state = StateOf(record);
  return Held{.temp = state->temporary.get(), .state = state};
}

std::size_t LowerBound(const TempTable &temp, const TableDef &table, const void *record) {
  std::size_t first = 0;
  std::size_t last = temp.ops->count(temp.rows);
  const std::span<const FieldNo> key = PrimaryKey(table);
  while (first < last) {
    const std::size_t middle = first + ((last - first) / 2);
    if (Ordered(temp.ops->at(temp.rows, middle), record, table, key) < 0) {
      first = middle + 1;
    } else {
      last = middle;
    }
  }
  return first;
}

bool SameKeyAt(const TempTable &temp, const TableDef &table, std::size_t at, const void *record) {
  return at < temp.ops->count(temp.rows) &&
         Ordered(temp.ops->at(temp.rows, at), record, table, PrimaryKey(table)) == 0;
}

void Build(Held held, const TableDef &table) {
  RecordState &state = *held.state;
  state.view.clear();
  const RecordFilter filter(state.viewFilters, table);
  const std::size_t count = held.temp->ops->count(held.temp->rows);
  for (std::size_t index = 0; index < count; ++index) {
    const void *row = held.temp->ops->at(held.temp->rows, index);
    if (state.markedOnly && !state.marks.contains(MarkKey(row, table))) { continue; }
    if (filter.Matches(row)) { state.view.push_back(index); }
  }
  const RecordOrder by(table, state.viewKey, state.viewAscending);
  std::ranges::stable_sort(state.view, [&](std::size_t a, std::size_t b) {
    return by.Compare(held.temp->ops->at(held.temp->rows, a),
                      held.temp->ops->at(held.temp->rows, b)) < 0;
  });
  state.viewVersion = held.temp->version;
}

void Snapshot(Held held, const TableDef &table) {
  RecordState &state = *held.state;
  state.viewFilters = state.filters;
  state.viewKey = state.key;
  state.viewAscending = state.ascending;
  Build(held, table);
  state.viewDirty = false;
}

void Land(Held held, void *record, std::size_t at) {
  held.state->at = at;
  held.state->positioned = true;
  held.temp->ops->load(record, held.temp->ops->at(held.temp->rows, held.state->view[at]));
}

struct Anchor {
  std::size_t at;
  bool found;
};

Anchor Refresh(Held held, const TableDef &table, const void *record) {
  const bool selectionChanged = held.state->viewDirty;
  const bool rowsChanged = held.state->viewVersion != held.temp->version;
  if (selectionChanged) {
    Snapshot(held, table);
  } else if (rowsChanged) {
    Build(held, table);
  }
  if (!selectionChanged && !rowsChanged && held.state->at < held.state->view.size()) {
    return Anchor{.at = held.state->at, .found = true};
  }
  const RecordOrder by(table, held.state->viewKey, held.state->viewAscending);
  std::size_t at = 0;
  std::size_t end = held.state->view.size();
  while (at < end) {
    const std::size_t middle = at + (end - at) / 2;
    if (by.Compare(held.temp->ops->at(held.temp->rows, held.state->view[middle]), record) < 0) {
      at = middle + 1;
    } else {
      end = middle;
    }
  }
  const bool same = at < held.state->view.size() &&
                    Ordered(held.temp->ops->at(held.temp->rows, held.state->view[at]),
                            record,
                            table,
                            PrimaryKey(table)) == 0;
  held.state->at = same ? at : held.state->view.size();
  return Anchor{.at = at, .found = same};
}

}

TempTable *TempOf(void *record) {
  const RecordState *state = PeekOf(record);
  return state == nullptr ? nullptr : state->temporary.get();
}

const TempTable *TempOf(const void *record) {
  const RecordState *state = PeekOf(record);
  return state == nullptr ? nullptr : state->temporary.get();
}

void RuntimeMakeTemporary(void *record, const TempOps *ops) {
  RecordState *state = StateOf(record);
  state->temporary = TempHandle(new TempTable(ops, ops->make()));
  state->view.clear();
  state->viewDirty = true;
  state->positioned = false;
}

void RuntimeReset(void *record) {
  auto *handle = reinterpret_cast<StateHandle *>(record);
  const RecordState *state = PeekOf(record);
  if (state == nullptr) { return; }
  TempHandle keep = state->temporary;
  handle->Forget();
  if (keep != nullptr) { StateOf(record)->temporary = std::move(keep); }
}

Integer RuntimeFilterGroup(const void *record) {
  const RecordState *state = PeekOf(record);
  return state == nullptr ? 0 : state->group;
}

Integer RuntimeFilterGroup(void *record, Integer group) {
  const Integer was = RuntimeFilterGroup(record);
  if (group <= kMaximumFilterGroup && group != was) { StateOf(record)->group = group; }
  return was;
}

bool RuntimeHasFilter(const void *record) {
  const RecordState *state = PeekOf(record);
  return state != nullptr &&
         std::ranges::any_of(state->filters, [state](const FieldFilter &filter) {
           return filter.group == state->group && !filter.text.empty();
         });
}

bool RuntimeIsTemporary(const void *record) {
  return TempOf(record) != nullptr;
}

void RuntimeShareTemporary(void *record, const void *from) {
  const RecordState *source = PeekOf(from);
  const bool sourceTemporary = source != nullptr && source->temporary != nullptr;
  const bool targetTemporary = TempOf(record) != nullptr;
  if (!sourceTemporary || !targetTemporary) {
    throw Error(std::string("Record.Copy(From, true) shares temporary rows, and ") +
                (!sourceTemporary && !targetTemporary ? "neither record is temporary"
                 : !sourceTemporary                   ? "the source is not temporary"
                                                      : "the target is not temporary") +
                (source == nullptr ? " (the source has no state at all)" : ""));
  }
  RecordState *state = StateOf(record);
  state->temporary = source->temporary;
  state->view.clear();
  state->positioned = false;
}

void RuntimeBorrowTemporary(void *record, const void *from) {
  const RecordState *source = PeekOf(from);
  if (source == nullptr || source->temporary == nullptr) { return; }
  StateOf(record)->temporary = source->temporary;
}

void RuntimeAdoptTemporary(void *record, const void *from) {
  const RecordState *source = PeekOf(from);
  if (source == nullptr || source->temporary == nullptr) { return; }
  RecordState *state = StateOf(record);
  state->temporary = source->temporary;
  state->view.clear();
  state->positioned = false;
}

bool TempInsert(void *record, const TableDef &table) {
  const Held held = Reach(record);
  const std::size_t at = LowerBound(*held.temp, table, record);
  if (SameKeyAt(*held.temp, table, at, record)) { return false; }
  held.temp->ops->insert(held.temp->rows, at, record);
  ++held.temp->version;
  return true;
}

bool TempGet(void *record, const TableDef &table) {
  const Held held = Reach(record);
  const std::size_t at = LowerBound(*held.temp, table, record);
  if (!SameKeyAt(*held.temp, table, at, record)) { return false; }
  held.temp->ops->load(record, held.temp->ops->at(held.temp->rows, at));
  return true;
}

bool TempModify(void *record, const TableDef &table) {
  const Held held = Reach(record);
  const std::size_t at = LowerBound(*held.temp, table, record);
  if (!SameKeyAt(*held.temp, table, at, record)) { return false; }
  held.temp->ops->replace(held.temp->rows, at, record);
  ++held.temp->version;
  return true;
}

bool TempDelete(void *record, const TableDef &table) {
  const Held held = Reach(record);
  const std::size_t at = LowerBound(*held.temp, table, record);
  if (!SameKeyAt(*held.temp, table, at, record)) { return false; }
  held.temp->ops->erase(held.temp->rows, at);
  ++held.temp->version;
  return true;
}

std::int32_t TempDeleteAll(void *record, const TableDef &table) {
  const Held held = Reach(record);
  const RecordFilter filter(held.state->filters, table);
  std::int32_t removed = 0;
  std::size_t index = 0;
  while (index < held.temp->ops->count(held.temp->rows)) {
    if (filter.Matches(held.temp->ops->at(held.temp->rows, index))) {
      held.temp->ops->erase(held.temp->rows, index);
      ++removed;
    } else {
      ++index;
    }
  }
  if (removed != 0) { ++held.temp->version; }
  return removed;
}

std::int32_t TempCount(void *record, const TableDef &table) {
  const Held held = Reach(record);
  const RecordFilter filter(held.state->filters, table);
  std::int32_t count = 0;
  const std::size_t rows = held.temp->ops->count(held.temp->rows);
  for (std::size_t index = 0; index < rows; ++index) {
    const void *row = held.temp->ops->at(held.temp->rows, index);
    if (held.state->markedOnly && !held.state->marks.contains(MarkKey(row, table))) { continue; }
    if (filter.Matches(row)) { ++count; }
  }
  return count;
}

bool TempIsEmpty(void *record, const TableDef &table) {
  return TempCount(record, table) == 0;
}

void TempCalcSum(void *record, const TableDef &table, const FieldDef &def) {
  const Held held = Reach(record);
  const RecordFilter filter(held.state->filters, table);
  const auto at = [&def](const void *row) {
    return static_cast<const std::byte *>(row) + def.offset;
  };
  Decimal decimal{};
  std::int64_t whole = 0;
  const std::size_t rows = held.temp->ops->count(held.temp->rows);
  for (std::size_t index = 0; index < rows; ++index) {
    const void *row = held.temp->ops->at(held.temp->rows, index);
    if (held.state->markedOnly && !held.state->marks.contains(MarkKey(row, table))) { continue; }
    if (!filter.Matches(row)) { continue; }
    switch (def.type) {
      case FieldType::Decimal:
        decimal = decimal + *reinterpret_cast<const Decimal *>(at(row));
        break;
      case FieldType::Integer: whole += *reinterpret_cast<const Integer *>(at(row)); break;
      case FieldType::BigInteger: whole += *reinterpret_cast<const BigInteger *>(at(row)); break;
      case FieldType::Duration:
        whole += reinterpret_cast<const Duration *>(at(row))->Milliseconds();
        break;
      default: break;
    }
  }
  std::byte *into = static_cast<std::byte *>(record) + def.offset;
  switch (def.type) {
    case FieldType::Decimal: *reinterpret_cast<Decimal *>(into) = decimal; break;
    case FieldType::Integer:
      *reinterpret_cast<Integer *>(into) = static_cast<Integer>(whole);
      break;
    case FieldType::BigInteger: *reinterpret_cast<BigInteger *>(into) = whole; break;
    case FieldType::Duration: *reinterpret_cast<Duration *>(into) = Duration{whole}; break;
    default: break;
  }
}

bool TempFindSet(void *record, const TableDef &table) {
  const Held held = Reach(record);
  held.state->positioned = false;
  Snapshot(held, table);
  if (held.state->view.empty()) { return false; }
  Land(held, record, 0);
  return true;
}

namespace {

bool FindStep(const Held &held, void *record, const RecordOrder &by, char step) {
  const std::vector<std::size_t> &view = held.state->view;
  if (view.empty()) { return false; }
  const auto rowOf = [&](std::size_t at) { return held.temp->ops->at(held.temp->rows, view[at]); };
  switch (step) {
    case '-': Land(held, record, 0); return true;
    case '+': Land(held, record, view.size() - 1); return true;
    case '=':
      for (std::size_t at = 0; at < view.size(); ++at) {
        if (by.Compare(rowOf(at), record) == 0) {
          Land(held, record, at);
          return true;
        }
      }
      break;
    case '>':
      for (std::size_t at = 0; at < view.size(); ++at) {
        if (by.Compare(rowOf(at), record) > 0) {
          Land(held, record, at);
          return true;
        }
      }
      break;
    case '<':
      for (std::size_t at = view.size(); at > 0; --at) {
        if (by.Compare(rowOf(at - 1), record) < 0) {
          Land(held, record, at - 1);
          return true;
        }
      }
      break;
    default:
      throw Error("Record.Find: '" + std::string(1, step) +
                  "' is not one of the characters record-find-method.md declares");
  }
  return false;
}

}

bool TempFind(void *record, const TableDef &table, std::string_view which) {
  const Held held = Reach(record);
  if (which.empty()) { which = "="; }
  const RecordOrder by(table, held.state->key, held.state->ascending);
  return std::ranges::any_of(which, [&](const char step) {
    if ((step == '-' || step == '+') && which.size() != 1) {
      throw Error("Record.Find: '-' and '+' can only be used alone, and this one reads \"" +
                  std::string(which) + "\"");
    }
    held.state->positioned = false;
    Snapshot(held, table);
    return FindStep(held, record, by, step);
  });
}

std::int32_t TempNext(void *record, const TableDef &table, std::int32_t steps) {
  if (steps == 0) { return 0; }
  const Held held = Reach(record);
  if (!held.state->positioned) { return 0; }
  const Anchor anchor = Refresh(held, table, record);
  const std::int32_t wanted = steps;
  const std::int64_t count = wanted < 0 ? -std::int64_t{wanted} : wanted;
  std::int64_t taken = 0;
  std::size_t at = anchor.at;
  if (wanted > 0 && anchor.found) { ++at; }
  if (wanted < 0) {
    if (at == 0) { return 0; }
    --at;
  }
  std::size_t last = at;
  for (std::int64_t step = 0; step < count; ++step) {
    if (at >= held.state->view.size()) { break; }
    last = at;
    ++taken;
    if (wanted < 0 && at == 0) { break; }
    at = wanted > 0 ? at + 1 : at - 1;
  }
  if (taken != 0) { Land(held, record, last); }
  return static_cast<std::int32_t>(wanted < 0 ? -taken : taken);
}

}
