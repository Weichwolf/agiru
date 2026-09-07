#pragma once

#include "runtime/Codeunit.h"
#include "runtime/Events.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/BigInteger.h"
#include "type/ClientType.h"
#include "type/DataClassification.h"
#include "type/Date.h"
#include "type/Dictionary.h"
#include "type/Duration.h"
#include "type/ErrorInfo.h"
#include "type/Integer.h"
#include "type/List.h"
#include "type/ObjectType.h"
#include "type/StringValue.h"
#include "type/TelemetryScope.h"
#include "type/Text.h"
#include "type/Variant.h"
#include "type/Verbosity.h"

#include <concepts>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

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

/// \brief Whether a dialog kind has a handler standing in for the user right now.
/// \param kind The dialog kind, as `HandlerKind` numbers it.
/// \param text What the dialog would show.
/// \param reply Where a `Confirm` or a `StrMenu` puts the answer.
/// \return True when a handler answered, false when none is installed.
///
/// \note IT IS THE DOOR'S HALF OF THE HANDLER TABLE. The builtins that show something call it
///       before refusing, so a test with `[HandlerFunctions]` gets its answer and one without gets
///       the platform's refusal (board:0054).
[[nodiscard]] bool AnsweredByHandler(std::int32_t kind, std::string_view text, void *reply);

/// \brief AL `System.CanLoadType(DotNet)`. Whether a .NET type can be loaded.
/// \tparam T Whatever AL handed it -- a `DotNet` variable is a refusal in this tree.
/// \param DotNet The AL `DotNet`.
/// \return Never.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
///
/// \note IT IS A TEMPLATE AND IT LIVES HERE FOR THAT REASON. The generator writes a `Variant`
///       parameter, and a `DotNet` variable is a `dotnet::Refused` that does not convert to one --
///       65 call sites (measured 2026-09-07). A written declaration is how the door keeps a shape
///       the generator cannot know (board:0607).
template <typename T>::agiru::Boolean CanLoadType(const T &DotNet) {
  static_cast<void>(DotNet);
  throw ::agiru::Error("System.CanLoadType(DotNet) is declared and not implemented yet "
                       "(board:0035)");
}

/// \brief AL `System.CompressArray(Array of [Text])`. Moves every non-empty string to the front.
/// \tparam A The array's class, which AL knows and the door does not.
/// \param StringArray The AL `Array of [Text]`.
/// \return Never.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
template <typename A>
  requires requires(const A &array) { array.Length(); } ::agiru::Integer
CompressArray(A &StringArray) {
  static_cast<void>(StringArray);
  throw ::agiru::Error("System.CompressArray(Array of [Text]) is declared and not implemented yet "
                       "(board:0035)");
}

/// \brief AL `System.CopyArray(Array of [Any], Array of [Any], Integer [, Integer])`.
/// \tparam A The target array's class.
/// \tparam B The source array's class.
/// \param NewArray Where the elements go.
/// \param Array    Where they come from.
/// \param Position Where the copy starts, one-based.
/// \param Length   How many; the rest when omitted.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
template <typename A, typename B>
  requires requires(const B &array) { array.Length(); }
void CopyArray(A &NewArray,
               const B &Array,
               ::agiru::Integer Position,
               ::agiru::Integer Length = {}) {
  static_cast<void>(NewArray);
  static_cast<void>(Array);
  static_cast<void>(Position);
  static_cast<void>(Length);
  throw ::agiru::Error("System.CopyArray is declared and not implemented yet (board:0035)");
}

/// \brief AL `Evaluate(Variable, String [, Number])` where the STRING is a refusal.
/// \tparam Any1 What AL is evaluating into.
/// \tparam T    The refusal's type.
/// \param Variable The AL `Any`.
/// \param refusal  The refusal standing in for the text.
/// \param Number   The AL `Integer`.
/// \return Whatever the refusal answers -- which is to raise.
///
/// \note IT HANDS THE REFUSAL ITS OWN JOB. A refusal converts to anything, so without this
///       overload it would convert to `std::string_view` and the door would refuse in the name of
///       `Evaluate` rather than in the name of the absent thing that produced the text.
template <typename Any1, typename T>
  requires requires { typename T::IsAlRefusal; } ::agiru::Boolean
Evaluate(Any1 &Variable, const T &refusal, ::agiru::Integer Number = {}) {
  static_cast<void>(Variable);
  static_cast<void>(Number);
  return static_cast<::agiru::Boolean>(refusal);
}

/// \brief AL `StartSession(SessionId, CodeunitId, Timeout, Company, Record)` -- the record travels
///        as its OWN table.
/// \tparam R The record's class, because AL hands a `var Record` of any table.
/// \param SessionId  Where the new session's id goes.
/// \param CodeunitId Which codeunit runs.
/// \param Timeout    How long it may take.
/// \param Company    Which company it runs in.
/// \param Record     The record it starts on.
/// \return Never.
/// \throws Error always -- a background session needs a session layer (board:0035).
template <typename R>
  requires requires { ::agiru::TableTraits<R>::kTable; } ::agiru::Boolean
StartSession(::agiru::Integer &SessionId,
             ::agiru::Integer CodeunitId,
             ::agiru::Duration Timeout,
             std::string_view Company,
             R &Record) {
  static_cast<void>(SessionId);
  static_cast<void>(CodeunitId);
  static_cast<void>(Timeout);
  static_cast<void>(Company);
  static_cast<void>(Record);
  throw ::agiru::Error("Session.StartSession is declared and not implemented yet (board:0035)");
}

/// \brief AL `StartSession(SessionId, CodeunitId, Company, Record, Timeout)`.
/// \tparam R The record's class.
/// \param SessionId  Where the new session's id goes.
/// \param CodeunitId Which codeunit runs.
/// \param Company    Which company it runs in.
/// \param Record     The record it starts on.
/// \param Timeout    How long it may take.
/// \return Never.
/// \throws Error always -- a background session needs a session layer (board:0035).
template <typename R>
  requires requires { ::agiru::TableTraits<R>::kTable; } ::agiru::Boolean
StartSession(::agiru::Integer &SessionId,
             ::agiru::Integer CodeunitId,
             std::string_view Company,
             R &Record,
             ::agiru::Duration Timeout) {
  static_cast<void>(Timeout);
  static_cast<void>(SessionId);
  static_cast<void>(CodeunitId);
  static_cast<void>(Company);
  static_cast<void>(Record);
  throw ::agiru::Error("Session.StartSession is declared and not implemented yet (board:0035)");
}

/// \brief AL `StartSession(SessionId, CodeunitId, Company, Record)`.
/// \tparam R The record's class.
/// \param SessionId  Where the new session's id goes.
/// \param CodeunitId Which codeunit runs.
/// \param Company    Which company it runs in.
/// \param Record     The record it starts on.
/// \return Never.
/// \throws Error always -- a background session needs a session layer (board:0035).
template <typename R>
  requires requires { ::agiru::TableTraits<R>::kTable; } ::agiru::Boolean
StartSession(::agiru::Integer &SessionId,
             ::agiru::Integer CodeunitId,
             std::string_view Company,
             R &Record) {
  static_cast<void>(SessionId);
  static_cast<void>(CodeunitId);
  static_cast<void>(Company);
  static_cast<void>(Record);
  throw ::agiru::Error("Session.StartSession is declared and not implemented yet (board:0035)");
}

/// \brief AL `System.ClearCollectedErrors()`. Clears the errors collected so far.
///
/// \note IT DOES NOT TOUCH THE TRANSACTION -- `runtime/Scopes.h` says why.
void ClearCollectedErrors();

/// \brief AL `System.GetCollectedErrors(Boolean)`. The errors collected so far.
/// \param Clear Whether to empty the collection after reading it.
/// \return One `ErrorInfo` per collected error, in the order they were collected.
[[nodiscard]] ::agiru::List<::agiru::ErrorInfo> GetCollectedErrors(::agiru::Boolean Clear = {});

/// \brief AL `System.HasCollectedErrors()`. Whether anything has been collected.
/// \return True when the collection is not empty.
[[nodiscard]] ::agiru::Boolean HasCollectedErrors();

/// \brief AL `System.GuiAllowed()`. Whether a UI is there to answer a dialog.
/// \return True when a test has installed handlers; otherwise it refuses.
/// \throws Error when no handler table is installed -- there is no UI to allow (board:0030).
[[nodiscard]] ::agiru::Boolean GuiAllowed();

/// \brief AL `System.Hyperlink(Text)`. Opens a URL on the client.
/// \param URL The address.
/// \throws Error when no handler answers -- opening one needs a running client (board:0030).
void Hyperlink(std::string_view URL);

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
::agiru::Text<0>
ConvertStr(std::string_view String, std::string_view FromCharacters, std::string_view ToCharacters);

/// \brief AL `Text.CopyStr(Text, Integer, Integer)`. Copies a substring of any length from a
/// specific position in a string (text or code) to a new string.
/// \param String The AL `Text`.
/// \param Position The AL `Integer`.
/// \param Length The AL `Integer`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Text<0> CopyStr(std::string_view String,
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
::agiru::Text<0> DelChr(std::string_view String,
                        std::optional<std::string_view> Where = std::nullopt,
                        std::optional<std::string_view> Which = std::nullopt);

/// \brief AL `Text.DelStr(Text, Integer, Integer)`. Deletes a substring inside a string (text or
/// code).
/// \param String The AL `Text`.
/// \param Position The AL `Integer`.
/// \param Length The AL `Integer`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Text<0> DelStr(std::string_view String,
                        ::agiru::Integer Position,
                        std::optional<::agiru::Integer> Length = std::nullopt);

/// \brief AL `Text.IncStr(Text)`. Increases a positive number or decrease a negative number inside
/// a string by one (1).
/// \param String The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Text<0> IncStr(std::string_view String);

/// \brief AL `Text.IncStr(Text, BigInteger)`. Increases the number in a string by a given amount.
/// \param String The AL `Text`.
/// \param Increment The AL `BigInteger`.
/// \return The AL `Text`.
::agiru::Text<0> IncStr(std::string_view String, ::agiru::BigInteger Increment);

/// \brief AL `Text.InsStr(Text, Text, Integer)`. Inserts a substring into a string.
/// \param String The AL `Text`.
/// \param SubString The AL `Text`.
/// \param Position The AL `Integer`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Text<0>
InsStr(std::string_view String, std::string_view SubString, ::agiru::Integer Position);

/// \brief AL `Text.LowerCase(Text)`. Converts all letters in a string to lowercase.
/// \param String The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Text<0> LowerCase(std::string_view String);

/// \brief AL `Text.PadStr(Text, Integer, Text)`. Changes the length of a string to a specified
/// length. If the string is shorter than the specified length, length spaces are added at the end
/// of the string to match the length. If the string is longer than the specified length, the string
/// is truncated. If the specified length is less than 0, an exception is thrown.
/// \param String The AL `Text`.
/// \param Length The AL `Integer`.
/// \param FillCharacter The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Text<0> PadStr(std::string_view String,
                        ::agiru::Integer Length,
                        std::optional<std::string_view> FillCharacter = std::nullopt);

/// \brief AL `Text.SelectStr(Integer, Text)`. Retrieves a substring from a comma-separated string.
/// \param Number The AL `Integer`.
/// \param CommaString The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Text<0> SelectStr(::agiru::Integer Number, std::string_view CommaString);

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
::agiru::Text<0> UpperCase(std::string_view String);

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

/// \brief AL `System.GetUrl(ClientType [, Text, ObjectType, Integer, RecordRef, Boolean])`.
///
/// \param ClientType Which client the URL is for.
/// \param Company The company, defaulting to the session's.
/// \param ObjectType The object kind to open.
/// \param ObjectId Its number.
/// \param RecordRef The record to open it on.
/// \param UseFilters Whether the record's filters travel in the URL.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
///
/// \note EVERYTHING AFTER THE CLIENT TYPE IS OPTIONAL, WHICH THE SOURCE PROVES AND THE PAGE DOES
///       NOT SAY. `system-geturl-...-method.md` marks only `UseFilters` `[Optional]`, and the
///       BaseApp calls this with one argument 35 times, two 9 times, three once, four 24 times,
///       five 5 times and six once (measured over `Layers/W1`, 2026-09-04). Where the
///       documentation DESCRIBES and the source DECLARES, the source declares -- and 35 call sites
///       that do not compile are a declaration.
std::string GetUrl(const ::agiru::ClientType &ClientType,
                   std::string_view Company = {},
                   const ::agiru::ObjectType &ObjectType = {},
                   ::agiru::Integer ObjectId = {},
                   const ::agiru::RecordRef &RecordRef = ::agiru::RecordRef{},
                   ::agiru::Boolean UseFilters = {},
                   std::string_view Layout = {});

/// \brief AL `System.GetUrl(ClientType [, Company] [, ObjectType] [, ObjectId] [, Record]
///        [, UseFilters] [, Layout])` -- the RECORD form.
/// \tparam Record The generated table's class.
/// \param ClientType Which client the URL is for.
/// \param Company The company name.
/// \param ObjectType What kind of object the URL opens.
/// \param ObjectId Its number.
/// \param Record The record the URL opens ON.
/// \param UseFilters Whether the record's filters travel in the URL.
/// \param Layout The report layout, where the object is a report.
/// \return Never.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
///
/// \note `system-geturl-clienttype-string-objecttype-integer-table-boolean-method.md` IS ITS OWN
///       PAGE beside the `recordref` one, so a record and a `RecordRef` are two overloads and not
///       one with a conversion -- which is the overload-filename rule, and the reason a body that
///       passes `Rec` did not compile.
template <typename Record>
  requires requires {
    { Record::kId } -> std::convertible_to<::agiru::TableId>;
  }
std::string GetUrl(const ::agiru::ClientType &ClientType,
                   std::string_view Company,
                   const ::agiru::ObjectType &ObjectType,
                   ::agiru::Integer ObjectId,
                   const Record &Record_,
                   ::agiru::Boolean UseFilters = {},
                   std::string_view Layout = {}) {
  static_cast<void>(ClientType);
  static_cast<void>(Company);
  static_cast<void>(ObjectType);
  static_cast<void>(ObjectId);
  static_cast<void>(Record_);
  static_cast<void>(UseFilters);
  static_cast<void>(Layout);
  throw ::agiru::Error("System.GetUrl(ClientType, Text, ObjectType, Integer, Record, Boolean, "
                       "Text) is declared and not implemented yet (board:0035)");
}

/// \brief AL `Session.LogMessage(Text, Text, Verbosity, DataClassification, TelemetryScope,
/// Dictionary of [Text, Text])`. Logs a trace message with its dimensions in one dictionary.
/// \param EventId The event's identifier.
/// \param Message The message.
/// \param Verbosity How loud it is.
/// \param DataClassification What kind of data it carries.
/// \param TelemetryScope Who sees it.
/// \param CustomDimensions The dimensions, by name.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
///
/// \note IT IS A SEPARATE OVERLOAD IN THE DOCUMENTATION and not a shape of the one below:
///       `session-logmessage-...-dictionary[text,text]-method.md` is its own page, which is the
///       filename rule CLAUDE.md names -- behaviour hangs off the ARGUMENT.
void LogMessage(std::string_view EventId,
                std::string_view Message,
                const ::agiru::Verbosity &Verbosity,
                const ::agiru::DataClassification &DataClassification,
                const ::agiru::TelemetryScope &TelemetryScope,
                const ::agiru::Dictionary<std::string, std::string> &CustomDimensions);

/// \brief AL `Session.LogMessage(Text, Text, Verbosity, DataClassification, TelemetryScope, Text,
/// Text [, Text, Text])`. Logs a trace message to a telemetry account.
/// \param EventId The event's identifier.
/// \param Message The message.
/// \param Verbosity How loud it is.
/// \param DataClassification What kind of data it carries.
/// \param TelemetryScope Who sees it.
/// \param Dimension1 The first custom dimension's name.
/// \param Value1 Its value.
/// \param Dimension2 The second dimension's name, if there is one.
/// \param Value2 Its value.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
///
/// \note THE SECOND DIMENSION PAIR IS OPTIONAL, for the reason `GetUrl` above gives: the page
///       marks none of the four, and the BaseApp passes one pair.
void LogMessage(std::string_view EventId,
                std::string_view Message,
                const ::agiru::Verbosity &Verbosity,
                const ::agiru::DataClassification &DataClassification,
                const ::agiru::TelemetryScope &TelemetryScope,
                std::string_view Dimension1,
                std::string_view Value1,
                std::string_view Dimension2 = {},
                std::string_view Value2 = {});

/// \brief AL `Session.BindSubscription(Codeunit)`. Binds a codeunit's `[EventSubscriber]`
/// methods so the events they subscribe to reach them.
///
/// \tparam T The generated codeunit class.
/// \param Codeunit The codeunit instance whose subscribers are bound.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
///
/// \note THE PARAMETER IS A TEMPLATE, BECAUSE AL'S IS A `Codeunit` AND NOT A `Variant`. The
///       generated codeunits are unrelated classes with no common base a signature could name, and
///       a `Variant` takes none of them -- which made all 30 call sites over the 78 UT codeunits a
///       compile error. What they share is the `CodeunitTraits` their generator specialises.
template <typename T>
  requires requires(T &held) {
    ::agiru::CodeunitTraits<std::remove_cvref_t<decltype(*held.operator->())>>::kId;
  } || requires { ::agiru::CodeunitTraits<T>::kId; } ::agiru::Boolean
BindSubscription(T &Codeunit) {
  if constexpr (requires { ::agiru::CodeunitTraits<T>::kId; }) {
    return ::agiru::detail::BindSubscriptions(::agiru::CodeunitTraits<T>::kId, &Codeunit);
  } else {
    auto &held = *Codeunit.operator->();
    return ::agiru::detail::BindSubscriptions(
        ::agiru::CodeunitTraits<std::remove_cvref_t<decltype(held)>>::kId, &held);
  }
}

/// \brief AL `Session.UnbindSubscription(Codeunit)`. Unbinds what `BindSubscription` bound.
/// \tparam T The generated codeunit class.
/// \param Codeunit The codeunit instance.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
template <typename T>
  requires requires(T &held) {
    ::agiru::CodeunitTraits<std::remove_cvref_t<decltype(*held.operator->())>>::kId;
  } || requires { ::agiru::CodeunitTraits<T>::kId; } ::agiru::Boolean
UnbindSubscription(T &Codeunit) {
  if constexpr (requires { ::agiru::CodeunitTraits<T>::kId; }) {
    return ::agiru::detail::UnbindSubscriptions(::agiru::CodeunitTraits<T>::kId, &Codeunit);
  } else {
    auto &held = *Codeunit.operator->();
    return ::agiru::detail::UnbindSubscriptions(
        ::agiru::CodeunitTraits<std::remove_cvref_t<decltype(held)>>::kId, &held);
  }
}

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
namespace detail {

/// \brief Whether a type is one of AL's arrays.
///
/// \tparam T The type.
///
/// It asks for the pair only an array has: a length, and an index that yields the ELEMENT ITSELF.
/// A text has a length and an index too, and its index yields a `Char` by value -- so asking for
/// the length alone caught every `Text` and `Code` as well.
template <typename T>
concept IsAlArray = requires(T &array, ::agiru::Integer at) {
  array.Length();
  { array[at] } -> std::same_as<decltype(array[at])>;
  requires std::is_lvalue_reference_v<decltype(array[at])>;
};

}

/// \brief AL `Round(Duration, Precision)`: a Duration is a number of milliseconds and rounds as
/// one.
/// \param number    The duration.
/// \param precision The precision.
/// \return The rounded milliseconds.
template <std::same_as<::agiru::Duration> D>
::agiru::Decimal Round(const D &number, const ::agiru::Decimal &precision) {
  return ::agiru::Round(::agiru::Decimal{static_cast<std::int64_t>(number)}, precision);
}

template <typename T> void Clear(T &Variable) {
  if constexpr (requires { Variable.operator->(); } && requires { *Variable; }) {
    Clear(*Variable);
  } else if constexpr (requires { ::agiru::TableTraits<T>::kTable; }) {
    ::agiru::detail::RuntimeClear(&Variable, ::agiru::TableTraits<T>::kTable);
  } else if constexpr (detail::IsAlArray<T>) {
    for (::agiru::Integer at = 1; at <= Variable.Length(); ++at) { Clear(Variable[at]); }
  } else {
    Variable = T{};
  }
}

/// \brief AL `Database.CompanyName()`. Gets the current company name.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Text<0> CompanyName();

/// \brief AL `System.Today()`. Gets the current date set in the operating system.
/// \return The AL `Date`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Date Today();

/// \brief AL `System.IsNull(DotNet)`. Whether a .NET variable holds no object.
///
/// \tparam T The rebuilt .NET class, or a refused one.
/// \param Variable The variable.
/// \return Never.
///
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
///
/// \note THE PARAMETER IS A TEMPLATE, NOT A `Variant`. The .NET stand-ins this run emits are
///       unrelated structs with no common base, so a `Variant` signature takes none of them --
///       the same shape `BindSubscription` needed for the generated codeunits.
template <typename T>
  requires(!std::convertible_to<const T &, ::agiru::Variant>)::agiru::Boolean
IsNull(const T &Variable) {
  static_cast<void>(Variable);
  throw ::agiru::Error("System.IsNull(DotNet) is declared and not implemented yet (board:0035)");
}

/// \brief AL `Database.UserId()`. Gets the user name of the user account that is logged on.
/// \return The AL `Text` naming the user this session runs as.
/// \note IT COMES FROM THE SESSION and not from a constant: a user is a property of the session,
///       and 23 of the 61 UT cases that run today reach it through `Library - Utility`.
::agiru::Text<0> UserId();

/// \brief AL `Database.UserSecurityId()`. Gets the security id of the user who is logged on.
/// \return The AL `Guid` this session's user carries.
/// \note IT COMES FROM THE SESSION for the reason `UserId()` gives -- a user is a property of the
///       session -- and 23 of the cases that run today reach it through `Library - Lower
///       Permissions`.
::agiru::Guid UserSecurityId();

/// \brief AL `Dialog.Message(Text [, Any, ...])` -- a message with substitution values.
/// \tparam Values The values' types.
/// \param String The message, with `%1`-style placeholders.
/// \param values What the placeholders are replaced with.
/// \throws Error always -- a message needs a running UI (board:0030).
/// \note VARIADIC, BECAUSE AL'S IS: the BaseApp passes up to five values.
/// \brief AL `Dialog.Message(Text)` -- the message alone.
/// \param String The message.
/// \throws Error always -- a message needs a running UI (board:0030).
inline void Message(std::string_view String) {
  if (::agiru::AnsweredByHandler(1, String, nullptr)) { return; }
  throw ::agiru::Error(std::string("Message(") + std::string(String) +
                       ") needs a running UI (board:0030)");
}

template <typename First, typename... Values>
void Message(std::string_view String, const First &first, const Values &...values) {
  const std::string shown = ::agiru::StrSubstNo(String, first, values...);
  if (::agiru::AnsweredByHandler(1, shown, nullptr)) { return; }
  throw ::agiru::Error(std::string("Message(") + shown + ") needs a running UI (board:0030)");
}

/// \brief AL `Session.LogMessage(Text, Text, Verbosity, DataClassification, TelemetryScope,
///        Dictionary of [Text, Text])` -- the dictionary form of the custom dimensions.
/// \param EventId            The event id.
/// \param Message            The message.
/// \param Verbosity          The verbosity.
/// \param DataClassification The classification.
/// \param Scope              The telemetry scope.
/// \param CustomDimensions   The dimensions, as a dictionary.
/// \note Telemetry has no sink here yet (board:0035), so the call is accepted and records nothing.
/// \tparam K The dictionary's key type -- `Text<0>` from a generated variable, `std::string` from
///           the door; both spell AL's `Text`.
/// \tparam V The dictionary's value type.
template <typename K, typename V>
void LogMessage(std::string_view EventId,
                std::string_view Message,
                ::agiru::Verbosity Verbosity,
                ::agiru::DataClassification DataClassification,
                ::agiru::TelemetryScope Scope,
                const ::agiru::Dictionary<K, V> &CustomDimensions) {
  static_cast<void>(EventId);
  static_cast<void>(Message);
  static_cast<void>(Verbosity);
  static_cast<void>(DataClassification);
  static_cast<void>(Scope);
  static_cast<void>(CustomDimensions);
}

/// \brief AL `Dialog.Confirm(Text [, Boolean] [, Any, ...])`. Asks the user a yes/no question.
/// \tparam Values The substitution values' types.
/// \param String The question, with `%1`-style placeholders.
/// \param Default Which button the dialog opens on.
/// \param values What the placeholders are replaced with.
/// \return Never.
/// \throws Error always -- a confirm needs a running UI (board:0030).
/// \note VARIADIC, BECAUSE AL'S IS. The BaseApp passes up to five values, and the generated
///       three-parameter declaration refused the fourth.
template <typename... Values>
::agiru::Boolean
Confirm(std::string_view String, ::agiru::Boolean Default, const Values &...values) {
  const std::string asked =
      sizeof...(values) == 0 ? std::string(String) : ::agiru::StrSubstNo(String, values...);
  ::agiru::Boolean reply = Default;
  if (::agiru::AnsweredByHandler(0, asked, &reply)) { return reply; }
  throw ::agiru::Error(std::string("Confirm(") + asked + ") needs a running UI (board:0030)");
}

/// \brief AL `Dialog.Confirm(Text)` -- the one-argument form.
/// \param String The question.
/// \return Never.
/// \throws Error always -- a confirm needs a running UI (board:0030).
inline ::agiru::Boolean Confirm(std::string_view String) {
  return Confirm(String, false);
}

/// \brief A text builtin over anything that RENDERS as text, which is how AL hands a GUID to
///        `CopyStr`, `LowerCase` or `UpperCase`.
/// \tparam T The source, which must carry a `ToText()` and must not already read as text.
template <typename T>
concept RendersAsText = (!std::convertible_to<const T &, std::string_view>) &&
                        requires(const T &value) { std::string_view{value.ToText()}; };

/// \brief AL `Text.LowerCase(Text)` over a value that renders as text.
/// \tparam T The source.
/// \param String The value.
/// \return The lower-cased rendering.
template <RendersAsText T>::agiru::Text<0> LowerCase(const T &String) {
  return LowerCase(std::string_view(String.ToText()));
}

/// \brief AL `Text.UpperCase(Text)` over a value that renders as text.
/// \tparam T The source.
/// \param String The value.
/// \return The upper-cased rendering.
template <RendersAsText T>::agiru::Text<0> UpperCase(const T &String) {
  return UpperCase(std::string_view(String.ToText()));
}

/// \brief AL `Text.CopyStr(Text, Integer [, Integer])` over a value that renders as text.
/// \tparam T The source.
/// \param String The value.
/// \param Position Where the copy starts, one-based.
/// \param Length How many characters; the rest when omitted.
/// \return The copied rendering.
template <RendersAsText T>
::agiru::Text<0> CopyStr(const T &String, ::agiru::Integer Position, ::agiru::Integer Length = {}) {
  return CopyStr(std::string_view(String.ToText()), Position, Length);
}
}
