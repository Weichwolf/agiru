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
#include "type/RecordId.h"
#include "type/Stream.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `JsonValue` -- the surface the platform documentation declares.

namespace agiru {

class JsonToken;

/// \brief AL `JsonValue`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/jsonvalue/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class JsonValue {
public:
  /// \brief AL `JsonValue.AsBigInteger()`. Converts the value in a JsonValue to an BigInteger data
  /// type.
  /// \return The AL `BigInteger`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::BigInteger AsBigInteger();

  /// \brief AL `JsonValue.AsBoolean()`. Converts the value in a JsonValue to a Boolean data type.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AsBoolean();

  /// \brief AL `JsonValue.AsByte()`. Converts the value in a JsonValue to a Byte data type.
  /// \return The AL `Byte`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Byte AsByte();

  /// \brief AL `JsonValue.AsChar()`. Converts the value in a JsonValue to a Char data type.
  /// \return The AL `Char`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Char AsChar();

  /// \brief AL `JsonValue.AsCode()`. Converts the value in a JsonValue to a Code data type.
  /// \return The AL `Code`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string AsCode();

  /// \brief AL `JsonValue.AsDate()`. Converts the value in a JsonValue to a Date data type.
  /// \return The AL `Date`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Date AsDate();

  /// \brief AL `JsonValue.AsDateTime()`. Converts the value in a JsonValue to a DateTime data type.
  /// \return The AL `DateTime`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::DateTime AsDateTime();

  /// \brief AL `JsonValue.AsDecimal()`. Converts the value in a JsonValue to a Decimal data type.
  /// \return The AL `Decimal`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Decimal AsDecimal();

  /// \brief AL `JsonValue.AsDuration()`. Converts the value in a JsonValue to a Duration data type.
  /// \return The AL `Duration`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Duration AsDuration();

  /// \brief AL `JsonValue.AsInteger()`. Converts the value in a JsonValue to an Integer data type.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer AsInteger();

  /// \brief AL `JsonValue.AsOption()`. Converts the value in a JsonValue to an Option data type.
  /// \return The AL `Option`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer AsOption();

  /// \brief AL `JsonValue.AsText()`. Converts the value in a JsonValue to a Text data type.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string AsText();

  /// \brief AL `JsonValue.AsTime()`. Converts the value in a JsonValue to a Time data type.
  /// \return The AL `Time`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Time AsTime();

  /// \brief AL `JsonValue.AsToken()`. Converts the value in a JsonValue to a JsonToken data type.
  /// \return The AL `JsonToken`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::JsonToken AsToken();

  /// \brief AL `JsonValue.Clone()`. Creates a deep-copy of the JsonToken value.
  /// \return The AL `JsonToken`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::JsonToken Clone();

  /// \brief AL `JsonValue.IsNull()`. Indicates whether the JsonValue contains the JSON value of
  /// NULL.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsNull();

  /// \brief AL `JsonValue.IsUndefined()`. Indicates whether the JsonValue contains the JSON value
  /// of UNDEFINED.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsUndefined();

  /// \brief AL `JsonValue.Path()`. Retrieves the JSON path of the value relative to its containing
  /// tree.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Path();

  /// \brief AL `JsonValue.ReadFrom(InStream)`. Reads the JSON data from the stream into a JsonValue
  /// variable.
  /// \param Data The AL `InStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReadFrom(const ::agiru::InStream &Data);

  /// \brief AL `JsonValue.ReadFrom(Text)`. Reads the JSON data into a JsonValue variable.
  /// \param Data The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReadFrom(std::string_view Data);

  /// \brief AL `JsonValue.SelectToken(Text, JsonToken)`. Selects a JsonToken using a JPath
  /// expression.
  /// \param Path The AL `Text`.
  /// \param Result The AL `JsonToken`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SelectToken(std::string_view Path, ::agiru::JsonToken &Result);

  /// \brief AL `JsonValue.SetValue(BigInteger)`. Set the contents of the JsonValue variable to the
  /// JSON representation of the given value.
  /// \param Value The AL `BigInteger`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetValue(::agiru::BigInteger Value);

  /// \brief AL `JsonValue.SetValue(Boolean)`. Set the contents of the JsonValue variable to the
  /// JSON representation of the given value.
  /// \param Value The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetValue(::agiru::Boolean Value);

  /// \brief AL `JsonValue.SetValue(Byte)`. Set the contents of the JsonValue variable to the JSON
  /// representation of the given value.
  /// \param Value The AL `Byte`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetValue(::agiru::Byte Value);

  /// \brief AL `JsonValue.SetValue(Char)`. Set the contents of the JsonValue variable to the JSON
  /// representation of the given value.
  /// \param Value The AL `Char`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetValue(::agiru::Char Value);

  /// \brief AL `JsonValue.SetValue(Date)`. Set the contents of the JsonValue variable to the JSON
  /// representation of the given value.
  /// \param Value The AL `Date`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetValue(::agiru::Date Value);

  /// \brief AL `JsonValue.SetValue(DateTime)`. Set the contents of the JsonValue variable to the
  /// JSON representation of the given value.
  /// \param Value The AL `DateTime`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetValue(::agiru::DateTime Value);

  /// \brief AL `JsonValue.SetValue(Decimal)`. Set the contents of the JsonValue variable to the
  /// JSON representation of the given value.
  /// \param Value The AL `Decimal`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetValue(::agiru::Decimal Value);

  /// \brief AL `JsonValue.SetValue(Duration)`. Set the contents of the JsonValue variable to the
  /// JSON representation of the given value.
  /// \param Value The AL `Duration`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetValue(::agiru::Duration Value);

  /// \brief AL `JsonValue.SetValue(Integer)`. Set the contents of the JsonValue variable to the
  /// JSON representation of the given value.
  /// \param Value The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetValue(::agiru::Integer Value);

  /// \brief AL `JsonValue.SetValue(Text)`. Set the contents of the JsonValue variable to the JSON
  /// representation of the given value.
  /// \param Value The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetValue(std::string_view Value);

  /// \brief AL `JsonValue.SetValue(Time)`. Set the contents of the JsonValue variable to the JSON
  /// representation of the given value.
  /// \param Value The AL `Time`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetValue(::agiru::Time Value);

  /// \brief AL `JsonValue.SetValueToNull()`. Set the contents of the JsonValue variable to the JSON
  /// representation of NULL.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetValueToNull();

  /// \brief AL `JsonValue.SetValueToUndefined()`. Set the contents of the JsonValue variable to the
  /// JSON representation of UNDEFINED.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void SetValueToUndefined();

  /// \brief AL `JsonValue.WriteTo(OutStream)`. Serializes and writes the JSON data of the JsonValue
  /// to a given object.
  /// \param Data The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(const ::agiru::OutStream &Data);

  /// \brief AL `JsonValue.WriteTo(Text)`. Serializes and writes the JSON data of the JsonValue to a
  /// given object.
  /// \param Data The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean WriteTo(std::string &Data);
};

}
