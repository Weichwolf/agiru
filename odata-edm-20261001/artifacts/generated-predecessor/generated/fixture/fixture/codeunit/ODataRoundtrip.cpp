// Generated from OData.Codeunit.al. Do not edit.

#include "ODataRoundtrip.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "dotnet/Refused.h"
#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/Blob.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/Text.h"
#include "type/TextEncoding.h"


namespace agiru::Fixture {

namespace {
namespace ODataRoundtrip_unit {
const RegisterCodeunit<ODataRoundtrip_Codeunit> kInCodeunitCatalogue;
} // namespace ODataRoundtrip_unit
} // namespace

::agiru::Integer ODataRoundtrip_Codeunit::Exercise() {
  [[maybe_unused]] absent::ODataEdmType Definition{};
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
