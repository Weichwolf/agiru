#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/SystemFields.h"
#include "meta/TableDef.h"
#include "platform/Company.h"
#include "platform/Field.h"
#include "platform/ReflectionOptions.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "runtime/Record.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/FieldClass.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Variant.h"

#include "Check.h"
#include "FieldMetadata.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

using agiru::FieldType;
using agiru::RecordRef;
using agiru::Temporary;
using agiru::platform::Field;
using agiru::platform::FieldClass;
using agiru::platform::FieldDataType;

namespace {

/// AL table 27 is `Item` and its field 1 is `"No."`. The numbers are AL's, so they are named.
constexpr agiru::Integer kItem = 27;
constexpr agiru::Integer kItemNo = 1;
constexpr agiru::Integer kItemDescription = 3;
constexpr agiru::Integer kNoLength = 20;
constexpr agiru::Integer kDescriptionLength = 100;

constexpr agiru::TableId kMetadataFixtureId{50151};
constexpr agiru::TableId kMetadataTargetId{50152};
constexpr agiru::TableId kLongMetadataId{50153};
constexpr agiru::TableId kTypeMetadataId{50154};
constexpr agiru::TableId kPolicyMetadataId{50156};
constexpr agiru::TableId kCustomizationMetadataId{50157};
constexpr agiru::Integer kTypeNameField = 9;
constexpr FieldType kUnsupportedType = static_cast<FieldType>(250);
constexpr agiru::Integer kNativeCodeOrdinal = 31489;
constexpr agiru::Integer kNativeFieldRefCodeOrdinal = 31490;
constexpr agiru::Integer kMissingMetadataField = 99;
constexpr agiru::Integer kMissingMetadataTable = 50155;

struct ReadFailure {
  std::string message;
  std::string code;
};

ReadFailure CaptureReadFailure(auto &&read) {
  try {
    read();
  } catch (const agiru::Error &error) {
    return {.message = error.what(), .code = std::string(error.Code())};
  }
  return {};
}

void MetadataGetPreservesOptionalFailureContext() {
  Field row;
  row.SetRange(row.No, 2);
  row.Get(kMetadataFixtureId.Value(), 1);
  CHECK_TRUE("native Get ignores filters when finding its primary key", row.No == 1);
  CHECK_TEXT("native Get does not reset filters", row.GetFilter(row.No), "2");
  const bool found = row.Get(kMetadataFixtureId.Value(), kMissingMetadataField);
  CHECK_TRUE("a consumed native missing-field Get answers false", !found);
  CHECK_TRUE("a consumed native missing-table Get answers false",
             !row.Get(kMissingMetadataTable, 1));
  const ReadFailure missingField =
      CaptureReadFailure([&] { row.Get(kMetadataFixtureId.Value(), kMissingMetadataField); });
  CHECK_TEXT("a discarded native missing-field Get carries its searched key",
             missingField.message,
             "The Field does not exist. Identification fields and values: 50151, 99");
  CHECK_TEXT("a discarded native missing-field Get carries the record error code",
             missingField.code,
             "DB:RecordNotFound");
  const ReadFailure missingTable = CaptureReadFailure([&] { row.Get(kMissingMetadataTable, 1); });
  CHECK_TEXT("a discarded native missing-table Get carries its searched key",
             missingTable.message,
             "The Field does not exist. Identification fields and values: 50155, 1");
  CHECK_TEXT("a discarded native missing-table Get carries the record error code",
             missingTable.code,
             "DB:RecordNotFound");
}

void MovedReadFailuresPreserveOwnedKeyText() {
  const ReadFailure missing = CaptureReadFailure([] {
    agiru::detail::Found source{false, "Field", "50151, 99"};
    const agiru::detail::Found moved{std::move(source)};
  });
  CHECK_TEXT("moved read failure retains exact key text",
             missing.message,
             "The Field does not exist. Identification fields and values: 50151, 99");
  CHECK_TEXT("moved read failure retains the record error code", missing.code, "DB:RecordNotFound");
  const ReadFailure consumed = CaptureReadFailure([] {
    agiru::detail::Found source{false, "Field", "50151, 99"};
    agiru::detail::Found moved{std::move(source)};
    CHECK_TRUE("a moved consumed failure remains false", !static_cast<bool>(moved));
  });
  CHECK_SILENT("moving a consumed read result does not duplicate its assertion", consumed.message);
}

void MetadataGetDefaultsAndTemporaryReadsShareTheContract() {
  Field native;
  CHECK_TRUE("native Get defaults an omitted field number to an absent zero key",
             !native.Get(agiru::platform::Company::kId.Value()));
  CHECK_TRUE("the omitted native key retains its searched zero value", native.No == 0);
  const ReadFailure zero =
      CaptureReadFailure([&] { native.Get(agiru::platform::Company::kId.Value(), 0); });
  CHECK_TEXT("a discarded native timestamp catalogue lookup names its searched key",
             zero.message,
             "The Field does not exist. Identification fields and values: 2000000006, 0");
  CHECK_TEXT("a discarded native timestamp catalogue lookup retains its error code",
             zero.code,
             "DB:RecordNotFound");
  CHECK_TRUE("native Get defaults all omitted keys to zero", !native.Get());
  const ReadFailure empty = CaptureReadFailure([&] { native.Get(); });
  CHECK_TEXT("a discarded native default-key read names zero keys",
             empty.message,
             "The Field does not exist. Identification fields and values: 0, 0");

  Temporary<Field> rows;
  rows.TableNo = kMetadataFixtureId.Value();
  rows.No = 0;
  rows.FieldName = "Stored zero key";
  rows.Insert();
  rows.No = kMissingMetadataField;
  CHECK_TRUE("temporary Get defaults an omitted trailing key",
             rows.Get(kMetadataFixtureId.Value()));
  CHECK_TEXT("temporary Get reads its own rows, not installed metadata",
             rows.FieldName.Value(),
             "Stored zero key");
  CHECK_TRUE("a consumed temporary missing-field Get answers false",
             !rows.Get(kMetadataFixtureId.Value(), kMissingMetadataField));
  const ReadFailure missing =
      CaptureReadFailure([&] { rows.Get(kMetadataFixtureId.Value(), kMissingMetadataField); });
  CHECK_TEXT("a discarded temporary missing-field Get carries the searched key",
             missing.message,
             "The Field does not exist. Identification fields and values: 50151, 99");
  CHECK_TEXT("a discarded temporary missing-field Get carries the record error code",
             missing.code,
             "DB:RecordNotFound");
}

void TypedAndReflectedGetShareInstalledFields() {
  Field typed;
  typed.SetRange(typed.No, kMissingMetadataField);
  CHECK_TRUE("typed installed metadata is readable without a database session",
             typed.Get(kMetadataFixtureId.Value(), 1));
  RecordRef reference;
  reference.GetTable(typed);
  const auto originalView = reference.GetView();
  typed.Init();
  typed.TableNo = kMetadataFixtureId.Value();
  typed.No = 2;
  CHECK_TRUE("RecordRef Get reads installed fields rather than a SQL copy",
             reference.Get(typed.RecordId()));
  reference.SetTable(typed);
  CHECK_TRUE("RecordRef Get lands on the searched field", typed.No == 2);
  CHECK_TEXT("RecordRef Get retains the installed source name", typed.FieldName, "Flow filter");
  CHECK_TEXT("RecordRef Get preserves the caller's filters", reference.GetView(), originalView);

  Field expected;
  CHECK_TRUE("typed Get reads the same source field", expected.Get(kMetadataFixtureId.Value(), 2));
  const auto &definition = agiru::TableTraits<Field>::kTable;
  for (const auto &field : definition.fields) {
    if (!agiru::Stored(field)) { continue; }
    CHECK_TEXT("typed and RecordRef Get preserve the same exact field values",
               agiru::detail::StorageText(&typed, field),
               agiru::detail::StorageText(&expected, field));
  }

  typed.TableNo = agiru::platform::Company::kId.Value();
  typed.No = 0;
  CHECK_TRUE("RecordRef Get excludes timestamp from the native catalogue",
             !reference.Get(typed.RecordId()));
  CHECK_TRUE("typed Get excludes negative catalogue field keys",
             !typed.Get(agiru::platform::Company::kId.Value(), -1));
  const auto identity = agiru::SystemFieldNumbers::SystemId.Value();
  CHECK_TRUE("positive reserved system fields remain in the native catalogue",
             typed.Get(agiru::platform::Company::kId.Value(), identity));
  CHECK_TEXT("native system identity retains its reflected name", typed.FieldName, "$systemId");

  agiru::platform::Company company;
  reference.GetTable(company);
  CHECK_TRUE("timestamp zero remains directly addressable through FieldRef",
             reference.Field(0).Number() == 0);

  Temporary<Field> rows;
  rows.TableNo = kMetadataFixtureId.Value();
  rows.No = 0;
  rows.FieldName = "Stored zero key";
  rows.Insert();
  reference.GetTable(rows);
  CHECK_TRUE("RecordRef temporary Get retains its zero-key row", reference.Get(rows.RecordId()));
  reference.SetTable(rows);
  CHECK_TEXT("RecordRef temporary Get does not substitute installed metadata",
             rows.FieldName,
             "Stored zero key");
}

void NativeGetRequiresItsQualifiedFieldBinding() {
  Field row;
  auto definition = agiru::TableTraits<Field>::kTable;
  definition.fields = {};
  const ReadFailure unqualified = CaptureReadFailure([&] {
    const auto found = agiru::detail::GetInstalledFieldMetadata(&row, definition);
    CHECK_TRUE("an unqualified binding does not become a missing row", !found.has_value());
  });
  CHECK_TEXT("native Get refuses an unqualified field binding before accessing its buffer",
             unqualified.message,
             "Field.Get requires the qualified native field binding");
  const auto ordinary = agiru::detail::GetInstalledFieldMetadata(
      nullptr, agiru::TableTraits<agiru::platform::Company>::kTable);
  CHECK_TRUE("the Field reader leaves ordinary table providers unchanged", !ordinary.has_value());
}

struct TypeNameCase {
  FieldType type;
  std::uint16_t length;
  std::string_view name;
  agiru::Integer effectiveLength;
};

constexpr std::array<TypeNameCase, 21> kTypeNames{{
    {.type = FieldType::Boolean, .length = 0, .name = "Boolean", .effectiveLength = 4},
    {.type = FieldType::Integer, .length = 0, .name = "Integer", .effectiveLength = 4},
    {.type = FieldType::BigInteger, .length = 0, .name = "BigInteger", .effectiveLength = 8},
    {.type = FieldType::Decimal, .length = 0, .name = "Decimal", .effectiveLength = 12},
    {.type = FieldType::Option, .length = 0, .name = "Option", .effectiveLength = 4},
    {.type = FieldType::Enum, .length = 0, .name = "Option", .effectiveLength = 4},
    {.type = FieldType::Duration, .length = 0, .name = "Duration", .effectiveLength = 8},
    {.type = FieldType::Code, .length = 20, .name = "Code20", .effectiveLength = 20},
    {.type = FieldType::Text, .length = 100, .name = "Text100", .effectiveLength = 100},
    {.type = FieldType::Code, .length = 0, .name = "Code0", .effectiveLength = 0},
    {.type = FieldType::Text, .length = 2048, .name = "Text2048", .effectiveLength = 2048},
    {.type = FieldType::Date, .length = 0, .name = "Date", .effectiveLength = 4},
    {.type = FieldType::Time, .length = 0, .name = "Time", .effectiveLength = 4},
    {.type = FieldType::DateTime, .length = 0, .name = "DateTime", .effectiveLength = 8},
    {.type = FieldType::Guid, .length = 0, .name = "GUID", .effectiveLength = 16},
    {.type = FieldType::RecordId, .length = 0, .name = "RecordID", .effectiveLength = 448},
    {.type = FieldType::DateFormula, .length = 0, .name = "DateFormula", .effectiveLength = 32},
    {.type = FieldType::Blob, .length = 0, .name = "BLOB", .effectiveLength = 8},
    {.type = FieldType::TableFilter, .length = 0, .name = "TableFilter", .effectiveLength = 504},
    {.type = FieldType::MediaSet, .length = 0, .name = "MediaSet", .effectiveLength = 16},
    {.type = FieldType::Media, .length = 0, .name = "Media", .effectiveLength = 16},
}};
constexpr agiru::Integer kUnknownTypeField = static_cast<agiru::Integer>(kTypeNames.size() + 1);

constexpr auto kTypeMetadataFields = [] {
  std::array<agiru::FieldDef, kTypeNames.size() + 1> fields{};
  for (std::size_t i = 0; i < kTypeNames.size(); ++i) {
    fields[i] = {.name = kTypeNames[i].name,
                 .no = agiru::FieldNo{static_cast<agiru::Integer>(i + 1)},
                 .length = kTypeNames[i].length,
                 .type = kTypeNames[i].type};
  }
  fields[kTypeNames.size()] = {
      .name = "Unknown", .no = agiru::FieldNo{kUnknownTypeField}, .type = kUnsupportedType};
  return fields;
}();

constexpr agiru::TableDef kTypeMetadataTable{
    .id = kTypeMetadataId, .name = "Fixture Primitive Types", .fields = kTypeMetadataFields};

struct FieldPolicyCase {
  std::string_view access;
  agiru::platform::FieldAccess native;
  bool textSearch;
};

constexpr auto kFieldPolicies = std::to_array<FieldPolicyCase>({
    {.access = "", .native = agiru::platform::FieldAccess::Public, .textSearch = false},
    {.access = "Public", .native = agiru::platform::FieldAccess::Public, .textSearch = true},
    {.access = "iNtErNaL", .native = agiru::platform::FieldAccess::Internal, .textSearch = false},
    {.access = "Protected", .native = agiru::platform::FieldAccess::Protected, .textSearch = true},
    {.access = "Local", .native = agiru::platform::FieldAccess::Local, .textSearch = true},
});
constexpr agiru::Integer kUnknownPolicyField =
    static_cast<agiru::Integer>(kFieldPolicies.size() + 1);
constexpr auto kPolicyMetadataFields = [] {
  std::array<agiru::FieldDef, kFieldPolicies.size() + 1> fields{};
  for (std::size_t i = 0; i < kFieldPolicies.size(); ++i) {
    fields[i] = {.name = "Policy value",
                 .access = kFieldPolicies[i].access,
                 .no = agiru::FieldNo{static_cast<agiru::Integer>(i + 1)},
                 .type = FieldType::Text,
                 .optimizeForTextSearch = kFieldPolicies[i].textSearch};
  }
  fields[kFieldPolicies.size()] = {.name = "Unsupported access",
                                   .access = "Undocumented",
                                   .no = agiru::FieldNo{kUnknownPolicyField},
                                   .type = FieldType::Text};
  return fields;
}();
constexpr agiru::TableDef kPolicyMetadataTable{
    .id = kPolicyMetadataId, .name = "Fixture Field Policies", .fields = kPolicyMetadataFields};

struct CustomizationCase {
  std::string_view property;
  bool allowed;
};

constexpr auto kCustomizations = std::to_array<CustomizationCase>({
    {.property = "", .allowed = true},
    {.property = "ToBeClassified", .allowed = true},
    {.property = "nEvEr", .allowed = false},
    {.property = "AsReadOnly", .allowed = true},
    {.property = "AsReadWrite", .allowed = true},
    {.property = "Always", .allowed = true},
    {.property = "Never", .allowed = false},
});
constexpr agiru::Integer kUnknownCustomization =
    static_cast<agiru::Integer>(kCustomizations.size() + 1);
constexpr auto kCustomizationFields = [] {
  std::array<agiru::FieldDef, kCustomizations.size() + 1> fields{};
  for (std::size_t i = 0; i < kCustomizations.size(); ++i) {
    fields[i] = {.name = "Customization value",
                 .allowInCustomizations = kCustomizations[i].property,
                 .no = agiru::FieldNo{static_cast<agiru::Integer>(i + 1)},
                 .type = FieldType::Integer,
                 .editable = false};
  }
  fields[kCustomizations.size()] = {.name = "Unsupported customization",
                                    .allowInCustomizations = "Undocumented",
                                    .no = agiru::FieldNo{kUnknownCustomization},
                                    .type = FieldType::Integer};
  return fields;
}();
constexpr agiru::TableDef kCustomizationTable{.id = kCustomizationMetadataId,
                                              .name = "Fixture Customization Policies",
                                              .fields = kCustomizationFields};
constexpr std::string_view kLongName = "012345678901234567890123456789😀tail";
constexpr std::string_view kLongFieldName = "äääääääääääääääääääääääääääääätail";
constexpr std::array<agiru::EnumValueDef, 6> kBlankOptions{{
    {.ordinal = 0, .name = "", .caption = ""},
    {.ordinal = 1, .name = "", .caption = ""},
    {.ordinal = 2, .name = "Alpha", .caption = "Alpha"},
    {.ordinal = 3, .name = "", .caption = ""},
    {.ordinal = 4, .name = "Omega", .caption = "Omega"},
    {.ordinal = 5, .name = "", .caption = ""},
}};
constexpr std::array<agiru::EnumValueDef, 2> kEnumValues{{
    {.ordinal = 0, .name = "Zero", .caption = "Zero"},
    {.ordinal = 10, .name = "Ten", .caption = "Ten"},
}};
constexpr std::array<agiru::FieldDef, 9> kMetadataFields{{
    {.name = kLongFieldName,
     .caption = "Total amount",
     .obsoleteState = "Pending",
     .obsoleteReason = "Replaced by a calculated field",
     .externalName = "external_total",
     .no = agiru::FieldNo{1},
     .fieldClass = agiru::FieldClass::FlowField,
     .type = FieldType::Decimal,
     .enabled = false},
    {.name = "Flow filter",
     .no = agiru::FieldNo{2},
     .fieldClass = agiru::FieldClass::FlowFilter,
     .length = 20,
     .type = FieldType::Code},
    {.name = "Choice",
     .values = kBlankOptions,
     .optionOrdinalValues = "-1,10,20,30,40,50",
     .no = agiru::FieldNo{3},
     .type = FieldType::Option},
    {.name = "Enumeration",
     .values = kEnumValues,
     .no = agiru::FieldNo{4},
     .type = FieldType::Enum},
    {.name = "Unsupported state",
     .obsoleteState = "Undocumented",
     .no = agiru::FieldNo{5},
     .type = FieldType::Integer},
    {.name = "Removed",
     .obsoleteState = "Removed",
     .no = agiru::FieldNo{6},
     .type = FieldType::Integer},
    {.name = "Relation",
     .relationTable = "Fixture Metadata Target",
     .relationField = "Target No.",
     .no = agiru::FieldNo{7},
     .type = FieldType::Integer},
    {.name = "Moved",
     .obsoleteState = "Moved",
     .no = agiru::FieldNo{8},
     .type = FieldType::Integer},
    {.name = "Pending move",
     .obsoleteState = "PendingMove",
     .no = agiru::FieldNo{9},
     .type = FieldType::Integer},
}};
constexpr std::array<agiru::FieldNo, 1> kPrimaryField{agiru::FieldNo{1}};
constexpr std::array<agiru::FieldNo, 1> kSecondaryField{agiru::FieldNo{2}};
constexpr std::array<agiru::KeyDef, 2> kMetadataKeys{{
    {.name = "Primary", .fields = kPrimaryField},
    {.name = "Secondary", .fields = kSecondaryField},
}};
constexpr agiru::TableDef kMetadataTable{.id = kMetadataFixtureId,
                                         .name = "Fixture Metadata",
                                         .fields = kMetadataFields,
                                         .keys = kMetadataKeys};
constexpr agiru::TableDef kLongMetadataTable{
    .id = kLongMetadataId, .name = kLongName, .fields = kMetadataFields, .keys = kMetadataKeys};
constexpr std::array<agiru::FieldDef, 1> kTargetFields{{
    {.name = "Target No.", .no = agiru::FieldNo{11}, .type = FieldType::Integer},
}};
constexpr agiru::TableDef kMetadataTarget{
    .id = kMetadataTargetId, .name = "Fixture Metadata Target", .fields = kTargetFields};

constexpr agiru::TableEntry MetadataEntry(const agiru::TableDef &table) {
  return {.table = &table,
          .make = nullptr,
          .free = nullptr,
          .validate = nullptr,
          .copy = nullptr,
          .insert = nullptr,
          .modify = nullptr,
          .remove = nullptr,
          .rename = nullptr};
}

constexpr agiru::TableEntry kMetadataEntry = MetadataEntry(kMetadataTable);
constexpr agiru::TableEntry kTargetEntry = MetadataEntry(kMetadataTarget);
constexpr agiru::TableEntry kLongMetadataEntry = MetadataEntry(kLongMetadataTable);
constexpr agiru::TableEntry kTypeMetadataEntry = MetadataEntry(kTypeMetadataTable);
constexpr agiru::TableEntry kPolicyMetadataEntry = MetadataEntry(kPolicyMetadataTable);
constexpr agiru::TableEntry kCustomizationEntry = MetadataEntry(kCustomizationTable);

std::string ReadTypeName(Field &row) {
  RecordRef ref;
  ref.GetTable(row);
  if (!ref.FieldExist(kTypeNameField)) { return "missing Type Name"; }
  const agiru::Variant held = ref.Field(kTypeNameField).Value();
  return std::string{static_cast<std::string_view>(held)};
}

std::string ReadMetadata(Field &row, agiru::Integer no, agiru::TableId table = kMetadataFixtureId) {
  try {
    return row.Get(table.Value(), no) ? "" : "missing field";
  } catch (const agiru::Error &error) { return error.what(); }
}

void MetadataSearchAndAccessRetainDeclaredProperties() {
  Field row;
  for (std::size_t i = 0; i < kFieldPolicies.size(); ++i) {
    CHECK_TRUE("a declared field policy is readable",
               row.Get(kPolicyMetadataId.Value(), static_cast<agiru::Integer>(i + 1)));
    CHECK_TRUE("metadata access retains the declared native member",
               row.Access == kFieldPolicies[i].native);
    CHECK_TRUE("metadata text search retains the declared flag",
               row.OptimizeForTextSearch == kFieldPolicies[i].textSearch);
  }
  CHECK_TEXT("unknown field access refuses rather than granting Public",
             ReadMetadata(row, kUnknownPolicyField, kPolicyMetadataId),
             "Field.Access has no verified member 'Undocumented'");
  CHECK_TRUE("a refused policy does not replace the preceding access",
             row.Access == agiru::platform::FieldAccess::Local);
  CHECK_TRUE("a refused policy does not replace the preceding search flag",
             row.OptimizeForTextSearch);
}

void MetadataCustomizationsRetainDeclaredPolicies() {
  Field row;
  for (std::size_t i = 0; i < kCustomizations.size(); ++i) {
    CHECK_TRUE("a declared customization policy is readable",
               row.Get(kCustomizationMetadataId.Value(), static_cast<agiru::Integer>(i + 1)));
    CHECK_TRUE("metadata customization retains the declared availability",
               row.IsAllowedInCustomizations == kCustomizations[i].allowed);
  }
  const auto previous = row.No;
  CHECK_TEXT("unknown field customization refuses rather than granting availability",
             ReadMetadata(row, kUnknownCustomization, kCustomizationMetadataId),
             "Field.AllowInCustomizations has no verified member 'Undocumented'");
  CHECK_TRUE("a refused customization leaves preceding availability unchanged",
             !row.IsAllowedInCustomizations);
  CHECK_TRUE("a refused customization does not project an unsupported field name",
             row.FieldName == "Customization value");
  CHECK_TRUE("a refused customization still retains the searched primary key",
             previous != row.No && row.No == kUnknownCustomization);
}

void MetadataTypeNamesMatchTheNativePrimitiveContract() {
  Field row;
  for (std::size_t i = 0; i < kTypeNames.size(); ++i) {
    CHECK_SILENT("a primitive declaration is readable",
                 ReadMetadata(row, static_cast<agiru::Integer>(i + 1), kTypeMetadataId));
    CHECK_TEXT("Type Name uses the native primitive spelling and length",
               ReadTypeName(row),
               kTypeNames[i].name);
    CHECK_TRUE("every primitive maps to a declared native code", row.Type.IsDeclared());
    CHECK_TRUE("Len uses the original effective size, not the host wrapper or declared zero",
               row.Len == kTypeNames[i].effectiveLength);
    CHECK_TRUE("the shared primitive agrees with the native row",
               agiru::detail::EffectiveFieldLength(kTypeMetadataFields[i]) == row.Len);
    CHECK_TRUE("an internal metadata tag cannot escape the Type boundary",
               row.Type.AsInteger() != static_cast<int>(kTypeNames[i].type));
    if (kTypeNames[i].type == FieldType::Code) {
      CHECK_TRUE("Code uses the Field.Table.al ordinal",
                 row.Type.AsInteger() == kNativeCodeOrdinal);
    }
  }
  CHECK_TEXT("an unknown primitive type refuses rather than returning blank",
             ReadMetadata(row, kUnknownTypeField, kTypeMetadataId),
             "Field metadata: unsupported type name");
  CHECK_TEXT("a refused declaration preserves the previous row", ReadTypeName(row), "Media");
  CHECK_TRUE("a refused declaration preserves the previous length",
             row.Len == kTypeNames.back().effectiveLength);
  std::string said;
  try {
    (void)agiru::detail::EffectiveFieldLength(kTypeMetadataFields.back());
  } catch (const agiru::Error &error) { said = error.what(); }
  CHECK_TEXT("the shared length primitive refuses an unknown type",
             said,
             "Field metadata: unsupported length");
  const ReadFailure invalidType = CaptureReadFailure([&] {
    const bool readable = row.Get(kTypeMetadataId.Value(), kUnknownTypeField);
    static_cast<void>(readable);
  });
  CHECK_TEXT("metadata projection errors are not converted to missing-row answers",
             invalidType.message,
             "Field metadata: unsupported type name");
}

void MetadataKeepsDeclaredValuesAndRecordState() {
  Field row;
  row.SetRange(row.TableNo, kMetadataFixtureId.Value());
  CHECK_SILENT("a long table name is fitted without an assignment error",
               ReadMetadata(row, 2, kLongMetadataId));
  CHECK_TEXT("the table name never splits a surrogate pair",
             row.TableName.Value(),
             "012345678901234567890123456789");
  CHECK_SILENT("a long field name is fitted without an assignment error", ReadMetadata(row, 1));
  CHECK_TEXT("a name length counts UTF-16 units rather than UTF-8 bytes",
             row.FieldName.Value(),
             "ääääääääääääääääääääääääääääää");
  CHECK_TRUE("a calculated field retains its class", row.Class == FieldClass::FlowField);
  CHECK_TRUE("disabled is not silently enabled", !row.Enabled);
  CHECK_TRUE("the first key marks its member", row.IsPartOfPrimaryKey);
  CHECK_TRUE("obsolete state comes from the declaration",
             row.ObsoleteState == agiru::platform::ObsoleteState::Pending);
  CHECK_TEXT(
      "obsolete reason is populated", row.ObsoleteReason.Value(), "Replaced by a calculated field");
  CHECK_TEXT("external name is populated", row.ExternalName.Value(), "external_total");
  CHECK_TEXT(
      "caption does not replace the declared field name", row.FieldCaption.Value(), "Total amount");
  CHECK_TEXT("Get preserves record filters", row.GetFilter(row.TableNo), "50151");

  CHECK_SILENT("the next declaration is readable", ReadMetadata(row, 2));
  CHECK_TRUE("a filter field retains its class", row.Class == FieldClass::FlowFilter);
  CHECK_TRUE("a secondary key does not make a field primary", !row.IsPartOfPrimaryKey);
  CHECK_TRUE("the declared length is reported", row.Len == kNoLength);
  CHECK_TRUE("enabled is restored from this declaration", row.Enabled);
  CHECK_TRUE("a missing obsolete state defaults to No",
             row.ObsoleteState == agiru::platform::ObsoleteState::No);
  CHECK_TEXT("the previous obsolete reason does not leak", row.ObsoleteReason.Value(), "");
  CHECK_TEXT("the previous external name does not leak", row.ExternalName.Value(), "");
  CHECK_TEXT("an absent caption uses the field name", row.FieldCaption.Value(), "Flow filter");
}

void ImplicitNamesFollowTheRealReflectionLookupPath() {
  agiru::platform::Company source;
  source.SystemId = agiru::Guid("11111111-2222-3333-4444-555555555555");
  const std::string name{source.FieldName(source.SystemId)};
  CHECK_TEXT("Record.FieldName uses the original getter spelling", name, "$systemId");
  const auto &table = agiru::TableTraits<agiru::platform::Company>::kTable;
  const auto *identity = agiru::Field(table, agiru::SystemFieldNumbers::SystemId);
  CHECK_TRUE("the source identity declaration remains present", identity != nullptr);
  if (identity == nullptr) { return; }
  CHECK_TEXT("reflection does not rename the source/SQL declaration", identity->name, "SystemId");
  Temporary<Field> catalogue;
  for (const agiru::FieldDef &def : table.fields) {
    agiru::detail::LoadFieldMetadata(catalogue, table, def);
    catalogue.Insert();
  }
  catalogue.SetRange(catalogue.TableNo, agiru::platform::Company::kId.Value());
  catalogue.SetRange(catalogue.FieldName, name);
  CHECK_TRUE("the caller's FieldName finds its catalogue row", catalogue.FindFirst());
  CHECK_TRUE("the name resolves the original reserved number",
             catalogue.No == agiru::SystemFieldNumbers::SystemId.Value());
  RecordRef reference;
  reference.GetTable(source);
  const auto field = reference.Field(catalogue.No);
  CHECK_TEXT("FieldRef.Name shares the getter spelling", field.Name(), name);
  const auto value = field.Value();
  CHECK_TRUE("the name-derived field retains its exact type", value.IsGuid());
  CHECK_TRUE("the name-derived field reads actual member storage",
             value.IsGuid() && value.Get<agiru::Guid>() == source.SystemId);
}

void MetadataKeepsBlankOptionsAndEnumIdentityAtTheTypeBoundary() {
  Field row;
  CHECK_SILENT("an option declaration is readable", ReadMetadata(row, 3));
  CHECK_TEXT("leading, middle and trailing blank members keep their positions",
             row.OptionString.Value(),
             ",,Alpha,,Omega,");
  CHECK_TRUE("an ordinary field is Normal", row.Class == FieldClass::Normal);
  CHECK_SILENT("an enum declaration is readable", ReadMetadata(row, 4));
  CHECK_TRUE("the private Enum type code does not escape", row.Type == FieldDataType::Option);
  CHECK_TRUE("the reported type has a declared member", row.Type.IsDeclared());
  CHECK_TEXT("enum members retain declaration order", row.OptionString.Value(), "Zero,Ten");
  CHECK_SILENT("a scalar declaration is readable", ReadMetadata(row, 2));
  CHECK_TEXT("an option string does not leak into a scalar", row.OptionString.Value(), "");
}

void MetadataLoadsRelationsAndRefusesUnknownStates() {
  Field row;
  CHECK_SILENT("a relation declaration is readable", ReadMetadata(row, 7));
  CHECK_TRUE("the relation points to its installed table",
             row.RelationTableNo == kMetadataTargetId.Value());
  CHECK_TRUE("the relation points to its declared field", row.RelationFieldNo == 11);
  CHECK_SILENT("a non-relation declaration is readable", ReadMetadata(row, 2));
  CHECK_TRUE("the previous relation table does not leak", row.RelationTableNo == 0);
  CHECK_TRUE("the previous relation field does not leak", row.RelationFieldNo == 0);
  CHECK_SILENT("a removed declaration is still metadata", ReadMetadata(row, 6));
  CHECK_TRUE("a removed field does not become No",
             row.ObsoleteState == agiru::platform::ObsoleteState::Removed);
  CHECK_TEXT("an unknown obsolete state refuses rather than returning No",
             ReadMetadata(row, 5),
             "Field metadata: unsupported ObsoleteState Undocumented");
  CHECK_TEXT("Moved is not guessed into the pinned native option",
             ReadMetadata(row, 8),
             "Field metadata: unsupported ObsoleteState Moved");
  CHECK_TEXT("PendingMove is not guessed into the pinned native option",
             ReadMetadata(row, 9),
             "Field metadata: unsupported ObsoleteState PendingMove");
  CHECK_TRUE("an absent declaration remains missing", !row.Get(kMetadataFixtureId.Value(), 99));
}

/// A TEMPORARY VIRTUAL TABLE NEEDS NOTHING BUT ITS DECLARATION, and that is what makes declaring
/// these tables the first step rather than computing their rows. Measured over BCApps: 282 of the
/// 1 069 `Record Field` declarations carry `temporary`, and `Temporary<T>` keeps its own store and
/// touches no database at all.
void ATemporaryFieldIsAContainerAndNeedsNoPlatform() {
  Temporary<Field> rows;
  CHECK_TRUE("a fresh store is empty", rows.IsEmpty());

  rows.TableNo = kItem;
  rows.No = kItemNo;
  rows.FieldName = "No.";
  rows.Type = FieldDataType::Code;
  rows.Len = kNoLength;
  rows.Insert();

  rows.TableNo = kItem;
  rows.No = kItemDescription;
  rows.FieldName = "Description";
  rows.Type = FieldDataType::Text;
  rows.Len = kDescriptionLength;
  rows.Insert();

  CHECK_TRUE("two rows went in", rows.Count() == 2);

  Temporary<Field> read;
  read.Copy(rows, true);
  CHECK_TRUE("a row is found by its primary key", read.Get(kItem, kItemNo));
  CHECK_TEXT("carrying its name", std::string(read.FieldName.Value()), "No.");
  CHECK_TRUE("and its type", read.Type == FieldDataType::Code);
  CHECK_TRUE("a key that matches nothing answers false", !read.Get(kItem, agiru::Integer{2}));
}

/// THE TABLE IS THE PLATFORM'S, AND IT SAYS SO. 2000000041 is in the reserved range, and the fields
/// carry the AL names the BaseApp writes -- `Field."No."` 437 times, `Field."Field Caption"` 219.
void ItIsTheTableTheBaseAppReadsFrom() {
  Field one;
  RecordRef ref;
  ref.GetTable(one);

  CHECK_TRUE("the AL table number", ref.Number() == 2000000041);
  CHECK_TEXT("the AL name", std::string(ref.Name()), "Field");
  // THE INDEX COUNTS THE FIELDS AL DECLARES AND NEVER THE FIVE THE PLATFORM ADDS: with
  // `SystemId` in `FieldCount()`, `ApplicationAreaMgmt` read a Guid into a Boolean (214 cases,
  // commit 1b27053). The system fields exist all the same and AL reaches them BY NUMBER --
  // `Config. Package Management` writes `Field.FieldNo(SystemId)` -- which `FieldExist` answers.
  CHECK_TRUE("twenty-four declared fields, and not the five the platform adds",
             ref.FieldCount() == 24);
  CHECK_TRUE("while a system field is reachable by number",
             ref.FieldExist(agiru::kSystemFields.front().no.Value()));
  CHECK_TEXT("the field AL calls \"No.\" keeps its dot", std::string(ref.Field(2).Name()), "No.");
  CHECK_TEXT(
      "and the caption field keeps its space", std::string(ref.Field(20).Name()), "Field Caption");
}

/// Current metadata-tag compatibility only; native Field.Type/FieldRef.Type remain board:0034.
void TheCompatibilityTypeKeepsItsExistingOptionVocabulary() {
  const agiru::Option<FieldType> code{FieldType::Code};
  CHECK_TRUE("Code retains the current metadata tag 33", code.AsInteger() == 33);
  CHECK_TEXT("and it names itself", std::string(code.Name()), "Code");

  const agiru::Option<FieldType> guid{FieldType::Guid};
  CHECK_TEXT("GUID is spelled as AL spells it", std::string(guid.Name()), "GUID");

  // THE GAPS ARE DECLARED AND BLANK, not absent: an option that simply skipped them would not be
  // dense, and `Option` asserts density at compile time precisely so this cannot drift.
  const agiru::Option<FieldType> gap{4};
  CHECK_TRUE("a gap is a declared member", gap.IsDeclared());
  CHECK_TEXT("with no name", std::string(gap.Name()), "");
  CHECK_TRUE("and one past the last is not declared", !agiru::Option<FieldType>{41}.IsDeclared());

  const agiru::Option<FieldClass> flow{FieldClass::FlowField};
  CHECK_TEXT("the field class names itself too", std::string(flow.Name()), "FlowField");

  Field row;
  RecordRef ref;
  ref.GetTable(row);
  CHECK_TRUE("Field.Type reflection exposes native members without padded gaps",
             ref.Field(Field::Field_No::Type.Value())
                 .OptionMembers()
                 .starts_with("TableFilter,RecordID,OemText,Date,Time"));
}

void NativeTypeCodesRoundtripThroughTemporaryStorageAndReflection() {
  Temporary<Field> row;
  RecordRef ref;
  ref.GetTable(row);
  CHECK_TRUE("a fresh coded option preserves the zero default", row.Type.AsInteger() == 0);
  CHECK_TRUE("zero is not a fabricated native member", !row.Type.IsDeclared());
  const auto &values = agiru::OptionTraits<FieldDataType>::kValues;
  CHECK_TRUE("native vocabulary is compact", values.size() == 21);
  for (const auto &value : values) {
    ref.Field(Field::Field_No::Type.Value()).Value(agiru::Variant(value.ordinal));
    ref.SetTable(row);
    CHECK_TRUE("reflection preserves the coded ordinal", row.Type.AsInteger() == value.ordinal);
    CHECK_TEXT("reflection preserves the native name", row.Type.Name(), value.name);
    CHECK_TRUE("a native code is declared", row.Type.IsDeclared());
    row.TableNo = kItem;
    row.No = value.ordinal;
    row.Insert();
  }
  CHECK_TRUE("no native option vanished from temporary storage", row.Count() == 21);
  Temporary<Field> read;
  read.Copy(row, true);
  for (const auto &value : values) {
    CHECK_TRUE("each native ordinal can be read by its key", read.Get(kItem, value.ordinal));
    CHECK_TRUE("temporary storage keeps the native code", read.Type.AsInteger() == value.ordinal);
  }
  CHECK_TRUE("the internal Code tag is not a native member",
             !agiru::Option<FieldDataType>{static_cast<int>(FieldType::Code)}.IsDeclared());
  CHECK_TRUE("the FieldRef Code code is not the Field.Type Code code",
             !agiru::Option<FieldDataType>{kNativeFieldRefCodeOrdinal}.IsDeclared());
}

} // namespace

int main() {
  return gate::Run("PlatformField", [] {
    agiru::RegisterTableEntry(&kMetadataEntry);
    agiru::RegisterTableEntry(&kTargetEntry);
    agiru::RegisterTableEntry(&kLongMetadataEntry);
    agiru::RegisterTableEntry(&kTypeMetadataEntry);
    agiru::RegisterTableEntry(&kPolicyMetadataEntry);
    agiru::RegisterTableEntry(&kCustomizationEntry);
    MetadataSearchAndAccessRetainDeclaredProperties();
    MetadataCustomizationsRetainDeclaredPolicies();
    MetadataGetPreservesOptionalFailureContext();
    MovedReadFailuresPreserveOwnedKeyText();
    MetadataGetDefaultsAndTemporaryReadsShareTheContract();
    TypedAndReflectedGetShareInstalledFields();
    NativeGetRequiresItsQualifiedFieldBinding();
    ATemporaryFieldIsAContainerAndNeedsNoPlatform();
    ItIsTheTableTheBaseAppReadsFrom();
    TheCompatibilityTypeKeepsItsExistingOptionVocabulary();
    NativeTypeCodesRoundtripThroughTemporaryStorageAndReflection();
    MetadataKeepsDeclaredValuesAndRecordState();
    ImplicitNamesFollowTheRealReflectionLookupPath();
    MetadataKeepsBlankOptionsAndEnumIdentityAtTheTypeBoundary();
    MetadataLoadsRelationsAndRefusesUnknownStates();
    MetadataTypeNamesMatchTheNativePrimitiveContract();
  });
}
