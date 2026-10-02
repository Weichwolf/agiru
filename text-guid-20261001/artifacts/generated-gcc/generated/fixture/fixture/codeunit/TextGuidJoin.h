// Generated from Join.Codeunit.al. Do not edit.

#pragma once

#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/SecretText.h"
#include "type/Text.h"


#include <array>
#include <cstdint>
#include <string_view>

namespace agiru::Fixture {

class TextGuidJoin_Codeunit;

class TextGuidJoin_Codeunit : public Codeunit<TextGuidJoin_Codeunit> {
public:
  using Codeunit<TextGuidJoin_Codeunit>::operator=;

  ::agiru::Integer Exercise();
  ::agiru::Text<3> Limited();

  void ClearAll();

private:
  ::agiru::Integer Which(::agiru::Text<0> Value);
  ::agiru::Integer Which(Guid Value);
  ::agiru::Integer Which(SecretText Value);
};

extern const CodeunitDef kTextGuidJoinCodeunit;

} // namespace agiru::Fixture

template <> struct agiru::CodeunitTraits<agiru::Fixture::TextGuidJoin_Codeunit> {
  static constexpr CodeunitId kId{50199};
  static constexpr std::string_view kName{"Text Guid Join"};
  static constexpr Subtype kSubtype{Subtype::Normal};
  static constexpr bool kSingleInstance = false;
  static constexpr const CodeunitDef &kCodeunit = agiru::Fixture::kTextGuidJoinCodeunit;
};
