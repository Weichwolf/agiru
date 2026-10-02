// Generated from Probe.Codeunit.al. Do not edit.

#pragma once

#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Text.h"


#include <array>
#include <cstdint>
#include <string_view>

namespace agiru::Fixture {

class Probe_Codeunit;

class Probe_Codeunit : public Codeunit<Probe_Codeunit> {
public:
  using Codeunit<Probe_Codeunit>::operator=;

  ::agiru::Boolean VerifyContract();

  void ClearAll();

private:
  ::agiru::Integer Choose(::agiru::Integer Value);
  ::agiru::Integer Choose(::agiru::Text<0> Value);
};

extern const CodeunitDef kProbeCodeunit;

} // namespace agiru::Fixture

template <> struct agiru::CodeunitTraits<agiru::Fixture::Probe_Codeunit> {
  static constexpr CodeunitId kId{50196};
  static constexpr std::string_view kName{"Probe"};
  static constexpr Subtype kSubtype{Subtype::Normal};
  static constexpr bool kSingleInstance = false;
  static constexpr const CodeunitDef &kCodeunit = agiru::Fixture::kProbeCodeunit;
};
