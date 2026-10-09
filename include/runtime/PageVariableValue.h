#pragma once

#include "meta/Declare.h"
#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/PageValue.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Integer.h"

#include <memory>
#include <string_view>
#include <type_traits>

/// \file
/// \brief Declared scalar bindings for generated page variables and array elements.

namespace agiru {

/// \brief Reads an AL variable's actual scalar type, never its localized display text.
/// \tparam T The declared AL storage type; unsupported types refuse explicitly.
/// \param value The original page variable or indexed element.
/// \param domain Stable page/control identity for Option and Enum values.
/// \return Owned lossless scalar text and borrowed immutable type-choice metadata.
/// \note Choice spans reference static type declarations, never the local FieldDef.
/// \throws Error for unsupported storage or an invalid declaration identity.
template <typename T>
[[nodiscard]] PageValue ReadPageVariable(const T &value, std::string_view domain) {
  if constexpr (requires { FieldTypeOf<T>::kType; } &&
                (!std::is_arithmetic_v<T> || std::is_same_v<T, Boolean> ||
                 std::is_same_v<T, Integer> || std::is_same_v<T, BigInteger>)) {
    constexpr FieldDef declaration{.values = FieldTypeOf<T>::kValues,
                                   .displayOrdinals = DisplayOrdinalsOf<T>(),
                                   .length = FieldTypeOf<T>::kLength,
                                   .type = FieldTypeOf<T>::kType};
    return ReadPageScalar(std::addressof(value), declaration, domain);
  } else {
    throw Error("Page variable type has no scalar transport", "PageValueUnsupported");
  }
}

}
