#pragma once

#include "dotnet/Refused.h"
#include "type/Boolean.h"
#include "type/Text.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

namespace agiru::dotnet {

/// \brief .NET `System.Type`, as much of it as AL asks for: a name. `Type.GetType("System.Int32")`
///        is what a `DataColumn.DataType` is set from and `Format(...)` reads back as the full
///        name (`Business Chart Impl.`), and a caught exception's type is compared with
///        `Equals` (`SFTP Client`).
class Type {
public:
  /// \brief The binder AL never calls -- no AL writes `Type.Type()` -- kept as the mark of a
  ///        rebuilt class; there is no member of that name, which is what lets the class carry
  ///        the constructor below.
  struct Binder {};

  /// \brief A type of no name, which is what `var T: DotNet Type` declares.
  Type() = default;

  /// \brief `T := GetDotNetType(X)`, whose answer AL hands over as an `Any`: the type of the
  ///        full name the Variant renders. \param variant The Variant.
  explicit(false) Type(const Variant &variant)
      : fullName_(variant.Is<std::string>() ? variant.Get<std::string>() : std::string{}) {}

  /// \brief `Type.GetType(name)`: the type of that full name. \param name `System.Int32` and the
  ///        like. \return A type carrying the name; nothing is loaded.
  [[nodiscard]] static class Type GetType(std::string_view name) {
    class Type made;
    made.fullName_ = std::string(name);
    return made;
  }

  /// \brief `Type.FullName`: the namespace-qualified name. \return It.
  [[nodiscard]] ::agiru::Text<0> FullName() const { return fullName_; }

  /// \brief `Type.Name`: the name after the last dot. \return It.
  [[nodiscard]] ::agiru::Text<0> Name() const {
    const std::size_t dot = fullName_.rfind('.');
    return dot == std::string::npos ? fullName_ : fullName_.substr(dot + 1);
  }

  /// \brief `Type.ToString()`: the full name, which is what .NET renders. \return It.
  [[nodiscard]] ::agiru::Text<0> ToString() const { return fullName_; }

  /// \brief What `Format(Type)` renders: the full name. \return It.
  [[nodiscard]] std::string ToText() const { return fullName_; }

  /// \brief `Type.Equals(other)`: two types are one when they carry the same full name.
  /// \param other The other type. \return Whether the names agree.
  [[nodiscard]] Boolean Equals(const class Type &other) const {
    return fullName_ == other.fullName_;
  }

  /// \brief `Type.Equals(Any)` over what `GetDotNetType` hands back. \param other The Variant.
  /// \return Whether it renders as this type's full name.
  [[nodiscard]] Boolean Equals(const Variant &other) const {
    return other.Is<std::string>() && other.Get<std::string>() == fullName_;
  }

  /// \brief `Type := AbsentObject.GetType()`: the call refused before the assignment.
  /// \tparam R The refusal. \param refused It. \return This.
  template <typename R>
    requires requires { typename R::IsAlRefusal; }
  class Type &operator=(const R &refused) {
    static_cast<void>(refused);
    return *this;
  }

  /// \brief `Type.GetField(name)`: reflection this runtime does not carry.
  Refused GetField{{.type = "Type", .member = "GetField"}};
  /// \brief `Type.MakeGenericType(...)`: reflection this runtime does not carry.
  Refused MakeGenericType{{.type = "Type", .member = "MakeGenericType"}};

private:
  std::string fullName_;
};

}
