#include "Temporary.h"

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"

#include "Filter.h"

#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
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

std::vector<FieldNo> OrderOf(const std::vector<SortField> &key, const TableDef &table) {
  std::vector<FieldNo> named;
  for (const SortField &one : key) { named.push_back(one.field); }
  if (table.keys.empty()) { return named; }
  for (const FieldNo no : table.keys[0].fields) {
    if (std::ranges::find(named, no) == named.end()) { named.push_back(no); }
  }
  return named;
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

bool Passes(const void *row, const std::vector<FieldFilter> &filters, const TableDef &table) {
  for (const FieldFilter &filter : filters) {
    const FieldDef &def = FieldOf(table, filter.field);
    if (!Matches(ParseFilter(filter.text), FieldText(row, def), def)) { return false; }
  }
  return true;
}

void Build(Held held, const TableDef &table) {
  RecordState &state = *held.state;
  if (state.markedOnly) {
    throw Error("Record.MarkedOnly over a temporary record is not carried yet (board:0583)");
  }
  state.view.clear();
  const std::size_t count = held.temp->ops->count(held.temp->rows);
  for (std::size_t index = 0; index < count; ++index) {
    if (Passes(held.temp->ops->at(held.temp->rows, index), state.viewFilters, table)) {
      state.view.push_back(index);
    }
  }
  const std::vector<FieldNo> by = OrderOf(state.viewKey, table);
  std::ranges::stable_sort(state.view, [&](std::size_t a, std::size_t b) {
    const std::strong_ordering order = Ordered(
        held.temp->ops->at(held.temp->rows, a), held.temp->ops->at(held.temp->rows, b), table, by);
    return state.viewAscending ? order < 0 : order > 0;
  });
  state.viewVersion = held.temp->version;
}

void Snapshot(Held held, const TableDef &table) {
  RecordState &state = *held.state;
  state.viewFilters = state.filters;
  state.viewKey = state.key;
  state.viewAscending = state.ascending;
  Build(held, table);
}

void Land(Held held, void *record, std::size_t at) {
  held.state->at = at;
  held.state->positioned = true;
  held.temp->ops->load(record, held.temp->ops->at(held.temp->rows, held.state->view[at]));
}

void Refresh(Held held, const TableDef &table, const void *record) {
  if (held.state->viewVersion == held.temp->version) { return; }
  Build(held, table);
  const std::vector<FieldNo> by = OrderOf(held.state->viewKey, table);
  std::size_t at = 0;
  while (at < held.state->view.size()) {
    const std::strong_ordering order =
        Ordered(held.temp->ops->at(held.temp->rows, held.state->view[at]), record, table, by);
    if (held.state->viewAscending ? order >= 0 : order <= 0) { break; }
    ++at;
  }
  held.state->at = at;
  const bool same = at < held.state->view.size() &&
                    Ordered(held.temp->ops->at(held.temp->rows, held.state->view[at]),
                            record,
                            table,
                            PrimaryKey(table)) == 0;
  if (!same) { held.state->at = at == 0 ? 0 : at - 1; }
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
  state->positioned = false;
}

bool RuntimeIsTemporary(const void *record) {
  return TempOf(record) != nullptr;
}

void RuntimeShareTemporary(void *record, const void *from) {
  const RecordState *source = PeekOf(from);
  if (source == nullptr || source->temporary == nullptr || TempOf(record) == nullptr) {
    throw Error("Record.Copy(From, true) shares temporary rows, and one of the two records is "
                "not temporary");
  }
  RecordState *state = StateOf(record);
  state->temporary = source->temporary;
  state->view.clear();
  state->positioned = false;
}

bool TempInsert(void *record, const TableDef &table) {
  const Held held = Reach(record);
  const std::size_t at = LowerBound(*held.temp, table, record);
  if (SameKeyAt(*held.temp, table, at, record)) {
    throw Error("The " + std::string(table.caption) + " already exists.");
  }
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
  std::int32_t removed = 0;
  std::size_t index = 0;
  while (index < held.temp->ops->count(held.temp->rows)) {
    if (Passes(held.temp->ops->at(held.temp->rows, index), held.state->filters, table)) {
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
  std::int32_t count = 0;
  const std::size_t rows = held.temp->ops->count(held.temp->rows);
  for (std::size_t index = 0; index < rows; ++index) {
    if (Passes(held.temp->ops->at(held.temp->rows, index), held.state->filters, table)) { ++count; }
  }
  return count;
}

bool TempIsEmpty(void *record, const TableDef &table) {
  return TempCount(record, table) == 0;
}

bool TempFindSet(void *record, const TableDef &table) {
  const Held held = Reach(record);
  held.state->positioned = false;
  Snapshot(held, table);
  if (held.state->view.empty()) { return false; }
  Land(held, record, 0);
  return true;
}

bool TempFind(void *record, const TableDef &table, std::string_view which) {
  const Held held = Reach(record);
  if (which.empty()) { which = "="; }
  const std::vector<FieldNo> by = OrderOf(held.state->key, table);
  for (const char step : which) {
    if ((step == '-' || step == '+') && which.size() != 1) {
      throw Error("Record.Find: '-' and '+' can only be used alone, and this one reads \"" +
                  std::string(which) + "\"");
    }
    held.state->positioned = false;
    Snapshot(held, table);
    const std::vector<std::size_t> &view = held.state->view;
    if (view.empty()) { continue; }
    const auto rowOf = [&](std::size_t at) {
      return held.temp->ops->at(held.temp->rows, view[at]);
    };
    switch (step) {
      case '-': Land(held, record, 0); return true;
      case '+': Land(held, record, view.size() - 1); return true;
      case '=':
        for (std::size_t at = 0; at < view.size(); ++at) {
          if (Ordered(rowOf(at), record, table, by) == 0) {
            Land(held, record, at);
            return true;
          }
        }
        break;
      case '>':
        for (std::size_t at = 0; at < view.size(); ++at) {
          const std::strong_ordering order = Ordered(rowOf(at), record, table, by);
          if (held.state->viewAscending ? order > 0 : order < 0) {
            Land(held, record, at);
            return true;
          }
        }
        break;
      case '<':
        for (std::size_t at = view.size(); at > 0; --at) {
          const std::strong_ordering order = Ordered(rowOf(at - 1), record, table, by);
          if (held.state->viewAscending ? order < 0 : order > 0) {
            Land(held, record, at - 1);
            return true;
          }
        }
        break;
      default:
        throw Error("Record.Find: '" + std::string(1, step) +
                    "' is not one of the characters record-find-method.md declares");
    }
  }
  return false;
}

std::int32_t TempNext(void *record, const TableDef &table, std::int32_t steps) {
  const Held held = Reach(record);
  if (!held.state->positioned) { return 0; }
  Refresh(held, table, record);
  const std::int32_t wanted = steps == 0 ? 1 : steps;
  const std::int32_t way = wanted > 0 ? 1 : -1;
  std::int32_t taken = 0;
  std::size_t at = held.state->at;
  for (std::int32_t step = 0; step < (wanted > 0 ? wanted : -wanted); ++step) {
    if (way > 0 ? at + 1 >= held.state->view.size() : at == 0) {
      held.state->positioned = false;
      return taken;
    }
    at = way > 0 ? at + 1 : at - 1;
    taken += way;
  }
  Land(held, record, at);
  return taken;
}

}
