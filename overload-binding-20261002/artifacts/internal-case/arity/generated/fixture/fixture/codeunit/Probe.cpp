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


namespace agiru::Fixture {

namespace {
namespace Probe_unit {
const RegisterCodeunit<Probe_Codeunit> kInCodeunitCatalogue;
} // namespace Probe_unit
} // namespace

::agiru::Boolean Probe_Codeunit::VerifyContract() {
  return ChooseValue(7) == 7;
}

::agiru::Integer Probe_Codeunit::ChooseValue(::agiru::Integer Value, ::agiru::Integer Extra) {
  return Value + Extra;
}

::agiru::Integer Probe_Codeunit::Choosevalue(::agiru::Integer Value) {
  return Value;
}

void Probe_Codeunit::ClearAll() {
}

constexpr CodeunitDef kProbeCodeunit{
    .id = ::agiru::CodeunitTraits<Probe_Codeunit>::kId,
    .name = ::agiru::CodeunitTraits<Probe_Codeunit>::kName,
    .subtype = ::agiru::CodeunitTraits<Probe_Codeunit>::kSubtype,
};

} // namespace agiru::Fixture
