// Generated from OData.Codeunit.al. Do not edit.

#include "ODataRoundtrip.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "platform/ODataEdmType.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/Blob.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/Text.h"
#include "type/TextEncoding.h"

#include "platform/ODataEdmType.h"
#include "platform/ODataEdmType.h"
#include "meta/Declare.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include <cstddef>

static_assert(::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.id == ::agiru::TableId{2000000179} && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.name == "OData Edm Type", "native table identity mismatch: OData Edm Type");
static_assert([] {
  std::size_t declared = 0;
  for (const auto &field : ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.fields) {
    bool system = false;
    for (const auto &implicit : ::agiru::kSystemFields) {
      system = system || field.no == implicit.no;
    }
    if (!system) { ++declared; }
  }
  return declared == 3;
}(), "native field count mismatch: OData Edm Type");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable, ::agiru::FieldNo{1});
  if (field == nullptr || field->name != "Key" || field->caption != "Key" || field->type != ::agiru::FieldType::Code || field->length != 50) { return false; }
  return true;
}(), "native field declaration mismatch: OData Edm Type.Key");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable, ::agiru::FieldNo{2});
  if (field == nullptr || field->name != "Description" || field->caption != "Description" || field->type != ::agiru::FieldType::Text || field->length != 250) { return false; }
  return true;
}(), "native field declaration mismatch: OData Edm Type.Description");
static_assert([] {
  const auto *field = ::agiru::Field(::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable, ::agiru::FieldNo{10});
  if (field == nullptr || field->name != "Edm Xml" || field->caption != "Edm Xml" || field->type != ::agiru::FieldType::Blob || field->length != 0) { return false; }
  return true;
}(), "native field declaration mismatch: OData Edm Type.Edm Xml");
static_assert(::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys.size() == 1, "native key count mismatch: OData Edm Type");
static_assert(::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys.size() > 0 && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].name == "Key1" && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].fields.size() == 1 && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].fields[0] == ::agiru::FieldNo{1} && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].clustered == true && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].enabled == true && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].maintainSiftIndex == true && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].maintainSqlIndex == true && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].unique == false && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].includedFields == "" && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].description == "" && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].obsoleteState == "" && ::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.keys[0].sumIndexFields.size() == 0, "native key declaration mismatch: OData Edm Type.Key1");
static_assert(::agiru::TableTraits<::agiru::platform::ODataEdmType>::kTable.dataPerCompany == false, "native company scope mismatch: OData Edm Type");

#include "platform/ODataEdmType.h"

namespace agiru::Fixture {

namespace {
namespace ODataRoundtrip_unit {
const RegisterCodeunit<ODataRoundtrip_Codeunit> kInCodeunitCatalogue;
} // namespace ODataRoundtrip_unit
} // namespace

::agiru::Integer ODataRoundtrip_Codeunit::Exercise() {
  [[maybe_unused]] Temporary<::agiru::platform::ODataEdmType> Definition{};
  [[maybe_unused]] ::agiru::RecordRef Reflected{};
  [[maybe_unused]] OutStream Writer{};
  [[maybe_unused]] InStream Reader{};
  [[maybe_unused]] ::agiru::Text<0> Payload{};

  if (!Definition.IsTemporary()) {
    ::agiru::RaiseOrCollect("Temporary record");
  }
  if (Definition.FieldNo(Definition.Key) != 1) {
    ::agiru::RaiseOrCollect("Original key field");
  }
  if (Definition.FieldNo(Definition.Description) != 2) {
    ::agiru::RaiseOrCollect("Original description field");
  }
  if (Definition.FieldNo(Definition.EdmXml) != 10) {
    ::agiru::RaiseOrCollect("Original Blob field");
  }
  Reflected.GetTable(Definition);
  if (Reflected.Number() != 2000000179) {
    ::agiru::RaiseOrCollect("Original System identity");
  }
  if (!Reflected.FieldExist(10) || Reflected.FieldExist(3)) {
    ::agiru::RaiseOrCollect("Original reflected field identity");
  }
  if (Reflected.Field(10).Name() != "Edm Xml") {
    ::agiru::RaiseOrCollect("Original reflected field name");
  }
  Definition.Key = "SCHEMA";
  Definition.Description = "Definition";
  if (Definition.Key != "SCHEMA" || Definition.Description != "Definition") {
    ::agiru::RaiseOrCollect("Original Code and Text fields");
  }
  if (Definition.EdmXml.HasValue()) {
    ::agiru::RaiseOrCollect("Initially empty Blob");
  }
  Definition.EdmXml.CreateOutStream(Writer, ::agiru::TextEncoding::UTF8);
  Writer.WriteText("<Schema Name=\"Ägirū\"/>");
  if (!Definition.EdmXml.HasValue()) {
    ::agiru::RaiseOrCollect("Populated UTF8 Blob");
  }
  Definition.EdmXml.CreateInStream(Reader, ::agiru::TextEncoding::UTF8);
  Reader.Read(Payload);
  if (Payload != "<Schema Name=\"Ägirū\"/>") {
    ::agiru::RaiseOrCollect("Original UTF8 Blob read");
  }
  Clear(Definition.EdmXml);
  if (Definition.EdmXml.HasValue()) {
    ::agiru::RaiseOrCollect("Blob Clear");
  }
  Definition.EdmXml.CreateOutStream(Writer, ::agiru::TextEncoding::UTF8);
  Writer.WriteText("<B/>");
  Definition.EdmXml.CreateInStream(Reader, ::agiru::TextEncoding::UTF8);
  Reader.Read(Payload);
  if (Payload != "<B/>") {
    ::agiru::RaiseOrCollect("Shorter replacement has no old tail");
  }
  return 13;
}

void ODataRoundtrip_Codeunit::ClearAll() {
}

constexpr CodeunitDef kODataRoundtripCodeunit{
    .id = ::agiru::CodeunitTraits<ODataRoundtrip_Codeunit>::kId,
    .name = ::agiru::CodeunitTraits<ODataRoundtrip_Codeunit>::kName,
    .subtype = ::agiru::CodeunitTraits<ODataRoundtrip_Codeunit>::kSubtype,
};

} // namespace agiru::Fixture
