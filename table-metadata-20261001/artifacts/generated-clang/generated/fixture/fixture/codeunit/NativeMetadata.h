// Generated from Metadata.Codeunit.al. Do not edit.

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

class NativeMetadata_Codeunit;

class NativeMetadata_Codeunit : public Codeunit<NativeMetadata_Codeunit> {
public:
  using Codeunit<NativeMetadata_Codeunit>::operator=;

  ::agiru::Integer Exercise();

  void ClearAll();
};

extern const CodeunitDef kNativeMetadataCodeunit;

} // namespace agiru::Fixture

template <> struct agiru::CodeunitTraits<agiru::Fixture::NativeMetadata_Codeunit> {
  static constexpr CodeunitId kId{50198};
  static constexpr std::string_view kName{"Native Metadata"};
  static constexpr Subtype kSubtype{Subtype::Normal};
  static constexpr bool kSingleInstance = false;
  static constexpr const CodeunitDef &kCodeunit = agiru::Fixture::kNativeMetadataCodeunit;
};
