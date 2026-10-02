#include "meta/Declare.h"
#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/Field.h"
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
constexpr agiru::Integer kTypeNameField = 9;
constexpr FieldType kUnsupportedType = static_cast<FieldType>(250);

struct TypeNameCase {
  FieldType type;
  std::uint16_t length;
  std::string_view name;
  agiru::Integer ordinal;
};

constexpr std::array<TypeNameCase, 21> kTypeNames{{
    {.type = FieldType::Boolean, .length = 0, .name = "Boolean", .ordinal = 34047},
    {.type = FieldType::Integer, .length = 0, .name = "Integer", .ordinal = 34559},
    {.type = FieldType::BigInteger, .length = 0, .name = "BigInteger", .ordinal = 36095},
    {.type = FieldType::Decimal, .length = 0, .name = "Decimal", .ordinal = 12799},
    {.type = FieldType::Option, .length = 0, .name = "Option", .ordinal = 35583},
    {.type = FieldType::Enum, .length = 0, .name = "Option", .ordinal = 35583},
    {.type = FieldType::Duration, .length = 0, .name = "Duration", .ordinal = 36863},
    {.type = FieldType::Code, .length = 20, .name = "Code20", .ordinal = 31489},
    {.type = FieldType::Text, .length = 100, .name = "Text100", .ordinal = 31488},
    {.type = FieldType::Code, .length = 0, .name = "Code0", .ordinal = 31489},
    {.type = FieldType::Text, .length = 2048, .name = "Text2048", .ordinal = 31488},
    {.type = FieldType::Date, .length = 0, .name = "Date", .ordinal = 11775},
    {.type = FieldType::Time, .length = 0, .name = "Time", .ordinal = 11776},
    {.type = FieldType::DateTime, .length = 0, .name = "DateTime", .ordinal = 37375},
    {.type = FieldType::Guid, .length = 0, .name = "GUID", .ordinal = 37119},
    {.type = FieldType::RecordId, .length = 0, .name = "RecordID", .ordinal = 4988},
    {.type = FieldType::DateFormula, .length = 0, .name = "DateFormula", .ordinal = 11797},
    {.type = FieldType::Blob, .length = 0, .name = "BLOB", .ordinal = 33793},
    {.type = FieldType::TableFilter, .length = 0, .name = "TableFilter", .ordinal = 4912},
    {.type = FieldType::MediaSet, .length = 0, .name = "MediaSet", .ordinal = 26208},
    {.type = FieldType::Media, .length = 0, .name = "Media", .ordinal = 26207},
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
    CHECK_TRUE("Field.Type uses its native code, not an internal tag",
               row.Type.AsInteger() == kTypeNames[i].ordinal);
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

/// Internal metadata tags do not change when the native Field boundary is corrected.
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
  CHECK_TRUE(
      "FieldRef exposes the native option inventory without ordinal-gap padding",
      ref.Field(Field::Field_No::Type.Value()).OptionMembers() ==
          "TableFilter,RecordID,OemText,Date,Time,DateFormula,Decimal,Media,MediaSet,Text,"
          "Code,Binary,BLOB,Boolean,Integer,OemCode,Option,BigInteger,Duration,GUID,DateTime");
}

void NativeCodesHaveCompactMetadataAndSurviveTemporaryRows() {
  constexpr std::array<agiru::Integer, 21> codes{4912,  4988,  11519, 11775, 11776, 11797, 12799,
                                                 26207, 26208, 31488, 31489, 33791, 33793, 34047,
                                                 34559, 35071, 35583, 36095, 36863, 37119, 37375};
  constexpr std::array<std::string_view, 21> names{
      "TableFilter", "RecordID", "OemText", "Date",       "Time",     "DateFormula", "Decimal",
      "Media",       "MediaSet", "Text",    "Code",       "Binary",   "BLOB",        "Boolean",
      "Integer",     "OemCode",  "Option",  "BigInteger", "Duration", "GUID",        "DateTime"};
  const auto *type = agiru::Field(agiru::platform::kFieldTable, agiru::FieldNo{5});
  CHECK_TRUE("native type metadata has twenty-one entries",
             type != nullptr && type->values.size() == 21);
  if (type == nullptr || type->values.size() != codes.size()) { return; }
  Temporary<Field> rows;
  for (std::size_t i = 0; i < codes.size(); ++i) {
    CHECK_TRUE("the native code is not a member position", type->values[i].ordinal == codes[i]);
    CHECK_TEXT("the native member keeps its AL spelling", type->values[i].name, names[i]);
    rows.TableNo = kItem;
    rows.No = static_cast<agiru::Integer>(i + 1);
    rows.Type = codes[i];
    CHECK_TRUE("a coded value is declared", rows.Type.IsDeclared());
    CHECK_TEXT("a coded option resolves its name", rows.Type.Name(), names[i]);
    CHECK_TEXT("a coded option resolves its caption", rows.Type.Caption(), names[i]);
    rows.Insert();
  }
  Temporary<Field> read;
  read.Copy(rows, true);
  for (std::size_t i = 0; i < codes.size(); ++i) {
    CHECK_TRUE("a coded temporary row is found",
               read.Get(kItem, static_cast<agiru::Integer>(i + 1)));
    CHECK_TRUE("temporary storage preserves the native ordinal", read.Type.AsInteger() == codes[i]);
  }
  const agiru::Option<FieldDataType> gap{31490};
  CHECK_TRUE("the FieldRef Code value is not declared by Field.Type", !gap.IsDeclared());
  CHECK_TRUE("an undeclared native code has no guessed name", gap.Name().empty());
  CHECK_TRUE("the zero default is unchanged and undeclared",
             !agiru::Option<FieldDataType>{}.IsDeclared());
}

void NativeAddedFieldsHaveSourceNumbersAndTemporaryStorage() {
  Temporary<Field> row;
  row.TableNo = kItem;
  row.No = kItemNo;
  row.ExternalName = std::string(100, 'x');
  row.SQLDataType = agiru::platform::FieldSQLDataType::BigInteger;
  row.DataClassification = agiru::platform::FieldDataClassification::SystemMetadata;
  row.AppPackageID = agiru::Guid{"6f918a07-c568-4fd1-bc15-184737e90b30"};
  row.AppRuntimePackageID = agiru::Guid{"7936dfb2-3e4e-4acf-a5e0-71eb5d8a1eca"};
  row.OptimizeForTextSearch = true;
  row.Access = agiru::platform::FieldAccess::Protected;
  row.IsAllowedInCustomizations = true;
  row.Insert();
  Temporary<Field> read;
  read.Copy(row, true);
  CHECK_TRUE("new fields survive a temporary Get", read.Get(kItem, kItemNo));
  CHECK_TEXT("ExternalName retains all one hundred characters",
             read.ExternalName.Value(),
             std::string(100, 'x'));
  CHECK_TRUE("SQLDataType keeps its native ordinal", read.SQLDataType.AsInteger() == 3);
  CHECK_TRUE("DataClassification uses native order", read.DataClassification.AsInteger() == 6);
  CHECK_TRUE("package ID is stored rather than inferred", read.AppPackageID == row.AppPackageID);
  CHECK_TRUE("runtime package ID remains distinct",
             read.AppRuntimePackageID == row.AppRuntimePackageID &&
                 read.AppRuntimePackageID != read.AppPackageID);
  CHECK_TRUE("text search is stored", read.OptimizeForTextSearch);
  CHECK_TRUE("Access uses native order", read.Access.AsInteger() == 2);
  CHECK_TRUE("customization availability is stored", read.IsAllowedInCustomizations);
  RecordRef ref;
  ref.GetTable(read);
  CHECK_TRUE("ExternalName is field ten", ref.FieldExist(10) && ref.Field(10).Length() == 100);
  CHECK_TRUE("the invented field twenty-nine is absent", !ref.FieldExist(29));
  CHECK_TEXT("SQLDataType is field twenty-three", ref.Field(23).Name(), "SQLDataType");
  CHECK_TEXT("package ID is field sixty", ref.Field(60).Name(), "App Package ID");
  CHECK_TEXT(
      "runtime package ID is field sixty-one", ref.Field(61).Name(), "App Runtime Package ID");
  CHECK_TEXT("text search is field sixty-two", ref.Field(62).Name(), "OptimizeForTextSearch");
  CHECK_TEXT("Access is field sixty-three", ref.Field(63).Name(), "Access");
  CHECK_TEXT(
      "customizations is field sixty-four", ref.Field(64).Name(), "IsAllowedInCustomizations");
}

} // namespace

int main() {
  return gate::Run("PlatformField", [] {
    agiru::RegisterTableEntry(&kMetadataEntry);
    agiru::RegisterTableEntry(&kTargetEntry);
    agiru::RegisterTableEntry(&kLongMetadataEntry);
    agiru::RegisterTableEntry(&kTypeMetadataEntry);
    ATemporaryFieldIsAContainerAndNeedsNoPlatform();
    ItIsTheTableTheBaseAppReadsFrom();
    TheCompatibilityTypeKeepsItsExistingOptionVocabulary();
    MetadataKeepsDeclaredValuesAndRecordState();
    MetadataKeepsBlankOptionsAndEnumIdentityAtTheTypeBoundary();
    MetadataLoadsRelationsAndRefusesUnknownStates();
    MetadataTypeNamesMatchTheNativePrimitiveContract();
    NativeCodesHaveCompactMetadataAndSurviveTemporaryRows();
    NativeAddedFieldsHaveSourceNumbersAndTemporaryStorage();
  });
}
