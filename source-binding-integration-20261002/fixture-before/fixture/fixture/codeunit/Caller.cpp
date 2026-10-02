// Generated from Caller.Codeunit.al. Do not edit.

#include "Caller.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "runtime/Table.h"
#include "type/Boolean.h"
#include "type/Decimal.h"
#include "type/Option.h"
#include "type/Text.h"

#include "fixture/table/SourceRow.h"
#include "options/Types.h"
#include "fixture/table/SourceRow.h"

namespace agiru::Fixture {

namespace {
namespace Caller_unit {
const RegisterCodeunit<Caller_Codeunit> kInCodeunitCatalogue;
} // namespace Caller_unit
} // namespace

::agiru::Boolean Caller_Codeunit::Run() {
  [[maybe_unused]] Temporary<::agiru::Fixture::SourceRow_Table> Row{};
  [[maybe_unused]] ::agiru::Text<30> Value{};
  [[maybe_unused]] ::agiru::Text<30> Copy{};
  [[maybe_unused]] Option<::agiru::options::OptionBlankAB> State{};
  [[maybe_unused]] Decimal Amount{};

  Value = "original";
  Copy = "updated";
  Row.Change(Value, Copy);
  if (Value != "updated") {
    ::agiru::RaiseOrCollect("Reference argument was not written back");
  }
  if (Copy != "updated") {
    ::agiru::RaiseOrCollect("Value argument escaped its copy");
  }
  Row.AddedChange(Value);
  if (Value != "extension") {
    ::agiru::RaiseOrCollect("Extension reference argument was not written back");
  }
  State = ::agiru::Option<::agiru::options::OptionBlankAB>{::agiru::options::OptionBlankAB::A};
  Row.ChangeOption(State);
  if (State != ::agiru::Option<::agiru::options::OptionBlankAB>{::agiru::options::OptionBlankAB::B}) {
    ::agiru::RaiseOrCollect("Option reference argument was not written back");
  }
  if (!::agiru::Tried([&] { return Row.TryChange(Amount); })) {
    ::agiru::RaiseOrCollect("TryFunction call failed");
  }
  if (Amount != 1) {
    ::agiru::RaiseOrCollect("Decimal reference argument was not written back");
  }
  return true;
}

void Caller_Codeunit::ClearAll() {
}

constexpr CodeunitDef kCallerCodeunit{
    .id = ::agiru::CodeunitTraits<Caller_Codeunit>::kId,
    .name = ::agiru::CodeunitTraits<Caller_Codeunit>::kName,
    .subtype = ::agiru::CodeunitTraits<Caller_Codeunit>::kSubtype,
};

} // namespace agiru::Fixture
