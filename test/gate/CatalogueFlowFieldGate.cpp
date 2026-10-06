#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/ModuleDef.h"
#include "meta/PageDef.h"
#include "meta/TableDef.h"
#include "platform/Field.h"
#include "platform/PageMetadata.h"
#include "platform/TableMetadata.h"
#include "runtime/Catalogue.h"
#include "runtime/Database.h"
#include "runtime/ErrorValue.h"
#include "runtime/Record.h"
#include "runtime/RecordRef.h"
#include "runtime/RecordState.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/TableDefinition.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Decimal.h"
#include "type/FieldClass.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Option.h"
#include "type/Text.h"

#include "CatalogueNavigation.h"
#include "Check.h"
#include "Filter.h"
#include "OwnedDatabase.h"

#include <array>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

namespace {

constexpr std::size_t kPhraseLength = 50;
constexpr std::size_t kTableCaptionLength = 250;
constexpr std::size_t kFieldCaptionLength = 80;
constexpr std::size_t kResultLength = 500;
constexpr agiru::Integer kSourceBufferID = 71;
constexpr agiru::Integer kSourceFilterID = 900;
constexpr agiru::FieldNo kTableCaptionNo{10};
constexpr agiru::FieldNo kFieldCaptionNo{11};
constexpr agiru::FieldNo kPageCaptionNo{12};
constexpr agiru::FieldNo kWholeNo{13};
constexpr agiru::FieldNo kExactNo{14};
constexpr agiru::FieldNo kTruthNo{15};
constexpr agiru::FieldNo kIdentityNo{16};
constexpr agiru::FieldNo kClassNo{17};
constexpr agiru::FieldNo kVersionNo{18};
constexpr agiru::FieldNo kTextNo{19};

struct ScanCounts {
  std::size_t keys = 0;
  std::size_t projections = 0;
  std::size_t visits = 0;
  agiru::Integer sum = 0;
  bool stop = false;
};

constexpr agiru::ModuleDef kOwner{.id = "118874ab-44bc-4ccb-9daf-59763539ab16",
                                  .name = "Catalogue FlowField gate",
                                  .publisher = "agiru tests",
                                  .version = "1.0.0.0"};

struct Row : agiru::Table<Row> {
  static constexpr agiru::TableId kId{62431};
  static constexpr std::string_view kName = "Catalogue FlowField Gate";
  agiru::detail::StateHandle State_Block;
  agiru::Integer ID{};
  agiru::Text<kPhraseLength> Phrase;
  agiru::Text<1> Small;
  agiru::Text<2> Wide;
  agiru::Integer TargetID{kId.Value()};
  agiru::Integer FieldID{2};
  agiru::Integer PageID{62440};
  agiru::Integer NumberFilter{};
  agiru::Text<kPhraseLength> Needle;
  agiru::Text<kTableCaptionLength> TableCaption;
  agiru::Text<kFieldCaptionLength> FieldCaption;
  agiru::Text<kTableCaptionLength> PageCaption;
  agiru::Integer Whole{};
  agiru::Decimal Exact;
  agiru::Boolean Truth{};
  agiru::Guid Identity;
  agiru::Option<agiru::platform::FieldClass> Class;
  agiru::BigInteger Version{};
  agiru::Text<kResultLength> TextValue;
  ScanCounts *Counters = nullptr;
};

}

template <> struct agiru::TableTraits<Row> {
  static constexpr std::array kFields{
      agiru::Declare<&Row::ID>(agiru::FieldNo{1}, "ID", "ID", offsetof(Row, ID)),
      agiru::Declare<&Row::Phrase>(
          agiru::FieldNo{2}, "Phrase", "  A*|B's  ", offsetof(Row, Phrase)),
      agiru::Declare<&Row::Small>(agiru::FieldNo{3}, "Small", "@A..B", offsetof(Row, Small)),
      agiru::Declare<&Row::Wide>(agiru::FieldNo{4}, "Wide", "Wide", offsetof(Row, Wide)),
      agiru::Declare<&Row::TargetID>(
          agiru::FieldNo{5}, "Target ID", "Target ID", offsetof(Row, TargetID)),
      agiru::Declare<&Row::FieldID>(
          agiru::FieldNo{6}, "Field ID", "Field ID", offsetof(Row, FieldID)),
      agiru::Declare<&Row::PageID>(agiru::FieldNo{7}, "Page ID", "Page ID", offsetof(Row, PageID)),
      agiru::Declare<&Row::NumberFilter>(
          agiru::FieldNo{8},
          "Number Filter",
          "Number Filter",
          offsetof(Row, NumberFilter),
          agiru::Declared{.fieldClass = agiru::FieldClass::FlowFilter}),
      agiru::Declare<&Row::Needle>(agiru::FieldNo{9}, "Needle", "Needle", offsetof(Row, Needle)),
      agiru::Declare<&Row::TableCaption>(
          kTableCaptionNo,
          "Table Caption",
          "Table Caption",
          offsetof(Row, TableCaption),
          agiru::Declared{.fieldClass = agiru::FieldClass::FlowField,
                          .calcFormula =
                              R"(lookup("Table Metadata".Caption where(ID=field("Target ID"))))"}),
      agiru::Declare<&Row::FieldCaption>(
          kFieldCaptionNo,
          "Field Caption",
          "Field Caption",
          offsetof(Row, FieldCaption),
          agiru::Declared{.fieldClass = agiru::FieldClass::FlowField,
                          .calcFormula =
                              "lookup(Field.\"Field Caption\" where(TableNo=field(\"Target "
                              "ID\"),\"No.\"=field(\"Field ID\")))"}),
      agiru::Declare<&Row::PageCaption>(
          kPageCaptionNo,
          "Page Caption",
          "Page Caption",
          offsetof(Row, PageCaption),
          agiru::Declared{.fieldClass = agiru::FieldClass::FlowField,
                          .calcFormula =
                              R"(lookup("Page Metadata".Caption where(ID=field("Page ID"))))"}),
      agiru::Declare<&Row::Whole>(kWholeNo, "Whole", "Whole", offsetof(Row, Whole)),
      agiru::Declare<&Row::Exact>(kExactNo, "Exact", "Exact", offsetof(Row, Exact)),
      agiru::Declare<&Row::Truth>(kTruthNo, "Truth", "Truth", offsetof(Row, Truth)),
      agiru::Declare<&Row::Identity>(kIdentityNo, "Identity", "Identity", offsetof(Row, Identity)),
      agiru::Declare<&Row::Class>(kClassNo, "Class", "Class", offsetof(Row, Class)),
      agiru::Declare<&Row::Version>(kVersionNo, "Version", "Version", offsetof(Row, Version)),
      agiru::Declare<&Row::TextValue>(
          kTextNo, "Text Value", "Text Value", offsetof(Row, TextValue))};
  static constexpr std::array kPrimary{agiru::FieldNo{1}};
  static constexpr std::array kKeys{agiru::KeyDef{.name = "Primary", .fields = kPrimary}};
  static constexpr agiru::TableDef kTable{.id = Row::kId,
                                          .name = Row::kName,
                                          .caption = "Independent table caption",
                                          .fields = kFields,
                                          .keys = kKeys,
                                          .module = &kOwner};
};

namespace {

const agiru::RegisterTable<Row> kRegister;
constexpr agiru::PageDef kPage{.id = agiru::PageId{62440},
                               .name = "First page",
                               .caption = "Independent page caption",
                               .source = Row::kId,
                               .module = &kOwner};
constexpr agiru::PageDef kOtherPage{.id = agiru::PageId{62441},
                                    .name = "Second page",
                                    .caption = "Second caption",
                                    .source = Row::kId,
                                    .module = &kOwner};
constexpr agiru::PageDef kUnownedPage{
    .id = agiru::PageId{62442}, .name = "Unqualified page", .source = Row::kId};
constexpr std::array kPages{agiru::PageEntry{.page = &kPage, .run = nullptr},
                            agiru::PageEntry{.page = &kOtherPage, .run = nullptr},
                            agiru::PageEntry{.page = &kUnownedPage, .run = nullptr}};

void Calculate(Row &row, agiru::FieldNo no, std::string_view formula) {
  auto fields = agiru::TableTraits<Row>::kFields;
  for (auto &field : fields) {
    if (field.no == no) {
      field.fieldClass = agiru::FieldClass::FlowField;
      field.calcFormula = formula;
    }
  }
  auto table = agiru::TableTraits<Row>::kTable;
  table.fields = fields;
  agiru::detail::CalcField(&row, table, row.State_Block.Peek(), no);
}

std::string Failure(auto &&operation) {
  try {
    operation();
  } catch (const agiru::Error &error) { return error.what(); }
  return {};
}

void NativeLookups() {
  Row row;
  row.ID = kSourceBufferID;
  row.Needle = "unchanged buffer";
  row.SetRange(row.ID, kSourceFilterID);
  row.FilterGroup(2);
  row.SetFilter(row.NumberFilter, "1..4");
  CHECK_TRUE("typed CalcFields uses all three native providers without a SQL session",
             row.CalcFields(row.TableCaption, row.FieldCaption, row.PageCaption));
  CHECK_TEXT("Table Metadata lookup keeps Caption independent of Name",
             row.TableCaption.Value(),
             "Independent table caption");
  CHECK_TEXT("Field lookup keeps whitespace and filter metacharacters",
             row.FieldCaption.Value(),
             "  A*|B's  ");
  CHECK_TEXT("Page lookup uses its independent caption",
             row.PageCaption.Value(),
             "Independent page caption");
  CHECK_TRUE("calculating only changes requested values",
             row.ID == kSourceBufferID && row.TargetID == Row::kId.Value() && row.FieldID == 2 &&
                 row.Needle.Value() == "unchanged buffer");
  CHECK_TRUE("source filter groups remain unchanged",
             row.FilterGroup() == 2 && row.GetFilter(row.NumberFilter) == "1..4");
  row.FilterGroup(0);
  CHECK_TEXT("ordinary source filters do not constrain the destination lookup",
             row.GetFilter(row.ID),
             "900");
  agiru::RecordRef ref;
  ref.GetTable(row);
  ref.Field(kFieldCaptionNo.Value()).Value("stale");
  CHECK_TRUE("FieldRef.CalcField shares the native CalcFields calculation",
             ref.Field(kFieldCaptionNo.Value()).CalcField());
  CHECK_TEXT("FieldRef retrieves the same exact caption",
             ref.Field(kFieldCaptionNo.Value()).ToText(),
             "  A*|B's  ");
  CHECK_TEXT(
      "RecordRef owns an independent calculated buffer", row.FieldCaption.Value(), "  A*|B's  ");
  row.TargetID = 0;
  row.FieldID = 0;
  row.PageID = 0;
  row.CalcFields(row.TableCaption, row.FieldCaption, row.PageCaption);
  CHECK_TRUE("missing native lookups clear stale values rather than fabricate rows",
             row.TableCaption.IsEmpty() && row.FieldCaption.IsEmpty() && row.PageCaption.IsEmpty());
}

void AggregatesAndExactValues() {
  Row row;
  constexpr auto lengths = R"(Field.Len where(TableNo=field("Target ID"),"No."=filter(2..4)))";
  Calculate(row, kWholeNo, std::string("sum(") + lengths + ")");
  CHECK_TRUE("native Sum includes all three declared lengths", row.Whole == 53);
  Calculate(row, kExactNo, std::string("average(") + lengths + ")");
  CHECK_TEXT("native Average uses Decimal rather than binary float",
             row.Exact.ToInvariantString(),
             "17.666666666666666666666666667");
  Calculate(
      row, kWholeNo, R"(average(Field.Len where(TableNo=field("Target ID"),"No."=filter(3..4))))");
  CHECK_TRUE("whole-number Average rounds the half away from zero", row.Whole == 2);
  Calculate(
      row, kWholeNo, R"(-average(Field.Len where(TableNo=field("Target ID"),"No."=filter(3..4))))");
  CHECK_TRUE("negative Average preserves whole-number rounding", row.Whole == -2);
  Calculate(row, kExactNo, std::string("-sum(") + lengths + ")");
  CHECK_TEXT("negative Sum applies the declared sign", row.Exact.ToInvariantString(), "-53");
  Calculate(row, kWholeNo, std::string("min(") + lengths + ")");
  CHECK_TRUE("native Min compares actual typed values", row.Whole == 1);
  Calculate(row, kWholeNo, std::string("max(") + lengths + ")");
  CHECK_TRUE("native Max compares actual typed values", row.Whole == 50);
  Calculate(row, kWholeNo, R"(count(Field where(TableNo=field("Target ID"),"No."=filter(2..4))))");
  CHECK_TRUE("native Count retains every matching identity", row.Whole == 3);
  Calculate(row, kWholeNo, R"(count("Page Metadata" where(ID=filter(62440..62442))))");
  CHECK_TRUE("native Count retains unqualified page identities without inventing properties",
             row.Whole == 3);
  Calculate(row, kTruthNo, R"(exist("Page Metadata" where(ID=const(62442))))");
  CHECK_TRUE("native Exist can test an unqualified identity without fabricating its caption",
             row.Truth);
  Calculate(row, kTruthNo, R"(exist(Field where(TableNo=field("Target ID"),"No."=const(2))))");
  CHECK_TRUE("native Exist finds a declared field", row.Truth);
  Calculate(row, kTruthNo, R"(-exist(Field where(TableNo=field("Target ID"),"No."=const(2))))");
  CHECK_TRUE("negative Exist negates the matching result", !row.Truth);
  Calculate(
      row, kClassNo, R"(lookup(Field.Class where(TableNo=field("Target ID"),"No."=const(10))))");
  CHECK_TRUE("native lookup preserves the original option ordinal",
             row.Class == agiru::platform::FieldClass::FlowField);
  Calculate(row, kIdentityNo, R"(lookup("Table Metadata"."App ID" where(ID=field("Target ID"))))");
  CHECK_TRUE("native lookup preserves the declaring module GUID",
             row.Identity == *agiru::Guid::FromText(kOwner.id));
  Calculate(
      row, kVersionNo, R"(lookup("Page Metadata".SystemRowVersion where(ID=field("Page ID"))))");
  CHECK_TRUE("native lookup retains its BigInteger version", row.Version == 1);
  Calculate(
      row, kTruthNo, R"(min(Field.Enabled where(TableNo=field("Target ID"),"No."=filter(2..4))))");
  CHECK_TRUE("native Boolean Min compares stored truth values", row.Truth);
  row.TargetID = 0;
  Calculate(row, kExactNo, std::string("average(") + lengths + ")");
  CHECK_TRUE("empty Average clears a stale Decimal", row.Exact == agiru::Decimal{});
  Calculate(row, kWholeNo, std::string("max(") + lengths + ")");
  CHECK_TRUE("empty Max clears a stale Integer", row.Whole == 0);
  Calculate(row, kTruthNo, "-exist(Field where(TableNo=field(\"Target ID\")))");
  CHECK_TRUE("negative Exist is true on an empty selection", row.Truth);
}

void FilterOperandModes() {
  Row row;
  row.SetFilter(row.NumberFilter, "2|4");
  Calculate(row,
            kWholeNo,
            R"(count(Field where(TableNo=field("Target ID"),"No."=field("Number Filter"))))");
  CHECK_TRUE("FIELD on FlowFilter applies its union", row.Whole == 2);
  row.SetFilter(row.FieldID, "2..4");
  Calculate(row,
            kWholeNo,
            R"(count(Field where(TableNo=field("Target ID"),"No."=field(filter("Field ID")))))");
  CHECK_TRUE("FIELD(FILTER) reads the source view rather than its scalar buffer", row.Whole == 3);
  Calculate(row,
            kWholeNo,
            "count(Field where(TableNo=field(\"Target "
            "ID\"),\"No.\"=field(upperlimit(filter(\"Field ID\")))))");
  CHECK_TRUE("FIELD(UPPERLIMIT(FILTER)) retains the bounded prefix", row.Whole == 4);
  row.FieldID = 3;
  Calculate(
      row,
      kWholeNo,
      R"(count(Field where(TableNo=field("Target ID"),"No."=field(upperlimit("Field ID")))))");
  CHECK_TRUE("FIELD(UPPERLIMIT) reads the scalar when not a FlowFilter", row.Whole == 3);
  Calculate(row,
            kWholeNo,
            "count(Field where(TableNo=const(database::\"Catalogue FlowField "
            "Gate\"),\"No.\"=filter(2..4),\"No.\"=filter(3..5)))");
  CHECK_TRUE("database constants and duplicate predicates retain both constraints", row.Whole == 2);
  row.Needle = "  A*|B's  ";
  Calculate(row,
            kWholeNo,
            R"(count(Field where(TableNo=field("Target ID"),"Field Caption"=field(Needle))))");
  CHECK_TRUE("ordinary FIELD values never become wildcard/union expressions", row.Whole == 1);
  row.Needle = "@A..B";
  Calculate(row,
            kWholeNo,
            R"(count(Field where(TableNo=field("Target ID"),"Field Caption"=field(Needle))))");
  CHECK_TRUE("ordinary FIELD retains literal at-sign and range punctuation", row.Whole == 1);
  Calculate(row,
            kWholeNo,
            "count(Field where(TableNo=field(\"Target "
            "ID\"),Enabled=const(true),Class=const(Normal),\"No.\"=filter(2..4)))");
  CHECK_TRUE("CONST preserves Boolean and option names", row.Whole == 3);
  row.SetRange(row.NumberFilter);
  Calculate(row,
            kWholeNo,
            R"(count(Field where(TableNo=field("Target ID"),"No."=field("Number Filter"))))");
  CHECK_TRUE("absent FlowFilters do not become scalar zero constraints", row.Whole == 19);
  Calculate(row, kTextNo, "lookup(\"Page Metadata\".Caption where(ID=filter(62440..62441)))");
  CHECK_TEXT("Lookup chooses the first matching primary-key identity",
             row.TextValue.Value(),
             "Independent page caption");
}

void ExplicitRefusalsAndReadOnly() {
  Row row;
  CHECK_TRUE(
      "native calculated predicate columns refuse instead of filtering stale defaults",
      Failure([&] {
        Calculate(
            row,
            kWholeNo,
            R"(count(Field where(TableNo=field("Target ID"),SystemCreatedByUserName=const(''))))");
      }).contains("unqualified calculated column"));
  CHECK_TRUE("unprojected Field result attributes refuse even without a filter",
             Failure([&] {
               Calculate(row,
                         kClassNo,
                         "lookup(Field.DataClassification where(TableNo=field(\"Target "
                         "ID\"),\"No.\"=const(2)))");
             }).contains("unqualified metadata attribute"));
  CHECK_TRUE("unprojected Field predicate attributes refuse",
             Failure([&] {
               Calculate(
                   row,
                   kWholeNo,
                   "count(Field where(TableNo=field(\"Target ID\"),SQLDataType=const(Integer)))");
             }).contains("unqualified metadata attribute"));
  CHECK_TRUE("unknown tables remain gaps instead of empty catalogues",
             Failure([&] {
               Calculate(row, kWholeNo, "count(UnknownCatalogue)");
             }).contains("does not carry"));
  CHECK_TRUE("unknown destination fields refuse",
             Failure([&] {
               Calculate(row, kTextNo, "lookup(Field.UnknownColumn)");
             }).contains("undeclared field"));
  CHECK_TRUE("unknown source operands refuse",
             Failure([&] {
               Calculate(row, kWholeNo, "count(Field where(TableNo=field(UnknownSource)))");
             }).contains("does not declare"));
  CHECK_TRUE("unqualified page rows refuse rather than invent captions",
             Failure([&] {
               Calculate(row, kTextNo, "lookup(\"Page Metadata\".Caption where(ID=const(62442)))");
             }).contains("original module"));
  CHECK_TRUE("numeric aggregates refuse text result columns",
             Failure([&] {
               Calculate(row,
                         kExactNo,
                         "sum(Field.\"Field Caption\" where(TableNo=field(\"Target ID\")))");
             }).contains("numeric fields"));
  agiru::platform::Field field;
  CHECK_TRUE("native Field remains read-only after catalogue calculations",
             Failure([&] { field.Insert(); }).contains("read-only live catalogue"));
  agiru::platform::TableMetadata table;
  CHECK_TRUE("native Table Metadata remains read-only",
             Failure([&] { table.Insert(); }).contains("read-only live catalogue"));
  agiru::platform::PageMetadata page;
  CHECK_TRUE("native Page Metadata remains read-only",
             Failure([&] { page.Insert(); }).contains("read-only live catalogue"));
}

void SharedSqlOperandRegression() {
  const gate::OwnedDatabase database("catalogue_flowfield_sql");
  const agiru::Session session(database.Dsn());
  agiru::CreateTable(session.Database(), agiru::TableTraits<Row>::kTable);
  Row first;
  first.ID = 2;
  first.Phrase = "  A*|B's  ";
  first.Exact = agiru::Decimal::FromInvariantString("0.1000000000000000000000000001");
  first.Insert();
  CHECK_TEXT("SQL writes retain the finer calculating buffer",
             first.Exact.ToInvariantString(),
             "0.1000000000000000000000000001");
  const auto stored =
      session.Database().Execute(R"(SELECT "Exact" FROM "Catalogue FlowField Gate" WHERE "ID"=2)");
  CHECK_TRUE("independent SQL observes the existing scale-20 storage boundary",
             stored.Value(0, 0) == "0.10000000000000000000");
  Row second;
  second.ID = 4;
  second.Phrase = "@A..B";
  second.Exact = agiru::Decimal::FromInvariantString("0.2000000000000000000000000002");
  second.Insert();
  Row request;
  request.Needle = first.Phrase;
  Calculate(request, kWholeNo, "count(\"Catalogue FlowField Gate\" where(Phrase=field(Needle)))");
  CHECK_TRUE("shared operand resolution preserves SQL literal whitespace/metacharacters",
             request.Whole == 1);
  request.Needle = second.Phrase;
  Calculate(request, kWholeNo, "count(\"Catalogue FlowField Gate\" where(Phrase=field(Needle)))");
  CHECK_TRUE("SQL FIELD operands retain literal at-sign and range punctuation", request.Whole == 1);
  request.SetFilter(request.NumberFilter, "2|4");
  Calculate(request,
            kExactNo,
            R"(sum("Catalogue FlowField Gate".Exact where(ID=field("Number Filter"))))");
  CHECK_TEXT("SQL FlowFilter aggregate uses persisted scale-20 values without binary float",
             request.Exact.ToInvariantString(),
             "0.30000000000000000000");
  Calculate(request,
            kWholeNo,
            "count(\"Catalogue FlowField Gate\" where(ID=filter(2..4),ID=filter(3..5)))");
  CHECK_TRUE("SQL operand resolution preserves duplicate intersection constraints",
             request.Whole == 1);
  request.CalcFields(request.TableCaption, request.FieldCaption, request.PageCaption);
  CHECK_TEXT("SQL-backed source records still read authoritative native Field metadata",
             request.FieldCaption.Value(),
             "  A*|B's  ");
  Row automatic;
  CHECK_TRUE("native lookup fields can be registered for automatic calculation",
             automatic.SetAutoCalcFields(
                 automatic.TableCaption, automatic.FieldCaption, automatic.PageCaption));
  CHECK_TRUE("ordinary SQL Get calculates the native lookup fields through the shared runtime",
             automatic.Get(2) && automatic.TableCaption.Value() == "Independent table caption" &&
                 automatic.FieldCaption.Value() == "  A*|B's  " &&
                 automatic.PageCaption.Value() == "Independent page caption");
  const auto physical = session.Database().Execute("SELECT to_regclass('\"Field\"') IS NULL");
  CHECK_TRUE("catalogue calculations do not create a physical Field copy",
             physical.Value(0, 0) == "t");
}

constexpr std::array kScanKeys{1, 2, 3, 4, 5};
constexpr std::size_t kScanKeyBudget = 12;
constexpr agiru::Integer kProjectedMultiplier = 10;

void ScanKey(void *record, std::size_t index) {
  auto &row = *static_cast<Row *>(record);
  row.ID = kScanKeys[index];
  ++row.Counters->keys;
}

void ScanProjection(void *record, std::size_t index) {
  auto &row = *static_cast<Row *>(record);
  row.Whole = kScanKeys[index] * kProjectedMultiplier;
  ++row.Counters->projections;
}

bool ScanVisit(void *context, void *record) {
  auto &counts = *static_cast<ScanCounts *>(context);
  ++counts.visits;
  counts.sum += static_cast<Row *>(record)->ID;
  return !counts.stop;
}

void BoundedScanContract() {
  ScanCounts counts;
  Row candidate;
  candidate.Counters = &counts;
  const agiru::detail::CatalogueReader reader{.size = kScanKeys.size(),
                                              .candidate = &candidate,
                                              .selected = nullptr,
                                              .anchor = nullptr,
                                              .key = ScanKey,
                                              .project = ScanProjection};
  const std::array keyFilter{agiru::detail::ColumnPredicate{
      .field = agiru::FieldNo{1}, .expression = agiru::detail::ParseFilter("2|4")}};
  agiru::detail::CatalogueScan scan{
      .filters = keyFilter, .project = false, .context = &counts, .visit = ScanVisit};
  agiru::detail::ScanCatalogue(agiru::TableTraits<Row>::kTable, reader, scan);
  constexpr agiru::Integer kMatchingKeySum = 6;
  CHECK_TRUE("single-pass scans retain key holes and visit each match once",
             counts.visits == 2 && counts.sum == kMatchingKeySum);
  CHECK_TRUE("key-only calculations never project fabricated row properties",
             counts.projections == 0);
  CHECK_TRUE("bounded leading-key scans do not restart for every returned row",
             counts.keys <= kScanKeyBudget);
  counts = ScanCounts{.stop = true};
  agiru::detail::ScanCatalogue(agiru::TableTraits<Row>::kTable, reader, scan);
  CHECK_TRUE("lookup/existence consumers stop after one matching identity",
             counts.visits == 1 && counts.sum == 2);
  const std::array mixedFilters{
      keyFilter[0],
      agiru::detail::ColumnPredicate{.field = kWholeNo,
                                     .expression = agiru::detail::ParseFilter(">=30")}};
  counts = ScanCounts{};
  scan.filters = mixedFilters;
  agiru::detail::ScanCatalogue(agiru::TableTraits<Row>::kTable, reader, scan);
  CHECK_TRUE("non-key predicates project each eligible candidate exactly once",
             counts.projections == 2);
  CHECK_TRUE("projected predicates do not select stale buffer values",
             counts.visits == 1 && counts.sum == 4);
}

}

int main() {
  return gate::Run("CatalogueFlowField", [] {
    for (const auto &page : kPages) { agiru::RegisterPageEntry(&page); }
    NativeLookups();
    AggregatesAndExactValues();
    FilterOperandModes();
    ExplicitRefusalsAndReadOnly();
    BoundedScanContract();
    SharedSqlOperandRegression();
  });
}
