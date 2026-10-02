// Generated from SavedOptions.Codeunit.al. Do not edit.

#pragma once

#include "meta/CodeunitDef.h"
#include "meta/Ids.h"
#include "runtime/Codeunit.h"
#include "runtime/Error.h"
#include "type/Integer.h"


namespace agiru::Fixture {
class StoredFlag_Table;
} // namespace agiru::Fixture

#include <array>
#include <cstdint>
#include <string_view>

namespace agiru::Fixture {

class SavedOptions_Codeunit;

class SavedOptions_Codeunit : public Codeunit<SavedOptions_Codeunit> {
public:
  using Codeunit<SavedOptions_Codeunit>::operator=;

  ::agiru::Integer Exercise();

  void ClearAll();
};

extern const CodeunitDef kSavedOptionsCodeunit;

} // namespace agiru::Fixture

template <> struct agiru::CodeunitTraits<agiru::Fixture::SavedOptions_Codeunit> {
  static constexpr CodeunitId kId{50232};
  static constexpr std::string_view kName{"Saved Options"};
  static constexpr Subtype kSubtype{Subtype::Normal};
  static constexpr bool kSingleInstance = false;
  static constexpr const CodeunitDef &kCodeunit = agiru::Fixture::kSavedOptionsCodeunit;
};
