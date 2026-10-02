#pragma once

#include "runtime/RecordRef.h"
#include "type/AuditCategory.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/ClientType.h"
#include "type/DataClassification.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Duration.h"
#include "type/ExecutionContext.h"
#include "type/ExecutionMode.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/KeyRef.h"
#include "type/SecurityOperationResult.h"
#include "type/Stream.h"
#include "type/TableConnectionType.h"
#include "type/Time.h"
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

[[noreturn]] void RefuseDoor(std::string_view what);

/// \brief AL `System.ApplicationPath()`. Returns the path of the directory where the executable
/// file for the product is installed.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string ApplicationPath();

/// \brief AL `System.CaptionClassTranslate(Text)`. Returns a translated version of the caption
/// string. The string is translated to the current local language.
/// \param CaptionClassText The AL `Text`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string CaptionClassTranslate(std::string_view CaptionClassText);

/// \brief AL `System.ClearAll()`. Clears all internal variables (except REC variables), keys, and
/// filters in the object and in any associated objects, such as reports, pages, codeunits, and so
/// on that contain AL code.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void ClearAll();

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

/// \brief AL `System.CreateEncryptionKey()`. Creates an encryption key for the current tenant.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean CreateEncryptionKey();

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

/// \brief AL `System.GetLastErrorObject()`. Gets the last System.Exception object that occurred.
/// \return The AL `DotNet`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Variant GetLastErrorObject();

/// \brief AL `System.GetLastErrorText(Boolean)`. Gets the last error that occurred in the debugger.
/// \param ExcludeCustomerContent The AL `Boolean`.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string GetLastErrorText(::agiru::Boolean ExcludeCustomerContent);

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

/// \brief AL `System.IsServiceTier()`. Gets a value indicating whether the runtime is a service
/// tier.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean IsServiceTier();

/// \brief AL `System.Sleep(Integer)`. Returns control to the operating system for a specified time.
/// \param Duration The AL `Integer`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void Sleep(::agiru::Integer Duration);

/// \brief AL `System.TemporaryPath()`. Gets the path of the directory where the temporary file is
/// stored.
/// \return The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
std::string TemporaryPath();

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

/// \brief AL `Database.CopyCompany(Text, Text)`. Creates a new company and copies all data from an
/// existing company in the same database.
/// \param SourceName The AL `Text`.
/// \param DestinationName The AL `Text`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean CopyCompany(std::string_view SourceName, std::string_view DestinationName);

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
                                     ::agiru::Text<0> &FileName,
                                     ::agiru::Text<0> &Description,
                                     ::agiru::Boolean &HasApplication,
                                     ::agiru::Boolean &HasApplicationData,
                                     ::agiru::Boolean &HasGlobalData,
                                     ::agiru::Text<0> &tenantId,
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
                            ::agiru::Text<0> &FileName,
                            std::string_view Description = {},
                            ::agiru::Boolean IncludeApplication = {},
                            ::agiru::Boolean IncludeApplicationData = {},
                            ::agiru::Boolean IncludeGlobalData = {},
                            const ::agiru::RecordRef &CompanyRecord = {});

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
                            ::agiru::Text<0> &FileName,
                            ::agiru::Boolean IncludeApplicationData = {},
                            ::agiru::Boolean IncludeGlobalData = {},
                            const ::agiru::RecordRef &CompanyRecord = {});

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

/// \brief AL `Session.GetModuleExecutionContext(Guid)`. Gets the current session's execution
/// context scoped to a specific module.
/// \param AppId The AL `Guid`.
/// \return The AL `ExecutionContext`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::ExecutionContext GetModuleExecutionContext(::agiru::Guid AppId = {});

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
                      const ::agiru::Variant &TargetType = {},
                      const ::agiru::Variant &TargetName = {});

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
                  const ::agiru::DataClassification &DataClassification = {});

/// \brief AL `Session.SetDocumentServiceToken(Text)`. Sets the document service token in the
/// current session.
/// \param Token The AL `Text`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
void SetDocumentServiceToken(std::string_view Token);

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
                                    ::agiru::Text<0> &ToFile);

/// \brief AL `File.GetStamp(Text, Date, Time)`. Gets the exact time that a file was last written
/// to.
/// \param Name The AL `Text`.
/// \param Date The AL `Date`.
/// \param Time The AL `Time`.
/// \return The AL `Boolean`.
/// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
::agiru::Boolean GetStamp(std::string_view Name, ::agiru::Date &Date, ::agiru::Time &Time);

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
                        ::agiru::Text<0> &ToFile);

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
                                  ::agiru::Text<0> &FromFile,
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
}
