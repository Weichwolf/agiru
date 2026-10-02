// Generated from OData.Codeunit.al. Do not edit.

#pragma once

#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "type/Integer.h"

#include "absent/Types.h"

#include <array>
#include <cstdint>
#include <string_view>

namespace agiru::Fixture {

class ODataRoundtrip_Codeunit;

class ODataRoundtrip_Codeunit : public Codeunit<ODataRoundtrip_Codeunit> {
public:
  using Codeunit<ODataRoundtrip_Codeunit>::operator=;

  ::agiru::Integer Exercise();

  void ClearAll();
};

extern const CodeunitDef kODataRoundtripCodeunit;

} // namespace agiru::Fixture

template <> struct agiru::CodeunitTraits<agiru::Fixture::ODataRoundtrip_Codeunit> {
  static constexpr CodeunitId kId{50251};
  static constexpr std::string_view kName{"OData Roundtrip"};
  static constexpr Subtype kSubtype{Subtype::Normal};
  static constexpr bool kSingleInstance = false;
  static constexpr const CodeunitDef &kCodeunit = agiru::Fixture::kODataRoundtripCodeunit;
};
