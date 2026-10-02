// Generated from Caller.Codeunit.al. Do not edit.

#pragma once

#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "type/Boolean.h"

#include "options/Types.h"

namespace agiru::Fixture {
class SourceRow_Table;
} // namespace agiru::Fixture

#include <array>
#include <cstdint>
#include <string_view>

namespace agiru::Fixture {

class Caller_Codeunit;

class Caller_Codeunit : public Codeunit<Caller_Codeunit> {
public:
  using Codeunit<Caller_Codeunit>::operator=;
  using Codeunit<Caller_Codeunit>::Run;

  ::agiru::Boolean Run();

  void ClearAll();
};

extern const CodeunitDef kCallerCodeunit;

} // namespace agiru::Fixture

template <> struct agiru::CodeunitTraits<agiru::Fixture::Caller_Codeunit> {
  static constexpr CodeunitId kId{50172};
  static constexpr std::string_view kName{"Caller"};
  static constexpr Subtype kSubtype{Subtype::Normal};
  static constexpr bool kSingleInstance = false;
  static constexpr const CodeunitDef &kCodeunit = agiru::Fixture::kCallerCodeunit;
};
