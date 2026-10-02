// Generated from Link.Codeunit.al. Do not edit.

#pragma once

#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "type/Integer.h"


namespace agiru::Fixture {
class CollisionSource_Table;
} // namespace agiru::Fixture

#include <array>
#include <cstdint>
#include <string_view>

namespace agiru::Fixture {

class NativeLink_Codeunit;

class NativeLink_Codeunit : public Codeunit<NativeLink_Codeunit> {
public:
  using Codeunit<NativeLink_Codeunit>::operator=;

  ::agiru::Integer Exercise();

  void ClearAll();
};

extern const CodeunitDef kNativeLinkCodeunit;

} // namespace agiru::Fixture

template <> struct agiru::CodeunitTraits<agiru::Fixture::NativeLink_Codeunit> {
  static constexpr CodeunitId kId{50196};
  static constexpr std::string_view kName{"Native Link"};
  static constexpr Subtype kSubtype{Subtype::Normal};
  static constexpr bool kSingleInstance = false;
  static constexpr const CodeunitDef &kCodeunit = agiru::Fixture::kNativeLinkCodeunit;
};
