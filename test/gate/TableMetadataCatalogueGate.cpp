#include "meta/Ids.h"
#include "meta/ModuleDef.h"
#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "platform/ReflectionTypes.h"
#include "platform/TableMetadata.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordRef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Guid.h"

#include "Check.h"
#include "TableMetadata.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

namespace {

using agiru::platform::TableMetadata;

constexpr std::int32_t kFirstId = 60110;
constexpr std::int32_t kSecondId = 60120;
constexpr std::int32_t kThirdId = 60130;
constexpr std::int32_t kUnownedId = 60140;
constexpr std::int32_t kQualifiedCount = 3;
constexpr std::int32_t kRawCount = 4;
constexpr agiru::ModuleDef kModule{.id = "118874ab-44bc-4ccb-9daf-59763539ab16",
                                   .name = "Catalogue fixture",
                                   .publisher = "agiru tests",
                                   .version = "1.0.0.0"};

constexpr agiru::TableDef Source(std::int32_t id,
                                 std::string_view name,
                                 std::string_view caption,
                                 std::string_view nameSpace) {
  agiru::TableDef result{.id = agiru::TableId{id}, .name = name, .caption = caption};
  result.module = id == kUnownedId ? nullptr : &kModule;
  result.nameSpace = nameSpace;
  if (id == kSecondId) { result.obsoleteState = "Pending"; }
  if (id == kThirdId) {
    result.obsoleteState = "Removed";
    result.tableType = agiru::TableType::Temporary;
  }
  return result;
}

constexpr std::array kSources{Source(kFirstId, "Same", "Caption Alpha", "Agiru.First"),
                              Source(kSecondId, "Same", "Caption Beta", "Agiru.Second"),
                              Source(kThirdId, "Zulu", "Caption Gamma", "Agiru.Third"),
                              Source(kUnownedId, "Unowned", "No provenance", "Agiru.Unowned")};

constexpr agiru::TableEntry Entry(const agiru::TableDef &source) {
  return {.table = &source,
          .make = nullptr,
          .free = nullptr,
          .validate = nullptr,
          .copy = nullptr,
          .insert = nullptr,
          .modify = nullptr,
          .remove = nullptr,
          .rename = nullptr};
}

constexpr std::array kEntries{
    Entry(kSources[0]), Entry(kSources[1]), Entry(kSources[2]), Entry(kSources[3])};

std::string Failure(auto &&operation) {
  try {
    operation();
  } catch (const agiru::Error &error) { return error.what(); }
  return {};
}

void CountsAndNavigationUseTheInstalledRegistry() {
  TableMetadata row;
  CHECK_TRUE("live Table Metadata is not temporary", !row.IsTemporary());
  CHECK_TRUE("unfiltered count preserves every installed identity",
             static_cast<std::size_t>(row.Count()) == agiru::InstalledTables().size());
  row.SetRange(row.ID, kFirstId, kUnownedId);
  CHECK_TRUE("key-only counts retain unqualified source identities", row.Count() == kRawCount);
  row.SetRange(row.ID, kFirstId, kThirdId);
  CHECK_TRUE("live metadata count respects declared ID ranges", row.Count() == kQualifiedCount);
  CHECK_TRUE("live metadata IsEmpty uses the same catalogue", !row.IsEmpty());
  CHECK_TRUE("live metadata FindSet requires no SQL snapshot", row.FindSet() && row.ID == kFirstId);
  CHECK_TEXT("live metadata keeps Name distinct from Caption", row.Name.Value(), "Same");
  CHECK_TEXT("live metadata retains the independent Caption", row.Caption.Value(), "Caption Alpha");
  const auto *state = row.State_Block.Peek();
  CHECK_TRUE("live metadata stores neither a copied row catalogue nor a SQL portal",
             state != nullptr && state->view.empty() && state->open.Held() == nullptr);
  CHECK_TRUE("live metadata signed navigation lands on actual installed rows",
             row.Next(2) == 2 && row.ID == kThirdId);
  CHECK_TRUE("live metadata keeps exact typed source values",
             row.TableType == agiru::platform::TableMetadataTableType::Temporary &&
                 row.ObsoleteState == agiru::platform::TableMetadataObsoleteState::Removed);
  CHECK_TRUE("native metadata retains frozen version and stable identity",
             row.SystemRowVersion == 1 && !row.SystemId.IsNull());
  CHECK_TRUE("live metadata Count and Next zero preserve the current row",
             row.Count() == kQualifiedCount && row.Next(0) == 0 && row.ID == kThirdId);
  CHECK_TRUE("extreme reverse steps stop at the real metadata beginning",
             row.Next(std::numeric_limits<std::int32_t>::min()) == -2 && row.ID == kFirstId);
  row.Ascending(false);
  CHECK_TRUE("descending metadata FindFirst selects the last installed key",
             row.FindFirst() && row.ID == kThirdId);
  CHECK_TRUE("descending metadata Next reverses primary order",
             row.Next() == 1 && row.ID == kSecondId);
  row.SetRange(row.ID, 0);
  CHECK_TRUE("missing metadata ranges return empty without fabricated rows",
             row.Count() == 0 && row.IsEmpty() && !row.FindSet() && row.Next() == 0);
  row.SetFilter(row.ID, "%1|%2", kFirstId, kThirdId);
  CHECK_TRUE("metadata key filters retain holes in a narrowed interval", row.Count() == 2);
}

void FiltersMarksOrderingAndBookmarksShareTheRecordContract() {
  TableMetadata row;
  row.SetRange(row.ID, kThirdId);
  CHECK_TRUE("metadata Get starts with a qualified prior bookmark",
             row.FindFirst() && row.ID == kThirdId);
  CHECK_TRUE("metadata Get is filter-blind", row.Get(kFirstId));
  CHECK_TRUE("metadata Next after Get anchors the actual read key",
             row.Next() == 1 && row.ID == kThirdId);
  row.Reset();
  row.SetRange(row.ID, kFirstId, kThirdId);
  CHECK_TRUE("metadata FindFirst starts the filtered native cursor", row.FindFirst());
  row.ID = kSecondId;
  CHECK_TRUE("metadata buffer edits preserve an unchanged cursor bookmark",
             row.Next() == 1 && row.ID == kSecondId);
  row.SetRange(row.ID, kThirdId);
  CHECK_TRUE("changed metadata filters re-anchor the next read",
             row.Next() == 1 && row.ID == kThirdId);
  row.Mark(true);
  row.SetRange(row.ID, kFirstId, kThirdId);
  row.MarkedOnly(true);
  CHECK_TRUE("metadata Count and Find share variable-local marks",
             row.Count() == 1 && row.FindFirst() && row.ID == kThirdId);
  CHECK_TRUE("metadata Next retains marked selection", row.Next() == 0);
  row.MarkedOnly(false);
  row.FilterGroup(-1);
  row.SetRange(row.ID, kFirstId);
  row.SetRange(row.Caption, "Caption Beta");
  row.FilterGroup(0);
  CHECK_TRUE("metadata cross-column OR retains normal-group intersections", row.Count() == 2);
  row.Reset();
  row.SetRange(row.ID, kFirstId, kThirdId);
  row.SetRange(row.ObsoleteState, agiru::platform::TableMetadataObsoleteState::Removed);
  CHECK_TRUE("metadata property filtering preserves native option members",
             row.Count() == 1 && row.FindFirst() && row.ID == kThirdId);
  row.Reset();
  row.SetView("SORTING(Name,ID) ORDER(Ascending) WHERE(ID=FILTER(60110..60130))");
  row.SetAscending(row.ID, false);
  CHECK_TRUE("metadata nonindexed mixed ordering keeps primary-key ties",
             row.FindFirst() && row.ID == kSecondId);
  CHECK_TRUE("metadata mixed ordering walks through equal names deterministically",
             row.Next() == 1 && row.ID == kFirstId && row.Next() == 1 && row.ID == kThirdId);
  CHECK_TRUE("invalid metadata Find forms refuse", !Failure([&] { row.Find("++"); }).empty());
}

void ReflectedReadsAndWritableTemporaryRowsStaySeparate() {
  TableMetadata row;
  row.SetRange(row.ID, kFirstId, kThirdId);
  agiru::RecordRef reflected;
  reflected.GetTable(row);
  CHECK_TRUE("RecordRef metadata Count shares typed cardinality", reflected.Count() == row.Count());
  CHECK_TRUE("RecordRef metadata FindSet uses the live provider", reflected.FindSet());
  reflected.SetTable(row);
  CHECK_TRUE("RecordRef metadata reads preserve exact ID and app identity",
             row.ID == kFirstId && row.AppID == agiru::Guid(kModule.id));
  CHECK_TRUE("RecordRef metadata Next shares actual signed movement", reflected.Next(2) == 2);
  reflected.SetTable(row);
  CHECK_TEXT("RecordRef metadata navigation projects the complete row",
             row.Caption.Value(),
             "Caption Gamma");
  CHECK_TRUE("native metadata insertion refuses", !Failure([&] { row.Insert(); }).empty());
  CHECK_TRUE("native metadata modification refuses", !Failure([&] { row.Modify(); }).empty());
  CHECK_TRUE("native metadata deletion refuses", !Failure([&] { row.Delete(); }).empty());
  row.SetRange(row.ID, 0);
  CHECK_TRUE("native empty metadata DeleteAll refuses", !Failure([&] { row.DeleteAll(); }).empty());
  CHECK_TRUE("native empty triggered metadata DeleteAll refuses",
             !Failure([&] { row.DeleteAll(true); }).empty());
  CHECK_TRUE("native empty metadata ModifyAll refuses",
             !Failure([&] { row.ModifyAll(row.Caption, "Changed"); }).empty());
  row.SetRange(row.ID, kUnownedId);
  const std::string before(row.Caption.Value());
  CHECK_TRUE("unqualified metadata projection refuses without overwriting the caller",
             Failure([&] { static_cast<void>(row.FindFirst()); }).contains("original module") &&
                 row.Caption.Value() == before);
  auto unbound = agiru::platform::kTableMetadataTable;
  unbound.fields = {};
  CHECK_TRUE("metadata navigation checks native binding before accessing a buffer",
             Failure([&] {
               static_cast<void>(agiru::detail::FindInstalledTableMetadata(nullptr, unbound, "-"));
             }).contains("qualified native field binding"));
  agiru::Temporary<TableMetadata> temporary;
  temporary.ID = 0;
  temporary.Name = "Temporary";
  temporary.Caption = "Writable";
  temporary.Insert();
  CHECK_TRUE("temporary metadata zero keys remain independently writable",
             temporary.Count() == 1 && temporary.FindSet() && temporary.ID == 0);
  temporary.ModifyAll(temporary.Caption, "Changed");
  CHECK_TRUE("temporary metadata bulk writes retain their ordinary contract",
             temporary.FindFirst() && temporary.Caption.Value() == "Changed");
  temporary.DeleteAll();
  CHECK_TRUE("temporary deletion does not remove installed declarations", temporary.IsEmpty());
  row.SetRange(row.ID, kFirstId, kThirdId);
  CHECK_TRUE("installed metadata survives temporary writes", row.Count() == kQualifiedCount);
}

}

int main() {
  for (const auto &entry : kEntries) { agiru::RegisterTableEntry(&entry); }
  return gate::Run("TableMetadataCatalogue", [] {
    CountsAndNavigationUseTheInstalledRegistry();
    FiltersMarksOrderingAndBookmarksShareTheRecordContract();
    ReflectedReadsAndWritableTemporaryRowsStaySeparate();
  });
}
