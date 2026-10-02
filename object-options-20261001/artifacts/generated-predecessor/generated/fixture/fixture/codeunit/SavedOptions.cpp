// Generated from SavedOptions.Codeunit.al. Do not edit.

#include "SavedOptions.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "dotnet/Refused.h"
#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/Page.h"
#include "runtime/Record.h"
#include "runtime/Report.h"
#include "runtime/Table.h"
#include "type/Integer.h"
#include "type/ObjectType.h"
#include "type/Stream.h"
#include "type/Text.h"
#include "type/TextEncoding.h"

#include "fixture/table/StoredFlag.h"
#include "fixture/table/StoredFlag.h"

namespace agiru::Fixture {

namespace {
namespace SavedOptions_unit {
const RegisterCodeunit<SavedOptions_Codeunit> kInCodeunitCatalogue;
} // namespace SavedOptions_unit
} // namespace

::agiru::Integer SavedOptions_Codeunit::Exercise() {
  [[maybe_unused]] absent::ObjectOptions Options{};
  [[maybe_unused]] absent::ObjectOptions MemoryOptions{};
  [[maybe_unused]] Temporary<::agiru::Fixture::StoredFlag_Table> Flag{};
  [[maybe_unused]] ::agiru::Integer Ordinal{};
  [[maybe_unused]] OutStream Output{};
  [[maybe_unused]] InStream Input{};
  [[maybe_unused]] ::agiru::Text<0> Payload{};

  Options.ObjectType = RefusedOption("Options.Object Type::Report");
  Ordinal = Options.ObjectType;
  if (Ordinal != 3) {
    ::agiru::RaiseOrCollect("Report ordinal");
  }
  Options.ObjectType = RefusedOption("Options.Object Type::XMLport");
  Ordinal = Options.ObjectType;
  if (Ordinal != 6) {
    ::agiru::RaiseOrCollect("XMLport ordinal");
  }
  Options.ObjectType = RefusedOption("Options.Object Type::Page");
  Ordinal = Options.ObjectType;
  if (Ordinal != 8) {
    ::agiru::RaiseOrCollect("Page ordinal");
  }
  Options.Temporary(true);
  if (!Options.Temporary() || Options.IsTemporary()) {
    ::agiru::RaiseOrCollect("Stored flag");
  }
  MemoryOptions.Temporary(true);
  if (!MemoryOptions.Temporary() || !MemoryOptions.IsTemporary()) {
    ::agiru::RaiseOrCollect("Native temporary flag");
  }
  Flag.Temporary = true;
  if (!Flag.Temporary || !Flag.IsTemporary()) {
    ::agiru::RaiseOrCollect("Ordinary temporary flag");
  }
  Options.OptionData.CreateOutStream(Output, ::agiru::TextEncoding::UTF8);
  Output.WriteText("Previous content that must disappear");
  Clear(Options.OptionData);
  Options.OptionData.CreateOutStream(Output, ::agiru::TextEncoding::UTF8);
  Output.WriteText("München 中文");
  Options.OptionData.CreateInStream(Input, ::agiru::TextEncoding::UTF8);
  Input.ReadText(Payload);
  if (Payload != "München 中文") {
    ::agiru::RaiseOrCollect("UTF8 replacement payload");
  }
  Options.ObjectType = RefusedOption("Options.Object Type::Report");
  if (Format(Options.ObjectType) != "Report") {
    ::agiru::RaiseOrCollect("Report format");
  }
  Options.SetRange(Options.ObjectType, RefusedOption("Options.Object Type::Report"));
  Ordinal = Options.GetRangeMin(Options.ObjectType);
  if (Ordinal != 3) {
    ::agiru::RaiseOrCollect("Typed report range");
  }
  return 9;
}

void SavedOptions_Codeunit::ClearAll() {
}

constexpr CodeunitDef kSavedOptionsCodeunit{
    .id = ::agiru::CodeunitTraits<SavedOptions_Codeunit>::kId,
    .name = ::agiru::CodeunitTraits<SavedOptions_Codeunit>::kName,
    .subtype = ::agiru::CodeunitTraits<SavedOptions_Codeunit>::kSubtype,
};

} // namespace agiru::Fixture
