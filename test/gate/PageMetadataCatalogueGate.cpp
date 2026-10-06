#include "meta/Ids.h"
#include "meta/ModuleDef.h"
#include "meta/PageDef.h"
#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "platform/PageMetadata.h"
#include "platform/ReflectionTypes.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "runtime/Record.h"
#include "runtime/RecordRef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/DateTime.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Language.h"
#include "type/RecordId.h"
#include "type/Text.h"

#include "Check.h"
#include "MetadataText.h"
#include "PageMetadata.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

namespace {

using Row = agiru::platform::PageMetadata;
constexpr int kFirstId = 61110;
constexpr int kSecondId = 61120;
constexpr int kThirdId = 61130;
constexpr int kUnownedId = 61140;
constexpr int kSourceId = 61210;
constexpr int kTemporarySourceId = 61220;
constexpr int kMissingSourceId = 61230;
constexpr agiru::Integer kGermanGermany = 1031;
constexpr std::size_t kRawCount = 4;
constexpr std::int32_t kQualifiedCount = 3;
constexpr agiru::ModuleDef kOwner{.id = "118874ab-44bc-4ccb-9daf-59763539ab16",
                                  .name = "Page declarations",
                                  .publisher = "agiru tests",
                                  .version = "1.2.3.4"};
constexpr agiru::TableDef kSource{
    .id = agiru::TableId{kSourceId}, .name = "Source", .module = &kOwner};
constexpr agiru::TableDef kTemporarySource{.id = agiru::TableId{kTemporarySourceId},
                                           .name = "Temporary Source",
                                           .tableType = agiru::TableType::Temporary,
                                           .module = &kOwner};

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

constexpr agiru::TableEntry kSourceEntry = Entry(kSource);
constexpr agiru::TableEntry kTemporaryEntry = Entry(kTemporarySource);
constexpr agiru::PageDef kFirst{.id = agiru::PageId{kFirstId},
                                .name = "Same",
                                .caption = "Caption Alpha",
                                .type = agiru::PageType::List,
                                .source = kSource.id,
                                .editable = "false",
                                .cardPageId = agiru::PageId{kSecondId},
                                .module = &kOwner,
                                .nameSpace = "Microsoft.Fixture",
                                .multipleNewLines = true};
constexpr agiru::PageDef kSecond{.id = agiru::PageId{kSecondId},
                                 .name = "Same",
                                 .caption = "Caption Beta",
                                 .source = kSource.id,
                                 .module = &kOwner,
                                 .nameSpace = "Microsoft.Fixture"};
constexpr agiru::PageDef kThird{.id = agiru::PageId{kThirdId},
                                .name = "Other",
                                .type = agiru::PageType::Worksheet,
                                .source = kTemporarySource.id,
                                .insertAllowed = "FALSE",
                                .modifyAllowed = "false",
                                .deleteAllowed = "false",
                                .linksAllowed = false,
                                .showFilter = false,
                                .module = &kOwner};
constexpr agiru::PageDef kUnowned{
    .id = agiru::PageId{kUnownedId}, .name = "Unowned", .source = kSource.id};
constexpr std::array kPages{agiru::PageEntry{.page = &kFirst, .run = nullptr},
                            agiru::PageEntry{.page = &kSecond, .run = nullptr},
                            agiru::PageEntry{.page = &kThird, .run = nullptr},
                            agiru::PageEntry{.page = &kUnowned, .run = nullptr}};

std::string Failure(auto &&operation) {
  try {
    operation();
  } catch (const agiru::Error &error) { return error.what(); }
  return {};
}

void ProjectionAndOptionalReads() {
  Row row;
  row.SetRange(row.Name, "Not the read key");
  CHECK_TRUE("Page Metadata.Get uses the installed page, not SQL", row.Get(kFirstId));
  CHECK_TEXT("original Name remains independent of Caption", row.Name.Value(), "Same");
  CHECK_TEXT("declared Caption remains independent", row.Caption.Value(), "Caption Alpha");
  CHECK_TRUE("page type uses its qualified native option",
             row.PageType == agiru::platform::PageMetadataPageType::List);
  CHECK_TRUE("declared source and card page IDs remain exact",
             row.SourceTable == kSourceId && row.CardPageID == kSecondId);
  CHECK_TRUE("static false Editable and MultipleNewLines remain distinct",
             !row.Editable && row.MultipleNewLines);
  CHECK_TRUE("source-object default policies remain true",
             row.InsertAllowed && row.ModifyAllowed && row.DeleteAllowed && row.ShowFilter &&
                 row.LinksAllowed);
  CHECK_TRUE("page policy defaults are not invented state",
             !row.DelayedInsert && !row.RefreshOnActivate && !row.SaveValues && !row.AutoSplitKey &&
                 !row.PopulateAllFields && !row.ChangeTrackingAllowed);
  CHECK_TRUE("unassigned API and expression fields remain empty",
             row.APIPublisher.IsEmpty() && row.APIGroup.IsEmpty() && row.APIVersion.IsEmpty() &&
                 row.EntityName.IsEmpty() && row.EntitySetName.IsEmpty() &&
                 row.SourceTableView.IsEmpty() && row.DataCaptionFields.IsEmpty() &&
                 row.DataCaptionExpr.IsEmpty());
  CHECK_TEXT("AL namespace does not become the C++ spelling",
             row.ALNamespace.Value(),
             "Microsoft.Fixture");
  CHECK_TRUE("App ID borrows the original declaring application",
             row.AppID == *agiru::Guid::FromText(kOwner.id));
  CHECK_TRUE("page metadata uses a frozen version and its original identity encoding",
             row.SystemRowVersion == 1 &&
                 row.SystemId == *agiru::Guid::FromText("7735948a-eeb6-0000-0000-000000000000"));
  CHECK_TRUE("metadata audits remain unstamped",
             row.SystemCreatedAt == agiru::DateTime{} &&
                 row.SystemModifiedAt == agiru::DateTime{} && row.SystemCreatedBy.IsNull() &&
                 row.SystemModifiedBy.IsNull());
  CHECK_TEXT("Get preserves ordinary filters", row.GetFilter(row.Name), "Not the read key");
  CHECK_TRUE("missing optional reads return false", !row.Get(0));
  bool missing = false;
  try {
    row.Get(0);
  } catch (const agiru::Error &error) {
    missing = std::string_view(error.what()).contains("ID='0'");
  }
  CHECK_TRUE("missing statement Get reports the searched ID", missing);
  agiru::RecordRef reference;
  CHECK_TRUE("unopened RecordRef.Get shares the native page reader",
             reference.Get(agiru::RecordId{Row::kId, "Page Metadata", {std::to_string(kFirstId)}}));
  CHECK_TEXT("RecordRef.Get preserves exact typed captions",
             reference.Field(Row::Field_No::Caption.Value()).Value().Get<agiru::Text<0>>().Value(),
             "Caption Alpha");
  CHECK_TRUE("native SourceTableTemporary includes the underlying table type",
             row.Get(kThirdId) && row.SourceTableTemporary);
  CHECK_TRUE("declared false policies survive projection",
             !row.InsertAllowed && !row.ModifyAllowed && !row.DeleteAllowed && !row.ShowFilter &&
                 !row.LinksAllowed);
}

void NavigationUsesTheSharedCatalogue() {
  Row row;
  row.SetRange(row.ID, kFirstId, kUnownedId);
  CHECK_TRUE("key-only counts retain unqualified page identities",
             row.Count() == static_cast<std::int32_t>(kRawCount));
  row.SetRange(row.ID, kFirstId, kThirdId);
  CHECK_TRUE("qualified page counts retain every selected identity",
             row.Count() == kQualifiedCount && !row.IsEmpty());
  CHECK_TRUE("FindSet begins on the declared first page", row.FindSet() && row.ID == kFirstId);
  row.ID = kThirdId;
  CHECK_TRUE("buffer edits do not replace the native page bookmark",
             row.Next() == 1 && row.ID == kSecondId);
  row.SetRange(row.PageType, agiru::platform::PageMetadataPageType::Worksheet);
  CHECK_TRUE("changed views re-anchor against native option filters",
             row.Next() == 1 && row.ID == kThirdId);
  row.Reset();
  row.SetRange(row.ID, kFirstId, kThirdId);
  CHECK_TRUE("Get resets the shared native navigation anchor",
             row.Get(kFirstId) && row.Next() == 1 && row.ID == kSecondId);
  CHECK_TRUE("negative native page navigation returns the actual step count",
             row.Next(-1) == -1 && row.ID == kFirstId);
  CHECK_TRUE("extreme navigation remains bounded by the installed population",
             row.Next(std::numeric_limits<agiru::Integer>::max()) == 2 && row.ID == kThirdId);
  CHECK_TRUE("the end of the catalogue does not invent a page",
             row.Next() == 0 && row.ID == kThirdId);
  const auto *state = row.State_Block.Peek();
  CHECK_TRUE("page navigation holds neither a SQL portal nor a copied population",
             state != nullptr && state->open.Held() == nullptr && state->view.empty());
}

void QualifiedTextAndRefusals() {
  auto source = kFirst;
  const std::string caption(Row::kCaptionLength + 1, 'c');
  source.caption = caption;
  auto row = agiru::detail::ProjectPageMetadata(source);
  CHECK_TRUE("page metadata captions truncate to the declared UTF-16 width",
             row.Caption.Length() == static_cast<agiru::Integer>(Row::kCaptionLength));
  CHECK_TEXT("truncation keeps the original caption prefix",
             row.Caption.Value(),
             std::string(Row::kCaptionLength, 'c'));
  source.caption = " \t\n";
  row = agiru::detail::ProjectPageMetadata(source);
  CHECK_TEXT("blank captions fall back to the original AL name", row.Caption.Value(), "Same");
  CHECK_TEXT("BMP Unicode truncation is not byte truncation",
             agiru::detail::MetadataText("ä中é", 2),
             "ä中");
  CHECK_TEXT("complete surrogate pairs count as two UTF-16 units",
             agiru::detail::MetadataText("😀AB", 3),
             "😀A");
  bool split = false;
  try {
    static_cast<void>(agiru::detail::MetadataText("😀AB", 1));
  } catch (const agiru::Error &error) {
    split = std::string_view(error.what()).contains("unsupported UTF-16 surrogate boundary");
  }
  CHECK_TRUE("isolated-surrogate truncation refuses instead of changing Unicode data", split);
  CHECK_TEXT("zero-width metadata remains empty", agiru::detail::MetadataText("ä中", 0), "");
  CHECK_TEXT(
      "exact UTF-16 boundaries preserve complete text", agiru::detail::MetadataText("😀", 2), "😀");
  for (const auto member : {&agiru::PageDef::sourceTableView,
                            &agiru::PageDef::dataCaptionExpression,
                            &agiru::PageDef::dataCaptionFields,
                            &agiru::PageDef::editable,
                            &agiru::PageDef::inherentPermissions,
                            &agiru::PageDef::inherentEntitlements}) {
    source = kFirst;
    source.*member = "Unqualified";
    bool refused = false;
    try {
      static_cast<void>(agiru::detail::ProjectPageMetadata(source));
    } catch (const agiru::Error &error) {
      refused = std::string_view(error.what()).contains("no qualified declaration projection");
    }
    CHECK_TRUE("unqualified properties never project fabricated defaults", refused);
  }
  bool unowned = false;
  try {
    static_cast<void>(Row{}.Get(kUnownedId));
  } catch (const agiru::Error &error) {
    unowned = std::string_view(error.what()).contains("no original module");
  }
  CHECK_TRUE("unowned registered pages refuse rather than reporting a missing page", unowned);
  Row retained;
  CHECK_TRUE("a qualified page is available before a refused read", retained.Get(kFirstId));
  CHECK_TRUE("refused projection changes only the caller-supplied search key",
             Failure([&] {
               static_cast<void>(retained.Get(kUnownedId));
             }).contains("no original module") &&
                 retained.ID == kUnownedId && retained.Name.Value() == "Same" &&
                 retained.Caption.Value() == "Caption Alpha");
  agiru::Temporary<Row> temporary;
  temporary.ID = 0;
  temporary.Name = "Writable temporary";
  temporary.Insert();
  CHECK_TRUE("temporary page metadata does not consult the live catalogue",
             temporary.Get(0) && temporary.Count() == 1);
  Row live;
  bool writable = false;
  try {
    static_cast<void>(live.Insert());
  } catch (const agiru::Error &error) {
    writable = std::string_view(error.what()).contains("read-only live catalogue");
  }
  CHECK_TRUE("live page metadata cannot be persisted as a SQL replacement", writable);
}

void DeclaredPoliciesDoNotBecomeDefaults() {
  auto source = kFirst;
  source.type = agiru::PageType::HeadlinePart;
  source.delayedInsert = true;
  source.refreshOnActivate = true;
  source.saveValues = true;
  source.autoSplitKey = true;
  source.populateAllFields = true;
  source.changeTrackingAllowed = true;
  const auto row = agiru::detail::ProjectPageMetadata(source);
  CHECK_TRUE("HeadlinePart is its native member rather than the declaration ordinal",
             row.PageType == agiru::platform::PageMetadataPageType::HeadlinePart);
  CHECK_TRUE("declared delayed insert is not an omitted default", row.DelayedInsert);
  CHECK_TRUE("declared refresh policy is not an omitted default", row.RefreshOnActivate);
  CHECK_TRUE("declared save-values policy is not an omitted default", row.SaveValues);
  CHECK_TRUE("declared auto-split-key policy is not an omitted default", row.AutoSplitKey);
  CHECK_TRUE("declared populate-all-fields policy is not an omitted default",
             row.PopulateAllFields);
  CHECK_TRUE("declared change-tracking policy is not an omitted default",
             row.ChangeTrackingAllowed);
}

void LocalizedCaptionsRemainCounted() {
  const auto original = agiru::Language::Current();
  agiru::Language::MakeCurrent(kGermanGermany);
  const auto refusal =
      Failure([] { static_cast<void>(agiru::detail::ProjectPageMetadata(kFirst)); });
  agiru::Language::MakeCurrent(original);
  CHECK_TRUE("unqualified locale reads never substitute untranslated captions",
             refusal.contains("localized Caption"));
}

void UnicodeBlankCaptions() {
  constexpr std::array<std::string_view, 25> whitespace{
      "\t",     "\n",     "\v",     "\f",     "\r",     " ",      "\u0085", "\u00A0", "\u1680",
      "\u2000", "\u2001", "\u2002", "\u2003", "\u2004", "\u2005", "\u2006", "\u2007", "\u2008",
      "\u2009", "\u200A", "\u2028", "\u2029", "\u202F", "\u205F", "\u3000"};
  auto source = kFirst;
  std::string mixed;
  for (const auto character : whitespace) {
    source.caption = character;
    CHECK_TEXT("every .NET whitespace caption falls back to its original name",
               agiru::detail::ProjectPageMetadata(source).Caption.Value(),
               source.name);
    mixed += character;
  }
  source.caption = mixed;
  CHECK_TEXT("mixed Unicode whitespace captions retain the name fallback",
             agiru::detail::ProjectPageMetadata(source).Caption.Value(),
             source.name);
  constexpr std::array<std::string_view, 9> nonblank{
      "\u180E", "\u200B", "\u200C", "\u200D", "\u2060", "\uFEFF", "\b", "😀", "中"};
  for (const auto character : nonblank) {
    const auto padded = std::string("\u00A0") + std::string(character) + "\u3000";
    source.caption = padded;
    CHECK_TEXT("non-whitespace Unicode captions preserve every original byte",
               agiru::detail::ProjectPageMetadata(source).Caption.Value(),
               source.caption);
  }
  source.caption = std::string_view("\0", 1);
  CHECK_TEXT("embedded NUL is caption data rather than whitespace",
             agiru::detail::ProjectPageMetadata(source).Caption.Value(),
             source.caption);
}

void ProjectionBoundsAndIntegrity() {
  auto source = kSecond;
  const std::string name(Row::kCaptionLength + 1, 'n');
  source.name = name;
  source.caption = {};
  auto row = agiru::detail::ProjectPageMetadata(source);
  CHECK_TEXT("Name truncation uses the original name width",
             row.Name.Value(),
             std::string(Row::kNameLength, 'n'));
  CHECK_TEXT("caption fallback precedes its own wider truncation",
             row.Caption.Value(),
             std::string(Row::kCaptionLength, 'n'));
  source = kFirst;
  source.caption = "  padded caption  ";
  row = agiru::detail::ProjectPageMetadata(source);
  CHECK_TEXT("nonblank captions preserve original padding", row.Caption.Value(), source.caption);
  const std::string apiName(Row::kApiNameLength + 1, 'a');
  const std::string expression(Row::kExpressionLength + 1, 'v');
  source.type = agiru::PageType::Api;
  source.apiPublisher = apiName;
  source.apiGroup = apiName;
  source.apiVersion = expression;
  source.entityName = expression;
  source.entitySetName = expression;
  row = agiru::detail::ProjectPageMetadata(source);
  CHECK_TRUE("API metadata uses its native option rather than the declaration ordinal",
             row.PageType == agiru::platform::PageMetadataPageType::Api);
  CHECK_TEXT("API publisher truncates independently",
             row.APIPublisher.Value(),
             apiName.substr(0, Row::kApiNameLength));
  CHECK_TEXT("API group truncates independently",
             row.APIGroup.Value(),
             apiName.substr(0, Row::kApiNameLength));
  CHECK_TEXT("API version truncates independently",
             row.APIVersion.Value(),
             expression.substr(0, Row::kExpressionLength));
  CHECK_TEXT("API entity truncates independently",
             row.EntityName.Value(),
             expression.substr(0, Row::kExpressionLength));
  CHECK_TEXT("API entity set truncates independently",
             row.EntitySetName.Value(),
             expression.substr(0, Row::kExpressionLength));
  source = kSecond;
  source.sourceTableTemporary = true;
  source.editable = "TrUe";
  source.insertAllowed = "TRUE";
  row = agiru::detail::ProjectPageMetadata(source);
  CHECK_TRUE("declared temporary source and case-folded true policies survive",
             row.SourceTableTemporary && row.Editable && row.InsertAllowed);
  source.source = agiru::TableId{};
  CHECK_TRUE("unknown SourceObject presence remains a counted refusal",
             Failure([&] {
               static_cast<void>(agiru::detail::ProjectPageMetadata(source));
             }).contains("SourceObject presence"));
  source.source = agiru::TableId{kMissingSourceId};
  CHECK_TRUE("missing source declarations never imply a normal table",
             Failure([&] {
               static_cast<void>(agiru::detail::ProjectPageMetadata(source));
             }).contains("SourceTable declaration"));
  source = kSecond;
  source.type = agiru::PageType::ReportPreview;
  CHECK_TRUE("unqualified page kinds do not become native Card",
             Failure([&] {
               static_cast<void>(agiru::detail::ProjectPageMetadata(source));
             }).contains("PageType"));
  source = kFirst;
  const agiru::ModuleDef invalid{.id = "00000000-0000-0000-0000-000000000000",
                                 .name = "Invalid",
                                 .publisher = {},
                                 .version = {}};
  source.module = &invalid;
  CHECK_TRUE("null original app identities remain a refusal",
             Failure([&] {
               static_cast<void>(agiru::detail::ProjectPageMetadata(source));
             }).contains("no valid original identity"));
  for (const std::string_view version : {"", "'v1.0'", "v1.0,v2.0"}) {
    source = kFirst;
    source.type = agiru::PageType::Api;
    source.apiVersion = version;
    CHECK_TRUE("unqualified compiled API version formats never fabricate beta",
               Failure([&] {
                 static_cast<void>(agiru::detail::ProjectPageMetadata(source));
               }).contains("APIVersion"));
  }
  auto incompatible = agiru::TableTraits<Row>::kTable;
  incompatible.fields = {};
  CHECK_TRUE("native page reader rejects incompatible fields before touching the buffer",
             Failure([&] {
               static_cast<void>(agiru::detail::GetInstalledPageMetadata(nullptr, incompatible));
             }).contains("qualified native field binding"));
}

void ViewsAndReadOnlyBoundaries() {
  Row row;
  row.SetRange(row.ID, kFirstId, kThirdId);
  row.SetFilter(row.ID, "%1|%2", kFirstId, kThirdId);
  CHECK_TRUE("page key filters retain holes in a narrowed interval", row.Count() == 2);
  row.SetRange(row.ID, kFirstId, kThirdId);
  row.SetRange(row.Name, "Same");
  CHECK_TRUE("page ordinary predicates use original names", row.Count() == 2);
  row.FilterGroup(-1);
  row.SetRange(row.Caption, "Caption Alpha");
  row.SetRange(row.PageType, agiru::platform::PageMetadataPageType::Card);
  CHECK_TRUE("page cross-column predicates retain their union", row.Count() == 2);
  row.FilterGroup(0);
  row.Reset();
  row.SetRange(row.ID, kFirstId, kThirdId);
  CHECK_TRUE("page metadata can sort by a nonindexed exact caption",
             row.SetCurrentKey(row.Caption));
  row.Ascending(false);
  CHECK_TRUE("descending caption sort preserves its actual first page",
             row.FindFirst() && row.ID == kThirdId);
  Row independent;
  independent.SetRange(independent.ID, kFirstId, kThirdId);
  CHECK_TRUE("a second page handle starts with an independent bookmark",
             independent.FindFirst() && independent.ID == kFirstId);
  CHECK_TRUE("descending page navigation preserves its original view",
             row.Next() == 1 && row.ID == kSecondId);
  CHECK_TRUE("other handle does not inherit descending order",
             independent.Next() == 1 && independent.ID == kSecondId);
  independent.Mark(true);
  CHECK_TRUE("marking a page does not move its cursor",
             independent.Next() == 1 && independent.ID == kThirdId);
  independent.MarkedOnly(true);
  CHECK_TRUE("page metadata marks use native keys",
             independent.Count() == 1 && independent.FindFirst() && independent.ID == kSecondId);
  agiru::RecordRef reference;
  reference.Open(Row::kId.Value());
  reference.Field(Row::Field_No::ID.Value()).SetRange(kFirstId, kThirdId);
  CHECK_TRUE("RecordRef uses the same installed page population",
             reference.Count() == kQualifiedCount && reference.FindSet());
  CHECK_TEXT("RecordRef navigation starts on the same exact ID",
             reference.Field(Row::Field_No::ID.Value()).ToText(),
             std::to_string(kFirstId));
  CHECK_TRUE("RecordRef advances through the shared page reader", reference.Next() == 1);
  CHECK_TEXT("RecordRef navigation retains the original caption",
             reference.Field(Row::Field_No::Caption.Value()).ToText(),
             "Caption Beta");
  row.Reset();
  row.SetRange(row.ID, 0);
  CHECK_TRUE("empty native page ModifyAll remains read-only",
             Failure([&] {
               row.ModifyAll(row.Name, "No replacement");
             }).contains("read-only live catalogue"));
  CHECK_TRUE("empty native page triggered DeleteAll remains read-only",
             Failure([&] { row.DeleteAll(true); }).contains("read-only live catalogue"));
  CHECK_TRUE("unqualified native page SystemId reads refuse rather than consulting SQL",
             Failure([&] {
               static_cast<void>(row.GetBySystemId(
                   *agiru::Guid::FromText("7735948a-eeb6-0000-0000-000000000000")));
             }).contains("read-only live catalogue"));
}

}

int main() {
  return gate::Run("PageMetadataCatalogue", [] {
    agiru::RegisterTableEntry(&kSourceEntry);
    agiru::RegisterTableEntry(&kTemporaryEntry);
    for (const auto &page : kPages) { agiru::RegisterPageEntry(&page); }
    ProjectionAndOptionalReads();
    NavigationUsesTheSharedCatalogue();
    QualifiedTextAndRefusals();
    DeclaredPoliciesDoNotBecomeDefaults();
    LocalizedCaptionsRemainCounted();
    UnicodeBlankCaptions();
    ProjectionBoundsAndIntegrity();
    ViewsAndReadOnlyBoundaries();
  });
}
