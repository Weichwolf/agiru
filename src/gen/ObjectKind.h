#pragma once

#include <cstdint>

namespace agiru::gen {

enum class ObjectKind : std::uint8_t {
  Table,
  Codeunit,
  Page,
  Report,
  Query,
  XmlPort,
  Enum,
  Interface,
  PermissionSet,
  Profile,
};

}
