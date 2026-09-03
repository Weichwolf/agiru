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
/// \brief AL `BigText` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `BigText`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/bigtext/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class BigText {
public:
  /// \brief AL `BigText.AddText(BigText, Integer)`. Adds a text string to a BigText variable.
  /// \param String The AL `BigText`.
  /// \param Position The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void AddText(const ::agiru::BigText &String, ::agiru::Integer Position);

  /// \brief AL `BigText.AddText(Text, Integer)`. Adds a text string to a BigText variable.
  /// \param String The AL `Text`.
  /// \param Position The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void AddText(std::string_view String, ::agiru::Integer Position);

  /// \brief AL `BigText.GetSubText(BigText, Integer, Integer)`. Gets part of a BigText variable.
  /// \param Variable The AL `BigText`.
  /// \param Position The AL `Integer`.
  /// \param Length The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer
  GetSubText(::agiru::BigText &Variable, ::agiru::Integer Position, ::agiru::Integer Length);

  /// \brief AL `BigText.GetSubText(Text, Integer, Integer)`. Gets part of a BigText variable.
  /// \param Variable The AL `Text`.
  /// \param Position The AL `Integer`.
  /// \param Length The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer
  GetSubText(std::string &Variable, ::agiru::Integer Position, ::agiru::Integer Length);

  /// \brief AL `BigText.Length()`. Retrieves the length of the text stored in this BigText
  /// instance.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Length();

  /// \brief AL `BigText.Read(InStream)`. Streams a BigText object that is stored as a BLOB in a
  /// table to a BigText variable.
  /// \param InStream The AL `InStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Read(const ::agiru::InStream &InStream);

  /// \brief AL `BigText.TextPos(Text)`. Gets the position at which a specific string first occurs
  /// in this BigText instance.
  /// \param String The AL `Text`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer TextPos(std::string_view String);

  /// \brief AL `BigText.Write(OutStream)`. Streams a BigText object to a BLOB field in a table.
  /// \param OutStream The AL `OutStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Write(const ::agiru::OutStream &OutStream);
};

}
