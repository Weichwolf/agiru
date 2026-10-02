// Generated from every AL body that names a type this run does not have.
// Do not edit.

#pragma once

#include "dotnet/Refused.h"

namespace agiru::dotnet {

} // namespace agiru::dotnet

namespace agiru::absent {

struct ODataEdmType : ::agiru::dotnet::AbsentObject {
  using ::agiru::dotnet::AbsentObject::AbsentObject;
  ODataEdmType() = default;
  ::agiru::dotnet::Refused Description{{.type = "ODataEdmType", .member = "Description"}};
  ::agiru::dotnet::Refused EdmXml{{.type = "ODataEdmType", .member = "EdmXml"}};
  ::agiru::dotnet::Refused FieldNo{{.type = "ODataEdmType", .member = "FieldNo"}};
  ::agiru::dotnet::Refused IsTemporary{{.type = "ODataEdmType", .member = "IsTemporary"}};
  ::agiru::dotnet::Refused Key{{.type = "ODataEdmType", .member = "Key"}};
  [[nodiscard]] const ::agiru::dotnet::RefusedResult *begin() const {
    return ::agiru::dotnet::Refused{{.type = "ODataEdmType", .member = "GetEnumerator"}}();
  }
  [[nodiscard]] const ::agiru::dotnet::RefusedResult *end() const { return begin(); }
  template <typename T> auto &operator=(const T &) {
    ::agiru::dotnet::Refused{{.type = "ODataEdmType", .member = "="}}();
    return *this;
  }
};

} // namespace agiru::absent
