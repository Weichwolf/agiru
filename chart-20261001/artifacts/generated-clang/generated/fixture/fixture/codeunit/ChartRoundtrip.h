// Generated from Chart.Codeunit.al. Do not edit.

#pragma once

#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "type/Integer.h"


#include <array>
#include <cstdint>
#include <string_view>

namespace agiru::Fixture {

class ChartRoundtrip_Codeunit;

class ChartRoundtrip_Codeunit : public Codeunit<ChartRoundtrip_Codeunit> {
public:
  using Codeunit<ChartRoundtrip_Codeunit>::operator=;

  ::agiru::Integer Exercise();

  void ClearAll();
};

extern const CodeunitDef kChartRoundtripCodeunit;

} // namespace agiru::Fixture

template <> struct agiru::CodeunitTraits<agiru::Fixture::ChartRoundtrip_Codeunit> {
  static constexpr CodeunitId kId{50261};
  static constexpr std::string_view kName{"ChartRoundtrip"};
  static constexpr Subtype kSubtype{Subtype::Normal};
  static constexpr bool kSingleInstance = false;
  static constexpr const CodeunitDef &kCodeunit = agiru::Fixture::kChartRoundtripCodeunit;
};
