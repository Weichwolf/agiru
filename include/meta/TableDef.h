#pragma once

#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "type/FieldClass.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

/// \file
/// \brief The static declaration of an AL table: its fields, its keys, and how to reach them.

namespace agiru {

/// \brief An AL field's data type, as far as the generator can emit it.
///
/// This list grows as the generator learns types. A type it cannot emit is a translation error
/// rather than a silent fallback to Text.
///
/// \warning THE NUMBERS ARE THE PLATFORM'S OWN AND NOT A COUNTER. `FieldRef.Type()` and
///          `Field.Type` return this, and AL compares the result against `Field.Type::Code`
///          directly -- so a dense 0, 1, 2 ... of this tree's own invention would make every such
///          comparison quietly false. The sparse values are BC's: measured from
///          `~/Git/openerp/openerp/runtime/al_system_enums.py`, which mirrors AL's `FieldType`
///          system type, and cross-checked against `fieldtype-option.md`, whose eighteen documented
///          members are exactly a subset of them.
enum class FieldType : std::uint8_t {
  Boolean = 3,      ///< AL `Boolean`.
  Option = 5,       ///< AL `Option`, and what an Enum field reports too.
  Integer = 7,      ///< AL `Integer`.
  Decimal = 9,      ///< AL `Decimal`.
  Date = 11,        ///< AL `Date`.
  Time = 12,        ///< AL `Time`.
  Blob = 14,        ///< AL `Blob`.
  DateFormula = 15, ///< AL `DateFormula`.
  BigInteger = 18,  ///< AL `BigInteger`.
  Duration = 20,    ///< AL `Duration`.
  Guid = 21,        ///< AL `Guid`.
  DateTime = 22,    ///< AL `DateTime`.
  RecordId = 23,    ///< AL `RecordId`.
  TableFilter = 24, ///< AL `TableFilter`, which only the Permission table uses.
  Text = 31,        ///< AL `Text`.
  Code = 33,        ///< AL `Code`.
  MediaSet = 39,    ///< AL `MediaSet`.
  Media = 40,       ///< AL `Media`.

  /// \brief An enum field, which the PLATFORM does not distinguish and this metadata does.
  ///
  /// It is deliberately outside the platform's own range: `FieldRef::Type()` reports `Option` for
  /// a field declared this way, because `fieldtype-option.md` has no `Enum` member at all, and
  /// `IsEnum()` is the one place BC puts the difference. So this value never leaves the metadata,
  /// and giving it a platform number would let it escape looking like one.
  Enum = 200,
};

/// \brief One field's declaration, as static const data.
///
/// Every `FieldDef` the generator writes is a `constexpr` aggregate in `.rodata`: demand-paged by
/// the kernel, shared between processes, costing nothing until it is touched and nothing at
/// startup. The predecessor built the equivalent as heap objects while loading and paid about a
/// gigabyte per process for it (CLAUDE.md, board:0006).
///
/// \note `offset` is what lets the runtime reach a field by number without a virtual call and
///       without a map, and it is why a generated record must be standard-layout.
/// \brief AL's `TableType` values, `devenv-tabletype-property.md`, in the order the page lists
/// them.
enum class TableType : std::uint8_t {
  Normal,
  CRM,
  CDS,
  ExternalSQL,
  Exchange,
  MicrosoftGraph,
  Temporary,
};

struct FieldDef {
  std::size_t offset{};       ///< `offsetof` within the generated record.
  std::string_view name{};    ///< The AL name, spaces and all: `"Work Type Code"`.
  std::string_view caption{}; ///< The `Caption` property, which AL error messages quote.

  /// \brief The declared values of an Option or Enum field, empty otherwise.
  ///
  /// They live in the metadata rather than in the value because they are the same for every record
  /// of the table: one `.rodata` run for the whole table instead of a copy per instance. An error
  /// message needs them, since AL renders either enumeration by its value name and never by its
  /// ordinal.
  std::span<const EnumValueDef> values{};

  /// \brief The `CalcFormula` property, as AL wrote it, for a FlowField.
  std::string_view calcFormula{};

  /// \brief The `TableRelation`'s target table, empty where the declaration is conditional or
  ///        filtered.
  ///
  /// \note THE SIMPLE FORM IS THE MAJORITY AND THE WHOLE GRAMMAR IS NOT. `TableRelation` is a small
  ///       language -- `if (...) T.F where(...) else ...` -- and 72 % of the BaseApp's declarations
  ///       are the bare `Table[.Field]` (measured over 400 declarations, 2026-09-06). What is here
  ///       is that majority; the rest is board:0043, and an empty target says so.
  std::string_view relationTable{};

  /// \brief The `TableRelation`'s target field, empty where it names the table's own primary key.
  std::string_view relationField{};

  /// \brief The whole `TableRelation` as AL wrote it, whitespace collapsed -- `if (Type =
  ///        const(Resource)) Resource else if (...) "Resource Group"`, `"Item Unit of Measure".Code
  ///        where("Item No." = field("Asset No."))` -- for the 736 conditional and 2 502 filtered
  ///        declarations the two fields above cannot carry (measured 2026-09-10). The runtime's
  ///        `ResolveRelation` reads it against the record (board:0658; board:0043 asked for a
  ///        `constexpr` parsed form, and this is the string it would parse -- the filter language
  ///        inside it is parsed at run time everywhere else in this tree too).
  std::string_view relation{};

  /// \brief The `MinValue` and `MaxValue` properties, as AL wrote them.
  ///
  /// \warning THEY ARE INPUT BOUNDS AND NOT WRITE BOUNDS. The client refuses a value outside them
  ///          before it takes it; a programmatic `Validate` does not (openerp, measured). They are
  ///          carried so the page layer can enforce what the page enforces.
  std::string_view minValue{};
  std::string_view maxValue{}; ///< \see minValue

  /// \brief The `DecimalPlaces` property, as AL wrote it: `2` or `2:5`.
  std::string_view decimalPlaces{};

  /// \brief The `BlankNumbers` property, as AL wrote it: `DontBlank`, `BlankNeg`,
  /// `BlankNegAndZero`,
  ///        `BlankZero` or `BlankPos`.
  ///
  /// \warning IT BELONGS TO THE FIELD'S RENDERING PATH and never to `Format(Decimal)`, which is
  ///          given a value and not a field (board:0323).
  std::string_view blankNumbers{};

  /// \brief The `CharAllowed` property, as AL wrote it: `A-Z0-9` -- pairs of range ends.
  std::string_view charAllowed{};

  /// \brief The `ValuesAllowed` property, as AL wrote it: the comma list of permitted values.
  std::string_view valuesAllowed{};

  /// \brief The `ExtendedDataType` property, as AL wrote it: `EMail`, `URL`, `Ratio`, `Masked`,
  ///        `Person`, `PhoneNo`, `Barcode` or `None`.
  std::string_view extendedDataType{};

  /// \brief The `MaskType` property, as AL wrote it.
  std::string_view maskType{};

  /// \brief The `ToolTip` property, which a TABLE FIELD may declare and 12 660 of them do.
  ///
  /// \note IT IS NOT ONLY A PAGE PROPERTY. `devenv-tooltip-property.md` applies to a table field
  ///       as well as to a page control, and a page that shows the field without declaring its own
  ///       shows this one (board:0385).
  std::string_view toolTip{};

  /// \brief The `AccessByPermission` property: the field is gone for a user without it
  ///        (board:0377).
  std::string_view accessByPermission{};

  /// \brief The `CaptionClass` property, as AL wrote it: the expression a caption is built from.
  std::string_view captionClass{};

  /// \brief The `AutoFormatType` and `AutoFormatExpression` properties, as AL wrote them.
  std::string_view autoFormatType{};
  std::string_view autoFormatExpression{}; ///< \see autoFormatType

  /// \brief The `AllowInCustomizations` property, as AL wrote it: `Never`, `Always` or a value in
  ///        between (board:0480).
  std::string_view allowInCustomizations{};

  /// \brief The `Access` property, as AL wrote it: `Public`, `Internal`, `Local` or `Protected`.
  std::string_view access{};

  /// \brief The `Subtype` property on a BLOB or Media field, as AL wrote it: `Bitmap`, `Memo`,
  ///        `Json`, `Xml` or `UserDefined`.
  std::string_view subtype{};

  /// \brief The `MovedFrom` and `MovedTo` properties on the FIELD (board:0357).
  std::string_view movedFrom{};
  std::string_view movedTo{}; ///< \see movedFrom

  /// \brief The `Description` property, which the compiler carries and nothing reads.
  std::string_view description{};

  /// \brief The `ObsoleteState` property, as AL wrote it -- `Pending` or `Removed`, else empty.
  std::string_view obsoleteState{};

  /// \brief The `ObsoleteReason` property: the text a diagnostic prints.
  std::string_view obsoleteReason{};

  /// \brief The `ObsoleteTag` property: the version the removal is scheduled for.
  std::string_view obsoleteTag{};

  /// \brief The `ExternalName` property: the name the OTHER database gives this field.
  ///
  /// \note IT IS NOT A RENAME. `devenv-externalname-property.md` applies it to a table and to a
  ///       table field of an EXTERNAL table -- one whose `TableType` names a database this tree
  ///       does not have -- so it is carried and read by whatever opens that database.
  std::string_view externalName{};

  /// \brief The `OptionOrdinalValues` property, as AL wrote it: a member's FOREIGN number.
  ///
  /// \note IT IS NOT THE ORDINAL. An option member's ordinal is its position in this table; this
  ///       is the number some other system gives the same member, and the two are different
  ///       questions about one value.
  std::string_view optionOrdinalValues{};
  /// \brief The `InitValue` property, as the COLUMN spells it, or nothing when AL declared none.
  ///
  /// `devenv-initvalue-property.md`: "Sets the initial value of this field when a user creates a
  /// new record", and it is what `Init`, `Clear` and `ClearAll` reach for. 815 fields declare one
  /// under `Layers/W1` (measured 2026-09-04), most of them `true` on a Boolean.
  ///
  /// \note THE MEMBER NAME IS RESOLVED TO ITS ORDINAL BY THE GENERATOR. AL writes
  ///       `InitValue = "Gen. Prod. Posting Group"` and the column holds a number, so the
  ///       translation happens where the enumeration is in scope and not at run time.
  ///
  /// \note EMPTY IS NOT ABSENT. `InitValue = ''` on a Code field is a declaration and an absent
  ///       property is not, and a bare `string_view` could not tell them apart.
  std::optional<std::string_view> initValue{};
  FieldNo no{}; ///< The AL field number.

  /// \brief The `FieldClass` property: whether the field is stored, computed or a filter.
  ///
  /// `properties/devenv-fieldclass-property.md`. A `FlowField` is COMPUTED and has no column
  /// (board:0047); a `FlowFilter` holds a filter and has none either. 2 712 declarations under the
  /// read roots.
  ::agiru::FieldClass fieldClass = ::agiru::FieldClass::Normal;

  /// \brief The `LookupPageId` property: the list the dropdown opens (board:0334).
  PageId lookupPageId{};

  /// \brief The `DrillDownPageId` property: the rows behind a value (board:0335).
  PageId drillDownPageId{};
  std::uint16_t length{}; ///< Declared length for Code and Text, 0 otherwise.

  /// \brief The `Width` property: the column width the client shows, 0 when none is declared.
  std::uint16_t width = 0;
  FieldType type{}; ///< The AL data type.

  /// \brief The `NotBlank` property: the field refuses the empty value. 949 declarations.
  bool notBlank = false;

  /// \brief The `AutoIncrement` property: the platform assigns the number. 151 declarations.
  bool autoIncrement = false;

  /// \brief The `Editable` property. A table field's `false` is a UI refusal and not a write one.
  bool editable = true;

  /// \brief The `ValidateTableRelation` property, which `Validate` reads before the trigger.
  bool validateTableRelation = true;

  /// \brief The `BlankZero` property: a zero renders as nothing.
  bool blankZero = false;

  /// \brief The `Compressed` property: whether a BLOB is stored compressed. The default is true,
  ///        which is BC's own (board:0372).
  bool compressed = true;

  /// \brief The `Numeric` property: the client accepts only digits (board:0320).
  bool numeric = false;

  /// \brief The `ClosingDates` property: the field takes BC's closing dates (board:0326).
  bool closingDates = false;

  /// \brief The `OptimizeForTextSearch` property (board:0370).
  bool optimizeForTextSearch = false;

  /// \brief The `SqlTimestamp` property: the field IS the row's version rather than a column.
  ///
  /// \note EVERY TABLE HAS A ROWVERSION AND THIS FIELD EXPOSES IT. `devenv-table-system-fields.md`
  ///       gives every table a `SystemRowVersion`; a field declared this way is another name for
  ///       it, so it is READ from the rowversion and never written (board:0013).
  bool sqlTimestamp = false;

  /// \brief The `Enabled` property on a FIELD: a disabled field is declared and not maintained.
  bool enabled = true;
};

/// \brief Whether a field is a COLUMN.
///
/// \param field The field.
/// \return True when the database holds it.
///
/// A `FlowField` is computed and a `FlowFilter` holds a filter, so neither is stored -- 2 153 of
/// them under `Layers/W1`, and a column for each would always read its default (board:0047). A
/// field whose `ObsoleteState` is `Removed` is gone from the schema too, which is what removed
/// means.
[[nodiscard]] constexpr bool Stored(const FieldDef &field) {
  return field.fieldClass == ::agiru::FieldClass::Normal && field.obsoleteState != "Removed";
}

/// \brief One key's declaration. The first key of a table is its primary key.
/// \brief The most fields a PRIMARY key may name.
///
/// `devenv-table-keys.md`: a table's primary key is up to 16 fields. A `static_assert` beside every
/// table is where that is checked, because it is knowable at translation time (board:0520).
inline constexpr std::size_t kMaximumPrimaryKeyFields = 16;

/// \brief The most keys a table may declare, the primary key included.
///
/// `devenv-table-keys.md`: 40. `Sales Line` declares 17, which is the widest in the BaseApp.
inline constexpr std::size_t kMaximumKeys = 40;

struct KeyDef {
  std::string_view name{};           ///< The AL key name, `Key1` by convention.
  std::span<const FieldNo> fields{}; ///< The key fields, in declaration order.
  bool clustered{};                  ///< The `Clustered` property.

  /// \brief The `Enabled` property: a disabled key is declared and NOT maintained as an index.
  bool enabled = true;

  /// \brief The `SumIndexFields` property: the fields the SIFT aggregate sums, by number and in
  ///        declaration order. Empty on a key that declares none.
  std::span<const FieldNo> sumIndexFields{};

  /// \brief The `MaintainSiftIndex` property: whether the aggregate is stored or computed.
  bool maintainSiftIndex = true;

  /// \brief The `MaintainSqlIndex` property: whether the declared key becomes an index at all.
  bool maintainSqlIndex = true;

  /// \brief The `Unique` property: the index is emitted with `UNIQUE` (board:0350).
  bool unique = false;

  /// \brief The `IncludedFields` property, as AL wrote it -- kept SEPARATE from the key's own
  ///        fields, because `SetCurrentKey` never selects a key by them (board:0351).
  std::string_view includedFields{};

  /// \brief The `Description` property, which the compiler carries and nothing reads.
  std::string_view description{};

  /// \brief The `ObsoleteState` property on the KEY, as AL wrote it.
  std::string_view obsoleteState{};
};

/// \brief How many system fields the platform adds to every table.
///
/// `devenv-table-system-fields.md` tabulates five with their numbers -- SystemId 2000000000,
/// SystemCreatedAt 2000000001, SystemCreatedBy 2000000002, SystemModifiedAt 2000000003,
/// SystemModifiedBy 2000000004 -- and says they are "automatically included in every table object
/// by the platform".
///
/// \note IT IS NAMED SO THAT A GENERATED ASSERTION CAN KEEP THE AL NUMBER VISIBLE. A table's field
///       count is asserted as `declared + kSystemFieldCount`, so a reader checks the first number
///       against the `.al` file and the second against this page. Folding them into one total would
///       leave nothing that either source can be compared with.
///
/// \note SystemRowVersion is NOT among them. AL exposes the SQL rowversion under that name and the
///       page gives it no field number, unlike the five it tabulates (board:0013).
inline constexpr std::size_t kSystemFieldCount = 5;

/// \brief One table's declaration.
/// \brief The first table number the platform reserves for its own tables
/// (`devenv-object-ranges.md`:
///        2 000 000 000 and above are system objects). What lives there here is the translated
///        SUBSET of the platform's virtual tables -- `AllObj` carries the objects this build has,
///        `Field` the fields of the tables it has -- so a `TableRelation` into that range is not
///        a data-integrity question this runtime can answer: `Data Exch. Def."Reading/Writing
///        XMLport"` names an XMLport that exists in BC and not here (29 UT cases refused,
///        chain 87, 2026-09-10).
inline constexpr std::int32_t kPlatformTableFloor = 2000000000;

/// \brief Whether a table number is in the platform's own range. \param id The number.
/// \return True at and above `kPlatformTableFloor`.
[[nodiscard]] constexpr bool IsPlatformTable(TableId id) {
  return id.Value() >= kPlatformTableFloor;
}

struct TableDef {
  TableId id{};                       ///< The AL table number.
  std::string_view name{};            ///< The AL name: `"Resource Cost"`.
  std::string_view caption{};         ///< The `Caption` property.
  std::span<const FieldDef> fields{}; ///< Every declared field, in declaration order.
  std::span<const KeyDef> keys{};     ///< Every declared key; `keys[0]` is the primary key.

  /// \brief The `ExternalName` property: the name the OTHER database gives this table.
  std::string_view externalName{};

  /// \brief The `ExternalSchema` property: the schema that name lives in, over there.
  std::string_view externalSchema{};

  /// \brief The `TableType` property. `Normal` is a relation; `Temporary` is an in-memory row set
  ///        (board:0032) and gets a relation until that exists; the external kinds are refused by
  ///        the transpiler because they name a database this tree does not have (board:0364).
  TableType tableType = TableType::Normal;

  /// \brief The `DataPerCompany` property: whether each company gets its own rows.
  bool dataPerCompany = true;

  /// \brief The `ReplicateData` property: whether cloud migration copies this table's rows.
  ///
  /// \note CARRIED AND ACTED ON BY NOTHING (board:0358). 1 064 declarations make a refusal a
  ///       translation error on two thirds of the schema, and the bit is what a backup, a fixture
  ///       load or a determinism digest would otherwise have to invent a list for.
  bool replicateData = true;

  /// \brief The `DataAccessIntent` property, as AL wrote it: `ReadOnly` or `ReadWrite`.
  ///
  /// \note CARRIED AND ROUTED BY NOTHING (board:0368): routing a read-only intent at a replica
  ///       needs a replica, and there is one database here.
  std::string_view dataAccessIntent{};

  /// \brief The `CompressionType` property, as AL wrote it (board:0373).
  std::string_view compressionType{};

  /// \brief The `DataCaptionFields` property: the fields a record's caption is built from, by
  ///        number and in declaration order; empty where the primary key stands in (board:0374).
  std::span<const FieldNo> dataCaptionFields{};

  /// \brief The `Permissions` property: what the table's own code may reach beyond its caller
  ///        (board:0376).
  std::string_view permissions{};

  /// \brief The `InherentPermissions` and `InherentEntitlements` properties, as AL wrote them: an
  ///        object granted its own access (board:0378).
  std::string_view inherentPermissions{};
  std::string_view inherentEntitlements{}; ///< \see inherentPermissions

  /// \brief The `Extensible` property, as AL wrote it: whether an extension may reach the object
  ///        (board:0360).
  std::string_view extensible{};

  /// \brief The `Access` property, as AL wrote it: what the generated name is visible to
  ///        (board:0359).
  std::string_view access{};

  /// \brief The `MovedFrom` and `MovedTo` properties, as AL wrote them: the app id an object came
  ///        from or went to (board:0357).
  std::string_view movedFrom{};
  std::string_view movedTo{}; ///< \see movedFrom

  /// \brief The `LookupPageId` and `DrillDownPageId` properties on the TABLE, which every field
  ///        of it falls back to (board:0334, board:0335).
  PageId lookupPageId{};
  PageId drillDownPageId{}; ///< \see lookupPageId

  /// \brief The `PasteIsValid` property: whether the client may paste rows into the table.
  bool pasteIsValid = true;

  /// \brief The `Description` property, which the compiler carries and nothing reads.
  std::string_view description{};

  /// \brief The `AllowInCustomizations` property on the TABLE (board:0480).
  std::string_view allowInCustomizations{};

  /// \brief The `ObsoleteState` property, as AL wrote it.
  std::string_view obsoleteState{};

  /// \brief The field whose VALUES this table's rows are, for a table that is computed rather than
  ///        stored; `0` for every stored table.
  ///
  /// \note A COMPUTED TABLE HAS NO ROWS UNTIL A FILTER NAMES THEM. AL writes
  ///       `Integer.SetRange(Number, 1, N); Integer.FindSet` where C would say `for`, so the rows
  ///       are the integers the filter admits and nothing is stored. A read on such a table is a
  ///       series over the admitted intervals; a filter that admits the whole domain yields
  ///       nothing, because an unbounded series is a loop AL ends with `CurrReport.Break` and this
  ///       runtime has no such brake yet (board:0697).
  FieldNo sequenceField{};
};

/// \brief Finds a field by its AL number.
///
/// \param table The table to search.
/// \param no    The AL field number.
/// \return The field, or `nullptr` when the table declares no such number.
///
/// \pre The field table is sorted by field number. The generator emits it that way and asserts it
///      beside every table, so the precondition is checked at compile time rather than trusted.
///
/// A binary search rather than a walk, and the numbers say why (measured over the BaseApp,
/// 2026-09-01): the median table has 9 fields but the widest has 240, and a `FieldDef` is some 70
/// bytes -- a walk over the widest touches around 17 KB, on a path `Validate` will take for every
/// field of every record. Sorting costs nothing: AL already declares 1 526 of 1 545 tables in
/// ascending order.
///
/// \note Field numbers are NOT dense and no index array can replace this: the median highest
///       number is 14 and the largest in the BaseApp is 99 008 500.
[[nodiscard]] constexpr const FieldDef *Field(const TableDef &table, FieldNo no) {
  std::size_t low = 0;
  std::size_t high = table.fields.size();
  while (low < high) {
    const std::size_t mid = low + ((high - low) / 2);
    if (table.fields[mid].no == no) { return &table.fields[mid]; }
    if (table.fields[mid].no < no) {
      low = mid + 1;
    } else {
      high = mid;
    }
  }
  return nullptr;
}

/// \brief Whether a field table is sorted by field number.
///
/// \param table The table to check.
/// \return True when Field() may binary-search it.
///
/// The generator asserts this beside every table it writes, so a mis-sorted table is a translation
/// error rather than a lookup that quietly finds nothing.
[[nodiscard]] constexpr bool FieldsAreSorted(const TableDef &table) {
  for (std::size_t i = 1; i < table.fields.size(); ++i) {
    if (!(table.fields[i - 1].no < table.fields[i].no)) { return false; }
  }
  return true;
}

/// \brief Finds a field by where it sits in the record.
///
/// \param table  The table to search.
/// \param offset The field's offset within the record.
/// \return The field, or `nullptr` when no field sits there.
///
/// This is what lets generated code name a FIELD where AL names a field -- `FieldError(Code)`
/// rather than `FieldError(Field_No::Code)` -- because the address of a member is enough to find
/// its declaration.
///
/// \note A walk rather than a search, because offsets have no useful order and this is only ever
///       reached on an error path, where one pass over a few hundred entries costs nothing.
[[nodiscard]] constexpr const FieldDef *FieldAtOffset(const TableDef &table, std::size_t offset) {
  for (const FieldDef &f : table.fields) {
    if (f.offset == offset) { return &f; }
  }
  return nullptr;
}

}
