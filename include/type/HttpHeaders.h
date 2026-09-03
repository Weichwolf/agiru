#pragma once

#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/List.h"
#include "type/RecordId.h"
#include "type/SecretText.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `HttpHeaders` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `HttpHeaders`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/httpheaders/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class HttpHeaders {
public:
  /// \brief AL `HttpHeaders.Add(Text, SecretText)`. Adds the specified secret header and its value
  /// into the HttpHeaders collection. Validates the provided value.
  /// \param Name The AL `Text`.
  /// \param Value The AL `SecretText`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Name, const ::agiru::SecretText &Value);

  /// \brief AL `HttpHeaders.Add(Text, Text)`. Adds the specified header and its value into the
  /// HttpHeaders collection. Validates the provided value.
  /// \param Name The AL `Text`.
  /// \param Value The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Name, std::string_view Value);

  /// \brief AL `HttpHeaders.Clear()`. Sets the HttpHeaders variable to the default value.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Clear();

  /// \brief AL `HttpHeaders.Contains(Text)`. Checks if the specified header exists in the
  /// HttpHeaders collection.
  /// \param Name The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Contains(std::string_view Name);

  /// \brief AL `HttpHeaders.ContainsSecret(Text)`. Returns if the header for the given key has a
  /// secret value.
  /// \param Key The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ContainsSecret(std::string_view Key);

  /// \brief AL `HttpHeaders.GetSecretValues(Text, List of [SecretText])`. Gets the secret values
  /// for the specified key.
  /// \param Key The AL `Text`.
  /// \param Values The AL `List of [SecretText]`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetSecretValues(std::string_view Key,
                                   const ::agiru::List<::agiru::SecretText> &Values);

  /// \brief AL `HttpHeaders.GetSecretValues(Text, Array of [SecretText])`. Gets the secret values
  /// for the specified key.
  /// \param Key The AL `Text`.
  /// \param Values The AL `Array of [SecretText]`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetSecretValues(std::string_view Key, const ::agiru::Variant &Values);

  /// \brief AL `HttpHeaders.GetValues(String, Array of [Text])`. Gets the values for the specified
  /// key.
  /// \param Key The AL `String`.
  /// \param Values The AL `Array of [Text]`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetValues(std::string_view Key, const ::agiru::Variant &Values);

  /// \brief AL `HttpHeaders.GetValues(Text, List of [Text])`. Gets the values for the specified
  /// key.
  /// \param Key The AL `Text`.
  /// \param Values The AL `List of [Text]`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetValues(std::string_view Key, const ::agiru::List<std::string> &Values);

  /// \brief AL `HttpHeaders.Keys()`. Gets the key name of all the headers
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Keys();

  /// \brief AL `HttpHeaders.Remove(Text)`. Removes the specified header from the HttpHeaders
  /// collection.
  /// \param Name The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Remove(std::string_view Name);

  /// \brief AL `HttpHeaders.TryAddWithoutValidation(Text, SecretText)`. Adds the specified secret
  /// header and its value into the HttpHeaders collection. Doesn't validate the provided value.
  /// \param Name The AL `Text`.
  /// \param Value The AL `SecretText`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean TryAddWithoutValidation(std::string_view Name, const ::agiru::SecretText &Value);

  /// \brief AL `HttpHeaders.TryAddWithoutValidation(Text, Text)`. Adds the specified header and its
  /// value into the HttpHeaders collection. Doesn't validate the provided value.
  /// \param Name The AL `Text`.
  /// \param Value The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean TryAddWithoutValidation(std::string_view Name, std::string_view Value);
};

}
