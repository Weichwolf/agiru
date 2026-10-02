// Generated from Profile.Codeunit.al. Do not edit.

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

class NativeProfile_Codeunit;

class NativeProfile_Codeunit : public Codeunit<NativeProfile_Codeunit> {
public:
  using Codeunit<NativeProfile_Codeunit>::operator=;

  ::agiru::Integer Exercise();

  void ClearAll();
};

extern const CodeunitDef kNativeProfileCodeunit;

} // namespace agiru::Fixture

template <> struct agiru::CodeunitTraits<agiru::Fixture::NativeProfile_Codeunit> {
  static constexpr CodeunitId kId{50195};
  static constexpr std::string_view kName{"Native Profile"};
  static constexpr Subtype kSubtype{Subtype::Normal};
  static constexpr bool kSingleInstance = false;
  static constexpr const CodeunitDef &kCodeunit = agiru::Fixture::kNativeProfileCodeunit;
};
