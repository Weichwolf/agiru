#pragma once

#include "dotnet/Refused.h"
#include "runtime/Codeunit.h"
#include "runtime/Events.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include "type/AuditCategory.h"
#include "type/BigInteger.h"
#include "type/ClientType.h"
#include "type/DataClassification.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/Decimal.h"
#include "type/Dictionary.h"
#include "type/Duration.h"
#include "type/ErrorInfo.h"
#include "type/ExecutionContext.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/List.h"
#include "type/ObjectType.h"
#include "type/SecretText.h"
#include "type/SecurityOperationResult.h"
#include "type/Stream.h"
#include "type/StringValue.h"
#include "type/TableConnectionType.h"
#include "type/TelemetryScope.h"
#include "type/Text.h"
#include "type/Time.h"
#include "type/Variant.h"
#include "type/Verbosity.h"

#include <concepts>
#include <cstdlib>
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

/// \brief AL `System.Date2DMY(Date, Integer)`. Gets the day, month or year of a Date.
///
/// \param Date  The date.
/// \param Value 1 for the day, 2 for the month, 3 for the year.
/// \return The part asked for, or 0 for the undefined date.
/// \throws Error when `Value` is none of the three, which the page calls the valid options.
::agiru::Integer Date2DMY(::agiru::Date Date, ::agiru::Integer Value);

/// \brief AL `System.Date2DWY(Date, Integer)`. Gets the weekday, week number or week year.
///
/// \param Date  The date.
/// \param Value 1 for the day of the week, 2 for the week number, 3 for the year.
/// \return The part asked for, or 0 for the undefined date.
/// \throws Error when `Value` is none of the three.
///
/// \note THE YEAR IS THE ISO WEEK-NUMBERING YEAR AND NOT THE CALENDAR ONE.
///       `system-date2dwy-method.md`: "Date2DWY always uses the ISO week-numbering year scheme",
///       so 2019-12-30 is week 1 of 2020 and its `Date2DWY(3)` is 2020.
::agiru::Integer Date2DWY(::agiru::Date Date, ::agiru::Integer Value);

/// \brief AL `System.EncryptionEnabled()`. Whether the tenant allows encryption.
///
/// \return False, because no key has been created here.
///
/// \note IT IS AN ANSWER AND NOT A REFUSAL. The page asks whether the TENANT is configured for
///       encryption, and this one is not: `CreateEncryptionKey` refuses, so nothing could have
///       enabled it. A refusal here stopped 22 UT procedures that only ask the question before
///       choosing a branch (measured 2026-09-08).
::agiru::Boolean EncryptionEnabled();

/// \brief AL `System.GlobalLanguage([Integer])`. Gets and sets the session's language.
///
/// \param NewLanguageID The Windows language id to move to; 0 -- which no language has -- leaves
///        it where it is, and that is how the reading form is spelled.
/// \return The language that was in force BEFORE the call.
///
/// \note THE RETURN IS THE OLD ONE, because that is what every call site does with it:
///       `Saved := GlobalLanguage; GlobalLanguage(Other); ...; GlobalLanguage(Saved)` is the shape
///       the BaseApp writes, and it needs the value it is about to replace.
///
/// \note IT IS THE SESSION'S AND NOT THE PROCESS'S. A service tier holds ten thousand sessions in
///       one process, so a global here would be one session answering for every other.
::agiru::Integer GlobalLanguage(::agiru::Integer NewLanguageID = {});

/// \brief AL `System.CalcDate(DateFormula)`. Calculates a new date from the current system date.
///
/// \param DateExpression The formula.
/// \return The date the formula reaches from today.
///
/// \note THE OMITTED REFERENCE DATE IS TODAY AND NOT THE WORKDATE.
///       `system-calcdate-dateformula-date-method.md` says "the current system date", which is
///       `Today()`; the workdate is a session setting a formula never consults.
///
/// \note THE ONE-ARGUMENT FORM IS ITS OWN OVERLOAD, because a default of `0D` cannot say whether
///       the caller omitted the date or passed a blank one -- and those two answers differ: an
///       explicit blank date stays blank, an omitted one is today.
::agiru::Date CalcDate(const ::agiru::DateFormula &DateExpression);

/// \brief AL `Session.CurrentClientType()`. Gets the client type that is running in the current
///        session.
/// \return `Web`, the client BC's test tool runs a test session in.
///
/// \note THE BASEAPP'S `= ClientType::Background` CHECKS guard the paths with no user to talk to,
///       and a test session is the opposite of that; 124 UT failures stood on the refusal
///       (measured 2026-09-08). A session opened for a job queue will say `Background` when there
///       is one (board:0035).
::agiru::ClientType CurrentClientType();

/// \brief AL `Session.ApplicationArea(Text)`. Gets or sets the application areas for the current
///        session.
/// \param ApplicationArea The areas to set, `#Basic,#Suite` style; empty reads without setting.
/// \return The areas that were current before the call.
/// \note PER SESSION, held with the session's other per-thread state; a test that sets
///       `ApplicationArea('#Basic')` and reads it back sees its own value (12 UT cases,
///       2026-09-09). Nothing here decides what an area SHOWS -- that is the page renderer's.
std::string ApplicationArea(std::string_view ApplicationArea = {});

/// \brief AL `System.ClosingDate(Date)`. The closing date of a normal date: after every posting
///        of that day and before the next day (`date-data-type.md`).
/// \param Date The normal date.
/// \return Its closing twin; a closing date answers itself, the undefined date itself.
::agiru::Date ClosingDate(::agiru::Date Date);

/// \brief AL `Session.LogAuditMessage(Text, SecurityOperationResult, AuditCategory, Integer,
///        Integer, Dictionary of [Text, Text])`. Writes an audit entry to the platform's telemetry.
/// \param SecurityAuditDescription   What happened.
/// \param SecurityAuditOperationResult Whether it succeeded.
/// \param SecurityAuditCategory      The category.
/// \param AuditMessageOperation      The operation code.
/// \param AuditMessageOperationResult The result code.
/// \param CustomDimensions           Further dimensions.
/// \note THERE IS NO TELEMETRY SINK HERE, the same as `Session.LogMessage`: the call is accepted
///       and the message goes nowhere. `Price Calculation - V16` logs feature uptake on every
///       price line, so a refusal here stopped the whole V16 path (ERM Document Totals UT,
///       2026-09-09).
void LogAuditMessage(
    std::string_view SecurityAuditDescription,
    const ::agiru::SecurityOperationResult &SecurityAuditOperationResult,
    const ::agiru::AuditCategory &SecurityAuditCategory,
    ::agiru::Integer AuditMessageOperation,
    ::agiru::Integer AuditMessageOperationResult,
    const ::agiru::Dictionary<::agiru::Text<0>, std::string> &CustomDimensions = {});

/// \brief AL `Database.SessionId()`. The number of the current session.
/// \return The operating system's id of this process: one process is one session here, and the
///         number is what `Session.LogMessage` and the job queue compare, never a key.
::agiru::Integer SessionId();

/// \brief AL `System.NormalDate(Date)`. The normal date of a closing date, and a normal date
///        unchanged (`system-normaldate-method.md`).
/// \param Date A normal or a closing date.
/// \return The date with the closing flag off.
/// \throws Error when the date is undefined: "A run-time error occurs if the value of Date is set
///         to the undefined date (0D)".
::agiru::Date NormalDate(::agiru::Date Date);

/// \brief AL `Session.GetExecutionContext()`. Gets the current session's execution context.
/// \return `Normal`; install, upgrade and background contexts have no runner yet (board:0035).
::agiru::ExecutionContext GetExecutionContext();

/// \brief AL `System.CalcDate(DateFormula, Date)`. Calculates a new date from a reference date.
///
/// \param DateExpression The formula.
/// \param Date The reference date.
/// \return The date the formula reaches from it, or the undefined date when it is undefined.
::agiru::Date CalcDate(const ::agiru::DateFormula &DateExpression, ::agiru::Date Date);

/// \brief AL `System.CalcDate(Text)`. Calculates a new date from the current system date.
///
/// \param DateExpression The formula, as text.
/// \return The date the formula reaches from today.
///
/// \note THE TEXT IS READ IN THE INVARIANT FORM. `DateFormula::FromText` takes the formula with
///       or without its angle brackets and refuses a language-dependent spelling by answering an
///       empty formula, which moves the date nowhere.
::agiru::Date CalcDate(std::string_view DateExpression);

/// \brief AL `System.CalcDate(Text, Date)`. Calculates a new date from a reference date.
///
/// \param DateExpression The formula, as text.
/// \param Date The reference date.
/// \return The date the formula reaches from it, or the undefined date when it is undefined.
::agiru::Date CalcDate(std::string_view DateExpression, ::agiru::Date Date);

/// \brief AL `System.CreateDateTime(Date, Time)`. Creates a DateTime from a date and a time.
/// \param Date The date.
/// \param Time The time.
/// \return The instant.
///
/// \note IT IS THE ONLY WAY IN. `datetime-data-type.md`: "The only constant available when you
///       use the DateTime data type is the undefined DateTime, 0DT" -- there is no literal, so
///       every DateTime an AL body builds comes through here.
::agiru::DateTime CreateDateTime(::agiru::Date Date, ::agiru::Time Time);

/// \brief AL `System.DT2Date(DateTime)`. Gets the date part of a DateTime.
/// \param Datetime The instant.
/// \return Its date.
::agiru::Date DT2Date(::agiru::DateTime Datetime);

/// \brief AL `System.DT2Time(DateTime)`. Gets the time part of a DateTime.
/// \param Datetime The instant.
/// \return Its time.
::agiru::Time DT2Time(::agiru::DateTime Datetime);

/// \brief The precision `RoundDateTime` uses when the caller names none: one second.
///
/// `system-rounddatetime-datetime-biginteger-text-method.md`: "The default value is 1000, which
/// rounds to the nearest second."
constexpr ::agiru::BigInteger kRoundToTheSecond = 1000;

/// \brief The base `Evaluate` reads a number in, which AL fixes at ten.
constexpr int kDecimal = 10;

/// \brief AL `System.RoundDateTime(DateTime [, BigInteger] [, Text])`. Rounds a DateTime.
///
/// \param Datetime  The instant to round.
/// \param Precision How many milliseconds one step is; 1000 -- a second -- when omitted.
/// \param Direction `=` to the nearest (the default), `>` up, `<` down.
/// \return The rounded instant.
/// \throws Error when the precision is not positive, which the page forbids, and when the
///         direction is none of the three.
///
/// \note IT ROUNDS THE MILLISECONDS AND NOTHING ELSE. The page says so outright -- "the DateTime
///       integer value is rounded as a numeric variable" -- so no calendar arithmetic is involved
///       and a precision of 3 600 000 rounds to the hour by division.
::agiru::DateTime RoundDateTime(::agiru::DateTime Datetime,
                                ::agiru::BigInteger Precision = kRoundToTheSecond,
                                std::string_view Direction = "=");

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
/// \param StringArray The AL `Array of [Text]`, rearranged in place.
/// \return How many entries are not empty, which is where the empty ones begin.
/// \note THE LENGTH DOES NOT CHANGE. `system-compressarray-method.md`: "The resulting
///       StringArray has the same number of elements as the input array, but empty entries
///       appear at the end of the array" -- so this fills the tail with blanks rather than
///       shortening anything, and the order of the non-empty entries is kept.
template <typename A>
  requires requires(const A &array) { array.Length(); } ::agiru::Integer
CompressArray(A &StringArray) {
  ::agiru::Integer kept = 0;
  for (::agiru::Integer at = 1; at <= StringArray.Length(); ++at) {
    if (std::string_view(StringArray[at]).empty()) { continue; }
    ++kept;
    if (kept != at) {
      StringArray[kept] = StringArray[at];
      StringArray[at] = {};
    }
  }
  return kept;
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

namespace detail {

/// \brief The number a `StartSession` hands back: counted up per call, never the caller's own.
/// \return The next session number.
[[nodiscard]] ::agiru::Integer NextStartedSession();

}

/// \brief AL `StartSession(SessionId, CodeunitId)` and `StartSession(SessionId, CodeunitId,
///        Company)`: `session-startsession-integer-integer-string-table-method.md` writes both
///        the company and the record in brackets, so both are optional.
/// \param SessionId  Where the new session's id goes.
/// \param CodeunitId Which codeunit runs.
/// \param Company    Which company it runs in; this one when omitted.
/// \return True, the way the platform answers when the session started.
/// \note THE CODEUNIT RUNS IN THIS SESSION, as the record-carrying overloads below explain.
::agiru::Boolean StartSession(::agiru::Integer &SessionId,
                              ::agiru::Integer CodeunitId,
                              std::string_view Company = {});

/// \brief AL `StartSession(SessionId, CodeunitId, Timeout, Company, Record)` -- the record travels
///        as its OWN table.
/// \tparam R The record's class, because AL hands a `var Record` of any table.
/// \param SessionId  Where the new session's id goes.
/// \param CodeunitId Which codeunit runs.
/// \param Timeout    How long it may take.
/// \param Company    Which company it runs in.
/// \param Record     The record it starts on.
/// \return Never.
/// \note THE CODEUNIT RUNS IN THIS SESSION, synchronously and inside the caller's transaction:
///       there is no second session layer yet (board:0035), so what the platform would run in
///       the background runs here before the call returns, and `SessionId` counts the calls.
///       A test that starts a session and then reads what it wrote sees it; one that expects
///       the work NOT to be there yet sees the deviation.
/// \return True, the way the platform answers when the session started.
template <typename R>
  requires requires { ::agiru::TableTraits<R>::kTable; } ::agiru::Boolean
StartSession(::agiru::Integer &SessionId,
             ::agiru::Integer CodeunitId,
             ::agiru::Duration Timeout,
             std::string_view Company,
             R &Record) {
  SessionId = ::agiru::detail::NextStartedSession();
  static_cast<void>(CodeunitId);
  static_cast<void>(Timeout);
  static_cast<void>(Company);
  return ::agiru::detail::RunCodeunitByNumber(true, CodeunitId, Record);
}

/// \brief AL `StartSession(SessionId, CodeunitId, Company, Record, Timeout)`.
/// \tparam R The record's class.
/// \param SessionId  Where the new session's id goes.
/// \param CodeunitId Which codeunit runs.
/// \param Company    Which company it runs in.
/// \param Record     The record it starts on.
/// \param Timeout    How long it may take.
/// \return Never.
/// \note THE CODEUNIT RUNS IN THIS SESSION, synchronously and inside the caller's transaction:
///       there is no second session layer yet (board:0035), so what the platform would run in
///       the background runs here before the call returns, and `SessionId` counts the calls.
///       A test that starts a session and then reads what it wrote sees it; one that expects
///       the work NOT to be there yet sees the deviation.
/// \return True, the way the platform answers when the session started.
template <typename R>
  requires requires { ::agiru::TableTraits<R>::kTable; } ::agiru::Boolean
StartSession(::agiru::Integer &SessionId,
             ::agiru::Integer CodeunitId,
             std::string_view Company,
             R &Record,
             ::agiru::Duration Timeout) {
  static_cast<void>(Timeout);
  SessionId = ::agiru::detail::NextStartedSession();
  static_cast<void>(CodeunitId);
  static_cast<void>(Company);
  return ::agiru::detail::RunCodeunitByNumber(true, CodeunitId, Record);
}

/// \brief AL `StartSession(SessionId, CodeunitId, Company, Record)`.
/// \tparam R The record's class.
/// \param SessionId  Where the new session's id goes.
/// \param CodeunitId Which codeunit runs.
/// \param Company    Which company it runs in.
/// \param Record     The record it starts on.
/// \return Never.
/// \note THE CODEUNIT RUNS IN THIS SESSION, synchronously and inside the caller's transaction:
///       there is no second session layer yet (board:0035), so what the platform would run in
///       the background runs here before the call returns, and `SessionId` counts the calls.
///       A test that starts a session and then reads what it wrote sees it; one that expects
///       the work NOT to be there yet sees the deviation.
/// \return True, the way the platform answers when the session started.
template <typename R>
  requires requires { ::agiru::TableTraits<R>::kTable; } ::agiru::Boolean
StartSession(::agiru::Integer &SessionId,
             ::agiru::Integer CodeunitId,
             std::string_view Company,
             R &Record) {
  SessionId = ::agiru::detail::NextStartedSession();
  static_cast<void>(CodeunitId);
  static_cast<void>(Company);
  return ::agiru::detail::RunCodeunitByNumber(true, CodeunitId, Record);
}

/// \brief AL `System.CopyStream(OutStream, InStream [, Integer])` between two AL streams.
///
/// \param OutStream   Where the bytes go.
/// \param InStream    Where they come from.
/// \param BytesToRead How many; the rest of the source when omitted or zero.
/// \return True when the bytes were copied.
///
/// \note IT COPIES BYTES AND ADDS NOTHING. `WriteText` is what puts characters in without a
///       terminator, and that is what a copy is: `system-copystream-outstream-instream-method.md`
///       calls it "the information that is contained in an InStream object to an OutStream
///       object", which is the bytes and no framing of this runtime's invention.
::agiru::Boolean CopyStream(::agiru::OutStream &OutStream,
                            ::agiru::InStream &InStream,
                            ::agiru::Integer BytesToRead = {});

/// \brief AL `System.CopyStream(OutStream, InStream [, Integer])` where either side is a .NET
///        stream this run does not have.
///
/// \tparam Out The destination -- an AL `OutStream` or an absent .NET type.
/// \tparam In  The source -- an AL `InStream` or an absent .NET type.
/// \param OutStream   Where the bytes go.
/// \param InStream    Where they come from.
/// \param BytesToRead How many; the rest when omitted.
/// \return Never.
/// \throws Error always -- an absent .NET type has no stream behind it (board:0035).
///
/// \note NOT `[[nodiscard]]`, because AL's own syntax block writes the return as OPTIONAL --
///       `[Ok := ] CopyStream(...)` -- and `RSACryptoServiceProviderImpl` calls it as a statement.
///
/// \note AL PASSES A `DotNet MemoryStream` STRAIGHT TO `CopyStream`, in either position:
///       `RSACryptoServiceProviderImpl` writes `CopyStream(OutputOutStream, DotNetMemoryStream)`
///       and `CopyStream(DotNetMemoryStream, InputInStream)` one procedure apart. The platform
///       bridges a .NET `Stream` to an AL stream; this tree has the .NET type as a refusing stub,
///       so the overload exists to REFUSE at the right place rather than to fail to compile at the
///       call site (board:0609).
template <typename Out, typename In>
  requires(::agiru::dotnet::IsAbsent<Out> || ::agiru::dotnet::IsAbsent<In> ||
           ::agiru::dotnet::IsRefusal<Out> || ::agiru::dotnet::IsRefusal<In>)::agiru::Boolean
CopyStream(Out &&OutStream, In &&InStream, ::agiru::Integer BytesToRead = {}) {
  static_cast<void>(OutStream);
  static_cast<void>(InStream);
  static_cast<void>(BytesToRead);
  throw ::agiru::Error("System.CopyStream reached a .NET stream this run does not have "
                       "(board:0609)");
}

/// \brief AL `System.DMY2Date(Day [, Month] [, Year])`. Builds a date from its parts.
/// \param Day   The day of the month, 1 to 31.
/// \param Month The month, 1 to 12; the current month when omitted.
/// \param Year  The four-digit year; the current year when omitted.
/// \return The date.
/// \throws Error when the three do not name a day.
///
/// \note THE DEFAULTS ARE THE PAGE'S, not zero: `dmy2date-method.md` says "if you omit this
///       optional parameter, the current month will be used as the default", and the same for the
///       year. Zero would build an undefined date and say nothing.
[[nodiscard]] ::agiru::Date
DMY2Date(::agiru::Integer Day, ::agiru::Integer Month = {}, ::agiru::Integer Year = {});

/// \brief AL `System.IsNullGuid(Guid)`. Whether every byte of the GUID is zero.
/// \param Guid The GUID.
/// \return True when it is the null GUID.
[[nodiscard]] ::agiru::Boolean IsNullGuid(::agiru::Guid Guid);

/// \brief AL `System.Power(Number, Power)`. Raises a number to a power.
/// \param Number The base.
/// \param Power  The exponent.
/// \return The result, as a Decimal.
/// \throws Error when the result is not a number -- a negative base with a fractional exponent.
///
/// \warning IT COMPUTES IN BINARY FLOATING POINT AND CONVERTS BACK, and that is allowed here for
///          the reason the invariant names: no binary float carries an AMOUNT. A power is a
///          multiplier -- `Power(10, Decimals)` is what the test library writes -- and BC computes
///          it the same way, through .NET's `Math.Pow`. Nothing in a posting line is produced by
///          this function; where a rounding matters, `Round` is what AL writes next to it.
[[nodiscard]] ::agiru::Decimal Power(::agiru::Decimal Number, ::agiru::Decimal Power);

/// \brief AL `System.Randomize([Seed])`. Seeds the session's random generator.
/// \param Seed The seed; 1 when omitted.
///
/// \note THE OMITTED SEED IS 1 AND NOT THE CLOCK. `randomize-method.md` says BC seeds from "the
///       total number of milliseconds since midnight" when the seed is omitted -- and DETERMINISM
///       IS COMPULSORY here (CLAUDE.md), so the same run must produce the same entries twice. The
///       BaseApp's own `Library - Random` calls `SetSeed` with 1, which is where the number comes
///       from; a test that wants a different sequence passes one.
void Randomize(::agiru::Integer Seed = {});

/// \brief AL `System.Random(Number)`. A pseudo-random integer between 1 and Number.
/// \param MaxNumber The largest number that may come back.
/// \return A number in `[1, MaxNumber]`.
///
/// \note IT IS .NET'S OWN GENERATOR, REBUILT. BC's `Random` is `System.Random.Next`, whose
///       algorithm is a subtractive lagged-Fibonacci generator with a documented seeding step --
///       and a test that seeds it and compares a SEQUENCE fails against any other generator. The
///       predecessor rebuilt the same one for the same reason.
///
/// \note THE GENERATOR IS PER SESSION, which the page states outright: "the random generator is
///       specific to each connection". It is `thread_local` here, which is what a session owns.
[[nodiscard]] ::agiru::Integer Random(::agiru::Integer MaxNumber);

/// \brief AL `System.Abs(Number)`. The absolute value of a number.
/// \param Number The input value.
/// \return Never.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
///
/// \note IT MOVED HERE WITH ITS DURATION SIBLING AND NOT BEFORE. `gen_builtins.py` skips a
///       builtin whose name another door header declares, so declaring only the Duration form
///       took the documented `Abs(Decimal)` out of the generated door entirely. A name is
///       written WHOLE or not at all.
::agiru::Decimal Abs(::agiru::Decimal Number);

/// \brief AL `System.Abs(Number)` where the number is a `Duration`.
/// \param Number The duration.
/// \return Its magnitude, as a duration.
///
/// \note AL HANDS A DURATION TO A DECIMAL PARAMETER, because a Duration IS a number of
///       milliseconds there: `BackupManagement` writes `if Abs(Time - WindowsUpdateTime) > 1000`.
///       The implicit conversion that would carry it was taken back once, measured -- it made
///       every mixed arithmetic ambiguous -- so the ONE builtin that needs it says so by name and
///       answers in the type it was handed.
///
/// \warning IT TAKES THE EXACT TYPE. A plain `Abs(Duration)` beside the documented
///          `Abs(Decimal)` makes `Abs(SomeInteger)` ambiguous -- an `Integer` reaches both in one
///          user-defined conversion -- which is 31 call sites in the slice (measured 2026-09-07).
template <typename T>
  requires std::same_as<T, ::agiru::Duration>
[[nodiscard]] constexpr ::agiru::Duration Abs(T Number) {
  return Number.Milliseconds() < 0 ? -Number : Number;
}

namespace detail {

/// \brief Reads one value out of its text form, for `Evaluate`.
///
/// \tparam T The value's type.
/// \param into The value.
/// \param text Its text.
/// \return True when the text spelled one.
///
/// \note EVERY READER HERE IS THE ONE THE DATABASE ALREADY USES for that type, which is what
///       keeps `Evaluate` one function instead of a table of parsers.
/// \brief AL `Evaluate(Boolean, Text)`: the spellings AL accepts for a Boolean.
/// \tparam T `Boolean` or `bool`.
/// \param into Where the value lands.
/// \param text The text.
/// \return Whether the text spelled a Boolean.
template <typename T>
[[nodiscard]] ::agiru::Boolean EvaluatedBoolean(T &into, std::string_view text) {
  if (text == "1" || text == "true" || text == "Yes" || text == "yes") {
    into = true;
    return true;
  }
  if (text == "0" || text == "false" || text == "No" || text == "no" || text.empty()) {
    into = false;
    return true;
  }
  return false;
}

/// \brief AL `Evaluate(Date, Text)` over the ISO form the runtime renders.
/// \param into Where the date lands.
/// \param text The text, `yyyy-mm-dd` or empty for the undefined date.
/// \return Whether the text spelled a date.
[[nodiscard]] inline ::agiru::Boolean EvaluatedDate(::agiru::Date &into, std::string_view text) {
  if (text.empty()) {
    into = ::agiru::Date{};
    return true;
  }
  constexpr std::size_t kIso = 10;
  constexpr std::size_t kYearAt = 0;
  constexpr std::size_t kYearDigits = 4;
  constexpr std::size_t kMonthAt = 5;
  constexpr std::size_t kDayAt = 8;
  constexpr std::size_t kPartDigits = 2;
  constexpr std::size_t kFirstDash = 4;
  constexpr std::size_t kSecondDash = 7;
  if (text.size() < kIso || text[kFirstDash] != '-' || text[kSecondDash] != '-') { return false; }
  const auto number = [text](std::size_t at, std::size_t digits) {
    return static_cast<int>(
        std::strtol(std::string(text.substr(at, digits)).c_str(), nullptr, kDecimal));
  };
  into = ::agiru::Date::FromYmd(number(kYearAt, kYearDigits),
                                static_cast<unsigned>(number(kMonthAt, kPartDigits)),
                                static_cast<unsigned>(number(kDayAt, kPartDigits)));
  return !into.IsUndefined();
}

template <typename T> [[nodiscard]] ::agiru::Boolean Evaluated(T &into, std::string_view text) {
  if constexpr (std::is_same_v<T, ::agiru::Boolean> || std::is_same_v<T, bool>) {
    return EvaluatedBoolean(into, text);
  } else if constexpr (std::is_same_v<T, ::agiru::Guid>) {
    const std::expected<::agiru::Guid, ::agiru::Refusal> read = ::agiru::Guid::FromText(text);
    if (!read.has_value()) { return false; }
    into = *read;
    return true;
  } else if constexpr (std::is_same_v<T, ::agiru::Decimal>) {
    if (text.find_first_not_of(' ') == std::string_view::npos) {
      into = ::agiru::Decimal{};
      return true;
    }
    try {
      into = ::agiru::Decimal::FromInvariantString(text);
    } catch (const ::agiru::DecimalError &) { return false; }
    return true;
  } else if constexpr (std::is_same_v<T, ::agiru::Date>) {
    return EvaluatedDate(into, text);
  } else if constexpr (std::is_same_v<T, ::agiru::DateFormula>) {
    const std::expected<::agiru::DateFormula, ::agiru::Refusal> read =
        ::agiru::DateFormula::FromText(text);
    if (!read.has_value()) { return false; }
    into = *read;
    return true;
  } else if constexpr (std::is_integral_v<T>) {
    if (text.empty()) {
      into = 0;
      return true;
    }
    const std::string held(text);
    char *end = nullptr;
    const long long read = std::strtoll(held.c_str(), &end, 10);
    if (end == nullptr || *end != '\0') { return false; }
    into = static_cast<T>(read);
    return true;
  } else if constexpr (std::derived_from<T, ::agiru::StringValue>) {
    into = text;
    return true;
  } else {
    static_cast<void>(into);
    static_cast<void>(text);
    return false;
  }
}

}

/// \brief AL `Evaluate(Variable, String)` where the VARIABLE belongs to a table this run does not
///        carry.
///
/// \tparam Any1 The refusal standing in for the field.
/// \tparam S    The text side, which is not a refusal.
/// \param Variable The refusal.
/// \return Never.
/// \throws Error always -- through the refusal, so the message names the absent field and not
///         `Evaluate` (board:0034).
template <typename Any1, typename S>
  requires requires { typename Any1::IsAlRefusal; } &&
           (!requires { typename S::IsAlRefusal; })::agiru::Boolean
Evaluate(Any1 &Variable, const S &String, ::agiru::Integer Number = {}) {
  static_cast<void>(String);
  static_cast<void>(Number);
  return static_cast<::agiru::Boolean>(Variable);
}

/// \brief AL `Evaluate(Variable, String [, Number])`. Reads a value out of its text form.
///
/// \tparam Any1 What AL is evaluating into, deduced from the `var` argument.
/// \param Variable Where the value goes.
/// \param String   The text.
/// \param Number   Which format the text is in; 0 and 9 are the ones this reads.
/// \return True when the text spelled a value of that type.
///
/// \note IT ANSWERS `false` AND DOES NOT RAISE, which is what the page's return value says: "true
///       if the operation was successful; otherwise false". A caller that discards the answer gets
///       AL's runtime error from the value context and not from here.
///
/// \note THE TYPE DECIDES THE READER, and every one of them already exists for the database:
///       `Decimal::FromInvariantString`, `Date::FromText`, `Guid::FromText`, and so on. This is
///       the same text a column round-trips through, which is what makes it one function rather
///       than a table of parsers.
template <typename Any1>
  requires(!requires { typename Any1::IsAlRefusal; })::agiru::Boolean
Evaluate(Any1 &Variable, std::string_view String, ::agiru::Integer Number = {}) {
  static_cast<void>(Number);
  return ::agiru::detail::Evaluated(Variable, String);
}

/// \brief AL `System.CreateGuid()`. A new unique GUID.
/// \return The GUID.
///
/// \note IT IS THE ONE PLACE THE RUNTIME IS DELIBERATELY NOT DETERMINISTIC, because the AL method
///       it implements is not either. `Guid::Create` carries the reasoning.
[[nodiscard]] ::agiru::Guid CreateGuid();

/// \brief AL `System.GetLastErrorCode()`. The CODE of the last error, not its text.
/// \return The code, which is the empty string for an error that carries none.
///
/// \note EVERY ERROR THIS RUNTIME RAISES IS UNCODED TODAY, and the empty string is what BC answers
///       for one. `system-getlasterrorcode-method.md` pairs it with `GetLastErrorText`: the code
///       identifies the KIND (`StreamIO`, `DB:RecordNotFound`) and is never translated, the text is
///       the message and is. When `agiru::Error` carries a code, this reads it instead of
///       answering empty -- and the test that compares against `'StreamIO'` is what will say so.
[[nodiscard]] ::agiru::Text<0> GetLastErrorCode();

/// \brief AL `System.Time()` -- the time of day, from the session's clock.
/// \return The current time.
///
/// \note THE DOOR SPELLS IT APART FROM AL, and this is the one place in the door that does.
///       `Time` is an AL data type as well as an AL builtin, and C++ cannot hold a class and a
///       function of one name in one namespace. The generator translates AL's `Time` into this
///       name (`kSpelledApart` in `src/gen/Door.cpp`); a reader who knows AL sees the deviation
///       here rather than guessing at it.
[[nodiscard]] ::agiru::Time CurrentTime();

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
::agiru::Text<0> Format(const ::agiru::Variant &Value,
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
::agiru::Text<0>
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
template <typename Handle>
  requires requires(Handle &held) {
    { *held } -> std::convertible_to<const typename std::remove_reference_t<decltype(*held)> &>;
    { std::remove_cvref_t<decltype(*held)>::kId };
  } && (!requires { Handle::kId; })
std::string GetUrl(const ::agiru::ClientType &ClientType,
                   std::string_view Company,
                   const ::agiru::ObjectType &ObjectType,
                   ::agiru::Integer ObjectId,
                   Handle &Record,
                   ::agiru::Boolean UseFilters = {},
                   std::string_view Layout = {}) {
  return GetUrl(ClientType, Company, ObjectType, ObjectId, *Record, UseFilters, Layout);
}

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
                const ::agiru::Dictionary<::agiru::Text<0>, std::string> &CustomDimensions);

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

/// \brief AL `System.IsNull(DotNet)` over a Variant, which is what an absent .NET stub becomes
///        on the way in.
/// \param DotNet The variable.
/// \return Never.
/// \throws Error always -- an absent .NET type holds nothing this runtime can ask (board:0035).
::agiru::Boolean IsNull(const ::agiru::Variant &DotNet);

/// \brief AL `System.IsNull(DotNet)` over a rebuilt .NET class: what the class says of itself
///        when it can (`Regex`, `Encoding` before a factory ran), else never null -- a value type
///        that exists is not a null reference.
/// \tparam T A rebuilt class, recognised by the `Binder` its constructor is spelled through.
/// \param Object The variable. \return Whether it holds nothing yet.
template <typename T>
  requires requires { typename T::Binder; } && (!requires(const T &value) { value.IsNullObject(); })
[[nodiscard]] ::agiru::Boolean IsNull(const T &Object) {
  if constexpr (requires {
                  { Object.IsNull() } -> std::convertible_to<bool>;
                }) {
    return Object.IsNull();
  } else {
    return false;
  }
}

/// \brief AL `System.CodeCoverageInclude(Record)`. Includes the code that has been logged.
/// \param ObjectRecord The `AllObj` record whose objects are included.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void CodeCoverageInclude(::agiru::RecordRef &ObjectRecord);

/// \brief AL `System.CodeCoverageInclude(Record)` over a record global held by handle.
/// \tparam T The table. \param ObjectRecord The record.
template <typename T> void CodeCoverageInclude(::agiru::Instance<T> &ObjectRecord) {
  ::agiru::RecordRef reference;
  reference.GetTable(ObjectRecord);
  CodeCoverageInclude(reference);
}

/// \brief AL `SecretText.SecretStrSubstNo(Text, [SecretText, ...])`: `%1`, `%2` ... stand for the
///        values, and the result stays a secret.
/// \tparam Values Whatever AL passed -- `SecretText`, `Text` and the rest render as themselves.
/// \param String The pattern.
/// \param values The values.
/// \return The substituted secret.
template <typename... Values>
[[nodiscard]] ::agiru::SecretText SecretStrSubstNo(std::string_view String,
                                                   const Values &...values) {
  return ::agiru::SecretText::SecretStrSubstNo(String, values...);
}

/// \brief AL `Database.ServiceInstanceId()`. \return 1: one service tier per process here.
[[nodiscard]] ::agiru::Integer ServiceInstanceId();

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
  requires(!std::convertible_to<const T &, ::agiru::Variant>) &&
          (!requires(const T &value) { value.IsNullObject(); }) &&
          (!requires { typename T::Binder; })::agiru::Boolean
IsNull(const T &Variable) {
  static_cast<void>(Variable);
  throw ::agiru::Error("System.IsNull(DotNet) is declared and not implemented yet (board:0035)");
}

/// \brief AL `System.IsNull(DotNet)` for a rebuilt .NET class that knows whether it refers to an
///        object: `XmlDocument.SelectSingleNode` answers null when nothing matches, and AL tests
///        `IsNull(XmlNode)` for exactly that.
/// \tparam T A rebuilt .NET class with `IsNullObject()`.
/// \param Variable The variable.
/// \return Whether it holds no object.
template <typename T>
  requires requires(const T &value) {
    { value.IsNullObject() } -> std::convertible_to<bool>;
  }
[[nodiscard]] ::agiru::Boolean IsNull(const T &Variable) {
  return Variable.IsNullObject();
}

/// \brief AL `Database.SetDefaultTableConnection(TableConnectionType, Text, Boolean)`: names the
///        connection a `TableType = CRM` / `CDS` table reads through. There is no such connection
///        on premises, and the call refuses saying so.
/// \param Type   The connection type.
/// \param Name   The connection's name.
/// \param Scoped Whether the default holds for the current scope only.
/// \throws Error always.
void SetDefaultTableConnection(const ::agiru::TableConnectionType &Type,
                               std::string_view Name,
                               ::agiru::Boolean Scoped = {});

/// \brief The same call with a `Guid` for the name, which AL converts to text on the way in:
///        `CRM Int. Table. Subscriber` names its connection `Format(CreateGuid())` held in a Guid.
/// \param Type   The connection type.
/// \param Name   The connection's name as a Guid.
/// \param Scoped Whether the default holds for the current scope only.
/// \throws Error always.
template <typename Name>
  requires std::same_as<std::remove_cvref_t<Name>, ::agiru::Guid>
void SetDefaultTableConnection(const ::agiru::TableConnectionType &Type,
                               const Name &Connection,
                               ::agiru::Boolean Scoped = {}) {
  SetDefaultTableConnection(Type, std::string_view(Connection.ToText()), Scoped);
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
