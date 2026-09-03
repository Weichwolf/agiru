#include "runtime/Catalogue.h"

#include "meta/Ids.h"
#include "meta/TableDef.h"

#include <algorithm>
#include <mutex>
#include <span>
#include <vector>

namespace agiru {

namespace {

std::vector<const TableEntry *> &Entries() {
  static std::vector<const TableEntry *> entries;
  return entries;
}

std::once_flag &Once() {
  static std::once_flag once;
  return once;
}

void Order() {
  std::call_once(Once(), [] {
    std::sort(Entries().begin(), Entries().end(), [](const TableEntry *a, const TableEntry *b) {
      return a->table->id.Value() < b->table->id.Value();
    });
  });
}

} // namespace

void RegisterTableEntry(const TableEntry *entry) {
  Entries().push_back(entry);
}

const TableEntry *FindTable(TableId id) {
  Order();
  const auto found = std::lower_bound(
      Entries().begin(), Entries().end(), id.Value(), [](const TableEntry *entry, auto number) {
        return entry->table->id.Value() < number;
      });
  if (found == Entries().end() || (*found)->table->id != id) { return nullptr; }
  return *found;
}

std::span<const TableEntry *const> InstalledTables() {
  Order();
  return Entries();
}

} // namespace agiru
