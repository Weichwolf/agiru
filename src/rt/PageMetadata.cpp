#include "PageMetadata.h"

#include "meta/ModuleDef.h"
#include "meta/PageDef.h"
#include "meta/TableDef.h"
#include "meta/TableType.h"
#include "platform/PageMetadata.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Guid.h"
#include "type/Language.h"
#include "type/StringValue.h"

#include "MetadataSystemId.h"
#include "MetadataText.h"
#include "ReflectionMetadata.h"

#include <cctype>
#include <optional>
#include <string>
#include <string_view>

namespace agiru::detail {

namespace {

using Row = platform::PageMetadata_Table;
constexpr auto kFrozenCatalogueRowVersion = 1;

[[noreturn]] void Refuse(std::string_view property, const PageDef &source) {
  throw Error("Page Metadata." + std::string(property) +
              " has no qualified declaration projection: " + std::string(source.name));
}

bool StaticBoolean(std::string_view value, std::string_view property, const PageDef &source) {
  if (value.empty()) { return true; }
  std::string normalized(value);
  for (char &character : normalized) {
    character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
  }
  if (normalized == "true") { return true; }
  if (normalized == "false") { return false; }
  Refuse(property, source);
}

Guid OriginalOwner(const PageDef &source) {
  if (source.module == nullptr) {
    throw Error("Page Metadata.App ID has no original module: " + std::string(source.name));
  }
  const auto owner = Guid::FromText(source.module->id);
  if (!owner || owner->IsNull()) {
    throw Error("Page Metadata.App ID has no valid original identity: " + std::string(source.name));
  }
  return *owner;
}

const TableDef &SourceTable(const PageDef &source) {
  if (source.source.Value() <= 0) { Refuse("SourceObject presence", source); }
  const auto *entry = FindTable(source.source);
  if (entry == nullptr) { Refuse("SourceTable declaration", source); }
  return *entry->table;
}

void RequireQualifiedProperties(const PageDef &source) {
  if (Language::Current() != Language::kEnglishUnitedStates) {
    Refuse("localized Caption", source);
  }
  if (!source.sourceTableView.empty()) { Refuse("SourceTableView", source); }
  if (!source.dataCaptionExpression.empty()) { Refuse("DataCaptionExpr.", source); }
  if (!source.dataCaptionFields.empty()) { Refuse("DataCaptionFields", source); }
  if (!source.inherentPermissions.empty()) { Refuse("InherentPermissions", source); }
  if (!source.inherentEntitlements.empty()) { Refuse("InherentEntitlements", source); }
  if (source.apiVersion.find_first_of(",'\"") != std::string_view::npos ||
      (source.type == PageType::Api && source.apiVersion.empty())) {
    Refuse("APIVersion", source);
  }
}

}

platform::PageMetadata_Table ProjectPageMetadata(const PageDef &source) {
  RequireQualifiedProperties(source);
  const auto owner = OriginalOwner(source);
  const auto type = MetadataPageType(source.type);
  if (!type) { throw Error(type.error()); }
  const auto &table = SourceTable(source);
  Row row;
  row.ID = source.id.Value();
  row.Name = MetadataText(source.name, Row::kNameLength);
  const auto caption =
      TrimText(source.caption, TrimSides::Both, {}).empty() ? source.name : source.caption;
  row.Caption = MetadataText(caption, Row::kCaptionLength);
  row.Editable = StaticBoolean(source.editable, "Editable", source);
  row.PageType = *type;
  row.CardPageID = source.cardPageId.Value();
  row.RefreshOnActivate = source.refreshOnActivate;
  row.APIPublisher = MetadataText(source.apiPublisher, Row::kApiNameLength);
  row.APIGroup = MetadataText(source.apiGroup, Row::kApiNameLength);
  row.APIVersion = MetadataText(source.apiVersion, Row::kExpressionLength);
  row.EntitySetName = MetadataText(source.entitySetName, Row::kExpressionLength);
  row.EntityName = MetadataText(source.entityName, Row::kExpressionLength);
  row.SourceTable = source.source.Value();
  row.InsertAllowed = StaticBoolean(source.insertAllowed, "InsertAllowed", source);
  row.ModifyAllowed = StaticBoolean(source.modifyAllowed, "ModifyAllowed", source);
  row.DeleteAllowed = StaticBoolean(source.deleteAllowed, "DeleteAllowed", source);
  row.DelayedInsert = source.delayedInsert;
  row.ShowFilter = source.showFilter;
  row.MultipleNewLines = source.multipleNewLines;
  row.SaveValues = source.saveValues;
  row.AutoSplitKey = source.autoSplitKey;
  row.SourceTableTemporary = source.sourceTableTemporary || table.tableType == TableType::Temporary;
  row.LinksAllowed = source.linksAllowed;
  row.ChangeTrackingAllowed = source.changeTrackingAllowed;
  row.PopulateAllFields = source.populateAllFields;
  row.AppID = owner;
  row.InherentPermissions = source.inherentPermissions;
  row.InherentEntitlements = source.inherentEntitlements;
  row.ALNamespace = source.nameSpace;
  row.SystemId = MetadataSystemId(Row::kId, source.id.Value());
  row.SystemRowVersion = kFrozenCatalogueRowVersion;
  return row;
}

bool IsInstalledPageMetadataProvider(const TableDef &table) {
  if (table.id != Row::kId) { return false; }
  if (table.fields.data() != platform::kPageMetadataFields.data() ||
      table.fields.size() != platform::kPageMetadataFields.size()) {
    throw Error("Page Metadata.Get requires the qualified native field binding");
  }
  return true;
}

std::optional<bool> GetInstalledPageMetadata(void *record, const TableDef &table) {
  if (!IsInstalledPageMetadataProvider(table)) { return std::nullopt; }
  const auto &buffer = *static_cast<Row *>(record);
  if (buffer.ID <= 0) { return false; }
  const auto *entry = FindPage(PageId{buffer.ID});
  if (entry == nullptr) { return false; }
  const auto row = ProjectPageMetadata(*entry->page);
  for (const FieldDef &field : table.fields) {
    if (Stored(field)) { SetFieldText(record, field, StorageText(&row, field)); }
  }
  auto &state = reinterpret_cast<StateHandle *>(record)->Ensure();
  state.open.Forget();
  state.positioned = true;
  state.viewDirty = true;
  return true;
}

}
