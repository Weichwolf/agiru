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
#include "type/Stream.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `JsonToken` -- the surface the platform documentation declares.

namespace agiru {

class JsonArray;
class JsonObject;
class JsonValue;

/// \brief AL `JsonToken`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/jsontoken/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class JsonToken {
public:
  /// \brief AL `JsonToken.AsArray()`. Converts the value in a JsonToken to a JsonArray data type.
  /// \return The AL `JsonArray`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::JsonArray AsArray();

  /// \brief AL `JsonToken.AsObject()`. Converts the value in a JsonToken to a JsonObject data type.
  /// \return The AL `JsonObject`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::JsonObject AsObject();

  /// \brief AL `JsonToken.AsValue()`. Converts the value in a JsonToken to a JsonValue data type.
  /// \return The AL `JsonValue`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::JsonValue AsValue();

  /// \brief AL `JsonToken.Clone()`. Creates a deep-copy of the JsonToken value.
  /// \return The AL `JsonToken`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::JsonToken Clone();

  /// \brief AL `JsonToken.IsArray()`. Indicates whether a JsonToken represents a JSON array.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsArray();

  /// \brief AL `JsonToken.IsObject()`. Indicates whether a JsonToken contains a JSON object.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsObject();

  /// \brief AL `JsonToken.IsValue()`. Indicates whether a JsonToken contains a JSON value.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsValue();

  /// \brief AL `JsonToken.Path()`. Retrieves the JSON path of the token relative to the root of its
  /// containing tree.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Path();

  /// \brief AL `JsonToken.ReadFrom(InStream)`. Reads the JSON data from the stream into a JsonToken
  /// variable.
  /// \param InStream The AL `InStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReadFrom(const ::agiru::InStream &InStream);

  /// \brief AL `JsonToken.ReadFrom(Text)`. Reads the JSON data from the string into a JsonToken
  /// variable.
  /// \param String The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReadFrom(std::string_view String);

  /// \brief AL `JsonToken.SelectToken(Text, JsonToken)`. Selects a JsonToken using a JPath
  /// expression.
  /// \param Path The AL `Text`.
  /// \param Result The AL `JsonToken`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectToken(std::string_view Path, ::agiru::JsonToken &Result);

  /// \brief AL `JsonToken.SelectTokens(Text, List of [JsonToken])`. Selects tokens based on a JPath
  /// expression and returns them in a new list.
  /// \param Path The AL `Text`.
  /// \param Result The AL `List of [JsonToken]`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectTokens(std::string_view Path, ::agiru::List<::agiru::JsonToken> &Result);

  /// \brief AL `JsonToken.WriteTo(OutStream)`. Serializes and writes the JSON data of the JsonToken
  /// to a given OutStream object.
  /// \param Data The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::OutStream &Data);

  /// \brief AL `JsonToken.WriteTo(Text)`. Serializes and writes the JSON data of the JsonToken to a
  /// given Text object.
  /// \param String The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(std::string &String);
};

} // namespace agiru
