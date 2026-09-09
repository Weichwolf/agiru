#include "runtime/Catalogue.h"

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Codeunit.h"

#include <algorithm>
#include <cctype>
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

}

void RegisterTableEntry(const TableEntry *entry) {
  Entries().push_back(entry);
}

namespace {

std::vector<const ProfileDef *> &Profiles() {
  static std::vector<const ProfileDef *> profiles;
  return profiles;
}

}

void RegisterProfileEntry(const ProfileDef *profile) {
  Profiles().push_back(profile);
}

std::span<const ProfileDef *const> InstalledProfiles() {
  return Profiles();
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

const TableEntry *FindTable(std::string_view name) {
  Order();
  for (const TableEntry *entry : Entries()) {
    const std::string_view candidate = entry->table->name;
    if (candidate.size() != name.size()) { continue; }
    bool same = true;
    for (std::size_t i = 0; i < name.size() && same; ++i) {
      same = std::tolower(static_cast<unsigned char>(candidate[i])) ==
             std::tolower(static_cast<unsigned char>(name[i]));
    }
    if (same) { return entry; }
  }
  return nullptr;
}

std::span<const TableEntry *const> InstalledTables() {
  Order();
  return Entries();
}

namespace {

std::vector<const PageEntry *> &PageEntries() {
  static std::vector<const PageEntry *> entries;
  return entries;
}

std::once_flag &PagesOnce() {
  static std::once_flag once;
  return once;
}

}

namespace {

std::vector<const CodeunitEntry *> &CodeunitEntries() {
  static std::vector<const CodeunitEntry *> entries;
  return entries;
}

std::once_flag &CodeunitsOnce() {
  static std::once_flag once;
  return once;
}

}

void RegisterCodeunitEntry(const CodeunitEntry *entry) {
  CodeunitEntries().push_back(entry);
}

const CodeunitEntry *FindCodeunit(CodeunitId id) {
  std::call_once(CodeunitsOnce(), [] {
    std::ranges::sort(CodeunitEntries(), [](const CodeunitEntry *a, const CodeunitEntry *b) {
      return a->id.Value() < b->id.Value();
    });
  });
  const auto found = std::lower_bound(
      CodeunitEntries().begin(),
      CodeunitEntries().end(),
      id.Value(),
      [](const CodeunitEntry *entry, auto number) { return entry->id.Value() < number; });
  if (found == CodeunitEntries().end() || (*found)->id != id) { return nullptr; }
  return *found;
}

void RegisterPageEntry(const PageEntry *entry) {
  PageEntries().push_back(entry);
}

const PageEntry *FindPage(PageId id) {
  std::call_once(PagesOnce(), [] {
    std::ranges::sort(PageEntries(), [](const PageEntry *a, const PageEntry *b) {
      return a->page->id.Value() < b->page->id.Value();
    });
  });
  const auto found = std::lower_bound(
      PageEntries().begin(),
      PageEntries().end(),
      id.Value(),
      [](const PageEntry *entry, auto number) { return entry->page->id.Value() < number; });
  if (found == PageEntries().end() || (*found)->page->id != id) { return nullptr; }
  return *found;
}

std::span<const CodeunitEntry *const> InstalledCodeunits() {
  return CodeunitEntries();
}

std::span<const PageEntry *const> InstalledPages() {
  return PageEntries();
}

}
