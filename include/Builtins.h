#pragma once

#include "platform/Date.h"
#include "platform/Integer.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "type/AuditCategory.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/ClientType.h"
#include "type/DataClassification.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Dictionary.h"
#include "type/Duration.h"
#include "type/ExecutionContext.h"
#include "type/ExecutionMode.h"
#include "type/Guid.h"
#include "type/KeyRef.h"
#include "type/ObjectType.h"
#include "type/RecordId.h"
#include "type/SecretText.h"
#include "type/SecurityOperationResult.h"
#include "type/Stream.h"
#include "type/TableConnectionType.h"
#include "type/TelemetryScope.h"
#include "type/Time.h"
#include "type/TransactionType.h"
#include "type/Variant.h"
#include "type/Verbosity.h"

#include <string>
#include <string_view>

/// \file
/// \brief The AL functions a body calls with NO RECEIVER.
///
/// The platform documents them under a type -- Text.StrSubstNo, System.WorkDate, Database.CalcDate
/// -- and every one carries the same note: "This method can be invoked without specifying the data
/// type name." That note is what selects them, and it is why they are here rather than on the
/// types: a generated body writes the name with nothing in front of it, so ordinary lookup in
/// `agiru` has to find it.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature is the one
///          methods-auto states, so a call site compiles and is CHECKED; the body refuses by name
///          rather than returning a plausible wrong answer (board:0035). What the UT milestone
///          leans on hardest is measured: StrSubstNo in 187 of its 2 392 test methods, WorkDate in
///          151, Format in 113 (board:0040).

namespace agiru {

/// \brief Refuses a door function by name.
///
/// \param what The AL signature, spelled as the page states it.
/// \throws Error always.
///
/// \note IT IS DECLARED HERE BECAUSE THE TEMPLATES NEED IT. A builtin whose parameter is `var Any`
///       is a template -- AL's `Clear` takes a record, a text or a list by reference and a Variant
///       takes none of them -- so its body is in this header rather than in the source, and it has
///       to reach the refusal from here.
[[noreturn]] void RefuseDoor(std::string_view what);

// ---- system ----

/// \brief AL `System.Abs(Decimal)`. Calculates the absolute value of a number (Decimal, Integer or
/// BigInteger). ABS always returns a positive numeric value or zero.
/// \param Number The AL `Decimal`.
/// \return The AL `Decimal`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Decimal Abs(::agiru::Decimal Number);

/// \brief AL `System.ApplicationPath()`. Returns the path of the directory where the executable
/// file for the product is installed.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string ApplicationPath();

/// \brief AL `System.ArrayLen(Array of [Any], Integer)`. Returns the total number of elements in an
/// array or the number of elements in a specific dimension.
/// \param Array The AL `Array of [Any]`.
/// \param Dimension The AL `Integer`.
/// \return The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Integer ArrayLen(const ::agiru::Variant &Array, ::agiru::Integer Dimension = {});

/// \brief AL `System.CalcDate(DateFormula, Date)`. Calculates a new date that is based on a date
/// expression and a reference date.
/// \param DateExpression The AL `DateFormula`.
/// \param Date The AL `Date`.
/// \return The AL `Date`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Date CalcDate(::agiru::DateFormula DateExpression, ::agiru::Date Date = {});

/// \brief AL `System.CalcDate(Text, Date)`. Calculates a new date that is based on a date
/// expression and a reference date.
/// \param DateExpression The AL `Text`.
/// \param Date The AL `Date`.
/// \return The AL `Date`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Date CalcDate(std::string_view DateExpression, ::agiru::Date Date = {});

/// \brief AL `System.CanLoadType(DotNet)`. Tests if the specified .NET Framework type can be
/// loaded.
/// \param DotNet The AL `DotNet`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean CanLoadType(const ::agiru::Variant &DotNet);

/// \brief AL `System.CaptionClassTranslate(Text)`. Returns a translated version of the caption
/// string. The string is translated to the current local language.
/// \param CaptionClassText The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string CaptionClassTranslate(std::string_view CaptionClassText);

/// \brief AL `System.Clear(Array of [Any])`. Clears the value of a single variable. Also, it clears
/// all the filters that were set if the variable is a record and resets the key to the primary key
/// and the company on a record variable.
/// \tparam Any1 What AL handed it.
/// \param Variable The AL `Array of [Any]`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
template <typename Any1> void Clear(Any1 &Variable) {
  static_cast<void>(Variable);
  RefuseDoor("System.Clear(Array of [Any])");
}

/// \brief AL `System.Clear(SecretText)`. Clears the value of a single variable.
/// \param Variable The AL `SecretText`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void Clear(::agiru::SecretText &Variable);

/// \brief AL `System.ClearAll()`. Clears all internal variables (except REC variables), keys, and
/// filters in the object and in any associated objects, such as reports, pages, codeunits, and so
/// on that contain AL code.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void ClearAll();

/// \brief AL `System.ClearCollectedErrors()`. Clears all collected errors from the current
/// collection scope.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void ClearCollectedErrors();

/// \brief AL `System.ClosingDate(Date)`. Gets the closing date for a Date Data Type.
/// \param Date The AL `Date`.
/// \return The AL `Date`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Date ClosingDate(::agiru::Date Date);

/// \brief AL `System.CodeCoverageInclude(Record)`. Includes the code that has been logged.
/// \param ObjectRecord The AL `Record`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void CodeCoverageInclude(::agiru::RecordRef &ObjectRecord);

/// \brief AL `System.CodeCoverageLoad()`. Loads the code that has been logged.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void CodeCoverageLoad();

/// \brief AL `System.CodeCoverageLog(Boolean, Boolean)`. Starts and stops the logging of code. You
/// can also use this method to retrieve the current logging status.
/// \param NewIsActive The AL `Boolean`.
/// \param MultiSession The AL `Boolean`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean CodeCoverageLog(::agiru::Boolean NewIsActive = {},
                                 ::agiru::Boolean MultiSession = {});

/// \brief AL `System.CodeCoverageRefresh()`. Refreshes the code that has been logged.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void CodeCoverageRefresh();

/// \brief AL `System.CompressArray(Array of [Text])`. Moves all non-empty strings (text) in an
/// array to the beginning of the array. The resulting StringArray has the same number of elements
/// as the input array, but empty entries appear at the end of the array.
/// \param StringArray The AL `Array of [Text]`.
/// \return The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Integer CompressArray(const ::agiru::Variant &StringArray);

/// \brief AL `System.CopyArray(Array of [Any], Array of [Any], Integer, Integer)`. Copies one or
/// more elements in an array to a new array.
/// \param NewArray The AL `Array of [Any]`.
/// \param Array The AL `Array of [Any]`.
/// \param Position The AL `Integer`.
/// \param Length The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void CopyArray(const ::agiru::Variant &NewArray,
               const ::agiru::Variant &Array,
               ::agiru::Integer Position,
               ::agiru::Integer Length = {});

/// \brief AL `System.CopyStream(OutStream, InStream, Integer)`. Copies the information that is
/// contained in an InStream to an OutStream.
/// \param OutStream The AL `OutStream`.
/// \param InStream The AL `InStream`.
/// \param BytesToRead The AL `Integer`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean CopyStream(const ::agiru::OutStream &OutStream,
                            const ::agiru::InStream &InStream,
                            ::agiru::Integer BytesToRead = {});

/// \brief AL `System.CreateDateTime(Date, Time)`. Creates a DateTime object from a date and a time.
/// \param Date The AL `Date`.
/// \param Time The AL `Time`.
/// \return The AL `DateTime`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::DateTime CreateDateTime(::agiru::Date Date, ::agiru::Time Time);

/// \brief AL `System.CreateEncryptionKey()`. Creates an encryption key for the current tenant.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean CreateEncryptionKey();

/// \brief AL `System.CreateGuid()`. Creates a new unique GUID. The value can then be assigned to a
/// GUID data type or a text data type. Use the text data type if you want to compare the GUID to
/// another text string.
/// \return The AL `Guid`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Guid CreateGuid();

/// \brief AL `System.Date2DMY(Date, Integer)`. Gets the day, month, or year of a Date Data Type.
/// \param Date The AL `Date`.
/// \param Value The AL `Integer`.
/// \return The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Integer Date2DMY(::agiru::Date Date, ::agiru::Integer Value);

/// \brief AL `System.Date2DWY(Date, Integer)`. Gets the day of the week, week number, or year of a
/// Date Data Type.
/// \param Date The AL `Date`.
/// \param Value The AL `Integer`.
/// \return The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Integer Date2DWY(::agiru::Date Date, ::agiru::Integer Value);

/// \brief AL `System.DaTi2Variant(Date, Time)`. Creates a variant that contains an encapsulation of
/// a COM VT\\_DATE.
/// \param Date The AL `Date`.
/// \param Time The AL `Time`.
/// \return The AL `Variant`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Variant DaTi2Variant(::agiru::Date Date, ::agiru::Time Time);

/// \brief AL `System.Decrypt(Text)`. Takes a string as input and returns the decrypted value of the
/// string.
/// \param EncryptedString The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string Decrypt(std::string_view EncryptedString);

/// \brief AL `System.DeleteEncryptionKey()`. Deletes an encryption key for the current tenant.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void DeleteEncryptionKey();

/// \brief AL `System.DMY2Date(Integer, Integer, Integer)`. Gets a Date object based on a day,
/// month, and year.
/// \param Day The AL `Integer`.
/// \param Month The AL `Integer`.
/// \param Year The AL `Integer`.
/// \return The AL `Date`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Date
DMY2Date(::agiru::Integer Day, ::agiru::Integer Month = {}, ::agiru::Integer Year = {});

/// \brief AL `System.DT2Date(DateTime)`. Gets the date part of a DateTime object.
/// \param Datetime The AL `DateTime`.
/// \return The AL `Date`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Date DT2Date(::agiru::DateTime Datetime);

/// \brief AL `System.DT2Time(DateTime)`. Gets the time part of a DateTime object.
/// \param Datetime The AL `DateTime`.
/// \return The AL `Time`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Time DT2Time(::agiru::DateTime Datetime);

/// \brief AL `System.DWY2Date(Integer, Integer, Integer)`. Gets a Date that is based on a week day,
/// a week, and a year.
/// \param WeekDay The AL `Integer`.
/// \param Week The AL `Integer`.
/// \param Year The AL `Integer`.
/// \return The AL `Date`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Date
DWY2Date(::agiru::Integer WeekDay, ::agiru::Integer Week = {}, ::agiru::Integer Year = {});

/// \brief AL `System.Encrypt(Text)`. Takes a string as input and returns the encrypted value of the
/// string.
/// \param PlainTextString The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string Encrypt(std::string_view PlainTextString);

/// \brief AL `System.EncryptionEnabled()`. Checks if the tenant is configured to allow encryption.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean EncryptionEnabled();

/// \brief AL `System.EncryptionKeyExists()`. Checks whether an encryption key for the current
/// tenant is present on the server tenant.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean EncryptionKeyExists();

/// \brief AL `System.Evaluate(Any, Text, Integer)`. Evaluates a string representation of a value
/// into its typical representation. The result is assigned to a variable.
/// \tparam Any1 What AL handed it.
/// \param Variable The AL `Any`.
/// \param String The AL `Text`.
/// \param Number The AL `Integer`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
template <typename Any1>
::agiru::Boolean Evaluate(Any1 &Variable, std::string_view String, ::agiru::Integer Number = {}) {
  static_cast<void>(Variable);
  static_cast<void>(String);
  static_cast<void>(Number);
  RefuseDoor("System.Evaluate(Any, Text, Integer)");
}

/// \brief AL `System.ExportEncryptionKey(Text)`. Returns a password protected temporary filepath
/// containing the encryption key. When encrypting or decrypting data in Dynamics 365 Business
/// Central, an encryption key is used. A single key is used per tenant and every tenant will have a
/// different key. Keys can be exported to a file which may be necessary in the case of upgrading or
/// migrating a system from one set of hardware to another. The EXPORTENCRYPTIONKEY method allows an
/// administrator to specify a destination file for the key and specify a password protection for
/// the file.
/// \param Password The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string ExportEncryptionKey(std::string_view Password);

/// \brief AL `System.ExportObjects(Text, Record, Integer)`. Exports application objects to a file.
/// \param FileName The AL `Text`.
/// \param ObjectRecord The AL `Record`.
/// \param Format The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void ExportObjects(std::string_view FileName,
                   ::agiru::RecordRef &ObjectRecord,
                   ::agiru::Integer Format = {});

/// \brief AL `System.Format(Any, Integer, Integer)`. Formats a value into a string.
/// \param Value The AL `Any`.
/// \param Length The AL `Integer`.
/// \param FormatNumber The AL `Integer`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string Format(const ::agiru::Variant &Value,
                   ::agiru::Integer Length = {},
                   ::agiru::Integer FormatNumber = {});

/// \brief AL `System.Format(Any, Integer, Text)`. Formats a value into a string.
/// \param Value The AL `Any`.
/// \param Length The AL `Integer`.
/// \param FormatString The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string
Format(const ::agiru::Variant &Value, ::agiru::Integer Length, std::string_view FormatString);

/// \brief AL `System.GetCollectedErrors(Boolean)`. Gets all collected errors in the current
/// collection scope.
/// \param Clear The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void GetCollectedErrors(::agiru::Boolean Clear = {});

/// \brief AL `System.GetDocumentUrl(Guid)`. Gets the URL for the specified temporary media object
/// ID.
/// \param ID The AL `Guid`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string GetDocumentUrl(::agiru::Guid ID);

/// \brief AL `System.GetDotNetType(Any)`. Gets the System.Type that corresponds to the given value.
/// \param Expression The AL `Any`.
/// \return The AL `DotNet`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Variant GetDotNetType(const ::agiru::Variant &Expression);

/// \brief AL `System.GetLastErrorCallStack()`. Gets the call stack from where the last error
/// occurred.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string GetLastErrorCallStack();

/// \brief AL `System.GetLastErrorCode()`. Gets the classification of the last error that occurred.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string GetLastErrorCode();

/// \brief AL `System.GetLastErrorObject()`. Gets the last System.Exception object that occurred.
/// \return The AL `DotNet`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Variant GetLastErrorObject();

/// \brief AL `System.GetUrl(ClientType, Text, ObjectType, Integer, RecordRef, Boolean)`. Generates
/// a URL for the specified client target that is based on the configuration of the server instance.
/// If the code runs in a multitenant deployment architecture, the generated URL will automatically
/// apply to the tenant ID of the current user.
/// \param ClientType The AL `ClientType`.
/// \param Company The AL `Text`.
/// \param ObjectType The AL `ObjectType`.
/// \param ObjectId The AL `Integer`.
/// \param RecordRef The AL `RecordRef`.
/// \param UseFilters The AL `Boolean`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string GetUrl(const ::agiru::ClientType &ClientType,
                   std::string_view Company,
                   const ::agiru::ObjectType &ObjectType,
                   ::agiru::Integer ObjectId,
                   const ::agiru::RecordRef &RecordRef,
                   ::agiru::Boolean UseFilters = {});

/// \brief AL `System.GetUrl(ClientType, Text, ObjectType, Integer, RecordRef, Boolean, Text)`.
/// Generates a URL for the specified client target that is based on the configuration of the server
/// instance. If the code runs in a multitenant deployment architecture, the generated URL will
/// automatically apply to the tenant ID of the current user.
/// \param ClientType The AL `ClientType`.
/// \param Company The AL `Text`.
/// \param ObjectType The AL `ObjectType`.
/// \param ObjectId The AL `Integer`.
/// \param RecordRef The AL `RecordRef`.
/// \param UseFilters The AL `Boolean`.
/// \param Layout The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string GetUrl(const ::agiru::ClientType &ClientType,
                   std::string_view Company,
                   const ::agiru::ObjectType &ObjectType,
                   ::agiru::Integer ObjectId,
                   const ::agiru::RecordRef &RecordRef,
                   ::agiru::Boolean UseFilters,
                   std::string_view Layout);

/// \brief AL `System.GlobalLanguage(Integer)`. Gets and sets the current global language setting.
/// \param NewLanguageID The AL `Integer`.
/// \return The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Integer GlobalLanguage(::agiru::Integer NewLanguageID = {});

/// \brief AL `System.GuiAllowed()`. Checks whether the AL code can show any information on the
/// screen.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean GuiAllowed();

/// \brief AL `System.HasCollectedErrors()`. Gets a value indicating whether errors have been
/// collected in the current error collection scope.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean HasCollectedErrors();

/// \brief AL `System.Hyperlink(Text)`. Passes a URL as an argument to an Internet browser, such as
/// Microsoft Edge.
/// \param URL The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void Hyperlink(std::string_view URL);

/// \brief AL `System.ImportEncryptionKey(Text, Text)`. Points to a password protected file that
/// contains the key on the current server. When encrypting or decrypting data in Dynamics 365
/// Business Central, an encryption key is used. A single key is used per tenant, and every tenant
/// will have a different key. Keys can be created or imported if one exists already, as may be the
/// case if upgrading or migrating a system from one set of hardware to another. The
/// IMPORTENCRYPTIONKEY method allows an administrator to specify a file (password protected) which
/// contains a key and imports it to the current Dynamics 365 Business Central service.
/// \param Path The AL `Text`.
/// \param Password The AL `Text`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean ImportEncryptionKey(std::string_view Path, std::string_view Password);

/// \brief AL `System.ImportObjects(Text, Integer)`. Imports application objects from a file.
/// \param FileName The AL `Text`.
/// \param Format The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void ImportObjects(std::string_view FileName, ::agiru::Integer Format = {});

/// \brief AL `System.ImportStreamWithUrlAccess(InStream, Text, Integer)`. Imports an object into a
/// media container to be used in a temporary URL with a default expiration time.
/// \param InStream The AL `InStream`.
/// \param Filename The AL `Text`.
/// \param MinutesToExpire The AL `Integer`.
/// \return The AL `Guid`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Guid ImportStreamWithUrlAccess(const ::agiru::InStream &InStream,
                                        std::string_view Filename,
                                        ::agiru::Integer MinutesToExpire = {});

/// \brief AL `System.IsCollectingErrors()`. Gets a value indicating whether errors are currently
/// being collected.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean IsCollectingErrors();

/// \brief AL `System.IsNull(DotNet)`. Gets a value indicating whether a DotNet object has been
/// created or not.
/// \param DotNet The AL `DotNet`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean IsNull(const ::agiru::Variant &DotNet);

/// \brief AL `System.IsNullGuid(Guid)`. Indicates whether a value has been assigned to a GUID. A
/// null GUID that consists only of zeros is valid but must never be used for references.
/// \param Guid The AL `Guid`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean IsNullGuid(::agiru::Guid Guid);

/// \brief AL `System.IsServiceTier()`. Gets a value indicating whether the runtime is a service
/// tier.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean IsServiceTier();

/// \brief AL `System.NormalDate(Date)`. Gets the regular date (instead of the closing date) for the
/// argument Date.
/// \param Date The AL `Date`.
/// \return The AL `Date`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Date NormalDate(::agiru::Date Date);

/// \brief AL `System.Power(Decimal, Decimal)`. Raises a number to a power. For example, you can use
/// this method to square the number 2 to get the result of 4.
/// \param Number The AL `Decimal`.
/// \param Power The AL `Decimal`.
/// \return The AL `Decimal`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Decimal Power(::agiru::Decimal Number, ::agiru::Decimal Power);

/// \brief AL `System.Random(Integer)`. Returns a pseudo-random number.
/// \param MaxNumber The AL `Integer`.
/// \return The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Integer Random(::agiru::Integer MaxNumber);

/// \brief AL `System.Randomize(Integer)`. Generates a set of random numbers from which the RANDOM
/// method (Integer) will select a random number.
/// \param Seed The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void Randomize(::agiru::Integer Seed = {});

/// \brief AL `System.RoundDateTime(DateTime, BigInteger, Text)`. Rounds a DateTime.
/// \param Datetime The AL `DateTime`.
/// \param Precision The AL `BigInteger`.
/// \param Direction The AL `Text`.
/// \return The AL `DateTime`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::DateTime RoundDateTime(::agiru::DateTime Datetime,
                                ::agiru::BigInteger Precision = {},
                                std::string_view Direction = {});

/// \brief AL `System.Sleep(Integer)`. Returns control to the operating system for a specified time.
/// \param Duration The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void Sleep(::agiru::Integer Duration);

/// \brief AL `System.TemporaryPath()`. Gets the path of the directory where the temporary file is
/// stored.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string TemporaryPath();

/// \brief AL `System.Today()`. Gets the current date set in the operating system.
/// \return The AL `Date`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Date Today();

/// \brief AL `System.Variant2Date(Variant)`. Gets a date from a variant.
/// \param Variant The AL `Variant`.
/// \return The AL `Date`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Date Variant2Date(const ::agiru::Variant &Variant);

/// \brief AL `System.Variant2Time(Variant)`. Gets a time from a variant.
/// \param Variant The AL `Variant`.
/// \return The AL `Time`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Time Variant2Time(const ::agiru::Variant &Variant);

/// \brief AL `System.WindowsLanguage()`. Gets the current Windows language setting.
/// \return The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Integer WindowsLanguage();

/// \brief AL `System.WorkDate(Date)`. Gets and sets the work date for the current session.
/// \param NewDate The AL `Date`.
/// \return The AL `Date`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Date WorkDate(::agiru::Date NewDate = {});

// ---- text ----

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
std::string
CopyStr(std::string_view String, ::agiru::Integer Position, ::agiru::Integer Length = {});

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
/// \param Where The AL `Text`.
/// \param Which The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string
DelChr(std::string_view String, std::string_view Where = {}, std::string_view Which = {});

/// \brief AL `Text.DelStr(Text, Integer, Integer)`. Deletes a substring inside a string (text or
/// code).
/// \param String The AL `Text`.
/// \param Position The AL `Integer`.
/// \param Length The AL `Integer`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string
DelStr(std::string_view String, ::agiru::Integer Position, ::agiru::Integer Length = {});

/// \brief AL `Text.IncStr(Text)`. Increases a positive number or decrease a negative number inside
/// a string by one (1).
/// \param String The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string IncStr(std::string_view String);

/// \brief AL `Text.IncStr(Text, BigInteger)`. Increments the last positive number by the provided
/// increment. The result of the increment must be zero or positive, otherwise an error is thrown.
/// \param String The AL `Text`.
/// \param Increment The AL `BigInteger`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
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
std::string
PadStr(std::string_view String, ::agiru::Integer Length, std::string_view FillCharacter = {});

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
/// \param Modulus The AL `Integer`.
/// \return The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Integer StrCheckSum(std::string_view String,
                             std::string_view WeightString = {},
                             ::agiru::Integer Modulus = {});

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

// ---- database ----

/// \brief AL `Database.AlterKey(KeyRef, Boolean)`. Alter a table's key in SQL, either disabling or
/// enabling it. Any alteration only pertains to the current transaction and will be reverted at the
/// end of the current transaction. Any alteration will fail if it's called on System or non-SQL
/// based tables. Disabling clustered or unique keys is also not supported and will fail at runtime.
/// \param KeyRef The AL `KeyRef`.
/// \param Enable The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void AlterKey(const ::agiru::KeyRef &KeyRef, ::agiru::Boolean Enable);

/// \brief AL `Database.ChangeUserPassword(Text, Text)`. Changes the password for the current user.
/// \param OldPassword The AL `Text`.
/// \param NewPassword The AL `Text`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean ChangeUserPassword(std::string_view OldPassword, std::string_view NewPassword);

/// \brief AL `Database.CheckLicenseFile(Integer)`. Checks a key in the license file of the system.
/// \param KeyNumber The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void CheckLicenseFile(::agiru::Integer KeyNumber);

/// \brief AL `Database.CompanyName()`. Gets the current company name.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string CompanyName();

/// \brief AL `Database.CopyCompany(Text, Text)`. Creates a new company and copies all data from an
/// existing company in the same database.
/// \param SourceName The AL `Text`.
/// \param DestinationName The AL `Text`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean CopyCompany(std::string_view SourceName, std::string_view DestinationName);

/// \brief AL `Database.CurrentTransactionType(TransactionType)`. Gets the current transaction type
/// and sets a new type to be assigned.
/// \param TransactionType The AL `TransactionType`.
/// \return The AL `TransactionType`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::TransactionType CurrentTransactionType(const ::agiru::TransactionType &TransactionType);

/// \brief AL `Database.DataFileInformation(Boolean, Text, Text, Boolean, Boolean, Boolean, Text,
/// DateTime, Record)`. Specifies data from a file that has been exported from a database.
/// \param ShowDialog The AL `Boolean`.
/// \param FileName The AL `Text`.
/// \param Description The AL `Text`.
/// \param HasApplication The AL `Boolean`.
/// \param HasApplicationData The AL `Boolean`.
/// \param HasGlobalData The AL `Boolean`.
/// \param tenantId The AL `Text`.
/// \param exportDate The AL `DateTime`.
/// \param CompanyRecord The AL `Record`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean DataFileInformation(::agiru::Boolean ShowDialog,
                                     std::string &FileName,
                                     std::string &Description,
                                     ::agiru::Boolean &HasApplication,
                                     ::agiru::Boolean &HasApplicationData,
                                     ::agiru::Boolean &HasGlobalData,
                                     std::string &tenantId,
                                     ::agiru::DateTime &exportDate,
                                     ::agiru::RecordRef &CompanyRecord);

/// \brief AL `Database.ExportData(Boolean, Text, Text, Boolean, Boolean, Boolean, Record)`. Exports
/// data from the database to a file. The data is not deleted from the database.
/// \param ShowDialog The AL `Boolean`.
/// \param FileName The AL `Text`.
/// \param Description The AL `Text`.
/// \param IncludeApplication The AL `Boolean`.
/// \param IncludeApplicationData The AL `Boolean`.
/// \param IncludeGlobalData The AL `Boolean`.
/// \param CompanyRecord The AL `Record`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean ExportData(::agiru::Boolean ShowDialog,
                            std::string &FileName,
                            std::string_view Description,
                            ::agiru::Boolean IncludeApplication,
                            ::agiru::Boolean IncludeApplicationData,
                            ::agiru::Boolean IncludeGlobalData,
                            const ::agiru::RecordRef &CompanyRecord);

/// \brief AL `Database.GetDefaultTableConnection(TableConnectionType)`. Gets the default table
/// connection based on the specified connection type. You must already have registered a table
/// connection of this type.
/// \param Type The AL `TableConnectionType`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string GetDefaultTableConnection(const ::agiru::TableConnectionType &Type);

/// \brief AL `Database.HasTableConnection(TableConnectionType, Text)`. Verifies if a connection to
/// an external database exists based on the specified name.
/// \param Type The AL `TableConnectionType`.
/// \param Name The AL `Text`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean HasTableConnection(const ::agiru::TableConnectionType &Type,
                                    std::string_view Name);

/// \brief AL `Database.ImportData(Boolean, Text, Boolean, Boolean, Record)`. Imports data from a
/// file that has been exported from a database.
/// \param ShowDialog The AL `Boolean`.
/// \param FileName The AL `Text`.
/// \param IncludeApplicationData The AL `Boolean`.
/// \param IncludeGlobalData The AL `Boolean`.
/// \param CompanyRecord The AL `Record`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean ImportData(::agiru::Boolean ShowDialog,
                            std::string &FileName,
                            ::agiru::Boolean IncludeApplicationData,
                            ::agiru::Boolean IncludeGlobalData,
                            const ::agiru::RecordRef &CompanyRecord);

/// \brief AL `Database.IsInWriteTransaction()`. Checks whether or not you are in a write
/// transaction.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean IsInWriteTransaction();

/// \brief AL `Database.LastUsedRowVersion()`. Gets the last used RowVersion from the database.
/// \return The AL `BigInteger`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::BigInteger LastUsedRowVersion();

/// \brief AL `Database.LockTimeout(Boolean)`. Determines whether the lock timeout setting is set to
/// On. You can also use this method to override the default setting.
/// \param LockTimeout The AL `Boolean`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean LockTimeout(::agiru::Boolean LockTimeout = {});

/// \brief AL `Database.LockTimeoutDuration(Integer)`. Gets or sets the current lock timeout
/// duration in seconds. Setting a lock timeout of 0 or less disables the lock timeout.
/// \param LockTimeoutDuration The AL `Integer`.
/// \return The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Integer LockTimeoutDuration(::agiru::Integer LockTimeoutDuration = {});

/// \brief AL `Database.MinimumActiveRowVersion()`. Returns the lowest active RowVersion in the
/// database. This is the lowest RowVersion for an uncomitted row, meaning rows with a lower
/// timestamp than this value are guaranteed to be comitted. If there are no active transactions,
/// this value is equal to LastUsedRowVersion + 1.
/// \return The AL `BigInteger`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::BigInteger MinimumActiveRowVersion();

/// \brief AL `Database.RegisterTableConnection(TableConnectionType, Text, Text)`. Registers a table
/// connection to an external database.
/// \param Type The AL `TableConnectionType`.
/// \param Name The AL `Text`.
/// \param Connection The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void RegisterTableConnection(const ::agiru::TableConnectionType &Type,
                             std::string_view Name,
                             std::string_view Connection);

/// \brief AL `Database.SelectLatestVersion()`. Forces the latest version of the database to be
/// used.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void SelectLatestVersion();

/// \brief AL `Database.SelectLatestVersion(Integer)`. Ensures that the table's latest version is
/// used, ignoring any cached values older than the method's call time.
/// \param Table The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void SelectLatestVersion(::agiru::Integer Table);

/// \brief AL `Database.SerialNumber()`. Gets a string that contains the serial number of the
/// license file for your system.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string SerialNumber();

/// \brief AL `Database.ServiceInstanceId()`. Gets the ID of the service instance.
/// \return The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Integer ServiceInstanceId();

/// \brief AL `Database.SessionId()`. Gets the ID of the current session.
/// \return The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Integer SessionId();

/// \brief AL `Database.SetDefaultTableConnection(TableConnectionType, Text, Boolean)`. Establishes
/// a connection to an external database based on a previously registered connection of the
/// specified type.
/// \param Type The AL `TableConnectionType`.
/// \param Name The AL `Text`.
/// \param Scoped The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void SetDefaultTableConnection(const ::agiru::TableConnectionType &Type,
                               std::string_view Name,
                               ::agiru::Boolean Scoped = {});

/// \brief AL `Database.SetUserPassword(Guid, Text)`. Sets a password for the user iwith the given
/// user security ID. If the given password is blank, an empty string will be stored instead of a
/// password hash. This will prevent the user from logging in using a password. Only SUPER can call
/// this method. Passwords cannot be set for the empty GUID or for the default Super ID.
/// \param USID The AL `Guid`.
/// \param Password The AL `Text`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean SetUserPassword(::agiru::Guid USID, std::string_view Password);

/// \brief AL `Database.SID(Text)`. Retrieves the security identifier (SID) of a Windows user
/// account.
/// \param UserAccount The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string SID(std::string_view UserAccount = {});

/// \brief AL `Database.TenantId()`. Gets the ID of the tenant that has started the current session.
/// Use this method when your code must be specific about which tenant database to access in a
/// multitenant deployment. For example, if your code imports data into a cache, you can make a
/// cache tenant-specific by using the tenant ID as a key. Also, if you want to write code that
/// saves documents, you can include the tenant ID in the file name or location, for example. In
/// those cases, you can use the TENANTID method in combination with the COMPANYNAME method to
/// identify the company and the tenant database.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string TenantId();

/// \brief AL `Database.UnregisterTableConnection(TableConnectionType, Text)`. Unregisters a table
/// connection to an external database.
/// \param Type The AL `TableConnectionType`.
/// \param Name The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void UnregisterTableConnection(const ::agiru::TableConnectionType &Type, std::string_view Name);

/// \brief AL `Database.UserId()`. Gets the user name of the user account that is logged on to the
/// current session.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string UserId();

/// \brief AL `Database.UserSecurityId()`. Gets the unique identifier of the user that is logged on
/// to the current session.
/// \return The AL `Guid`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Guid UserSecurityId();

// ---- session ----

/// \brief AL `Session.ApplicationArea(Text)`. Gets or sets the application areas for the current
/// session.
/// \param ApplicationArea The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string ApplicationArea(std::string_view ApplicationArea = {});

/// \brief AL `Session.ApplicationIdentifier()`. Gets the application ID associated with the current
/// thread.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string ApplicationIdentifier();

/// \brief AL `Session.BindSubscription(Codeunit)`. Binds the event subscriber methods in the
/// codeunit to the current codeunit instance for handling the events that they subscribe to. This
/// essentially activates the subscriber functions for the codeunit instance.
/// \param Codeunit The AL `Codeunit`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean BindSubscription(const ::agiru::Variant &Codeunit);

/// \brief AL `Session.CurrentClientType()`. Gets the client type that is running in current
/// session.
/// \return The AL `ClientType`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::ClientType CurrentClientType();

/// \brief AL `Session.CurrentExecutionMode()`. Specifies the mode in which the session is running.
/// \return The AL `ExecutionMode`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::ExecutionMode CurrentExecutionMode();

/// \brief AL `Session.DefaultClientType()`. Gets the default client that is configured for the
/// server instance that is used by the current session.
/// \return The AL `ClientType`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::ClientType DefaultClientType();

/// \brief AL `Session.EnableVerboseTelemetry(Boolean, Duration)`. Temporarily enable verbose
/// telemetry on the current session.
/// \param EnableFullALFunctionTracing The AL `Boolean`.
/// \param Duration The AL `Duration`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void EnableVerboseTelemetry(::agiru::Boolean EnableFullALFunctionTracing,
                            ::agiru::Duration Duration);

/// \brief AL `Session.GetCurrentModuleExecutionContext()`. Gets the current session's execution
/// context for the currently executing module.
/// \return The AL `ExecutionContext`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::ExecutionContext GetCurrentModuleExecutionContext();

/// \brief AL `Session.GetExecutionContext()`. Gets the current session's execution context.
/// \return The AL `ExecutionContext`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::ExecutionContext GetExecutionContext();

/// \brief AL `Session.GetModuleExecutionContext(Guid)`. Gets the current session's execution
/// context scoped to a specific module.
/// \param AppId The AL `Guid`.
/// \return The AL `ExecutionContext`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::ExecutionContext GetModuleExecutionContext(::agiru::Guid AppId = {});

/// \brief AL `Session.IsSessionActive(Integer)`. Tests if the specified SessionID is active on the
/// server instance where it was started.
/// \param SessionID The AL `Integer`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean IsSessionActive(::agiru::Integer SessionID);

/// \brief AL `Session.LogAuditMessage(Text, SecurityOperationResult, AuditCategory, Integer,
/// Integer, Dictionary of [Text, Text])`. Logs a message to an audit account. Note, these logs are
/// accessible to customers and will also be logged to a security audit account.
/// \param SecurityAuditDescription The AL `Text`.
/// \param SecurityAuditOperationResult The AL `SecurityOperationResult`.
/// \param SecurityAuditCategory The AL `AuditCategory`.
/// \param AuditMessageOperation The AL `Integer`.
/// \param AuditMessageOperationResult The AL `Integer`.
/// \param CustomDimensions The AL `Dictionary of [Text, Text]`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void LogAuditMessage(std::string_view SecurityAuditDescription,
                     const ::agiru::SecurityOperationResult &SecurityAuditOperationResult,
                     const ::agiru::AuditCategory &SecurityAuditCategory,
                     ::agiru::Integer AuditMessageOperation,
                     ::agiru::Integer AuditMessageOperationResult,
                     const ::agiru::Dictionary<std::string, std::string> &CustomDimensions);

/// \brief AL `Session.LogMessage(Text, Text, Verbosity, DataClassification, TelemetryScope,
/// Dictionary of [Text, Text])`. Logs a trace message to a telemetry account.
/// \param EventId The AL `Text`.
/// \param Message The AL `Text`.
/// \param Verbosity The AL `Verbosity`.
/// \param DataClassification The AL `DataClassification`.
/// \param TelemetryScope The AL `TelemetryScope`.
/// \param CustomDimensions The AL `Dictionary of [Text, Text]`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void LogMessage(std::string_view EventId,
                std::string_view Message,
                const ::agiru::Verbosity &Verbosity,
                const ::agiru::DataClassification &DataClassification,
                const ::agiru::TelemetryScope &TelemetryScope,
                const ::agiru::Dictionary<std::string, std::string> &CustomDimensions);

/// \brief AL `Session.LogMessage(Text, Text, Verbosity, DataClassification, TelemetryScope, Text,
/// Text, Text, Text)`. Logs a trace message to a telemetry account.
/// \param EventId The AL `Text`.
/// \param Message The AL `Text`.
/// \param Verbosity The AL `Verbosity`.
/// \param DataClassification The AL `DataClassification`.
/// \param TelemetryScope The AL `TelemetryScope`.
/// \param Dimension1 The AL `Text`.
/// \param Value1 The AL `Text`.
/// \param Dimension2 The AL `Text`.
/// \param Value2 The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void LogMessage(std::string_view EventId,
                std::string_view Message,
                const ::agiru::Verbosity &Verbosity,
                const ::agiru::DataClassification &DataClassification,
                const ::agiru::TelemetryScope &TelemetryScope,
                std::string_view Dimension1,
                std::string_view Value1,
                std::string_view Dimension2,
                std::string_view Value2);

/// \brief AL `Session.LogSecurityAudit(Text, SecurityOperationResult, Text, AuditCategory, Array of
/// [Text], Array of [Text])`. Logs an IfX audit message to a telemetry account.
/// \param Description The AL `Text`.
/// \param Result The AL `SecurityOperationResult`.
/// \param ResultDescription The AL `Text`.
/// \param AuditCategory The AL `AuditCategory`.
/// \param TargetType The AL `Array of [Text]`.
/// \param TargetName The AL `Array of [Text]`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void LogSecurityAudit(std::string_view Description,
                      const ::agiru::SecurityOperationResult &Result,
                      std::string_view ResultDescription,
                      const ::agiru::AuditCategory &AuditCategory,
                      const ::agiru::Variant &TargetType,
                      const ::agiru::Variant &TargetName);

/// \brief AL `Session.SendTraceTag(Text, Text, Verbosity, Text, DataClassification)`. Send a trace
/// tag to the telemetry service.
/// \param Tag The AL `Text`.
/// \param Category The AL `Text`.
/// \param Verbosity The AL `Verbosity`.
/// \param Message The AL `Text`.
/// \param DataClassification The AL `DataClassification`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void SendTraceTag(std::string_view Tag,
                  std::string_view Category,
                  const ::agiru::Verbosity &Verbosity,
                  std::string_view Message,
                  const ::agiru::DataClassification &DataClassification);

/// \brief AL `Session.SetDocumentServiceToken(Text)`. Sets the document service token in the
/// current session.
/// \param Token The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void SetDocumentServiceToken(std::string_view Token);

/// \brief AL `Session.StartSession(Integer, Integer, Duration, Text, Record)`. Starts a session
/// without a UI and runs the specified codeunit.
/// \param SessionId The AL `Integer`.
/// \param CodeunitId The AL `Integer`.
/// \param Timeout The AL `Duration`.
/// \param Company The AL `Text`.
/// \param Record The AL `Record`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean StartSession(::agiru::Integer &SessionId,
                              ::agiru::Integer CodeunitId,
                              ::agiru::Duration Timeout,
                              std::string_view Company,
                              ::agiru::RecordRef &Record);

/// \brief AL `Session.StartSession(Integer, Integer, Text, Record, Duration)`. Starts a session
/// without a UI and runs the specified codeunit.
/// \param SessionId The AL `Integer`.
/// \param CodeunitId The AL `Integer`.
/// \param Company The AL `Text`.
/// \param Record The AL `Record`.
/// \param Timeout The AL `Duration`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean StartSession(::agiru::Integer &SessionId,
                              ::agiru::Integer CodeunitId,
                              std::string_view Company,
                              ::agiru::RecordRef &Record,
                              ::agiru::Duration Timeout);

/// \brief AL `Session.StartSession(Integer, Integer, Text, Record)`. Starts a session without a UI
/// and runs the specified codeunit.
/// \param SessionId The AL `Integer`.
/// \param CodeunitId The AL `Integer`.
/// \param Company The AL `Text`.
/// \param Record The AL `Record`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean StartSession(::agiru::Integer &SessionId,
                              ::agiru::Integer CodeunitId,
                              std::string_view Company,
                              ::agiru::RecordRef &Record);

/// \brief AL `Session.StopSession(Integer, Text)`. Stops a session.
/// \param SessionId The AL `Integer`.
/// \param Comment The AL `Text`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean StopSession(::agiru::Integer SessionId, std::string_view Comment = {});

/// \brief AL `Session.UnbindSubscription(Codeunit)`. Unbinds the event subscriber methods from in
/// the codeunit instance. This essentially deactivates the subscriber methods for the codeunit
/// instance.
/// \param Codeunit The AL `Codeunit`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean UnbindSubscription(const ::agiru::Variant &Codeunit);

// ---- dialog ----

/// \brief AL `Dialog.Confirm(Text, Boolean, Any)`. Creates a dialog box that prompts the user for a
/// yes or no answer. The dialog box is centered on the screen.
/// \param String The AL `Text`.
/// \param Default The AL `Boolean`.
/// \param Value1 The AL `Any`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean
Confirm(std::string_view String, ::agiru::Boolean Default, const ::agiru::Variant &Value1);

/// \brief AL `Dialog.LogInternalError(Text, DataClassification, Verbosity)`. Log internal errors
/// for telemetry.
/// \param Message The AL `Text`.
/// \param DataClassificationInstance The AL `DataClassification`.
/// \param VerbosityInstance The AL `Verbosity`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void LogInternalError(std::string_view Message,
                      const ::agiru::DataClassification &DataClassificationInstance,
                      const ::agiru::Verbosity &VerbosityInstance);

/// \brief AL `Dialog.LogInternalError(Text, Text, DataClassification, Verbosity)`. Log internal
/// errors for telemetry.
/// \param Message The AL `Text`.
/// \param SubstitutionString The AL `Text`.
/// \param DataClassificationInstance The AL `DataClassification`.
/// \param VerbosityInstance The AL `Verbosity`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void LogInternalError(std::string_view Message,
                      std::string_view SubstitutionString,
                      const ::agiru::DataClassification &DataClassificationInstance,
                      const ::agiru::Verbosity &VerbosityInstance);

/// \brief AL `Dialog.Message(Text, Any)`. Displays a text string in a message window.
/// \param String The AL `Text`.
/// \param Value The AL `Any`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void Message(std::string_view String, const ::agiru::Variant &Value);

/// \brief AL `Dialog.StrMenu(Text, Integer, Text)`. Creates a menu window that displays a series of
/// options.
/// \param OptionMembers The AL `Text`.
/// \param DefaultNumber The AL `Integer`.
/// \param Instruction The AL `Text`.
/// \return The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Integer StrMenu(std::string_view OptionMembers,
                         ::agiru::Integer DefaultNumber = {},
                         std::string_view Instruction = {});

// ---- file ----

/// \brief AL `File.Copy(Text, Text)`. Copies a file.
/// \param FromName The AL `Text`.
/// \param ToName The AL `Text`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean Copy(std::string_view FromName, std::string_view ToName);

/// \brief AL `File.Download(Text, Text, Text, Text, Text)`. Sends a file from a server computer to
/// the client computer. The client computer is the computer that is running the Windows client or
/// the computer that is running a browser that accesses the web client.
/// \param FromFile The AL `Text`.
/// \param DialogTitle The AL `Text`.
/// \param ToFolder The AL `Text`.
/// \param ToFilter The AL `Text`.
/// \param ToFile The AL `Text`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean Download(std::string_view FromFile,
                          std::string_view DialogTitle,
                          std::string_view ToFolder,
                          std::string_view ToFilter,
                          std::string &ToFile);

/// \brief AL `File.DownloadFromStream(InStream, Text, Text, Text, Text)`. Sends a file from server
/// computer to the client computer. The client computer is the computer that is running the Windows
/// client or the computer that is running the browser that accesses the web client.
/// \param InStream The AL `InStream`.
/// \param DialogTitle The AL `Text`.
/// \param ToFolder The AL `Text`.
/// \param ToFilter The AL `Text`.
/// \param ToFile The AL `Text`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean DownloadFromStream(const ::agiru::InStream &InStream,
                                    std::string_view DialogTitle,
                                    std::string_view ToFolder,
                                    std::string_view ToFilter,
                                    std::string &ToFile);

/// \brief AL `File.Erase(Text)`. Deletes a file.
/// \param Name The AL `Text`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean Erase(std::string_view Name);

/// \brief AL `File.Exists(Text)`. Determines whether a file exists.
/// \param Name The AL `Text`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean Exists(std::string_view Name);

/// \brief AL `File.GetStamp(Text, Date, Time)`. Gets the exact time that a file was last written
/// to.
/// \param Name The AL `Text`.
/// \param Date The AL `Date`.
/// \param Time The AL `Time`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean GetStamp(std::string_view Name, ::agiru::Date &Date, ::agiru::Time &Time);

/// \brief AL `File.IsPathTemporary(Text)`. Validates whether the given path is located in the
/// current users temporary folder within the current service.
/// \param Name The AL `Text`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean IsPathTemporary(std::string_view Name);

/// \brief AL `File.Rename(Text, Text)`. Renames an ASCII or binary file.
/// \param OldName The AL `Text`.
/// \param NewName The AL `Text`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean Rename(std::string_view OldName, std::string_view NewName);

/// \brief AL `File.SetStamp(Text, Date, Time)`. Sets a timestamp for a file.
/// \param Name The AL `Text`.
/// \param Date The AL `Date`.
/// \param Time The AL `Time`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean SetStamp(std::string_view Name, ::agiru::Date Date, ::agiru::Time Time = {});

/// \brief AL `File.Upload(Text, Text, Text, Text, Text)`. Sends a file from the client computer to
/// the server computer. The client computer is the computer that is running the Windows client or
/// the computer that is running a browser that accesses the web client.
/// \param DialogTitle The AL `Text`.
/// \param FromFolder The AL `Text`.
/// \param FromFilter The AL `Text`.
/// \param FromFile The AL `Text`.
/// \param ToFile The AL `Text`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean Upload(std::string_view DialogTitle,
                        std::string_view FromFolder,
                        std::string_view FromFilter,
                        std::string_view FromFile,
                        std::string &ToFile);

/// \brief AL `File.UploadIntoStream(Text, InStream)`. Sends a file from the client computer to the
/// corresponding server. The client computer is the computer that is running a browser that
/// accesses the web client.
/// \param FromFilter The AL `Text`.
/// \param InStream The AL `InStream`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean UploadIntoStream(std::string_view FromFilter, ::agiru::InStream &InStream);

/// \brief AL `File.UploadIntoStream(Text, Text, Text, Text, InStream)`. Sends a file from the
/// client computer to the corresponding server. The client computer is the computer that is running
/// the Windows client or the computer that is running a browser that accesses the web client.
/// \param DialogTitle The AL `Text`.
/// \param FromFolder The AL `Text`.
/// \param FromFilter The AL `Text`.
/// \param FromFile The AL `Text`.
/// \param InStream The AL `InStream`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean UploadIntoStream(std::string_view DialogTitle,
                                  std::string_view FromFolder,
                                  std::string_view FromFilter,
                                  std::string &FromFile,
                                  ::agiru::InStream &InStream);

/// \brief AL `File.View(Text, Boolean)`. Opens a file from server computer on the client computer
/// in preview mode. The client computer is the computer that is running the browser that accesses
/// the web client.
/// \param FromFile The AL `Text`.
/// \param AllowDownloadAndPrint The AL `Boolean`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean View(std::string_view FromFile, ::agiru::Boolean AllowDownloadAndPrint = {});

/// \brief AL `File.ViewFromStream(InStream, Text, Boolean)`. Opens a file from the server on the
/// client computer in preview mode. The client computer is defined as the machine running the
/// browser accessing the web client.
/// \param InStream The AL `InStream`.
/// \param FileName The AL `Text`.
/// \param AllowDownloadAndPrint The AL `Boolean`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean ViewFromStream(const ::agiru::InStream &InStream,
                                std::string_view FileName,
                                ::agiru::Boolean AllowDownloadAndPrint = {});

// ---- secrettext ----

/// \brief AL `SecretText.SecretStrSubstNo(Text, SecretText)`. Replaces %1, %2, %3... and #1, #2,
/// #3... fields in a string with the values you provide as optional parameters.
/// \param String The AL `Text`.
/// \param Value1 The AL `SecretText`.
/// \return The AL `SecretText`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::SecretText SecretStrSubstNo(std::string_view String, const ::agiru::SecretText &Value1);

} // namespace agiru
