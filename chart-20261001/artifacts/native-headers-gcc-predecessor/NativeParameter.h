// Generated from NativeParameter.Codeunit.al. Do not edit.

#pragma once

#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "platform/ODataEdmType.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "type/Code.h"


#include <array>
#include <cstdint>
#include <string_view>

namespace agiru {

class NativeParameter_Codeunit;

class NativeParameter_Codeunit : public Codeunit<NativeParameter_Codeunit> {
public:
  using Codeunit<NativeParameter_Codeunit>::operator=;

  ::agiru::Code<50> Read(::agiru::platform::ODataEdmType &Row);

  void ClearAll();
};

extern const CodeunitDef kNativeParameterCodeunit;

} // namespace agiru

template <> struct agiru::CodeunitTraits<agiru::NativeParameter_Codeunit> {
  static constexpr CodeunitId kId{50271};
  static constexpr std::string_view kName{"NativeParameter"};
  static constexpr Subtype kSubtype{Subtype::Normal};
  static constexpr bool kSingleInstance = false;
  static constexpr const CodeunitDef &kCodeunit = agiru::kNativeParameterCodeunit;
};
