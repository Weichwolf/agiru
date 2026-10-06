#include "meta/Ids.h"
#include "meta/ModuleDef.h"
#include "meta/PageDef.h"
#include "meta/ProfileDef.h"
#include "meta/TableDef.h"
#include "platform/Integer.h"
#include "runtime/Catalogue.h"
#include "runtime/Codeunit.h"
#include "runtime/RecordRef.h"
#include "runtime/TableDefinition.h"

#include "Check.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

namespace {

// Authored catalogue identities; the same number is legal in separate object kinds.
constexpr int kFirst = 600001;
constexpr int kLast = 600003;
constexpr int kMissing = 600002;
constexpr int kLate = 600004;
constexpr agiru::TableDef kFirstTable{.id = agiru::TableId{kFirst}, .name = "First Table"};
constexpr agiru::TableDef kLastTable{.id = agiru::TableId{kLast}, .name = "Last Table"};
constexpr agiru::TableDef kLateTable{.id = agiru::TableId{kLate}, .name = "Late Table"};

constexpr agiru::TableEntry MetadataEntry(const agiru::TableDef &table) {
  agiru::TableEntry entry{};
  entry.table = &table;
  return entry;
}

constexpr auto kFirstEntry = MetadataEntry(kFirstTable);
constexpr auto kLastEntry = MetadataEntry(kLastTable);
constexpr auto kLateEntry = MetadataEntry(kLateTable);
constexpr agiru::PageDef kFirstPage{.id = agiru::PageId{kFirst},
                                    .name = "First Page",
                                    .type = agiru::PageType::Card,
                                    .source = kFirstTable.id};
constexpr agiru::PageDef kLastPage{.id = agiru::PageId{kLast},
                                   .name = "Last Page",
                                   .type = agiru::PageType::List,
                                   .source = kFirstTable.id};
constexpr agiru::PageEntry kFirstPageEntry{.page = &kFirstPage, .run = nullptr};
constexpr agiru::PageEntry kLastPageEntry{.page = &kLastPage, .run = nullptr};
constexpr agiru::CodeunitEntry kFirstCodeunit{
    .id = agiru::CodeunitId{kFirst}, .name = "First Codeunit", .run = nullptr};
constexpr agiru::CodeunitEntry kLastCodeunit{
    .id = agiru::CodeunitId{kLast}, .name = "Last Codeunit", .run = nullptr};
constexpr agiru::ProfileDef kFirstProfile{.profileId = "First Profile"};
constexpr agiru::ProfileDef kLastProfile{.profileId = "Last Profile"};

using Native = agiru::platform::Integer;
constexpr agiru::ModuleDef kNativeModule{.id = "12345678-1234-1234-1234-123456789abc",
                                         .name = "Authored Native",
                                         .publisher = "agiru tests",
                                         .version = "1.0.0.0"};
constexpr agiru::TableDef kNativeSource = [] {
  auto table = agiru::TableTraits<Native>::kTable;
  table.caption = "Authored Native Caption";
  table.module = &kNativeModule;
  table.nameSpace = "Authored.Native";
  table.scope = "Cloud";
  return table;
}();
constexpr auto &kNativeSourceEntry = agiru::kSourceTableEntry<Native, kNativeSource>;

template <typename Body> void Refuses(Body body, std::string_view diagnostic) {
  std::string said;
  try {
    body();
  } catch (const std::logic_error &error) { said = error.what(); }
  CHECK_TRUE("catalogue refuses invalid composition", said.find(diagnostic) != std::string::npos);
}

void RegisterInReverseOrder() {
  agiru::RegisterTableEntry(&kNativeSourceEntry);
  agiru::RegisterTableEntry(&kLastEntry);
  agiru::RegisterTableEntry(&kFirstEntry);
  agiru::RegisterPageEntry(&kLastPageEntry);
  agiru::RegisterPageEntry(&kFirstPageEntry);
  agiru::RegisterCodeunitEntry(&kLastCodeunit);
  agiru::RegisterCodeunitEntry(&kFirstCodeunit);
  agiru::RegisterProfileEntry(&kLastProfile);
  agiru::RegisterProfileEntry(&kFirstProfile);
}

void NativeSourceIsCanonical() {
  const auto *entry = agiru::FindTable(kNativeSource.id);
  CHECK_TRUE("native source qualifies the existing ABI once", entry == &kNativeSourceEntry);
  if (entry == nullptr) { return; }
  CHECK_TRUE("native ID has one installed row",
             std::ranges::count_if(agiru::InstalledTables(), [](const auto *installed) {
               return installed->table->id == kNativeSource.id;
             }) == 1);
  CHECK_TRUE("native typed definition borrows canonical declaration",
             &agiru::TableDefinition<Native>() == &kNativeSource);
  CHECK_TRUE("runtime primitive resolves the same ABI declaration",
             &agiru::detail::InstalledTableDefinition(&agiru::TableTraits<Native>::kTable) ==
                 &kNativeSource);
  CHECK_TRUE("original module owner is available", entry->table->module != nullptr);
  if (entry->table->module != nullptr) {
    CHECK_TEXT("original module owner retained", entry->table->module->id, kNativeModule.id);
  }
  CHECK_TEXT("original AL namespace retained", entry->table->nameSpace, "Authored.Native");
  Native record;
  CHECK_TEXT(
      "typed operations consume canonical caption", record.TableCaption(), kNativeSource.caption);
  agiru::RecordRef reflected;
  reflected.GetTable(record);
  CHECK_TRUE("reflected native definition borrows canonical declaration",
             reflected.TableDefinition() == &kNativeSource);
  CHECK_TRUE("reflected native records retain their actual class",
             reflected.As<Native>() != nullptr);
  CHECK_TEXT(
      "reflected operations consume canonical caption", reflected.Caption(), kNativeSource.caption);
  CHECK_TRUE("compile-time ABI declaration remains separate",
             agiru::TableTraits<Native>::kTable.module == nullptr);
}

void InvalidSourcesRefuse() {
  Refuses([] { static_cast<void>(agiru::detail::InstalledTableDefinition(nullptr)); },
          "null table ABI binding");
  auto badSource = kNativeSource;
  auto entry = kNativeSourceEntry;
  entry.table = &badSource;
  badSource.module = nullptr;
  Refuses([&] { agiru::RegisterTableEntry(&entry); }, "complete original module identity");
  badSource = kNativeSource;
  badSource.id = agiru::TableId{kFirst};
  Refuses([&] { agiru::RegisterTableEntry(&entry); }, "record ABI binding");
  badSource = kNativeSource;
  badSource.name = "Different Native";
  Refuses([&] { agiru::RegisterTableEntry(&entry); }, "record ABI binding");
  badSource = kNativeSource;
  badSource.fields = {};
  Refuses([&] { agiru::RegisterTableEntry(&entry); }, "record ABI binding");
  badSource = kNativeSource;
  badSource.keys = {};
  Refuses([&] { agiru::RegisterTableEntry(&entry); }, "record ABI binding");
}

void InvalidEntriesRefuse() {
  Refuses([] { agiru::RegisterTableEntry(nullptr); }, "null installed table");
  Refuses([] { agiru::RegisterPageEntry(nullptr); }, "null installed page");
  Refuses([] { agiru::RegisterCodeunitEntry(nullptr); }, "null installed codeunit");
  Refuses([] { agiru::RegisterProfileEntry(nullptr); }, "null installed profile");
  constexpr agiru::TableEntry missingTable{};
  constexpr agiru::PageEntry missingPage{};
  Refuses([&] { agiru::RegisterTableEntry(&missingTable); }, "without a declaration");
  Refuses([&] { agiru::RegisterPageEntry(&missingPage); }, "without a declaration");
}

void FreezeFrom(std::string_view reader) {
  if (reader == "table") {
    static_cast<void>(agiru::FindTable(kFirstTable.id));
  } else if (reader == "name") {
    static_cast<void>(agiru::FindTable("first table"));
  } else if (reader == "page") {
    static_cast<void>(agiru::FindPage(kFirstPage.id));
  } else if (reader == "codeunit") {
    static_cast<void>(agiru::FindCodeunit(kFirstCodeunit.id));
  } else if (reader == "profile") {
    static_cast<void>(agiru::InstalledProfiles());
  } else if (reader == "tables") {
    static_cast<void>(agiru::InstalledTables());
  } else if (reader == "pages") {
    static_cast<void>(agiru::InstalledPages());
  } else if (reader == "codeunits") {
    static_cast<void>(agiru::InstalledCodeunits());
  } else if (reader == "lookup") {
    static_cast<void>(agiru::FindLookupPage(kFirstTable));
  } else {
    throw std::invalid_argument("Unknown catalogue reader");
  }
}

void LateRegistrationsRefuse() {
  Refuses([] { agiru::RegisterTableEntry(&kLateEntry); }, "frozen; cannot register table");
  Refuses([] { agiru::RegisterPageEntry(&kFirstPageEntry); }, "frozen; cannot register page");
  Refuses([] { agiru::RegisterCodeunitEntry(&kFirstCodeunit); },
          "frozen; cannot register codeunit");
  Refuses([] { agiru::RegisterProfileEntry(&kFirstProfile); }, "frozen; cannot register profile");
}

void InstalledLookupIsSortedAndShared() {
  const auto tables = agiru::InstalledTables();
  const auto pages = agiru::InstalledPages();
  const auto codeunits = agiru::InstalledCodeunits();
  const auto profiles = agiru::InstalledProfiles();
  CHECK_TRUE("all installed tables sorted",
             std::ranges::is_sorted(
                 tables, {}, [](const auto *entry) { return entry->table->id.Value(); }));
  CHECK_TRUE("all installed pages sorted", std::ranges::is_sorted(pages, {}, [](const auto *entry) {
               return entry->page->id.Value();
             }));
  CHECK_TRUE(
      "all installed codeunits sorted",
      std::ranges::is_sorted(codeunits, {}, [](const auto *entry) { return entry->id.Value(); }));
  CHECK_TRUE("table ID reaches original declaration",
             agiru::FindTable(kFirstTable.id) == &kFirstEntry);
  CHECK_TRUE("last table ID reaches original declaration",
             agiru::FindTable(kLastTable.id) == &kLastEntry);
  CHECK_TRUE("table name remains case insensitive",
             agiru::FindTable("FIRST TABLE") == &kFirstEntry);
  CHECK_TRUE("same ID in page kind remains distinct",
             agiru::FindPage(kFirstPage.id) == &kFirstPageEntry);
  CHECK_TRUE("same ID in codeunit kind remains distinct",
             agiru::FindCodeunit(kFirstCodeunit.id) == &kFirstCodeunit);
  CHECK_TRUE("missing table is not nearest", agiru::FindTable(agiru::TableId{kMissing}) == nullptr);
  CHECK_TRUE("missing page is not nearest", agiru::FindPage(agiru::PageId{kMissing}) == nullptr);
  CHECK_TRUE("missing codeunit is not nearest",
             agiru::FindCodeunit(agiru::CodeunitId{kMissing}) == nullptr);
  CHECK_TRUE("missing name remains absent", agiru::FindTable("Absent Table") == nullptr);
  CHECK_TRUE("lookup prefers list over earlier card",
             agiru::FindLookupPage(kFirstTable) == &kLastPageEntry);
  auto declaredLookup = kFirstTable;
  declaredLookup.lookupPageId = kFirstPage.id;
  CHECK_TRUE("declared lookup wins", agiru::FindLookupPage(declaredLookup) == &kFirstPageEntry);
  CHECK_TRUE("profiles retain registration order",
             profiles.size() >= 2 && profiles[profiles.size() - 2] == &kLastProfile &&
                 profiles.back() == &kFirstProfile);
  CHECK_TRUE("table views borrow one stable catalogue",
             tables.data() == agiru::InstalledTables().data());
  CHECK_TRUE("page views borrow one stable catalogue",
             pages.data() == agiru::InstalledPages().data());
  CHECK_TRUE("codeunit views borrow one stable catalogue",
             codeunits.data() == agiru::InstalledCodeunits().data());
}

void ConcurrentFirstReadersAgree() {
  constexpr std::size_t kWorkers = 8;
  constexpr std::size_t kReads = 1000;
  std::atomic<bool> ready{false};
  std::array<bool, kWorkers> correct{};
  std::array<std::thread, kWorkers> workers;
  for (std::size_t i = 0; i < workers.size(); ++i) {
    workers[i] = std::thread([&, i] {
      while (!ready.load(std::memory_order_acquire)) { std::this_thread::yield(); }
      bool same = true;
      for (std::size_t read = 0; read < kReads; ++read) {
        same = same && agiru::FindTable(kFirstTable.id) == &kFirstEntry &&
               agiru::FindPage(kFirstPage.id) == &kFirstPageEntry &&
               agiru::FindCodeunit(kFirstCodeunit.id) == &kFirstCodeunit &&
               agiru::FindTable(kNativeSource.id) == &kNativeSourceEntry &&
               &agiru::TableDefinition<Native>() == &kNativeSource;
      }
      correct[i] = same;
    });
  }
  ready.store(true, std::memory_order_release);
  for (auto &worker : workers) { worker.join(); }
  for (const bool same : correct) {
    CHECK_TRUE("concurrent readers share complete metadata", same);
  }
}

void DuplicateIdentityRefuses(std::string_view kind) {
  RegisterInReverseOrder();
  if (kind == "table") {
    agiru::RegisterTableEntry(&kFirstEntry);
  } else if (kind == "page") {
    agiru::RegisterPageEntry(&kFirstPageEntry);
  } else if (kind == "codeunit") {
    agiru::RegisterCodeunitEntry(&kFirstCodeunit);
  } else {
    throw std::invalid_argument("Unknown duplicate object kind");
  }
  const auto diagnostic =
      "Duplicate installed " + std::string(kind) + " ID " + std::to_string(kFirst);
  Refuses([] { static_cast<void>(agiru::InstalledProfiles()); }, diagnostic);
  Refuses([] { static_cast<void>(agiru::InstalledTables()); }, diagnostic);
  Refuses([] { static_cast<void>(agiru::FindPage(kFirstPage.id)); }, diagnostic);
}

void DuplicateNativeSourceRefuses(std::string_view composition) {
  RegisterInReverseOrder();
  auto unrelated = agiru::kTableEntry<Native>;
  auto otherSource = kNativeSourceEntry;
  if (composition == "source") {
    agiru::RegisterTableEntry(&kNativeSourceEntry);
  } else if (composition == "base") {
    agiru::RegisterTableEntry(&agiru::kTableEntry<Native>);
  } else if (composition == "unrelated") {
    agiru::RegisterTableEntry(&unrelated);
  } else if (composition == "conflict") {
    agiru::RegisterTableEntry(&otherSource);
  } else {
    throw std::invalid_argument("Unknown native source composition");
  }
  const auto diagnostic =
      "Duplicate installed table ID " + std::to_string(kNativeSource.id.Value());
  Refuses([] { static_cast<void>(agiru::InstalledTables()); }, diagnostic);
  Refuses([] { static_cast<void>(agiru::FindTable(kNativeSource.id)); }, diagnostic);
  Refuses([] { static_cast<void>(agiru::InstalledProfiles()); }, diagnostic);
}

}

int main(int argc, char **argv) {
  return gate::Run("Catalogue", [&] {
    if (argc == 3 && std::string_view(argv[1]) == "duplicate") {
      DuplicateIdentityRefuses(argv[2]);
      return;
    }
    if (argc == 3 && std::string_view(argv[1]) == "native-duplicate") {
      DuplicateNativeSourceRefuses(argv[2]);
      return;
    }
    InvalidEntriesRefuse();
    InvalidSourcesRefuse();
    RegisterInReverseOrder();
    if (argc == 2) {
      FreezeFrom(argv[1]);
    } else if (argc == 1) {
      ConcurrentFirstReadersAgree();
    } else {
      throw std::invalid_argument("Invalid catalogue gate arguments");
    }
    LateRegistrationsRefuse();
    InstalledLookupIsSortedAndShared();
    NativeSourceIsCanonical();
  });
}
