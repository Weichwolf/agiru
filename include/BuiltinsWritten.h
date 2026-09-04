#pragma once

#include "runtime/Table.h"
#include "type/BigInteger.h"
#include "type/Date.h"
#include "type/Integer.h"
#include "type/StringValue.h"
#include "type/Variant.h"

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

/// \brief AL `System.WorkDate(Date)`. Gets and sets the work date for the current session.
/// \param NewDate The AL `Date`.
/// \return The AL `Date`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Date WorkDate(::agiru::Date NewDate = {});

/// \brief AL `System.Format(Any, Integer, Integer)`. Formats a value into a string.
///
/// \param Value The AL `Any`.
/// \param Length The AL `Text` length wanted; 0 for the whole of it.
/// \param FormatNumber The standard format, from the tables in `devenv-format-property.md`.
/// \return The AL `Text`.
/// \throws Error when the format number is one no table declares for that type, and when the value
///         is a record -- `Format(Record)` renders the primary key and no key exists at this layer.
///
/// \note THE STANDARD FORMATS ARE TABULATED AND THIS FOLLOWS THE TABLE. `devenv-format-property.md`
///       gives one table per type: 0 is the display format, 1 the edit format, 2 the AL CODE
///       CONSTANT format and 9 the XML format. The two that matter most are documented outright for
///       either enumeration -- `<Text>` for 0 and 1, `<Number>` for 2 and 9 -- which is what makes
///       `Assert.Equal`'s `Format(Left, 0, 2) = Format(Right, 0, 2)` a comparison of ORDINALS. The
///       predecessor paid for the other direction: it rendered the ordinal for every format, and
///       every `StrSubstNo` that substituted an enum leaked a number into the message.
///
/// \warning FORMATS 0 AND 1 ARE THE REGION'S AND THIS RUNTIME HAS NO REGION (board:0007). A date
///          reads `05-04-21` in Europe and `04/05/21` in the US, and a decimal's separators change
///          with it. Until a session carries a language, 0 and 1 render what 9 does for the types
///          whose display format is regional -- which is a WRONG answer rather than a plausible
///          one, and the tests that compare a formatted date will say so.
///
/// \note THE LENGTH TRUNCATES AND DOES NOT PAD. AL's own `Format(<caption>, 10)` into a `Code[10]`
///       is the case that needs it; padding every shorter result to the length is what the
///       predecessor measured as a regression across its asserts, so this cuts and leaves the rest
///       alone. A length of 0 is no length at all, which is AL's own default.
std::string Format(const ::agiru::Variant &Value,
                   ::agiru::Integer Length = {},
                   ::agiru::Integer FormatNumber = {});

/// \brief AL `System.Format(Any, Integer, Text)`. Formats a value with a format SPECIFICATION.
///
/// \param Value The AL `Any`.
/// \param Length The AL `Text` length wanted; 0 for whatever the specification produces.
/// \param FormatString The specification, built the way `devenv-format-property.md` builds one:
///        literal characters, and elements in angle brackets each optionally carrying an argument
///        after a comma.
/// \return The AL `Text`.
/// \throws Error when the specification names an element this runtime does not render yet, and
///         when an element wants a type the value is not.
///
/// \note THE ELEMENT IS REFUSED BY NAME RATHER THAN IGNORED. A specification that is half
///       understood renders a plausible string that no test can tell from a right one, which is
///       the class of defect this tree moves to the front. What is rendered today is
///       `<Integer>`, `<Sign>`, `<Text>`, `<Standard Format,n>`, `<Filler Character,c>` and the
///       numeric date and time elements; the rest -- `<Month Text>`, `<Precision,m:n>`,
///       `<Second dec.>`, `<Comma,.>` -- name themselves in the error (board:0007). Measured over
///       `Layers/W1`: `<Integer,n><Filler Character,0>` is the commonest specification at 26 sites.
///
/// \note THE FILLER IS READ FROM THE WHOLE SPECIFICATION BEFORE ANYTHING IS RENDERED, because AL
///       writes it AFTER what it fills: `'<Integer,2><Filler Character,0>'` pads the integer with
///       zeroes, and a left-to-right reading would have found the filler too late.
std::string
Format(const ::agiru::Variant &Value, ::agiru::Integer Length, std::string_view FormatString);

/// \brief AL `System.Clear(Any)` -- the value back to what it was before anything was assigned.
///
/// \tparam T The variable's type.
/// \param Variable The variable.
///
/// `system-clear-joker-method.md`: "For a composite data type, such as a record or an array, all
/// elements are cleared. Furthermore, all fields in a record will be initialized with the InitValue
/// Property of the field." It also names the Guid case -- "converts the GUID to zeros" -- and the
/// codeunit one, where "only the reference to the codeunit is deleted and not the codeunit itself".
///
/// \note THE COMPOSITE CASES ARE COMPILE-TIME BRANCHES AND NOT A RUNTIME TAG. A record is
///       recognised by the `TableTraits` its generator specialises and an array by the length its
///       declaration carries, so a `Clear` over a scalar compiles to one assignment and a `Clear`
///       over a record to one walk of its field table.
///
/// \note IT IS NOT `Init`. That one spares the primary key, which the page for it says outright;
///       this one clears everything, which the page for THIS one says just as plainly.
template <typename T> void Clear(T &Variable) {
  if constexpr (requires { ::agiru::TableTraits<T>::kTable; }) {
    ::agiru::detail::RuntimeClear(&Variable, ::agiru::TableTraits<T>::kTable);
  } else if constexpr (requires {
                         T::Length();
                         Variable[::agiru::Integer{1}];
                       }) {
    for (::agiru::Integer at = 1; at <= T::Length(); ++at) { Clear(Variable[at]); }
  } else {
    Variable = T{};
  }
}

/// \brief AL `Database.CompanyName()`. Gets the current company name.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string CompanyName();

/// \brief AL `System.Today()`. Gets the current date set in the operating system.
/// \return The AL `Date`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Date Today();
}
