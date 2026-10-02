// Generated from Probe.Codeunit.al. Do not edit.

#include "Probe.h"

#include "Builtins.h"
#include "BuiltinsWritten.h"
#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Guid.h"
#include "type/Text.h"


namespace agiru::Fixture {

namespace {
namespace Probe_unit {
const RegisterCodeunit<Probe_Codeunit> kInCodeunitCatalogue;
} // namespace Probe_unit
} // namespace

::agiru::Boolean Probe_Codeunit::VerifyContract() {
  return Choose(::agiru::Guid("typed")) == "text";
}

::agiru::Boolean Probe_Codeunit::Choose([[maybe_unused]] Guid Value) {
  return false;
}

::agiru::Text<0> Probe_Codeunit::Choose([[maybe_unused]] ::agiru::Text<0> Value) {
  return "text";
}

void Probe_Codeunit::ClearAll() {
}

constexpr CodeunitDef kProbeCodeunit{
    .id = ::agiru::CodeunitTraits<Probe_Codeunit>::kId,
    .name = ::agiru::CodeunitTraits<Probe_Codeunit>::kName,
    .subtype = ::agiru::CodeunitTraits<Probe_Codeunit>::kSubtype,
};

} // namespace agiru::Fixture
