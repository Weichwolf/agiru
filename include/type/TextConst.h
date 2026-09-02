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
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `TextConst` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `TextConst`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/textconst/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class TextConst {
public:
  /// \brief AL `TextConst.Contains(Text)`. Returns a value indicating whether a specified substring
  /// occurs within this string.
  /// \param Value The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Contains(std::string_view Value);

  /// \brief AL `TextConst.EndsWith(Text)`. Determines whether the end of this string instance
  /// matches the specified string.
  /// \param Value The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean EndsWith(std::string_view Value);

  /// \brief AL `TextConst.IndexOf(Text, Integer)`. Reports the one-based index of the first
  /// occurrence of the specified string in this instance.
  /// \param Value The AL `Text`.
  /// \param StartIndex The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOf(std::string_view Value, ::agiru::Integer StartIndex);

  /// \brief AL `TextConst.IndexOfAny(List of [Char], Integer)`. Reports the one-based index of the
  /// first occurrence in this instance of any character in a specified array of Unicode characters.
  /// The search starts at a specified character position.
  /// \param Values The AL `List of [Char]`.
  /// \param StartIndex The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOfAny(const ::agiru::List<::agiru::Char> &Values,
                              ::agiru::Integer StartIndex);

  /// \brief AL `TextConst.IndexOfAny(Text, Integer)`. Reports the one-based index of the first
  /// occurrence of the specified string in this instance. The search starts at a specified
  /// character position.
  /// \param Values The AL `Text`.
  /// \param StartIndex The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer IndexOfAny(std::string_view Values, ::agiru::Integer StartIndex);

  /// \brief AL `TextConst.LastIndexOf(Text, Integer)`. Reports the one-based index position of the
  /// last occurrence of a specified string in this instance.
  /// \param Value The AL `Text`.
  /// \param StartIndex The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer LastIndexOf(std::string_view Value, ::agiru::Integer StartIndex);

  /// \brief AL `TextConst.PadLeft(Integer, Char)`. Returns a new Text that right-aligns the
  /// characters in this instance by padding them on the left, for a specified total length.
  /// \param Count The AL `Integer`.
  /// \param Char The AL `Char`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string PadLeft(::agiru::Integer Count, ::agiru::Char Char);

  /// \brief AL `TextConst.PadRight(Integer, Char)`. Returns a new string that left-aligns the
  /// characters in this string by padding them with spaces on the right, for a specified total
  /// length.
  /// \param Count The AL `Integer`.
  /// \param Char The AL `Char`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string PadRight(::agiru::Integer Count, ::agiru::Char Char);

  /// \brief AL `TextConst.Remove(Integer, Integer)`. Returns a new Text in which a specified number
  /// of characters from the current string are deleted.
  /// \param StartIndex The AL `Integer`.
  /// \param Count The AL `Integer`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Remove(::agiru::Integer StartIndex, ::agiru::Integer Count);

  /// \brief AL `TextConst.Replace(Text, Text)`. Returns a new Text in which all occurrences of a
  /// specified string in the current instance are replaced with another specified string.
  /// \param OldValue The AL `Text`.
  /// \param NewValue The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Replace(std::string_view OldValue, std::string_view NewValue);

  /// \brief AL `TextConst.Split(List of [Char])`. Splits a string into a maximum number of
  /// substrings based on a collection of separators.
  /// \param Separators The AL `List of [Char]`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Split(const ::agiru::List<::agiru::Char> &Separators);

  /// \brief AL `TextConst.Split(List of [Text])`. Splits a string into a maximum number of
  /// substrings based on a collection of separators.
  /// \param Separators The AL `List of [Text]`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Split(const ::agiru::List<std::string> &Separators);

  /// \brief AL `TextConst.Split(Text)`. Splits a string into a maximum number of substrings based
  /// on a collection of separators.
  /// \param Separators The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Split(std::string_view Separators);

  /// \brief AL `TextConst.StartsWith(Text)`. Determines whether the beginning of this instance
  /// matches a specified string.
  /// \param Value The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean StartsWith(std::string_view Value);

  /// \brief AL `TextConst.Substring(Integer, Integer)`. Retrieves a substring from this instance.
  /// \param StartIndex The AL `Integer`.
  /// \param Count The AL `Integer`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Substring(::agiru::Integer StartIndex, ::agiru::Integer Count);

  /// \brief AL `TextConst.ToLower()`. Returns a copy of this string converted to lowercase.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string ToLower();

  /// \brief AL `TextConst.ToUpper()`. Returns a copy of this string converted to uppercase.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string ToUpper();

  /// \brief AL `TextConst.Trim()`. Returns a new Text in which all leading and trailing white-space
  /// characters from the current Text object are removed.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Trim();

  /// \brief AL `TextConst.TrimEnd(Text)`. Removes all trailing occurrences of a set of characters
  /// specified in an array from the current Text object.
  /// \param Chars The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string TrimEnd(std::string_view Chars);

  /// \brief AL `TextConst.TrimStart(Text)`. Removes all leading occurrences of a set of characters
  /// specified in an array from the current Text object.
  /// \param Chars The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string TrimStart(std::string_view Chars);
};

} // namespace agiru
