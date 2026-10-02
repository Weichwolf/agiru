// Generated from NativeGlobal.Codeunit.al. Do not edit.

#pragma once

#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "platform/ODataEdmType.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"


#include <array>
#include <cstdint>
#include <string_view>

namespace agiru {

class NativeGlobal_Codeunit;

class NativeGlobal_Codeunit : public Codeunit<NativeGlobal_Codeunit> {
public:
  using Codeunit<NativeGlobal_Codeunit>::operator=;


  void ClearAll();

private:
  Instance<::agiru::platform::ODataEdmType> Row;
};

extern const CodeunitDef kNativeGlobalCodeunit;

} // namespace agiru

template <> struct agiru::CodeunitTraits<agiru::NativeGlobal_Codeunit> {
  static constexpr CodeunitId kId{50270};
  static constexpr std::string_view kName{"NativeGlobal"};
  static constexpr Subtype kSubtype{Subtype::Normal};
  static constexpr bool kSingleInstance = false;
  static constexpr const CodeunitDef &kCodeunit = agiru::kNativeGlobalCodeunit;
};
