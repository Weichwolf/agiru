#pragma once

#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/DataScope.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/SecretText.h"
#include "type/StringValue.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `IsolatedStorage` -- the surface the platform documentation declares.
#include <concepts>
#include <type_traits>

namespace agiru {

/// \brief AL `IsolatedStorage`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/isolatedstorage/` states, so a call site compiles and is CHECKED; the
///          body refuses by name rather than returning a plausible wrong answer (board:0035).
class IsolatedStorage {
public:
  /// \brief AL `IsolatedStorage.Contains(Text, DataScope, Boolean)`. Determines whether the storage
  /// contains a value with the specified key.
  /// \param Key The AL `Text`.
  /// \param DataScope The AL `DataScope`.
  /// \param isSecret The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean
  Contains(std::string_view Key, const ::agiru::DataScope &DataScope, ::agiru::Boolean &isSecret);

  /// \brief AL `IsolatedStorage.Contains(Text, DataScope)`. Determines whether the storage contains
  /// a value with the specified key.
  /// \param Key The AL `Text`.
  /// \param DataScope The AL `DataScope`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Contains(std::string_view Key, const ::agiru::DataScope &DataScope);

  /// \brief AL `IsolatedStorage.Contains(Guid, DataScope)` and every other key AL converts to Text
  ///        on the way in -- `"OAuth 2.0 Setup"."Client ID"` is a Guid and the key is its text.
  /// \tparam K A type that renders itself as text.
  /// \param Key The key.
  /// \param DataScope The AL `DataScope`.
  /// \return True when the key is present.
  template <typename K>
    requires requires(const K &key) {
      { key.ToText() } -> std::convertible_to<std::string>;
    }
  static ::agiru::Boolean Contains(const K &Key, const ::agiru::DataScope &DataScope) {
    return Contains(std::string_view(Key.ToText()), DataScope);
  }

  /// \brief AL `IsolatedStorage.Delete(Text, DataScope)`. Deletes the value with the specified key
  /// from the isolated storage.
  /// \param Key The AL `Text`.
  /// \param DataScope The AL `DataScope`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Delete(std::string_view Key, const ::agiru::DataScope &DataScope = {});

  /// \brief AL `IsolatedStorage.Get(Text, DataScope, SecretText)`. Gets the value associated with
  /// the specified key.
  /// \param Key The AL `Text`.
  /// \param DataScope The AL `DataScope`.
  /// \param Value The AL `SecretText`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean
  Get(std::string_view Key, const ::agiru::DataScope &DataScope, ::agiru::SecretText &Value);

  /// \brief AL `IsolatedStorage.Get(Text, DataScope, Text)`. Gets the value associated with the
  /// specified key.
  /// \param Key The AL `Text`.
  /// \param DataScope The AL `DataScope`.
  /// \param Value The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean
  Get(std::string_view Key, const ::agiru::DataScope &DataScope, ::agiru::Text<0> &Value);

  /// \brief AL `IsolatedStorage.Get(Text, SecretText)`. Gets the value associated with the
  /// specified key.
  /// \param Key The AL `Text`.
  /// \param Value The AL `SecretText`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Get(std::string_view Key, ::agiru::SecretText &Value);

  /// \brief AL `IsolatedStorage.Get(Text, Text)`. Gets the value associated with the specified key.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Get(std::string_view Key, ::agiru::Text<0> &Value);

  /// \brief AL `IsolatedStorage.Set(Text, SecretText, DataScope)`. Sets the value associated with
  /// the specified key.
  /// \param Key The AL `Text`.
  /// \param Value The AL `SecretText`.
  /// \param DataScope The AL `DataScope`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \tparam S The secret's type -- `SecretText` itself, and nothing that merely
  ///         CONVERTS to one, so a plain text still picks the text overload.
  template <typename S>
    requires(std::is_same_v<std::remove_cvref_t<S>, ::agiru::SecretText>)
  static ::agiru::Boolean Set(std::string_view Key,
                              const S &Value,
                              const ::agiru::DataScope &DataScope = {}) {
    static_cast<void>(Key);
    static_cast<void>(Value);
    static_cast<void>(DataScope);
    throw Error("IsolatedStorage.Set(Text, SecretText, DataScope) is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `IsolatedStorage.Set(Text, Text, DataScope)`. Sets the value associated with the
  /// specified key.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Text`.
  /// \param DataScope The AL `DataScope`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean
  Set(std::string_view Key, std::string_view Value, const ::agiru::DataScope &DataScope = {});

  /// \brief AL `IsolatedStorage.SetEncrypted(Text, SecretText, DataScope)`. Encrypts and sets the
  /// value associated with the specified key. The input string cannot exceed a length of 215 plain
  /// characters; be aware that special characters take up more space.
  /// \param Key The AL `Text`.
  /// \param Value The AL `SecretText`.
  /// \param DataScope The AL `DataScope`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean SetEncrypted(std::string_view Key,
                                       const ::agiru::SecretText &Value,
                                       const ::agiru::DataScope &DataScope = {});

  /// \brief AL `IsolatedStorage.SetEncrypted(Text, Text, DataScope)`. Encrypts and sets the value
  /// associated with the specified key. The input string cannot exceed a length of 215 plain
  /// characters; be aware that special characters take up more space.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Text`.
  /// \param DataScope The AL `DataScope`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean SetEncrypted(std::string_view Key,
                                       std::string_view Value,
                                       const ::agiru::DataScope &DataScope = {});
};

}
