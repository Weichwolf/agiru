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
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `TextBuilder` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `TextBuilder`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/textbuilder/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class TextBuilder {
public:
  /// \brief AL `TextBuilder.Append(Text)`. Appends a copy of the specified string to this
  /// TextBuilder instance.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Append(std::string_view Text);

  /// \brief AL `TextBuilder.AppendLine(Text)`. Appends a copy of the specified string followed by
  /// the default line terminator to the end of the current TextBuilder object. If this parameter is
  /// omitted, only the line terminator will be appended.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean AppendLine(std::string_view Text);

  /// \brief AL `TextBuilder.Capacity(Integer)`. Gets or sets the maximum number of characters that
  /// can be contained in the memory allocated by the current instance.
  /// \param NewCapacity The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Capacity(::agiru::Integer NewCapacity);

  /// \brief AL `TextBuilder.Clear()`. Removes all characters from the current TextBuilder instance.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Clear();

  /// \brief AL `TextBuilder.EnsureCapacity(Integer)`. Ensures that the capacity of this TextBuilder
  /// instance is at least the specified value.
  /// \param NewCapacity The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean EnsureCapacity(::agiru::Integer NewCapacity);

  /// \brief AL `TextBuilder.Insert(Integer, Text)`. Inserts a string into this TextBuilder instance
  /// at the specified character position.
  /// \param Position The AL `Integer`.
  /// \param Text The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Insert(::agiru::Integer Position, std::string_view Text);

  /// \brief AL `TextBuilder.Length(Integer)`. Gets or sets the length of this TextBuilder instance.
  /// \param NewLength The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer Length(::agiru::Integer NewLength);

  /// \brief AL `TextBuilder.MaxCapacity()`. Gets the maximum capacity of this TextBuilder instance.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer MaxCapacity();

  /// \brief AL `TextBuilder.Remove(Integer, Integer)`. Removes the specified range of characters
  /// from this TextBuilder instance.
  /// \param StartIndex The AL `Integer`.
  /// \param Count The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Remove(::agiru::Integer StartIndex, ::agiru::Integer Count);

  /// \brief AL `TextBuilder.Replace(Text, Text, Integer, Integer)`. Replaces, within a substring of
  /// this instance, all occurrences of a specified string in this TextBuilder instance with another
  /// specified string.
  /// \param OldText The AL `Text`.
  /// \param NewText The AL `Text`.
  /// \param StartIndex The AL `Integer`.
  /// \param Count The AL `Integer`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view OldText,
                           std::string_view NewText,
                           ::agiru::Integer StartIndex,
                           ::agiru::Integer Count);

  /// \brief AL `TextBuilder.Replace(Text, Text)`. Replaces all occurrences of a specified string in
  /// this TextBuilder instance with another specified string.
  /// \param OldText The AL `Text`.
  /// \param NewText The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Replace(std::string_view OldText, std::string_view NewText);

  /// \brief AL `TextBuilder.ToText()`. Converts the value of this TextBuilder instance to a Text.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string ToText();

  /// \brief AL `TextBuilder.ToText(Integer, Integer)`. Converts the value of a substring of this
  /// TextBuilder instance to a Text.
  /// \param StartIndex The AL `Integer`.
  /// \param Count The AL `Integer`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string ToText(::agiru::Integer StartIndex, ::agiru::Integer Count);
};

}
