// Generated from NativeField.Codeunit.al. Do not edit.

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

class NativeField_Codeunit;

class NativeField_Codeunit : public Codeunit<NativeField_Codeunit> {
public:
  using Codeunit<NativeField_Codeunit>::operator=;

  ::agiru::Integer Exercise();

  void ClearAll();
};

extern const CodeunitDef kNativeFieldCodeunit;

} // namespace agiru::Fixture

template <> struct agiru::CodeunitTraits<agiru::Fixture::NativeField_Codeunit> {
  static constexpr CodeunitId kId{50181};
  static constexpr std::string_view kName{"Native Field"};
  static constexpr Subtype kSubtype{Subtype::Normal};
  static constexpr bool kSingleInstance = false;
  static constexpr const CodeunitDef &kCodeunit = agiru::Fixture::kNativeFieldCodeunit;
};
