#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/Field.h"
#include "platform/ReflectionOptions.h"
#include "runtime/Catalogue.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/FieldClass.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Variant.h"

#include "Check.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

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
constexpr agiru::TableId kPropertyMetadataId{50155};
constexpr agiru::Integer kTypeNameField = 9;
constexpr FieldType kUnsupportedType = static_cast<FieldType>(250);

struct TypeNameCase {
  FieldType type;
  std::uint16_t length;
  std::string_view name;
};

constexpr std::array<TypeNameCase, 21> kTypeNames{{
    {.type = FieldType::Boolean, .length = 0, .name = "Boolean"},
    {.type = FieldType::Integer, .length = 0, .name = "Integer"},
    {.type = FieldType::BigInteger, .length = 0, .name = "BigInteger"},
    {.type = FieldType::Decimal, .length = 0, .name = "Decimal"},
    {.type = FieldType::Option, .length = 0, .name = "Option"},
    {.type = FieldType::Enum, .length = 0, .name = "Option"},
    {.type = FieldType::Duration, .length = 0, .name = "Duration"},
    {.type = FieldType::Code, .length = 20, .name = "Code20"},
    {.type = FieldType::Text, .length = 100, .name = "Text100"},
    {.type = FieldType::Code, .length = 0, .name = "Code0"},
    {.type = FieldType::Text, .length = 2048, .name = "Text2048"},
    {.type = FieldType::Date, .length = 0, .name = "Date"},
    {.type = FieldType::Time, .length = 0, .name = "Time"},
    {.type = FieldType::DateTime, .length = 0, .name = "DateTime"},
    {.type = FieldType::Guid, .length = 0, .name = "GUID"},
    {.type = FieldType::RecordId, .length = 0, .name = "RecordID"},
    {.type = FieldType::DateFormula, .length = 0, .name = "DateFormula"},
    {.type = FieldType::Blob, .length = 0, .name = "BLOB"},
    {.type = FieldType::TableFilter, .length = 0, .name = "TableFilter"},
    {.type = FieldType::MediaSet, .length = 0, .name = "MediaSet"},
    {.type = FieldType::Media, .length = 0, .name = "Media"},
}};
constexpr agiru::Integer kUnknownTypeField = static_cast<agiru::Integer>(kTypeNames.size() + 1);
constexpr std::array<agiru::Integer, kTypeNames.size()> kNativeTypeCodes{
    34047, 34559, 36095, 12799, 35583, 35583, 36863, 31489, 31488, 31489, 31488,
    11775, 11776, 37375, 37119, 4988,  11797, 33793, 4912,  26208, 26207};

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

constexpr std::array<std::string_view, 7> kClassifications{"CustomerContent",
                                                           "ToBeClassified",
                                                           "EndUserIdentifiableInformation",
                                                           "AccountData",
                                                           "EndUserPseudonymousIdentifiers",
                                                           "OrganizationIdentifiableInformation",
                                                           "SystemMetadata"};
constexpr std::array<std::string_view, 4> kSqlTypes{"Varchar", "Integer", "Variant", "BigInteger"};
constexpr std::array<std::string_view, 4> kAccessModes{"Public", "Internal", "Protected", "Local"};
constexpr std::array<std::string_view, 6> kCustomizations{
    "", "ToBeClassified", "Always", "AsReadOnly", "AsReadWrite", "Never"};
constexpr auto kPropertyFields = [] {
  std::array<agiru::FieldDef, 28> fields{};
  for (std::size_t i = 0; i < fields.size(); ++i) {
    fields[i] = {.name = "Property fixture",
                 .no = agiru::FieldNo{static_cast<agiru::Integer>(i + 1)},
                 .length = 20,
                 .type = FieldType::Code};
  }
  for (std::size_t i = 0; i < kClassifications.size(); ++i) {
    fields[1 + i].dataClassification = kClassifications[i];
  }
  for (std::size_t i = 0; i < kSqlTypes.size(); ++i) { fields[8 + i].sqlDataType = kSqlTypes[i]; }
  for (std::size_t i = 0; i < kAccessModes.size(); ++i) { fields[12 + i].access = kAccessModes[i]; }
  for (std::size_t i = 0; i < kCustomizations.size(); ++i) {
    fields[16 + i].allowInCustomizations = kCustomizations[i];
  }
  fields[8].optimizeForTextSearch = true;
  fields[22].fieldClass = agiru::FieldClass::FlowField;
  fields[22].dataClassification = "AccountData";
  fields[22].optimizeForTextSearch = true;
  fields[23].fieldClass = agiru::FieldClass::FlowFilter;
  fields[23].dataClassification = "CustomerContent";
  fields[23].optimizeForTextSearch = true;
  fields[24].dataClassification = "Undocumented";
  fields[25].sqlDataType = "Undocumented";
  fields[26].access = "Undocumented";
  fields[27].allowInCustomizations = "Undocumented";
  return fields;
}();
constexpr agiru::TableDef kPropertyTable{
    .id = kPropertyMetadataId, .name = "Fixture Field Properties", .fields = kPropertyFields};
constexpr agiru::TableEntry kPropertyEntry = MetadataEntry(kPropertyTable);

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

void MetadataTypeNamesMatchTheNativePrimitiveContract() {
  Field row;
  for (std::size_t i = 0; i < kTypeNames.size(); ++i) {
    CHECK_SILENT("a primitive declaration is readable",
                 ReadMetadata(row, static_cast<agiru::Integer>(i + 1), kTypeMetadataId));
    CHECK_TEXT("Type Name uses the native primitive spelling and length",
               ReadTypeName(row),
               kTypeNames[i].name);
    CHECK_TRUE("primitive metadata uses the source-declared native code",
               row.Type.AsInteger() == kNativeTypeCodes[i]);
    CHECK_TRUE("every reported primitive has a declared native member", row.Type.IsDeclared());
  }
  CHECK_TEXT("an unknown primitive type refuses rather than returning blank",
             ReadMetadata(row, kUnknownTypeField, kTypeMetadataId),
             "Field metadata: unsupported type name");
  CHECK_TEXT("a refused declaration preserves the previous row", ReadTypeName(row), "Media");
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
  CHECK_TRUE("Field.Type reflection exposes native members instead of metadata-tag gaps",
             ref.Field(Field::Field_No::Type.Value())
                 .OptionMembers()
                 .starts_with("TableFilter,RecordID,OemText,Date,Time,DateFormula,Decimal"));
}

struct NativeFieldCase {
  agiru::Integer number;
  std::string_view name;
  FieldType type;
  std::uint16_t length;
};

constexpr std::array<NativeFieldCase, 24> kNativeFields{{
    {1, "TableNo", FieldType::Integer, 0},
    {2, "No.", FieldType::Integer, 0},
    {3, "TableName", FieldType::Text, 30},
    {4, "FieldName", FieldType::Text, 30},
    {5, "Type", FieldType::Option, 0},
    {6, "Len", FieldType::Integer, 0},
    {7, "Class", FieldType::Option, 0},
    {8, "Enabled", FieldType::Boolean, 0},
    {9, "Type Name", FieldType::Text, 30},
    {10, "ExternalName", FieldType::Text, 100},
    {20, "Field Caption", FieldType::Text, 80},
    {21, "RelationTableNo", FieldType::Integer, 0},
    {22, "RelationFieldNo", FieldType::Integer, 0},
    {23, "SQLDataType", FieldType::Option, 0},
    {24, "OptionString", FieldType::Text, 2047},
    {25, "ObsoleteState", FieldType::Option, 0},
    {26, "ObsoleteReason", FieldType::Text, 248},
    {27, "DataClassification", FieldType::Option, 0},
    {28, "IsPartOfPrimaryKey", FieldType::Boolean, 0},
    {60, "App Package ID", FieldType::Guid, 0},
    {61, "App Runtime Package ID", FieldType::Guid, 0},
    {62, "OptimizeForTextSearch", FieldType::Boolean, 0},
    {63, "Access", FieldType::Option, 0},
    {64, "IsAllowedInCustomizations", FieldType::Boolean, 0},
}};

void EveryNativeFieldRetainsItsSourceDeclaration() {
  const auto &table = agiru::TableTraits<Field>::kTable;
  CHECK_TRUE("complete source population plus implicit system fields",
             table.fields.size() == kNativeFields.size() + agiru::kSystemFieldCount);
  for (const auto &expected : kNativeFields) {
    const auto *field = agiru::Field(table, agiru::FieldNo{expected.number});
    CHECK_TRUE("every declared field number exists", field != nullptr);
    if (field == nullptr) { continue; }
    CHECK_TEXT(
        "source field names are not captions or guessed spellings", field->name, expected.name);
    CHECK_TEXT("absent captions default to the declared name", field->caption, expected.name);
    CHECK_TRUE("source primitive type is retained", field->type == expected.type);
    CHECK_TRUE("source text length is retained", field->length == expected.length);
  }
  CHECK_TRUE("ExternalName does not invent the old field number",
             agiru::Field(table, agiru::FieldNo{29}) == nullptr);
  CHECK_TEXT("source primary key name", table.keys.front().name, "pk");
  CHECK_TRUE("source primary key order",
             table.keys.front().fields[0].Value() == 1 &&
                 table.keys.front().fields[1].Value() == 2);
}

void NativeTypeCodesRemainDistinctFromMetadataTags() {
  const agiru::Option<FieldDataType> code{FieldDataType::Code};
  CHECK_TRUE("Field.Type Code uses its declared native code", code.AsInteger() == 31489);
  CHECK_TEXT("the native code retains its source name", code.Name(), "Code");
  CHECK_TRUE(
      "the internal Code tag is not a declared native value",
      !agiru::Option<FieldDataType>{static_cast<agiru::Integer>(FieldType::Code)}.IsDeclared());
  Field row;
  row.Type = code;
  RecordRef ref;
  ref.GetTable(row);
  CHECK_TRUE("reflection exports the native code, not an option index",
             static_cast<agiru::Integer>(ref.Field(Field::Field_No::Type.Value()).Value()) ==
                 31489);
  CHECK_TEXT("the SQL property vocabulary has source order",
             ref.Field(Field::Field_No::SQLDataType.Value()).OptionMembers(),
             "Varchar,Integer,Variant,BigInteger");
  CHECK_TEXT("the obsolete state does not invent relocation options",
             ref.Field(Field::Field_No::ObsoleteState.Value()).OptionMembers(),
             "No,Pending,Removed");
  CHECK_TEXT("the native classifier is not the telemetry classifier",
             ref.Field(Field::Field_No::DataClassification.Value()).OptionMembers(),
             "CustomerContent,ToBeClassified,EndUserIdentifiableInformation,AccountData,"
             "EndUserPseudonymousIdentifiers,OrganizationIdentifiableInformation,SystemMetadata");
  CHECK_TEXT("compile-time access retains the source vocabulary",
             ref.Field(Field::Field_No::Access.Value()).OptionMembers(),
             "Public,Internal,Protected,Local");
}

void FieldPropertiesUseNativeNamesDefaultsAndRefusals() {
  Field row;
  const auto get = [&row](std::size_t index) {
    CHECK_SILENT("property fixture is readable",
                 ReadMetadata(row, static_cast<agiru::Integer>(index + 1), kPropertyMetadataId));
  };
  get(0);
  CHECK_TRUE("an unclassified normal field defaults to ToBeClassified",
             row.DataClassification == agiru::platform::FieldDataClassification::ToBeClassified);
  CHECK_TRUE("source defaults are restored together",
             row.SQLDataType == agiru::platform::FieldSQLDataType::Varchar &&
                 row.Access == agiru::platform::FieldAccess::Public &&
                 row.IsAllowedInCustomizations && !row.OptimizeForTextSearch);
  for (std::size_t i = 0; i < kClassifications.size(); ++i) {
    get(1 + i);
    CHECK_TEXT("classification retains its source name",
               row.DataClassification.Name(),
               kClassifications[i]);
    CHECK_TRUE("classification uses native source order",
               row.DataClassification.AsInteger() == static_cast<agiru::Integer>(i));
  }
  for (std::size_t i = 0; i < kSqlTypes.size(); ++i) {
    get(8 + i);
    CHECK_TEXT("SQL type retains its source name", row.SQLDataType.Name(), kSqlTypes[i]);
    CHECK_TRUE("search property follows the current normal field",
               row.OptimizeForTextSearch == (i == 0));
  }
  for (std::size_t i = 0; i < kAccessModes.size(); ++i) {
    get(12 + i);
    CHECK_TEXT("access retains its source name", row.Access.Name(), kAccessModes[i]);
  }
  for (std::size_t i = 0; i < kCustomizations.size(); ++i) {
    get(16 + i);
    CHECK_TRUE("only Never forbids field customization", row.IsAllowedInCustomizations == (i != 5));
  }
  for (const std::size_t index : {22U, 23U}) {
    get(index);
    CHECK_TRUE("computed/filter fields force SystemMetadata",
               row.DataClassification == agiru::platform::FieldDataClassification::SystemMetadata);
    CHECK_TRUE("computed/filter fields are not text-search indexed", !row.OptimizeForTextSearch);
  }
  constexpr std::array<std::string_view, 4> invalid{
      "DataClassification", "SqlDataType", "Access", "AllowInCustomizations"};
  for (std::size_t i = 0; i < invalid.size(); ++i) {
    CHECK_TEXT("unknown properties refuse instead of manufacturing defaults",
               ReadMetadata(row, static_cast<agiru::Integer>(25 + i), kPropertyMetadataId),
               "Field metadata: unsupported " + std::string(invalid[i]) + " Undocumented");
    CHECK_TRUE("refused property does not replace the previous metadata row", row.No == 24);
    CHECK_TRUE("refused property does not replace the previous classification",
               row.DataClassification == agiru::platform::FieldDataClassification::SystemMetadata);
  }
}

} // namespace

int main() {
  return gate::Run("PlatformField", [] {
    agiru::RegisterTableEntry(&kMetadataEntry);
    agiru::RegisterTableEntry(&kTargetEntry);
    agiru::RegisterTableEntry(&kLongMetadataEntry);
    agiru::RegisterTableEntry(&kTypeMetadataEntry);
    agiru::RegisterTableEntry(&kPropertyEntry);
    ATemporaryFieldIsAContainerAndNeedsNoPlatform();
    ItIsTheTableTheBaseAppReadsFrom();
    TheCompatibilityTypeKeepsItsExistingOptionVocabulary();
    MetadataKeepsDeclaredValuesAndRecordState();
    MetadataKeepsBlankOptionsAndEnumIdentityAtTheTypeBoundary();
    MetadataLoadsRelationsAndRefusesUnknownStates();
    MetadataTypeNamesMatchTheNativePrimitiveContract();
    EveryNativeFieldRetainsItsSourceDeclaration();
    NativeTypeCodesRemainDistinctFromMetadataTags();
    FieldPropertiesUseNativeNamesDefaultsAndRefusals();
  });
}
