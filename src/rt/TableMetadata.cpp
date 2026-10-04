#include "TableMetadata.h"

#include "meta/Ids.h"
#include "meta/ModuleDef.h"
#include "meta/TableDef.h"
#include "platform/TableMetadata.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "type/Guid.h"

#include "ReflectionMetadata.h"

#include <optional>
#include <string>
#include <string_view>

namespace agiru::detail {

namespace {

auto Verified(const auto &value) {
  if (!value) { throw Error(value.error()); }
  return *value;
}

std::string_view
EffectiveProperty(const TableDef &source, std::string_view value, std::string_view fallback) {
  return value.empty() && !IsPlatformTable(source.id) ? fallback : value;
}

Guid OriginalOwner(const TableDef &source) {
  if (source.module == nullptr) {
    throw Error("Table Metadata.App ID has no original module: " + std::string(source.name));
  }
  const auto owner = Guid::FromText(source.module->id);
  if (!owner || owner->IsNull()) {
    throw Error("Table Metadata.App ID has no valid original identity: " +
                std::string(source.name));
  }
  return *owner;
}

std::string CaptionFields(const TableDef &source) {
  std::string result;
  for (const FieldNo no : source.dataCaptionFields) {
    if (Field(source, no) == nullptr) {
      throw Error("Table Metadata.DataCaptionFields names an absent field: " +
                  std::to_string(no.Value()));
    }
    if (!result.empty()) { result += ','; }
    result += std::to_string(no.Value());
  }
  return result;
}

}

platform::TableMetadata_Table ProjectTableMetadata(const TableDef &source) {
  platform::TableMetadata_Table result;
  result.ID = source.id.Value();
  result.Name = source.name;
  result.Caption = source.caption;
  result.DataPerCompany = source.dataPerCompany;
  result.LookupPageID = source.lookupPageId.Value();
  result.DrillDownPageID = source.drillDownPageId.Value();
  result.DataCaptionFields = CaptionFields(source);
  result.PasteIsValid = source.pasteIsValid;
  result.LinkedObject = source.linkedObject;
  result.DataIsExternal =
      source.tableType != TableType::Normal && source.tableType != TableType::Temporary;
  result.TableType = Verified(MetadataTableType(source.tableType));
  result.ExternalName = source.externalName;
  result.ObsoleteState =
      Verified(MetadataObsoleteState(EffectiveProperty(source, source.obsoleteState, "No")));
  result.ObsoleteReason = source.obsoleteReason;
  result.DataClassification = Verified(MetadataDataClassification(
      EffectiveProperty(source, source.dataClassification, "CustomerContent")));
  result.ReplicateData = source.replicateData;
  result.CompressionType = Verified(
      MetadataCompressionType(EffectiveProperty(source, source.compressionType, "Unspecified")));
  result.AppID = OriginalOwner(source);
  result.InherentPermissions = source.inherentPermissions;
  result.InherentEntitlements = source.inherentEntitlements;
  result.Scope = Verified(MetadataScope(EffectiveProperty(source, source.scope, "Cloud")));
  result.Access = Verified(MetadataAccess(EffectiveProperty(source, source.access, "Public")));
  result.ALNamespace = source.nameSpace;
  return result;
}

std::optional<platform::TableMetadata_Table> InstalledTableMetadata(TableId id) {
  const auto *entry = FindTable(id);
  if (entry == nullptr) { return std::nullopt; }
  return ProjectTableMetadata(*entry->table);
}

}
