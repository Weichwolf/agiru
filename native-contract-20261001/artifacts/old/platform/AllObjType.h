#pragma once

#include "meta/EnumDef.h"

#include <array>
#include <cstdint>

/// \file
/// \brief The `Object Type` option of the platform's object tables, with BC's own ordinals.
///
/// \note THE FIRST ELEVEN ARE THE BASEAPP'S OWN WORDS: `Permission."Object Type"` declares
///       `"Table Data","Table",,"Report",,"Codeunit","XMLport",MenuSuite,"Page","Query",System`,
///       and the gaps at 2 and 4 are `Form` and `Dataport`, the C/SIDE kinds. The twelve that
///       follow are the extension kinds in the order the platform numbers them [SET]; no AL in the
///       read roots filters on one of them by ordinal.

namespace agiru::platform {

enum class AllObjType : std::int32_t {
  TableData = 0,
  Table = 1,
  Form = 2,
  Report = 3,
  Dataport = 4,
  Codeunit = 5,
  XMLport = 6,
  MenuSuite = 7,
  Page = 8,
  Query = 9,
  System = 10,
  FieldNumber = 11,
  Blank12 = 12,
  Blank13 = 13,
  PageExtension = 14,
  TableExtension = 15,
  Enum = 16,
  EnumExtension = 17,
  Profile = 18,
  ProfileExtension = 19,
  PermissionSet = 20,
  PermissionSetExtension = 21,
  ReportExtension = 22,
};

}

template <> struct agiru::OptionTraits<agiru::platform::AllObjType> {
  static constexpr std::array<agiru::EnumValueDef, 23> kValues{{
      {.ordinal = 0, .name = "TableData", .caption = "TableData"},
      {.ordinal = 1, .name = "Table", .caption = "Table"},
      {.ordinal = 2, .name = "Form", .caption = "Form"},
      {.ordinal = 3, .name = "Report", .caption = "Report"},
      {.ordinal = 4, .name = "Dataport", .caption = "Dataport"},
      {.ordinal = 5, .name = "Codeunit", .caption = "Codeunit"},
      {.ordinal = 6, .name = "XMLport", .caption = "XMLport"},
      {.ordinal = 7, .name = "MenuSuite", .caption = "MenuSuite"},
      {.ordinal = 8, .name = "Page", .caption = "Page"},
      {.ordinal = 9, .name = "Query", .caption = "Query"},
      {.ordinal = 10, .name = "System", .caption = "System"},
      {.ordinal = 11, .name = "FieldNumber", .caption = "FieldNumber"},
      {.ordinal = 12, .name = "", .caption = ""},
      {.ordinal = 13, .name = "", .caption = ""},
      {.ordinal = 14, .name = "PageExtension", .caption = "PageExtension"},
      {.ordinal = 15, .name = "TableExtension", .caption = "TableExtension"},
      {.ordinal = 16, .name = "Enum", .caption = "Enum"},
      {.ordinal = 17, .name = "EnumExtension", .caption = "EnumExtension"},
      {.ordinal = 18, .name = "Profile", .caption = "Profile"},
      {.ordinal = 19, .name = "ProfileExtension", .caption = "ProfileExtension"},
      {.ordinal = 20, .name = "PermissionSet", .caption = "PermissionSet"},
      {.ordinal = 21, .name = "PermissionSetExtension", .caption = "PermissionSetExtension"},
      {.ordinal = 22, .name = "ReportExtension", .caption = "ReportExtension"},
  }};
};
