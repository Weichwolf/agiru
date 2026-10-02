// Generated from every AL body that names a type this run does not have.
// Do not edit.

#pragma once

#include "dotnet/Refused.h"

namespace agiru::dotnet {

} // namespace agiru::dotnet

namespace agiru::absent {

struct ObjectOptions : ::agiru::dotnet::AbsentObject {
  using ::agiru::dotnet::AbsentObject::AbsentObject;
  ObjectOptions() = default;
  ::agiru::dotnet::Refused GetRangeMin{{.type = "ObjectOptions", .member = "GetRangeMin"}};
  ::agiru::dotnet::Refused IsTemporary{{.type = "ObjectOptions", .member = "IsTemporary"}};
  ::agiru::dotnet::Refused ObjectType{{.type = "ObjectOptions", .member = "ObjectType"}};
  ::agiru::dotnet::Refused OptionData{{.type = "ObjectOptions", .member = "OptionData"}};
  ::agiru::dotnet::Refused SetRange{{.type = "ObjectOptions", .member = "SetRange"}};
  ::agiru::dotnet::Refused Temporary{{.type = "ObjectOptions", .member = "Temporary"}};
  [[nodiscard]] const ::agiru::dotnet::RefusedResult *begin() const {
    return ::agiru::dotnet::Refused{{.type = "ObjectOptions", .member = "GetEnumerator"}}();
  }
  [[nodiscard]] const ::agiru::dotnet::RefusedResult *end() const { return begin(); }
  template <typename T> auto &operator=(const T &) {
    ::agiru::dotnet::Refused{{.type = "ObjectOptions", .member = "="}}();
    return *this;
  }
};

} // namespace agiru::absent
