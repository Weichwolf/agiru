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
#include "type/Dictionary.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/List.h"
#include "type/RecordId.h"
#include "type/SecretText.h"
#include "type/Stream.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `JsonObject` -- the surface the platform documentation declares.

namespace agiru {

class JsonArray;
class JsonToken;
class JsonValue;

/// \brief AL `JsonObject`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/jsonobject/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class JsonObject {
public:
  /// \brief AL `JsonObject.Add(Text, BigInteger)`. Adds a new property to a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Value The AL `BigInteger`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Key, ::agiru::BigInteger Value);

  /// \brief AL `JsonObject.Add(Text, Boolean)`. Adds a new property to a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Key, ::agiru::Boolean Value);

  /// \brief AL `JsonObject.Add(Text, Byte)`. Adds a new property to a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Byte`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Key, ::agiru::Byte Value);

  /// \brief AL `JsonObject.Add(Text, Char)`. Adds a new property to a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Char`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Key, ::agiru::Char Value);

  /// \brief AL `JsonObject.Add(Text, Date)`. Adds a new property to a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Date`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Key, ::agiru::Date Value);

  /// \brief AL `JsonObject.Add(Text, DateTime)`. Adds a new property to a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Value The AL `DateTime`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Key, ::agiru::DateTime Value);

  /// \brief AL `JsonObject.Add(Text, Decimal)`. Adds a new property to a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Decimal`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Key, ::agiru::Decimal Value);

  /// \brief AL `JsonObject.Add(Text, Duration)`. Adds a new property to a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Duration`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Key, ::agiru::Duration Value);

  /// \brief AL `JsonObject.Add(Text, Integer)`. Adds a new property to a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Key, ::agiru::Integer Value);

  /// \brief AL `JsonObject.Add(Text, JsonArray)`. Adds a new property to a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Value The AL `JsonArray`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Key, const ::agiru::JsonArray &Value);

  /// \brief AL `JsonObject.Add(Text, JsonObject)`. Adds a new property to a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Value The AL `JsonObject`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Key, const ::agiru::JsonObject &Value);

  /// \brief AL `JsonObject.Add(Text, JsonToken)`. Adds a new property to a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Value The AL `JsonToken`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Key, const ::agiru::JsonToken &Value);

  /// \brief AL `JsonObject.Add(Text, JsonValue)`. Adds a new property to a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Value The AL `JsonValue`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Key, const ::agiru::JsonValue &Value);

  /// \brief AL `JsonObject.Add(Text, Text)`. Adds a new property to a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Key, std::string_view Value);

  /// \brief AL `JsonObject.Add(Text, Time)`. Adds a new property to a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Time`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Add(std::string_view Key, ::agiru::Time Value);

  /// \brief AL `JsonObject.AsToken()`. Converts the value in a JsonObject to a JsonToken data type.
  /// \return The AL `JsonToken`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::JsonToken AsToken();

  /// \brief AL `JsonObject.Clone()`. Creates a deep-copy of the JsonToken value.
  /// \return The AL `JsonToken`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::JsonToken Clone();

  /// \brief AL `JsonObject.Contains(Text)`. Verifies if a JsonObject contains a property with a
  /// given key.
  /// \param Key The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Contains(std::string_view Key);

  /// \brief AL `JsonObject.Get(Text, JsonToken)`. Retrieves the value of a property with a given
  /// key from a JsonObject.
  /// \param Key The AL `Text`.
  /// \param Result The AL `JsonToken`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Get(std::string_view Key, ::agiru::JsonToken &Result);

  /// \brief AL `JsonObject.GetArray(Text, Boolean)`. Retrieves the value of a property with a given
  /// key from a JsonObject as a JsonArray.
  /// \param Key The AL `Text`.
  /// \param DefaultIfNotFound The AL `Boolean`.
  /// \return The AL `JsonArray`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::JsonArray GetArray(std::string_view Key, ::agiru::Boolean DefaultIfNotFound);

  /// \brief AL `JsonObject.GetBigInteger(Text, Boolean)`. Retrieves the value of a property with a
  /// given key from a JsonObject as a BigInteger.
  /// \param Key The AL `Text`.
  /// \param DefaultIfNotFound The AL `Boolean`.
  /// \return The AL `BigInteger`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::BigInteger GetBigInteger(std::string_view Key, ::agiru::Boolean DefaultIfNotFound);

  /// \brief AL `JsonObject.GetBoolean(Text, Boolean)`. Retrieves the value of a property with a
  /// given key from a JsonObject as a Boolean.
  /// \param Key The AL `Text`.
  /// \param DefaultIfNotFound The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetBoolean(std::string_view Key, ::agiru::Boolean DefaultIfNotFound);

  /// \brief AL `JsonObject.GetByte(Text, Boolean)`. Retrieves the value of a property with a given
  /// key from a JsonObject as a Byte.
  /// \param Key The AL `Text`.
  /// \param DefaultIfNotFound The AL `Boolean`.
  /// \return The AL `Byte`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Byte GetByte(std::string_view Key, ::agiru::Boolean DefaultIfNotFound);

  /// \brief AL `JsonObject.GetChar(Text, Boolean)`. Retrieves the value of a property with a given
  /// key from a JsonObject as a Char.
  /// \param Key The AL `Text`.
  /// \param DefaultIfNotFound The AL `Boolean`.
  /// \return The AL `Char`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Char GetChar(std::string_view Key, ::agiru::Boolean DefaultIfNotFound);

  /// \brief AL `JsonObject.GetDate(Text, Boolean)`. Retrieves the value of a property with a given
  /// key from a JsonObject as a Date.
  /// \param Key The AL `Text`.
  /// \param DefaultIfNotFound The AL `Boolean`.
  /// \return The AL `Date`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Date GetDate(std::string_view Key, ::agiru::Boolean DefaultIfNotFound);

  /// \brief AL `JsonObject.GetDateTime(Text, Boolean)`. Retrieves the value of a property with a
  /// given key from a JsonObject as a DateTime.
  /// \param Key The AL `Text`.
  /// \param DefaultIfNotFound The AL `Boolean`.
  /// \return The AL `DateTime`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::DateTime GetDateTime(std::string_view Key, ::agiru::Boolean DefaultIfNotFound);

  /// \brief AL `JsonObject.GetDecimal(Text, Boolean)`. Retrieves the value of a property with a
  /// given key from a JsonObject as an Decimal.
  /// \param Key The AL `Text`.
  /// \param DefaultIfNotFound The AL `Boolean`.
  /// \return The AL `Decimal`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Decimal GetDecimal(std::string_view Key, ::agiru::Boolean DefaultIfNotFound);

  /// \brief AL `JsonObject.GetDuration(Text, Boolean)`. Retrieves the value of a property with a
  /// given key from a JsonObject as a Duration.
  /// \param Key The AL `Text`.
  /// \param DefaultIfNotFound The AL `Boolean`.
  /// \return The AL `Duration`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Duration GetDuration(std::string_view Key, ::agiru::Boolean DefaultIfNotFound);

  /// \brief AL `JsonObject.GetInteger(Text, Boolean)`. Retrieves the value of a property with a
  /// given key from a JsonObject as an Integer.
  /// \param Key The AL `Text`.
  /// \param DefaultIfNotFound The AL `Boolean`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer GetInteger(std::string_view Key, ::agiru::Boolean DefaultIfNotFound);

  /// \brief AL `JsonObject.GetObject(Text, Boolean)`. Retrieves the value of a property with a
  /// given key from a JsonObject as a JsonObject.
  /// \param Key The AL `Text`.
  /// \param DefaultIfNotFound The AL `Boolean`.
  /// \return The AL `JsonObject`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::JsonObject GetObject(std::string_view Key, ::agiru::Boolean DefaultIfNotFound);

  /// \brief AL `JsonObject.GetOption(Text, Boolean)`. Retrieves the value of a property with a
  /// given key from a JsonObject as an Option.
  /// \param Key The AL `Text`.
  /// \param DefaultIfNotFound The AL `Boolean`.
  /// \return The AL `Option`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer GetOption(std::string_view Key, ::agiru::Boolean DefaultIfNotFound);

  /// \brief AL `JsonObject.GetText(Text, Boolean)`. Retrieves the value of a property with a given
  /// key from a JsonObject as Text.
  /// \param Key The AL `Text`.
  /// \param DefaultIfNotFound The AL `Boolean`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string GetText(std::string_view Key, ::agiru::Boolean DefaultIfNotFound);

  /// \brief AL `JsonObject.GetTime(Text, Boolean)`. Retrieves the value of a property with a given
  /// key from a JsonObject as a Time.
  /// \param Key The AL `Text`.
  /// \param DefaultIfNotFound The AL `Boolean`.
  /// \return The AL `Time`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Time GetTime(std::string_view Key, ::agiru::Boolean DefaultIfNotFound);

  /// \brief AL `JsonObject.Keys()`. Gets a set of keys of the JsonObject.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Keys();

  /// \brief AL `JsonObject.Path()`. Retrieves the JSON path of the object relative to the root of
  /// its containing tree.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Path();

  /// \brief AL `JsonObject.ReadFrom(InStream)`. Reads the JSON data from the stream into a
  /// JsonObject variable.
  /// \param InStream The AL `InStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReadFrom(const ::agiru::InStream &InStream);

  /// \brief AL `JsonObject.ReadFrom(Text)`. Reads the JSON data from the string into a JsonObject
  /// variable.
  /// \param String The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReadFrom(std::string_view String);

  /// \brief AL `JsonObject.ReadFromYaml(InStream)`. Reads the YAML data from the stream into a
  /// JsonObject variable.
  /// \param InStream The AL `InStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReadFromYaml(const ::agiru::InStream &InStream);

  /// \brief AL `JsonObject.ReadFromYaml(Text)`. Reads the YAML data from the string into a
  /// JsonObject variable.
  /// \param String The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReadFromYaml(std::string_view String);

  /// \brief AL `JsonObject.Remove(Text)`. Removes the property with the given key from the object.
  /// \param Key The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Remove(std::string_view Key);

  /// \brief AL `JsonObject.Replace(Text, BigInteger)`. Replaces the value of the property with the
  /// given key with the new value.
  /// \param Key The AL `Text`.
  /// \param Value The AL `BigInteger`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view Key, ::agiru::BigInteger Value);

  /// \brief AL `JsonObject.Replace(Text, Boolean)`. Replaces the value of the property with the
  /// given key with the new value.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view Key, ::agiru::Boolean Value);

  /// \brief AL `JsonObject.Replace(Text, Byte)`. Replaces the value of the property with the given
  /// key with the new value.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Byte`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view Key, ::agiru::Byte Value);

  /// \brief AL `JsonObject.Replace(Text, Char)`. Replaces the value of the property with the given
  /// key with the new value.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Char`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view Key, ::agiru::Char Value);

  /// \brief AL `JsonObject.Replace(Text, Date)`. Replaces the value of the property with the given
  /// key with the new value.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Date`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view Key, ::agiru::Date Value);

  /// \brief AL `JsonObject.Replace(Text, DateTime)`. Replaces the value of the property with the
  /// given key with the new value.
  /// \param Key The AL `Text`.
  /// \param Value The AL `DateTime`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view Key, ::agiru::DateTime Value);

  /// \brief AL `JsonObject.Replace(Text, Decimal)`. Replaces the value of the property with the
  /// given key with the new value.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Decimal`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view Key, ::agiru::Decimal Value);

  /// \brief AL `JsonObject.Replace(Text, Duration)`. Replaces the value of the property with the
  /// given key with the new value.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Duration`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view Key, ::agiru::Duration Value);

  /// \brief AL `JsonObject.Replace(Text, Integer)`. Replaces the value of the property with the
  /// given key with the new value.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view Key, ::agiru::Integer Value);

  /// \brief AL `JsonObject.Replace(Text, JsonArray)`. Replaces the value of the property with the
  /// given key with the new value.
  /// \param Key The AL `Text`.
  /// \param Value The AL `JsonArray`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view Key, const ::agiru::JsonArray &Value);

  /// \brief AL `JsonObject.Replace(Text, JsonObject)`. Replaces the value of the property with the
  /// given key with the new value.
  /// \param Key The AL `Text`.
  /// \param Value The AL `JsonObject`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view Key, const ::agiru::JsonObject &Value);

  /// \brief AL `JsonObject.Replace(Text, JsonToken)`. Replaces the value of the property with the
  /// given key with the new value.
  /// \param Key The AL `Text`.
  /// \param Value The AL `JsonToken`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view Key, const ::agiru::JsonToken &Value);

  /// \brief AL `JsonObject.Replace(Text, JsonValue)`. Replaces the value of the property with the
  /// given key with the new value.
  /// \param Key The AL `Text`.
  /// \param Value The AL `JsonValue`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view Key, const ::agiru::JsonValue &Value);

  /// \brief AL `JsonObject.Replace(Text, Text)`. Replaces the value of the property with the given
  /// key with the new value.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view Key, std::string_view Value);

  /// \brief AL `JsonObject.Replace(Text, Time)`. Replaces the value of the property with the given
  /// key with the new value.
  /// \param Key The AL `Text`.
  /// \param Value The AL `Time`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view Key, ::agiru::Time Value);

  /// \brief AL `JsonObject.SelectToken(Text, JsonToken)`. Selects a JsonToken using a JPath
  /// expression.
  /// \param Path The AL `Text`.
  /// \param Result The AL `JsonToken`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectToken(std::string_view Path, ::agiru::JsonToken &Result);

  /// \brief AL `JsonObject.SelectTokens(Text, List of [JsonToken])`. Selects tokens based on a
  /// JPath expression and returns them in a new list.
  /// \param Path The AL `Text`.
  /// \param Result The AL `List of [JsonToken]`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectTokens(std::string_view Path, ::agiru::List<::agiru::JsonToken> &Result);

  /// \brief AL `JsonObject.Values()`. Gets a set of values of the JsonObject.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Values();

  /// \brief AL `JsonObject.WriteTo(OutStream)`. Serializes and writes the JSON data of the
  /// JsonObject to a given OutStream object.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::OutStream &OutStream);

  /// \brief AL `JsonObject.WriteTo(Text)`. Serializes and writes the JSON data of the JsonObject to
  /// a given Text object.
  /// \param String The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(std::string &String);

  /// \brief AL `JsonObject.WriteToYaml(OutStream)`. Serializes and writes the content of the
  /// JsonObject as YAML text to a given OutStream object.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteToYaml(const ::agiru::OutStream &OutStream);

  /// \brief AL `JsonObject.WriteToYaml(Text)`. Serializes and writes the JsonObject as YAML to a
  /// given Text object.
  /// \param String The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteToYaml(std::string &String);

  /// \brief AL `JsonObject.WriteWithSecretsTo(Dictionary of [Text, SecretText], SecretText)`.
  /// Replaces the placeholder values based on their paths with the provided secrets and then
  /// serializes and writes the content of the JsonObject to a SecretText.
  /// \param Secrets The AL `Dictionary of [Text, SecretText]`.
  /// \param Result The AL `SecretText`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean
  WriteWithSecretsTo(const ::agiru::Dictionary<std::string, ::agiru::SecretText> &Secrets,
                     ::agiru::SecretText &Result);

  /// \brief AL `JsonObject.WriteWithSecretsTo(Text, SecretText, SecretText)`. Replaces the
  /// placeholder value in the path with the secret and then serializes and writes the content of
  /// the JsonObject to a SecretText.
  /// \param Path The AL `Text`.
  /// \param Secret The AL `SecretText`.
  /// \param Result The AL `SecretText`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteWithSecretsTo(std::string_view Path,
                                      const ::agiru::SecretText &Secret,
                                      ::agiru::SecretText &Result);
};

} // namespace agiru
