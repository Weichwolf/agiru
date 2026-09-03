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
/// \brief AL `JsonArray` -- the surface the platform documentation declares.

namespace agiru {

class JsonObject;
class JsonToken;
class JsonValue;

/// \brief AL `JsonArray`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/jsonarray/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class JsonArray {
public:
  /// \brief AL `JsonArray.Add(BigInteger)`. Adds a new value at the end of the JsonArray.
  /// \param Value The AL `BigInteger`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Add(::agiru::BigInteger Value);

  /// \brief AL `JsonArray.Add(Boolean)`. Adds a new value at the end of the JsonArray.
  /// \param Value The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Add(::agiru::Boolean Value);

  /// \brief AL `JsonArray.Add(Byte)`. Adds a new value at the end of the JsonArray.
  /// \param Value The AL `Byte`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Add(::agiru::Byte Value);

  /// \brief AL `JsonArray.Add(Char)`. Adds a new value at the end of the JsonArray.
  /// \param Value The AL `Char`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Add(::agiru::Char Value);

  /// \brief AL `JsonArray.Add(Date)`. Adds a new value at the end of the JsonArray.
  /// \param Value The AL `Date`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Add(::agiru::Date Value);

  /// \brief AL `JsonArray.Add(DateTime)`. Adds a new value at the end of the JsonArray.
  /// \param Value The AL `DateTime`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Add(::agiru::DateTime Value);

  /// \brief AL `JsonArray.Add(Decimal)`. Adds a new value at the end of the JsonArray.
  /// \param Value The AL `Decimal`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Add(::agiru::Decimal Value);

  /// \brief AL `JsonArray.Add(Duration)`. Adds a new value at the end of the JsonArray.
  /// \param Value The AL `Duration`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Add(::agiru::Duration Value);

  /// \brief AL `JsonArray.Add(Integer)`. Adds a new value at the end of the JsonArray.
  /// \param Value The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Add(::agiru::Integer Value);

  /// \brief AL `JsonArray.Add(JsonArray)`. Adds a new value at the end of the JsonArray.
  /// \param Value The AL `JsonArray`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Add(const ::agiru::JsonArray &Value);

  /// \brief AL `JsonArray.Add(JsonObject)`. Adds a new value at the end of the JsonArray.
  /// \param Value The AL `JsonObject`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Add(const ::agiru::JsonObject &Value);

  /// \brief AL `JsonArray.Add(JsonToken)`. Adds a new value at the end of the JsonArray.
  /// \param Value The AL `JsonToken`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Add(const ::agiru::JsonToken &Value);

  /// \brief AL `JsonArray.Add(JsonValue)`. Adds a new value at the end of the JsonArray.
  /// \param Value The AL `JsonValue`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Add(const ::agiru::JsonValue &Value);

  /// \brief AL `JsonArray.Add(Text)`. Adds a new value at the end of the JsonArray.
  /// \param Value The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Add(std::string_view Value);

  /// \brief AL `JsonArray.Add(Time)`. Adds a new value at the end of the JsonArray.
  /// \param Value The AL `Time`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Add(::agiru::Time Value);

  /// \brief AL `JsonArray.AsToken()`. Converts the value in a JsonArray to a JsonToken data type.
  /// \return The AL `JsonToken`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::JsonToken AsToken();

  /// \brief AL `JsonArray.Clone()`. Creates a deep-copy of the JsonArray value.
  /// \return The AL `JsonToken`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::JsonToken Clone();

  /// \brief AL `JsonArray.Count()`. Gets the number of elements in the JsonArray.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Count();

  /// \brief AL `JsonArray.Get(Integer, JsonToken)`. Retrieves the value at the given index in the
  /// JsonArray.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `JsonToken`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Get(::agiru::Integer Index, ::agiru::JsonToken &Result);

  /// \brief AL `JsonArray.GetArray(Integer)`. Retrieves the value at the given index in the
  /// JsonArray.
  /// \param Index The AL `Integer`.
  /// \return The AL `JsonArray`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::JsonArray GetArray(::agiru::Integer Index);

  /// \brief AL `JsonArray.GetBigInteger(Integer)`. Retrieves the value at the given index in the
  /// JsonArray.
  /// \param Index The AL `Integer`.
  /// \return The AL `BigInteger`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::BigInteger GetBigInteger(::agiru::Integer Index);

  /// \brief AL `JsonArray.GetBoolean(Integer)`. Retrieves the value at the given index in the
  /// JsonArray.
  /// \param Index The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetBoolean(::agiru::Integer Index);

  /// \brief AL `JsonArray.GetByte(Integer)`. Retrieves the value at the given index in the
  /// JsonArray.
  /// \param Index The AL `Integer`.
  /// \return The AL `Byte`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Byte GetByte(::agiru::Integer Index);

  /// \brief AL `JsonArray.GetChar(Integer)`. Retrieves the value at the given index in the
  /// JsonArray.
  /// \param Index The AL `Integer`.
  /// \return The AL `Char`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Char GetChar(::agiru::Integer Index);

  /// \brief AL `JsonArray.GetDate(Integer)`. Retrieves the value at the given index in the
  /// JsonArray.
  /// \param Index The AL `Integer`.
  /// \return The AL `Date`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Date GetDate(::agiru::Integer Index);

  /// \brief AL `JsonArray.GetDateTime(Integer)`. Retrieves the value at the given index in the
  /// JsonArray.
  /// \param Index The AL `Integer`.
  /// \return The AL `DateTime`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::DateTime GetDateTime(::agiru::Integer Index);

  /// \brief AL `JsonArray.GetDecimal(Integer)`. Retrieves the value at the given index in the
  /// JsonArray.
  /// \param Index The AL `Integer`.
  /// \return The AL `Decimal`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Decimal GetDecimal(::agiru::Integer Index);

  /// \brief AL `JsonArray.GetDuration(Integer)`. Retrieves the value at the given index in the
  /// JsonArray.
  /// \param Index The AL `Integer`.
  /// \return The AL `Option`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer GetDuration(::agiru::Integer Index);

  /// \brief AL `JsonArray.GetInteger(Integer)`. Retrieves the value at the given index in the
  /// JsonArray.
  /// \param Index The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer GetInteger(::agiru::Integer Index);

  /// \brief AL `JsonArray.GetObject(Integer)`. Retrieves the value at the given index in the
  /// JsonArray.
  /// \param Index The AL `Integer`.
  /// \return The AL `JsonObject`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::JsonObject GetObject(::agiru::Integer Index);

  /// \brief AL `JsonArray.GetOption(Integer)`. Retrieves the value at the given index in the
  /// JsonArray.
  /// \param Index The AL `Integer`.
  /// \return The AL `Option`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer GetOption(::agiru::Integer Index);

  /// \brief AL `JsonArray.GetText(Integer)`. Retrieves the value at the given index in the
  /// JsonArray.
  /// \param Index The AL `Integer`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string GetText(::agiru::Integer Index);

  /// \brief AL `JsonArray.GetTime(Integer)`. Retrieves the value at the given index in the
  /// JsonArray.
  /// \param Index The AL `Integer`.
  /// \return The AL `Time`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Time GetTime(::agiru::Integer Index);

  /// \brief AL `JsonArray.IndexOf(BigInteger)`. Determines the index of a specific value in the
  /// JsonArray.
  /// \param Value The AL `BigInteger`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(::agiru::BigInteger Value);

  /// \brief AL `JsonArray.IndexOf(Boolean)`. Determines the index of a specific value in the
  /// JsonArray.
  /// \param Value The AL `Boolean`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(::agiru::Boolean Value);

  /// \brief AL `JsonArray.IndexOf(Byte)`. Determines the index of a specific value in the
  /// JsonArray.
  /// \param Value The AL `Byte`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(::agiru::Byte Value);

  /// \brief AL `JsonArray.IndexOf(Char)`. Determines the index of a specific value in the
  /// JsonArray.
  /// \param Value The AL `Char`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(::agiru::Char Value);

  /// \brief AL `JsonArray.IndexOf(Date)`. Determines the index of a specific value in the
  /// JsonArray.
  /// \param Value The AL `Date`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(::agiru::Date Value);

  /// \brief AL `JsonArray.IndexOf(DateTime)`. Determines the index of a specific value in the
  /// JsonArray.
  /// \param Value The AL `DateTime`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(::agiru::DateTime Value);

  /// \brief AL `JsonArray.IndexOf(Decimal)`. Determines the index of a specific value in the
  /// JsonArray.
  /// \param Value The AL `Decimal`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(::agiru::Decimal Value);

  /// \brief AL `JsonArray.IndexOf(Duration)`. Determines the index of a specific value in the
  /// JsonArray.
  /// \param Value The AL `Duration`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(::agiru::Duration Value);

  /// \brief AL `JsonArray.IndexOf(Integer)`. Determines the index of a specific value in the
  /// JsonArray.
  /// \param Value The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(::agiru::Integer Value);

  /// \brief AL `JsonArray.IndexOf(JsonArray)`. Determines the index of a specific value in the
  /// JsonArray.
  /// \param Value The AL `JsonArray`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(const ::agiru::JsonArray &Value);

  /// \brief AL `JsonArray.IndexOf(JsonObject)`. Determines the index of a specific value in the
  /// JsonArray.
  /// \param Value The AL `JsonObject`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(const ::agiru::JsonObject &Value);

  /// \brief AL `JsonArray.IndexOf(JsonToken)`. Determines the index of a specific value in the
  /// JsonArray.
  /// \param Value The AL `JsonToken`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(const ::agiru::JsonToken &Value);

  /// \brief AL `JsonArray.IndexOf(JsonValue)`. Determines the index of a specific value in the
  /// JsonArray.
  /// \param Value The AL `JsonValue`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(const ::agiru::JsonValue &Value);

  /// \brief AL `JsonArray.IndexOf(Text)`. Determines the index of a specific value in the
  /// JsonArray.
  /// \param Value The AL `Text`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(std::string_view Value);

  /// \brief AL `JsonArray.IndexOf(Time)`. Determines the index of a specific value in the
  /// JsonArray.
  /// \param Value The AL `Time`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(::agiru::Time Value);

  /// \brief AL `JsonArray.Insert(Integer, BigInteger)`. Inserts the value at the given index in the
  /// array while shifting all the values to the right by one position.
  /// \param Index The AL `Integer`.
  /// \param Value The AL `BigInteger`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Index, ::agiru::BigInteger Value);

  /// \brief AL `JsonArray.Insert(Integer, Boolean)`. Inserts the value at the given index in the
  /// array while shifting all the values to the right by one position.
  /// \param Index The AL `Integer`.
  /// \param Value The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Index, ::agiru::Boolean Value);

  /// \brief AL `JsonArray.Insert(Integer, Byte)`. Inserts the value at the given index in the array
  /// while shifting all the values to the right by one position.
  /// \param Index The AL `Integer`.
  /// \param Value The AL `Byte`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Index, ::agiru::Byte Value);

  /// \brief AL `JsonArray.Insert(Integer, Char)`. Inserts the value at the given index in the array
  /// while shifting all the values to the right by one position.
  /// \param Index The AL `Integer`.
  /// \param Value The AL `Char`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Index, ::agiru::Char Value);

  /// \brief AL `JsonArray.Insert(Integer, Date)`. Inserts the value at the given index in the array
  /// while shifting all the values to the right by one position.
  /// \param Index The AL `Integer`.
  /// \param Value The AL `Date`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Index, ::agiru::Date Value);

  /// \brief AL `JsonArray.Insert(Integer, DateTime)`. Inserts the value at the given index in the
  /// array while shifting all the values to the right by one position.
  /// \param Index The AL `Integer`.
  /// \param Value The AL `DateTime`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Index, ::agiru::DateTime Value);

  /// \brief AL `JsonArray.Insert(Integer, Decimal)`. Inserts the value at the given index in the
  /// array while shifting all the values to the right by one position.
  /// \param Index The AL `Integer`.
  /// \param Value The AL `Decimal`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Index, ::agiru::Decimal Value);

  /// \brief AL `JsonArray.Insert(Integer, Duration)`. Inserts the value at the given index in the
  /// array while shifting all the values to the right by one position.
  /// \param Index The AL `Integer`.
  /// \param Value The AL `Duration`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Index, ::agiru::Duration Value);

  /// \brief AL `JsonArray.Insert(Integer, Integer)`. Inserts the value at the given index in the
  /// array while shifting all the values to the right by one position.
  /// \param Index The AL `Integer`.
  /// \param Value The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Index, ::agiru::Integer Value);

  /// \brief AL `JsonArray.Insert(Integer, JsonArray)`. Inserts the value at the given index in the
  /// array while shifting all the values to the right by one position.
  /// \param Index The AL `Integer`.
  /// \param Value The AL `JsonArray`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Index, const ::agiru::JsonArray &Value);

  /// \brief AL `JsonArray.Insert(Integer, JsonObject)`. Inserts the value at the given index in the
  /// array while shifting all the values to the right by one position.
  /// \param Index The AL `Integer`.
  /// \param Value The AL `JsonObject`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Index, const ::agiru::JsonObject &Value);

  /// \brief AL `JsonArray.Insert(Integer, JsonToken)`. Inserts the value at the given index in the
  /// array while shifting all the values to the right by one position.
  /// \param Index The AL `Integer`.
  /// \param Value The AL `JsonToken`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Index, const ::agiru::JsonToken &Value);

  /// \brief AL `JsonArray.Insert(Integer, JsonValue)`. Inserts the value at the given index in the
  /// array while shifting all the values to the right by one position.
  /// \param Index The AL `Integer`.
  /// \param Value The AL `JsonValue`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Index, const ::agiru::JsonValue &Value);

  /// \brief AL `JsonArray.Insert(Integer, Text)`. Inserts the value at the given index in the array
  /// while shifting all the values to the right by one position.
  /// \param Index The AL `Integer`.
  /// \param Value The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Index, std::string_view Value);

  /// \brief AL `JsonArray.Insert(Integer, Time)`. Inserts the value at the given index in the array
  /// while shifting all the values to the right by one position.
  /// \param Index The AL `Integer`.
  /// \param Value The AL `Time`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Index, ::agiru::Time Value);

  /// \brief AL `JsonArray.Path()`. Retrieves the JSON path of the array relative to the root of its
  /// containing tree.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Path();

  /// \brief AL `JsonArray.ReadFrom(InStream)`. Reads the JSON data from the stream into a JsonArray
  /// variable.
  /// \param Data The AL `InStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReadFrom(const ::agiru::InStream &Data);

  /// \brief AL `JsonArray.ReadFrom(Text)`. Reads the JSON data from the string into a JsonArray
  /// variable.
  /// \param String The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReadFrom(std::string_view String);

  /// \brief AL `JsonArray.RemoveAt(Integer)`. Removes the token at the given index.
  /// \param Index The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean RemoveAt(::agiru::Integer Index);

  /// \brief AL `JsonArray.SelectToken(Text, JsonToken)`. Selects a JsonToken using a JPath
  /// expression.
  /// \param Path The AL `Text`.
  /// \param Result The AL `JsonToken`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectToken(std::string_view Path, ::agiru::JsonToken &Result);

  /// \brief AL `JsonArray.SelectTokens(Text, List of [JsonToken])`. Selects tokens based on a JPath
  /// expression and returns them in a new list.
  /// \param Path The AL `Text`.
  /// \param Result The AL `List of [JsonToken]`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectTokens(std::string_view Path, ::agiru::List<::agiru::JsonToken> &Result);

  /// \brief AL `JsonArray.Set(Integer, BigInteger)`. Replaces the value at the given index with a
  /// new value.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `BigInteger`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Set(::agiru::Integer Index, ::agiru::BigInteger Result);

  /// \brief AL `JsonArray.Set(Integer, Boolean)`. Replaces the value at the given index with a new
  /// value.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Set(::agiru::Integer Index, ::agiru::Boolean Result);

  /// \brief AL `JsonArray.Set(Integer, Byte)`. Replaces the value at the given index with a new
  /// value.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `Byte`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Set(::agiru::Integer Index, ::agiru::Byte Result);

  /// \brief AL `JsonArray.Set(Integer, Char)`. Replaces the value at the given index with a new
  /// value.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `Char`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Set(::agiru::Integer Index, ::agiru::Char Result);

  /// \brief AL `JsonArray.Set(Integer, Date)`. Replaces the value at the given index with a new
  /// value.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `Date`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Set(::agiru::Integer Index, ::agiru::Date Result);

  /// \brief AL `JsonArray.Set(Integer, DateTime)`. Replaces the value at the given index with a new
  /// value.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `DateTime`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Set(::agiru::Integer Index, ::agiru::DateTime Result);

  /// \brief AL `JsonArray.Set(Integer, Decimal)`. Replaces the value at the given index with a new
  /// value.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `Decimal`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Set(::agiru::Integer Index, ::agiru::Decimal Result);

  /// \brief AL `JsonArray.Set(Integer, Duration)`. Replaces the value at the given index with a new
  /// value.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `Duration`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Set(::agiru::Integer Index, ::agiru::Duration Result);

  /// \brief AL `JsonArray.Set(Integer, Integer)`. Replaces the value at the given index with a new
  /// value.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Set(::agiru::Integer Index, ::agiru::Integer Result);

  /// \brief AL `JsonArray.Set(Integer, JsonArray)`. Replaces the value at the given index with a
  /// new value.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `JsonArray`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Set(::agiru::Integer Index, const ::agiru::JsonArray &Result);

  /// \brief AL `JsonArray.Set(Integer, JsonObject)`. Replaces the value at the given index with a
  /// new value.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `JsonObject`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Set(::agiru::Integer Index, const ::agiru::JsonObject &Result);

  /// \brief AL `JsonArray.Set(Integer, JsonToken)`. Replaces the value at the given index with a
  /// new value.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `JsonToken`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Set(::agiru::Integer Index, const ::agiru::JsonToken &Result);

  /// \brief AL `JsonArray.Set(Integer, JsonValue)`. Replaces the value at the given index with a
  /// new value.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `JsonValue`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Set(::agiru::Integer Index, const ::agiru::JsonValue &Result);

  /// \brief AL `JsonArray.Set(Integer, Text)`. Replaces the value at the given index with a new
  /// value.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Set(::agiru::Integer Index, std::string_view Result);

  /// \brief AL `JsonArray.Set(Integer, Time)`. Replaces the value at the given index with a new
  /// value.
  /// \param Index The AL `Integer`.
  /// \param Result The AL `Time`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Set(::agiru::Integer Index, ::agiru::Time Result);

  /// \brief AL `JsonArray.WriteTo(OutStream)`. Serializes and writes the JSON data of the JsonArray
  /// to a given OutStream object.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::OutStream &OutStream);

  /// \brief AL `JsonArray.WriteTo(Text)`. Serializes and writes the JSON data of the JsonArray to a
  /// given Text object.
  /// \param String The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(std::string &String);
};

}
