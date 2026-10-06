#include "meta/Ids.h"
#include "meta/ModuleDef.h"
#include "meta/TableDef.h"
#include "platform/Field.h"
#include "platform/ReflectionOptions.h"
#include "platform/ReflectionTypes.h"
#include "platform/TableMetadata.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "runtime/Record.h"
#include "runtime/Table.h"
#include "runtime/TableDefinition.h"
#include "type/Guid.h"
#include "type/Integer.h"

#include "Check.h"
#include "TableMetadata.h"
#include "fixture/table/ComposedRow.h"
#include "fixture/table/ImplicitRow.h"
#include "group/fixture/table/OwnedRow.h"
#include "group/fixture/table/SecondOwnedRow.h"
#include "orphan/fixture/table/UnownedRow.h"

#include <string_view>

namespace {

using Row = agiru::Fixture::ImplicitRow_Table;
constexpr agiru::Integer kFirstKey = 7;

void CheckKeys() {
  const auto &table = agiru::TableTraits<Row>::kTable;
  CHECK_TRUE("extension keys never replace the implicit primary key", table.keys.size() == 2);
  if (table.keys.empty()) { return; }
  const auto &primary = table.keys.front();
  CHECK_TEXT("the primary key retains the original AL field name", primary.name, "Primary ID");
  CHECK_TRUE("the primary key belongs to the base table, not the lower-ID extension field",
             primary.fields.size() == 1 && primary.fields.front() == agiru::FieldNo{10});
  CHECK_TRUE("the primary key is clustered by default", primary.clustered);
  if (table.keys.size() == 2) {
    CHECK_TEXT("the extension retains its secondary key name", table.keys[1].name, "ExtensionKey");
    CHECK_TRUE("the extension key keeps its own field",
               table.keys[1].fields.size() == 1 &&
                   table.keys[1].fields.front() == agiru::FieldNo{1});
    CHECK_TRUE("an extension key is not implicitly clustered", !table.keys[1].clustered);
  }
  if (primary.fields.size() != 1 || primary.fields.front() != agiru::FieldNo{10}) { return; }
  agiru::Temporary<Row> row;
  row.PrimaryID = kFirstKey;
  row.EarlierExtension = "same";
  row.Later = "first";
  row.Insert();
  row.PrimaryID = 3;
  row.Later = "second";
  row.Insert();
  CHECK_TRUE("the implicit primary key retrieves a stored record",
             static_cast<bool>(row.Get(kFirstKey)));
  CHECK_TEXT("Get restores the record selected by the primary ID", row.Later.Value(), "first");
  CHECK_TRUE("a missing primary key stays missing", !row.Get(8));
  bool refused = false;
  try {
    row.PrimaryID = kFirstKey;
    row.Insert();
  } catch (const agiru::Error &) { refused = true; }
  CHECK_TRUE("temporary duplicate detection uses the implicit primary key", refused);
  CHECK_TRUE("duplicate refusal leaves the row count unchanged", row.Count() == 2);
  CHECK_TRUE("primary-order navigation finds the lowest ID first",
             static_cast<bool>(row.FindFirst()));
  CHECK_TRUE("primary-order navigation ignores secondary text and declaration order",
             row.PrimaryID == 3);
}

void CheckSourceIdentity() {
  const auto &table = agiru::TableTraits<Row>::kTable;
  CHECK_TRUE("table metadata retains the original app identity", table.module != nullptr);
  if (table.module == nullptr) { return; }
  CHECK_TEXT("table owner comes from the source manifest",
             table.module->id,
             "118874ab-44bc-4ccb-9daf-59763539ab16");
  CHECK_TEXT("the original app name is not the configured output name",
             table.module->name,
             "Table Declaration Fixture");
  CHECK_TEXT("the original app publisher is retained", table.module->publisher, "agiru tests");
  CHECK_TEXT("the original app version is retained", table.module->version, "1.0.0.0");
  CHECK_TEXT(
      "table namespace is AL spelling, not C++ spelling", table.nameSpace, "Microsoft.Fixture");
  CHECK_TEXT("table name is independent of the caption", table.name, "Implicit Row");
  CHECK_TEXT("table caption is retained separately", table.caption, "Different table caption");
  CHECK_TEXT("table scope is retained from source", table.scope, "Cloud");
  CHECK_TEXT(
      "table obsolete reason is retained", table.obsoleteReason, "Source-owned reflection fixture");
  CHECK_TEXT("table classification is retained", table.dataClassification, "AccountData");
  CHECK_TRUE("an absent linked-object declaration keeps the documented default",
             !table.linkedObject);
  CHECK_TRUE("a compilation unit owns components despite their development manifests",
             table.module == agiru::TableTraits<agiru::Fixture::ComposedRow_Table>::kTable.module);
}

void CheckSourceProjection() {
  using namespace agiru::platform;
  const auto row = agiru::detail::ProjectTableMetadata(agiru::TableDefinition<Row>());
  CHECK_TRUE("production source ID reaches the typed projection", row.ID == Row::kId.Value());
  CHECK_TEXT(
      "production AL name survives the runtime projection", row.Name.Value(), "Implicit Row");
  CHECK_TEXT(
      "production caption stays independent", row.Caption.Value(), "Different table caption");
  CHECK_TEXT("production caption fields are original numeric IDs in declared order",
             row.DataCaptionFields.Value(),
             "30,10");
  CHECK_TRUE("production scope and access reach their original native options",
             row.Scope == TableMetadataScope::Cloud && row.Access == TableMetadataAccess::Internal);
  CHECK_TRUE("production classification reaches its original native option",
             row.DataClassification == FieldDataClassification::AccountData);
  CHECK_TRUE("production obsolete and compression properties remain typed",
             row.ObsoleteState == TableMetadataObsoleteState::Pending &&
                 row.CompressionType == TableMetadataCompressionType::Row);
  CHECK_TRUE("production false flags do not revert to runtime defaults",
             !row.DataPerCompany && !row.ReplicateData && !row.PasteIsValid);
  CHECK_TRUE("production lookup and drilldown IDs survive",
             row.LookupPageID == 50176 && row.DrillDownPageID == 50177);
  CHECK_TRUE("production declaring app GUID reaches the runtime projection",
             row.AppID == *agiru::Guid::FromText("118874ab-44bc-4ccb-9daf-59763539ab16"));
  TableMetadata installed;
  CHECK_TRUE("generated declarations are readable through native Table Metadata.Get",
             installed.Get(Row::kId.Value()));
  CHECK_TRUE("generated metadata retains the frozen provider version",
             installed.SystemRowVersion == 1);
  for (const auto &field : kTableMetadataFields) {
    if (!agiru::Stored(field)) { continue; }
    CHECK_TEXT("generated installed metadata Get matches every projected stored field",
               agiru::detail::StorageText(&installed, field),
               agiru::detail::StorageText(&row, field));
  }
}

void CheckFieldCustomizationOwnership() {
  using namespace agiru::platform;
  Field field;
  const auto check = [&](agiru::FieldNo no, std::string_view expected, bool allowed) {
    const auto *declared = agiru::Field(agiru::TableDefinition<Row>(), no);
    CHECK_TRUE("a generated customization field is declared", declared != nullptr);
    if (declared == nullptr) { return; }
    CHECK_TEXT("generated customization retains the declaring owner or field override",
               declared->allowInCustomizations,
               expected);
    CHECK_TRUE("generated customization is readable through installed Field.Get",
               field.Get(Row::kId.Value(), no.Value()));
    CHECK_TRUE("generated customization availability reaches the native Field projection",
               field.IsAllowedInCustomizations == allowed);
  };
  check(Row::Field_No::Later, "Never", false);
  check(Row::Field_No::PrimaryID, "AsReadOnly", true);
  check(Row::Field_No::EarlierExtension, "AsReadWrite", true);
  check(Row::Field_No::PrivateExtension, "Never", false);
  check(Row::Field_No::UnclassifiedExtension, "", true);
  using Defaults = agiru::Fixture::ComposedRow_Table;
  CHECK_TRUE("an omitted owner default does not invent a source property",
             agiru::Field(agiru::TableDefinition<Defaults>(), Defaults::Field_No::ID)
                 ->allowInCustomizations.empty());
  CHECK_TRUE("an omitted owner default is readable through installed Field.Get",
             field.Get(Defaults::kId.Value(), 1));
  CHECK_TRUE("unclassified fields are available for customization",
             field.IsAllowedInCustomizations);
}

void CheckOmittedTableProperties() {
  using namespace agiru::platform;
  using Defaults = agiru::Fixture::ComposedRow_Table;
  const auto &source = agiru::TableDefinition<Defaults>();
  CHECK_TRUE("the production transpiler preserves omitted table properties",
             source.dataClassification.empty() && source.access.empty() &&
                 source.compressionType.empty() && source.obsoleteState.empty() &&
                 source.scope.empty());
  const auto row = agiru::detail::ProjectTableMetadata(source);
  CHECK_TRUE("generated ordinary tables use the compiled AL classification default",
             row.DataClassification == FieldDataClassification::CustomerContent);
  CHECK_TRUE("generated ordinary tables use Public access and Unspecified compression",
             row.Access == TableMetadataAccess::Public &&
                 row.CompressionType == TableMetadataCompressionType::Unspecified);
  CHECK_TRUE("generated ordinary tables use Cloud scope and No obsoletion",
             row.Scope == TableMetadataScope::Cloud &&
                 row.ObsoleteState == TableMetadataObsoleteState::No);
  CHECK_TRUE("generated Boolean defaults agree with compiled AL metadata",
             row.DataPerCompany && row.ReplicateData && row.PasteIsValid && !row.LinkedObject);
  CHECK_TRUE("effective defaults preserve the original app owner",
             row.AppID == *agiru::Guid::FromText(source.module->id));
}

void CheckInstalledLookup() {
  const auto catalogue = agiru::InstalledTables();
  const auto row = agiru::detail::InstalledTableMetadata(Row::kId);
  CHECK_TRUE("installed lookup selects the production declaration by original ID", row.has_value());
  if (!row) { return; }
  CHECK_TRUE("installed lookup projects only the requested original identity",
             row->ID == Row::kId.Value());
  CHECK_TEXT(
      "installed lookup retains source names, not captions", row->Name.Value(), "Implicit Row");
  CHECK_TEXT("installed lookup retains source caption field numbers",
             row->DataCaptionFields.Value(),
             "30,10");
  CHECK_TRUE("installed lookup preserves the original owning app GUID",
             row->AppID == *agiru::Guid::FromText("118874ab-44bc-4ccb-9daf-59763539ab16"));
  CHECK_TRUE("missing IDs do not create metadata rows",
             !agiru::detail::InstalledTableMetadata(agiru::TableId{0}));
  bool refused = false;
  try {
    static_cast<void>(agiru::detail::InstalledTableMetadata(agiru::Fixture::UnownedRow_Table::kId));
  } catch (const agiru::Error &error) {
    refused = std::string_view(error.what()).contains("Table Metadata.App ID");
  }
  CHECK_TRUE("an installed declaration without an owner stays a named refusal", refused);
  refused = false;
  try {
    static_cast<void>(agiru::detail::InstalledTableMetadata(agiru::platform::TableMetadata::kId));
  } catch (const agiru::Error &error) {
    refused = std::string_view(error.what()) ==
              "Table Metadata.App ID has no original module: Table Metadata";
  }
  CHECK_TRUE("an unqualified native declaration cannot fabricate its original app owner", refused);
  const auto after = agiru::InstalledTables();
  CHECK_TRUE("metadata reads reuse the one frozen catalogue without changing it",
             catalogue.data() == after.data() && catalogue.size() == after.size());
}

void CheckNestedIdentity() {
  using Nested = agiru::Fixture::OwnedRow_Table;
  const auto &table = agiru::TableTraits<Nested>::kTable;
  CHECK_TRUE("a nested manifest supplies an immutable table owner", table.module != nullptr);
  if (table.module == nullptr) { return; }
  CHECK_TEXT("nested tables belong to the nearest source manifest, not its parent or dependency",
             table.module->id,
             "834a40c9-7a26-46f2-9348-3f6cc8c71719");
  CHECK_TEXT("source identity strings are decoded before C++ emission",
             table.module->name,
             "Nested \"Café\" Fixture");
  CHECK_TEXT("nested source version is not the parent version", table.module->version, "2.3.4.5");
  CHECK_TRUE("an extension from the parent app does not replace the base owner",
             agiru::Field(table, agiru::FieldNo{2}) != nullptr);
  const auto *moved = agiru::Field(table, agiru::FieldNo{3});
  CHECK_TRUE("a takeover validates the nested source app rather than the output bucket",
             moved != nullptr && moved->obsoleteState.empty());
  CHECK_TRUE("distinct source apps never share the parent module pointer",
             table.module != agiru::TableTraits<Row>::kTable.module);
  CHECK_TRUE("tables of one nested app share one immutable module definition",
             table.module ==
                 agiru::TableTraits<agiru::Fixture::SecondOwnedRow_Table>::kTable.module);
  CHECK_TRUE("an app without a manifest never inherits an owner from outside its configured root",
             agiru::TableTraits<agiru::Fixture::UnownedRow_Table>::kTable.module == nullptr);
}

}

int main() {
  return gate::Run("GeneratedTableKeys", [] {
    CheckKeys();
    CheckSourceIdentity();
    CheckSourceProjection();
    CheckFieldCustomizationOwnership();
    CheckOmittedTableProperties();
    CheckNestedIdentity();
    CheckInstalledLookup();
  });
}
