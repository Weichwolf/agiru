#pragma once

#include "type/BigInteger.h"
#include "type/Integer.h"

#include <optional>
#include <string>
#include <string_view>

/// \file
/// \brief The AL free functions that are WRITTEN rather than generated.
///
/// \note THEY LIVE APART SO THE GENERATOR CANNOT OVERWRITE THEM. `include/Builtins.h` and
///       `src/rt/Builtins.cpp` are produced by `scripts/gen_builtins.py`, and twelve of their
///       functions had been written INTO afterwards -- `DelChr` computes, and takes
///       `std::optional<std::string_view>` where a generated refusal takes a defaulted view.
///       Re-running the script turned every one of them back into a refusal, silently, and that is
///       what board:0046 was actually about. The generator skips whatever another door header
///       declares, so moving them here is the whole mechanism: no new rule, no list to maintain.
///
/// \note THE OPTIONAL PARAMETERS ARE AL'S. `DelChr(String)` deletes spaces and
///       `DelChr(String, '<')` deletes leading ones -- an absent argument is not an empty one, so
///       `std::optional` says which, and a default of `{}` could not.

namespace agiru {

/// \brief AL `Text.ConvertStr(Text, Text, Text)`. Replaces all chars in source found in
/// FromCharacters with the corresponding char in ToCharacters and returns the converted string. If
/// the length of the FromCharacters parameter and the ToChars parameter are different, an exception
/// is thrown. If the parameter FromCharacters or the parameter ToChars is empty, the source is
/// returned unmodified. Each element in source is only converted ONCE a double-replacement cannot
/// happen.
/// \param String The AL `Text`.
/// \param FromCharacters The AL `Text`.
/// \param ToCharacters The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string
ConvertStr(std::string_view String, std::string_view FromCharacters, std::string_view ToCharacters);

/// \brief AL `Text.CopyStr(Text, Integer, Integer)`. Copies a substring of any length from a
/// specific position in a string (text or code) to a new string.
/// \param String The AL `Text`.
/// \param Position The AL `Integer`.
/// \param Length The AL `Integer`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string CopyStr(std::string_view String,
                    ::agiru::Integer Position,
                    std::optional<::agiru::Integer> Length = std::nullopt);

/// \brief AL `Text.DelChr(Text, Text, Text)`. Deletes chars contained in the which parameter in a
/// string based on the contents on the where parameter. If the where parameter contains an
/// equal-sign, then all occurrences of characters in which is deleted from the current value. If
/// the where parameter contains a less-than, then the characters are only deleted when they are
/// first in the string. If the where parameter contains a greater-than, then the characters are
/// only deleted when they are the last in the string. If the where parameter contains any other
/// char, an exception is thrown. If the where parameter or the which parameter is empty, the source
/// is returned unmodified. The which parameter is to be considered as an array of chars to delete
/// where the order does not matter.
/// \param String The AL `Text`.
/// \param Where Where to delete, as a set of `=`, `<` and `>`; nothing means `=`.
/// \param Which The characters to delete; nothing means a space.
/// \return The AL `Text`.
/// \note OMITTED IS NOT EMPTY. The page's sentence about an empty parameter is about a caller who
///       PASSES `''`, and AL's own defaults are `=` and a space -- which is what makes
///       `DelChr(S)` strip every space and `DelChr(S, '<>')` trim both ends.
std::string DelChr(std::string_view String,
                   std::optional<std::string_view> Where = std::nullopt,
                   std::optional<std::string_view> Which = std::nullopt);

/// \brief AL `Text.DelStr(Text, Integer, Integer)`. Deletes a substring inside a string (text or
/// code).
/// \param String The AL `Text`.
/// \param Position The AL `Integer`.
/// \param Length The AL `Integer`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string DelStr(std::string_view String,
                   ::agiru::Integer Position,
                   std::optional<::agiru::Integer> Length = std::nullopt);

/// \brief AL `Text.IncStr(Text)`. Increases a positive number or decrease a negative number inside
/// a string by one (1).
/// \param String The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string IncStr(std::string_view String);

/// \brief AL `Text.IncStr(Text, BigInteger)`. Increases the number in a string by a given amount.
/// \param String The AL `Text`.
/// \param Increment The AL `BigInteger`.
/// \return The AL `Text`.
std::string IncStr(std::string_view String, ::agiru::BigInteger Increment);

/// \brief AL `Text.InsStr(Text, Text, Integer)`. Inserts a substring into a string.
/// \param String The AL `Text`.
/// \param SubString The AL `Text`.
/// \param Position The AL `Integer`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string InsStr(std::string_view String, std::string_view SubString, ::agiru::Integer Position);

/// \brief AL `Text.LowerCase(Text)`. Converts all letters in a string to lowercase.
/// \param String The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string LowerCase(std::string_view String);

/// \brief AL `Text.PadStr(Text, Integer, Text)`. Changes the length of a string to a specified
/// length. If the string is shorter than the specified length, length spaces are added at the end
/// of the string to match the length. If the string is longer than the specified length, the string
/// is truncated. If the specified length is less than 0, an exception is thrown.
/// \param String The AL `Text`.
/// \param Length The AL `Integer`.
/// \param FillCharacter The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string PadStr(std::string_view String,
                   ::agiru::Integer Length,
                   std::optional<std::string_view> FillCharacter = std::nullopt);

/// \brief AL `Text.SelectStr(Integer, Text)`. Retrieves a substring from a comma-separated string.
/// \param Number The AL `Integer`.
/// \param CommaString The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string SelectStr(::agiru::Integer Number, std::string_view CommaString);

/// \brief AL `Text.StrCheckSum(Text, Text, Integer)`. Calculates a checksum for a string that
/// contains a number. If the source is empty, 0 is returned. Each char in the source and in the
/// weight must be a numeric character 0-9, otherwise an exception is thrown. If the WeightString
/// parameter is shorter then the source, it is padded with '1' up until the length of source. If
/// the WeightString parameter is longer than the source, an exception is thrown.
/// \param String The AL `Text`.
/// \param WeightString The AL `Text`.
/// \param Modulus The number in the checksum formula; nothing means 10, which the page gives as
///                the default.
/// \return The AL `Integer`.
::agiru::Integer StrCheckSum(std::string_view String,
                             std::string_view WeightString = {},
                             std::optional<::agiru::Integer> Modulus = std::nullopt);

/// \brief AL `Text.StrPos(Text, Text)`. Searches for the first occurrence of substring inside a
/// string.
/// \param String The AL `Text`.
/// \param SubString The AL `Text`.
/// \return The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Integer StrPos(std::string_view String, std::string_view SubString);

/// \brief AL `Text.UpperCase(Text)`. Converts all letters in a string to uppercase.
/// \param String The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string UpperCase(std::string_view String);

}
