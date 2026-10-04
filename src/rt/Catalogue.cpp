#include "runtime/Catalogue.h"

#include "meta/Ids.h"
#include "meta/ModuleDef.h"
#include "meta/PageDef.h"
#include "meta/ProfileDef.h"
#include "meta/TableDef.h"
#include "runtime/Codeunit.h"
#include "runtime/TableDefinition.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace agiru {

namespace {

std::int32_t Number(const TableEntry *entry) {
  return entry->table->id.Value();
}

std::int32_t Number(const PageEntry *entry) {
  return entry->page->id.Value();
}

std::int32_t Number(const CodeunitEntry *entry) {
  return entry->id.Value();
}

void ValidateSource(const TableEntry &entry) {
  if (entry.sourceBinding == nullptr) { return; }
  const auto *base = entry.sourceBinding->table;
  const auto &source = *entry.table;
  if (base == nullptr || entry.sourceBinding->sourceBinding != nullptr ||
      !IsPlatformTable(source.id) || source.id != base->id || source.name != base->name ||
      source.fields.data() != base->fields.data() || source.fields.size() != base->fields.size() ||
      source.keys.data() != base->keys.data() || source.keys.size() != base->keys.size()) {
    throw std::invalid_argument("Native table source does not match its record ABI binding");
  }
  if (source.module == nullptr || source.module->id.empty() || source.module->name.empty() ||
      source.module->publisher.empty() || source.module->version.empty()) {
    throw std::invalid_argument("Native table source has no complete original module identity");
  }
}

const TableEntry *QualifiedPair(const TableEntry *left, const TableEntry *right) {
  if (left->sourceBinding == right && right->sourceBinding == nullptr) { return left; }
  if (right->sourceBinding == left && left->sourceBinding == nullptr) { return right; }
  return nullptr;
}

template <typename Entry>
const Entry *QualifiedPair([[maybe_unused]] const Entry *left,
                           [[maybe_unused]] const Entry *right) {
  return nullptr;
}

class InstalledCatalogue {
  std::vector<const TableEntry *> tables_;
  std::vector<const PageEntry *> pages_;
  std::vector<const CodeunitEntry *> codeunits_;
  std::vector<const ProfileDef *> profiles_;
  std::mutex mutex_;
  std::once_flag once_;
  bool frozen_ = false;

  template <typename Entry>
  void Add(std::vector<const Entry *> &entries, const Entry *entry, std::string_view kind) {
    const std::scoped_lock lock{mutex_};
    if (entry == nullptr) {
      throw std::invalid_argument("Cannot register a null installed " + std::string(kind));
    }
    if (frozen_) {
      throw std::logic_error("Installed catalogue is frozen; cannot register " + std::string(kind));
    }
    entries.push_back(entry);
  }

  template <typename Entry>
  static void Sort(std::vector<const Entry *> &entries, std::string_view kind) {
    std::ranges::sort(entries,
                      [](const Entry *a, const Entry *b) { return Number(a) < Number(b); });
    for (std::size_t i = 1; i < entries.size(); ++i) {
      if (Number(entries[i - 1]) == Number(entries[i])) {
        const bool third = i + 1 < entries.size() && Number(entries[i + 1]) == Number(entries[i]);
        if (const auto *qualified = QualifiedPair(entries[i - 1], entries[i]);
            qualified != nullptr && !third) {
          entries[i - 1] = qualified;
          entries.erase(entries.begin() + static_cast<std::ptrdiff_t>(i));
          --i;
          continue;
        }
        throw std::logic_error("Duplicate installed " + std::string(kind) + " ID " +
                               std::to_string(Number(entries[i])));
      }
    }
  }

  void Freeze() {
    std::call_once(once_, [this] {
      const std::scoped_lock lock{mutex_};
      Sort(tables_, "table");
      Sort(pages_, "page");
      Sort(codeunits_, "codeunit");
      frozen_ = true;
    });
  }

public:
  void Register(const TableEntry *entry) {
    if (entry != nullptr && entry->table == nullptr) {
      throw std::invalid_argument("Cannot register an installed table without a declaration");
    }
    if (entry != nullptr) { ValidateSource(*entry); }
    Add(tables_, entry, "table");
  }

  void Register(const PageEntry *entry) {
    if (entry != nullptr && entry->page == nullptr) {
      throw std::invalid_argument("Cannot register an installed page without a declaration");
    }
    Add(pages_, entry, "page");
  }

  void Register(const CodeunitEntry *entry) { Add(codeunits_, entry, "codeunit"); }

  void Register(const ProfileDef *entry) { Add(profiles_, entry, "profile"); }

  std::span<const TableEntry *const> Tables() {
    Freeze();
    return tables_;
  }

  std::span<const PageEntry *const> Pages() {
    Freeze();
    return pages_;
  }

  std::span<const CodeunitEntry *const> Codeunits() {
    Freeze();
    return codeunits_;
  }

  std::span<const ProfileDef *const> Profiles() {
    Freeze();
    return profiles_;
  }
};

InstalledCatalogue &Catalogue() {
  static InstalledCatalogue catalogue;
  return catalogue;
}

template <typename Entry, typename Id>
const Entry *Find(std::span<const Entry *const> entries, Id id) {
  const auto found = std::lower_bound(
      entries.begin(), entries.end(), id.Value(), [](const Entry *entry, auto number) {
        return Number(entry) < number;
      });
  return found == entries.end() || Number(*found) != id.Value() ? nullptr : *found;
}

}

void RegisterTableEntry(const TableEntry *entry) {
  Catalogue().Register(entry);
}

void RegisterProfileEntry(const ProfileDef *profile) {
  Catalogue().Register(profile);
}

std::span<const ProfileDef *const> InstalledProfiles() {
  return Catalogue().Profiles();
}

const TableEntry *FindTable(TableId id) {
  return Find(Catalogue().Tables(), id);
}

const TableEntry *FindTable(std::string_view name) {
  for (const TableEntry *entry : Catalogue().Tables()) {
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
  return Catalogue().Tables();
}

namespace detail {

const TableDef &InstalledTableDefinition(const TableDef *binding) {
  if (binding == nullptr) {
    throw std::invalid_argument("Cannot resolve a null table ABI binding");
  }
  if (!IsPlatformTable(binding->id)) { return *binding; }
  const auto *entry = FindTable(binding->id);
  if (entry != nullptr && entry->sourceBinding != nullptr &&
      entry->sourceBinding->table == binding) {
    return *entry->table;
  }
  return *binding;
}

}

void RegisterCodeunitEntry(const CodeunitEntry *entry) {
  Catalogue().Register(entry);
}

const CodeunitEntry *FindCodeunit(CodeunitId id) {
  return Find(Catalogue().Codeunits(), id);
}

void RegisterPageEntry(const PageEntry *entry) {
  Catalogue().Register(entry);
}

const PageEntry *FindPage(PageId id) {
  return Find(Catalogue().Pages(), id);
}

std::span<const CodeunitEntry *const> InstalledCodeunits() {
  return Catalogue().Codeunits();
}

std::span<const PageEntry *const> InstalledPages() {
  return Catalogue().Pages();
}

const PageEntry *FindLookupPage(const TableDef &table) {
  if (table.lookupPageId.Value() != 0) {
    if (const PageEntry *declared = FindPage(table.lookupPageId); declared != nullptr) {
      return declared;
    }
  }
  const PageEntry *any = nullptr;
  for (const PageEntry *entry : Catalogue().Pages()) {
    if (entry->page->source != table.id) { continue; }
    if (entry->page->type == PageType::List) { return entry; }
    if (any == nullptr) { any = entry; }
  }
  return any;
}

}
