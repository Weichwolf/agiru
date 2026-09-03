#include "runtime/Catalogue.h"

#include "meta/Ids.h"
#include "meta/TableDef.h"

#include <algorithm>
#include <span>
#include <vector>

namespace agiru {

namespace {

std::vector<const TableEntry *> &Entries() {
  static std::vector<const TableEntry *> entries;
  return entries;
}

// THE SORT IS DEFERRED TO THE FIRST QUESTION, not done on every registration: the order static
// initialisation hands them over in is the linker's and means nothing, so the catalogue is
// unordered until somebody asks it something, and ordered from then on.
bool &Sorted() {
  static bool sorted = false;
  return sorted;
}

void Order() {
  if (Sorted()) { return; }
  std::sort(Entries().begin(), Entries().end(), [](const TableEntry *a, const TableEntry *b) {
    return a->table->id.Value() < b->table->id.Value();
  });
  Sorted() = true;
}

} // namespace

void RegisterTableEntry(const TableEntry *entry) {
  Entries().push_back(entry);
  Sorted() = false;
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
