// Generated from Chart.Codeunit.al. Do not edit.

#include "ChartRoundtrip.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/CodeunitDef.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/Blob.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/Text.h"
#include "type/TextEncoding.h"

#include "platform/Chart.h"
#include "platform/Chart.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::Chart>::kTable.id == ::agiru::TableId{2000000078} && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.name == "Chart", "native table identity mismatch: Chart");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::Chart>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 3;
}(), "native field count mismatch: Chart");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Chart>::kTable, ::agiru::FieldNo{3});
  if (field == nullptr || field->name != "ID" || field->caption != "ID" || field->type != ::agiru::FieldType::Code || field->length != 20) { return false; }
  return true;
}(), "native field declaration mismatch: Chart.ID");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Chart>::kTable, ::agiru::FieldNo{6});
  if (field == nullptr || field->name != "Name" || field->caption != "Name" || field->type != ::agiru::FieldType::Text || field->length != 30) { return false; }
  return true;
}(), "native field declaration mismatch: Chart.Name");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::Chart>::kTable, ::agiru::FieldNo{9});
  if (field == nullptr || field->name != "BLOB" || field->caption != "BLOB" || field->type != ::agiru::FieldType::Blob || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: Chart.BLOB");
static_assert(::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys.size() == 1, "native key count mismatch: Chart");
static_assert(::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].name == "Key1" && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].fields[0] == ::agiru::FieldNo{3} && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::Chart>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: Chart.Key1");
static_assert(::agiru::TableTraits<::agiru::platform::Chart>::kTable.dataPerCompany == false, "native company scope mismatch: Chart");

#include "platform/Chart.h"

namespace agiru::Fixture {

namespace {
namespace ChartRoundtrip_unit {
const RegisterCodeunit<ChartRoundtrip_Codeunit> kInCodeunitCatalogue;
} // namespace ChartRoundtrip_unit
} // namespace

::agiru::Integer ChartRoundtrip_Codeunit::Exercise() {
  [[maybe_unused]] Temporary<::agiru::platform::Chart> Source{};
  [[maybe_unused]] Temporary<::agiru::platform::Chart> Target{};
  [[maybe_unused]] ::agiru::RecordRef Ref{};
  [[maybe_unused]] OutStream Writer{};
  [[maybe_unused]] InStream Reader{};
  [[maybe_unused]] ::agiru::Text<0> Contents{};

  if (Source.FieldNo(Source.ID) != 3) {
    ::agiru::RaiseOrCollect("Chart ID field");
  }
  if (Source.FieldNo(Source.Name) != 6) {
    ::agiru::RaiseOrCollect("Chart Name field");
  }
  if (Source.FieldNo(Source.BLOB) != 9) {
    ::agiru::RaiseOrCollect("Chart Blob field");
  }
  if (MaxStrLen(Source.ID) != 20) {
    ::agiru::RaiseOrCollect("Chart ID length");
  }
  if (MaxStrLen(Source.Name) != 30) {
    ::agiru::RaiseOrCollect("Chart Name length");
  }
  Ref.GetTable(Source);
  if (Ref.Number() != 2000000078) {
    ::agiru::RaiseOrCollect("Chart source identity");
  }
  if (!Ref.FieldExist(3)) {
    ::agiru::RaiseOrCollect("Chart reflected ID");
  }
  if (!Ref.FieldExist(6)) {
    ::agiru::RaiseOrCollect("Chart reflected Name");
  }
  if (!Ref.FieldExist(9)) {
    ::agiru::RaiseOrCollect("Chart reflected Blob");
  }
  if (Ref.FieldExist(1)) {
    ::agiru::RaiseOrCollect("Chart cannot invent field 1");
  }
  Source.ID = "  source  ";
  Source.Name = "  Mixed title  ";
  if (Source.ID != "SOURCE") {
    ::agiru::RaiseOrCollect("Chart Code normalization");
  }
  if (Source.Name != "  Mixed title  ") {
    ::agiru::RaiseOrCollect("Chart Text value");
  }
  Source.BLOB.CreateOutStream(Writer, ::agiru::TextEncoding::UTF8);
  Writer.WriteText("<Chart Name=\"Ägirū\"/>");
  Source.BLOB.CreateInStream(Reader, ::agiru::TextEncoding::UTF8);
  Reader.Read(Contents);
  if (Contents != "<Chart Name=\"Ägirū\"/>") {
    ::agiru::RaiseOrCollect("Chart UTF8 Blob");
  }
  Source.Insert();
  Target = Source;
  Target.ID = "copy";
  Target.Name = "Independent title";
  if (Source.ID != "SOURCE") {
    ::agiru::RaiseOrCollect("Source ID independence");
  }
  if (Source.Name != "  Mixed title  ") {
    ::agiru::RaiseOrCollect("Source Name independence");
  }
  Target.BLOB.CreateInStream(Reader, ::agiru::TextEncoding::UTF8);
  Reader.Read(Contents);
  if (Contents != "<Chart Name=\"Ägirū\"/>") {
    ::agiru::RaiseOrCollect("Copied Blob value");
  }
  Clear(Target.BLOB);
  if (Target.BLOB.HasValue()) {
    ::agiru::RaiseOrCollect("Target Blob Clear");
  }
  if (!Source.BLOB.HasValue()) {
    ::agiru::RaiseOrCollect("Source Blob independence");
  }
  if (!Target.IsTemporary()) {
    ::agiru::RaiseOrCollect("Target temporary ownership");
  }
  Target.Insert();
  if (Target.Count() != 1) {
    ::agiru::RaiseOrCollect("Target owns its row store");
  }
  if (Source.Count() != 1) {
    ::agiru::RaiseOrCollect("Source owns its row store");
  }
  return 21;
}

void ChartRoundtrip_Codeunit::ClearAll() {
}

constexpr CodeunitDef kChartRoundtripCodeunit{
    .id = ::agiru::CodeunitTraits<ChartRoundtrip_Codeunit>::kId,
    .name = ::agiru::CodeunitTraits<ChartRoundtrip_Codeunit>::kName,
    .subtype = ::agiru::CodeunitTraits<ChartRoundtrip_Codeunit>::kSubtype,
};

} // namespace agiru::Fixture
