// Generated from Probe.Codeunit.al. Do not edit.

#include "Probe.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Text.h"


namespace agiru::Fixture {

namespace {
namespace Probe_unit {
const RegisterCodeunit<Probe_Codeunit> kInCodeunitCatalogue;
} // namespace Probe_unit
} // namespace

::agiru::Boolean Probe_Codeunit::VerifyContract() {
  [[maybe_unused]] ::agiru::Text<10> Value{};

  Value = "before";
  ChooseValue(Value);
  return Value == "after";
}

void Probe_Codeunit::ChooseValue(::agiru::Integer &Value) {
  Value = 1;
}

void Probe_Codeunit::Choosevalue(::agiru::Text<0> &Value) {
  Value = "after";
}

void Probe_Codeunit::ClearAll() {
}

constexpr CodeunitDef kProbeCodeunit{
    .id = ::agiru::CodeunitTraits<Probe_Codeunit>::kId,
    .name = ::agiru::CodeunitTraits<Probe_Codeunit>::kName,
    .subtype = ::agiru::CodeunitTraits<Probe_Codeunit>::kSubtype,
};

} // namespace agiru::Fixture
