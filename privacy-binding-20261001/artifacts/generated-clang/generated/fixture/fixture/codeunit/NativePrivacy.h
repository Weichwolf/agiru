// Generated from Privacy.Codeunit.al. Do not edit.

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

class NativePrivacy_Codeunit;

class NativePrivacy_Codeunit : public Codeunit<NativePrivacy_Codeunit> {
public:
  using Codeunit<NativePrivacy_Codeunit>::operator=;

  ::agiru::Integer Exercise();

  void ClearAll();
};

extern const CodeunitDef kNativePrivacyCodeunit;

} // namespace agiru::Fixture

template <> struct agiru::CodeunitTraits<agiru::Fixture::NativePrivacy_Codeunit> {
  static constexpr CodeunitId kId{50194};
  static constexpr std::string_view kName{"Native Privacy"};
  static constexpr Subtype kSubtype{Subtype::Normal};
  static constexpr bool kSingleInstance = false;
  static constexpr const CodeunitDef &kCodeunit = agiru::Fixture::kNativePrivacyCodeunit;
};
