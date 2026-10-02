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

class MetadataRoundtrip_Codeunit;

class MetadataRoundtrip_Codeunit : public Codeunit<MetadataRoundtrip_Codeunit> {
public:
  using Codeunit<MetadataRoundtrip_Codeunit>::operator=;

  ::agiru::Integer Exercise();

  void ClearAll();
};

extern const CodeunitDef kMetadataRoundtripCodeunit;

} // namespace agiru::Fixture

template <> struct agiru::CodeunitTraits<agiru::Fixture::MetadataRoundtrip_Codeunit> {
  static constexpr CodeunitId kId{50241};
  static constexpr std::string_view kName{"Metadata Roundtrip"};
  static constexpr Subtype kSubtype{Subtype::Normal};
  static constexpr bool kSingleInstance = false;
  static constexpr const CodeunitDef &kCodeunit = agiru::Fixture::kMetadataRoundtripCodeunit;
};
