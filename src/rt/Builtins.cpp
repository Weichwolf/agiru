#include "Builtins.h"

#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "type/AuditCategory.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/ClientType.h"
#include "type/DataClassification.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Dictionary.h"
#include "type/Duration.h"
#include "type/ExecutionContext.h"
#include "type/ExecutionMode.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/KeyRef.h"
#include "type/ObjectType.h"
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

namespace agiru {

[[noreturn]] void RefuseDoor(std::string_view what) {
  throw Error(std::string(what) + " is declared and not implemented yet (board:0035)");
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters,performance-unnecessary-value-param)

::agiru::Decimal Abs(::agiru::Decimal Number) {
  static_cast<void>(Number);
  RefuseDoor("System.Abs(Decimal)");
}

std::string ApplicationPath() {
  RefuseDoor("System.ApplicationPath()");
}

::agiru::Integer ArrayLen(const ::agiru::Variant &Array, ::agiru::Integer Dimension) {
  static_cast<void>(Array);
  static_cast<void>(Dimension);
  RefuseDoor("System.ArrayLen(Array of [Any], Integer)");
}

::agiru::Date CalcDate(::agiru::DateFormula DateExpression, ::agiru::Date Date) {
  static_cast<void>(DateExpression);
  static_cast<void>(Date);
  RefuseDoor("System.CalcDate(DateFormula, Date)");
}

::agiru::Date CalcDate(std::string_view DateExpression, ::agiru::Date Date) {
  static_cast<void>(DateExpression);
  static_cast<void>(Date);
  RefuseDoor("System.CalcDate(Text, Date)");
}

::agiru::Boolean CanLoadType(const ::agiru::Variant &DotNet) {
  static_cast<void>(DotNet);
  RefuseDoor("System.CanLoadType(DotNet)");
}

std::string CaptionClassTranslate(std::string_view CaptionClassText) {
  static_cast<void>(CaptionClassText);
  RefuseDoor("System.CaptionClassTranslate(Text)");
}

void Clear(::agiru::SecretText &Variable) {
  static_cast<void>(Variable);
  RefuseDoor("System.Clear(SecretText)");
}

void ClearAll() {
  RefuseDoor("System.ClearAll()");
}

void ClearCollectedErrors() {
  RefuseDoor("System.ClearCollectedErrors()");
}

::agiru::Date ClosingDate(::agiru::Date Date) {
  static_cast<void>(Date);
  RefuseDoor("System.ClosingDate(Date)");
}

void CodeCoverageInclude(::agiru::RecordRef &ObjectRecord) {
  static_cast<void>(ObjectRecord);
  RefuseDoor("System.CodeCoverageInclude(Record)");
}

void CodeCoverageLoad() {
  RefuseDoor("System.CodeCoverageLoad()");
}

::agiru::Boolean CodeCoverageLog(::agiru::Boolean NewIsActive, ::agiru::Boolean MultiSession) {
  static_cast<void>(NewIsActive);
  static_cast<void>(MultiSession);
  RefuseDoor("System.CodeCoverageLog(Boolean, Boolean)");
}

void CodeCoverageRefresh() {
  RefuseDoor("System.CodeCoverageRefresh()");
}

::agiru::Integer CompressArray(const ::agiru::Variant &StringArray) {
  static_cast<void>(StringArray);
  RefuseDoor("System.CompressArray(Array of [Text])");
}

void CopyArray(const ::agiru::Variant &NewArray,
               const ::agiru::Variant &Array,
               ::agiru::Integer Position,
               ::agiru::Integer Length) {
  static_cast<void>(NewArray);
  static_cast<void>(Array);
  static_cast<void>(Position);
  static_cast<void>(Length);
  RefuseDoor("System.CopyArray(Array of [Any], Array of [Any], Integer, Integer)");
}

::agiru::Boolean CopyStream(const ::agiru::OutStream &OutStream,
                            const ::agiru::InStream &InStream,
                            ::agiru::Integer BytesToRead) {
  static_cast<void>(OutStream);
  static_cast<void>(InStream);
  static_cast<void>(BytesToRead);
  RefuseDoor("System.CopyStream(OutStream, InStream, Integer)");
}

::agiru::DateTime CreateDateTime(::agiru::Date Date, ::agiru::Time Time) {
  static_cast<void>(Date);
  static_cast<void>(Time);
  RefuseDoor("System.CreateDateTime(Date, Time)");
}

::agiru::Boolean CreateEncryptionKey() {
  RefuseDoor("System.CreateEncryptionKey()");
}

::agiru::Guid CreateGuid() {
  RefuseDoor("System.CreateGuid()");
}

::agiru::Integer Date2DMY(::agiru::Date Date, ::agiru::Integer Value) {
  static_cast<void>(Date);
  static_cast<void>(Value);
  RefuseDoor("System.Date2DMY(Date, Integer)");
}

::agiru::Integer Date2DWY(::agiru::Date Date, ::agiru::Integer Value) {
  static_cast<void>(Date);
  static_cast<void>(Value);
  RefuseDoor("System.Date2DWY(Date, Integer)");
}

::agiru::Variant DaTi2Variant(::agiru::Date Date, ::agiru::Time Time) {
  static_cast<void>(Date);
  static_cast<void>(Time);
  RefuseDoor("System.DaTi2Variant(Date, Time)");
}

std::string Decrypt(std::string_view EncryptedString) {
  static_cast<void>(EncryptedString);
  RefuseDoor("System.Decrypt(Text)");
}

void DeleteEncryptionKey() {
  RefuseDoor("System.DeleteEncryptionKey()");
}

::agiru::Date DMY2Date(::agiru::Integer Day, ::agiru::Integer Month, ::agiru::Integer Year) {
  static_cast<void>(Day);
  static_cast<void>(Month);
  static_cast<void>(Year);
  RefuseDoor("System.DMY2Date(Integer, Integer, Integer)");
}

::agiru::Date DT2Date(::agiru::DateTime Datetime) {
  static_cast<void>(Datetime);
  RefuseDoor("System.DT2Date(DateTime)");
}

::agiru::Time DT2Time(::agiru::DateTime Datetime) {
  static_cast<void>(Datetime);
  RefuseDoor("System.DT2Time(DateTime)");
}

::agiru::Date DWY2Date(::agiru::Integer WeekDay, ::agiru::Integer Week, ::agiru::Integer Year) {
  static_cast<void>(WeekDay);
  static_cast<void>(Week);
  static_cast<void>(Year);
  RefuseDoor("System.DWY2Date(Integer, Integer, Integer)");
}

std::string Encrypt(std::string_view PlainTextString) {
  static_cast<void>(PlainTextString);
  RefuseDoor("System.Encrypt(Text)");
}

::agiru::Boolean EncryptionEnabled() {
  RefuseDoor("System.EncryptionEnabled()");
}

::agiru::Boolean EncryptionKeyExists() {
  RefuseDoor("System.EncryptionKeyExists()");
}

std::string ExportEncryptionKey(std::string_view Password) {
  static_cast<void>(Password);
  RefuseDoor("System.ExportEncryptionKey(Text)");
}

void ExportObjects(std::string_view FileName,
                   ::agiru::RecordRef &ObjectRecord,
                   ::agiru::Integer Format) {
  static_cast<void>(FileName);
  static_cast<void>(ObjectRecord);
  static_cast<void>(Format);
  RefuseDoor("System.ExportObjects(Text, Record, Integer)");
}

std::string
Format(const ::agiru::Variant &Value, ::agiru::Integer Length, ::agiru::Integer FormatNumber) {
  static_cast<void>(Value);
  static_cast<void>(Length);
  static_cast<void>(FormatNumber);
  RefuseDoor("System.Format(Any, Integer, Integer)");
}

std::string
Format(const ::agiru::Variant &Value, ::agiru::Integer Length, std::string_view FormatString) {
  static_cast<void>(Value);
  static_cast<void>(Length);
  static_cast<void>(FormatString);
  RefuseDoor("System.Format(Any, Integer, Text)");
}

void GetCollectedErrors(::agiru::Boolean Clear) {
  static_cast<void>(Clear);
  RefuseDoor("System.GetCollectedErrors(Boolean)");
}

std::string GetDocumentUrl(::agiru::Guid ID) {
  static_cast<void>(ID);
  RefuseDoor("System.GetDocumentUrl(Guid)");
}

::agiru::Variant GetDotNetType(const ::agiru::Variant &Expression) {
  static_cast<void>(Expression);
  RefuseDoor("System.GetDotNetType(Any)");
}

std::string GetLastErrorCallStack() {
  RefuseDoor("System.GetLastErrorCallStack()");
}

std::string GetLastErrorCode() {
  RefuseDoor("System.GetLastErrorCode()");
}

::agiru::Variant GetLastErrorObject() {
  RefuseDoor("System.GetLastErrorObject()");
}

std::string GetUrl(const ::agiru::ClientType &ClientType,
                   std::string_view Company,
                   const ::agiru::ObjectType &ObjectType,
                   ::agiru::Integer ObjectId,
                   const ::agiru::RecordRef &RecordRef,
                   ::agiru::Boolean UseFilters) {
  static_cast<void>(ClientType);
  static_cast<void>(Company);
  static_cast<void>(ObjectType);
  static_cast<void>(ObjectId);
  static_cast<void>(RecordRef);
  static_cast<void>(UseFilters);
  RefuseDoor("System.GetUrl(ClientType, Text, ObjectType, Integer, RecordRef, Boolean)");
}

std::string GetUrl(const ::agiru::ClientType &ClientType,
                   std::string_view Company,
                   const ::agiru::ObjectType &ObjectType,
                   ::agiru::Integer ObjectId,
                   const ::agiru::RecordRef &RecordRef,
                   ::agiru::Boolean UseFilters,
                   std::string_view Layout) {
  static_cast<void>(ClientType);
  static_cast<void>(Company);
  static_cast<void>(ObjectType);
  static_cast<void>(ObjectId);
  static_cast<void>(RecordRef);
  static_cast<void>(UseFilters);
  static_cast<void>(Layout);
  RefuseDoor("System.GetUrl(ClientType, Text, ObjectType, Integer, RecordRef, Boolean, Text)");
}

::agiru::Integer GlobalLanguage(::agiru::Integer NewLanguageID) {
  static_cast<void>(NewLanguageID);
  RefuseDoor("System.GlobalLanguage(Integer)");
}

::agiru::Boolean GuiAllowed() {
  RefuseDoor("System.GuiAllowed()");
}

::agiru::Boolean HasCollectedErrors() {
  RefuseDoor("System.HasCollectedErrors()");
}

void Hyperlink(std::string_view URL) {
  static_cast<void>(URL);
  RefuseDoor("System.Hyperlink(Text)");
}

::agiru::Boolean ImportEncryptionKey(std::string_view Path, std::string_view Password) {
  static_cast<void>(Path);
  static_cast<void>(Password);
  RefuseDoor("System.ImportEncryptionKey(Text, Text)");
}

void ImportObjects(std::string_view FileName, ::agiru::Integer Format) {
  static_cast<void>(FileName);
  static_cast<void>(Format);
  RefuseDoor("System.ImportObjects(Text, Integer)");
}

::agiru::Guid ImportStreamWithUrlAccess(const ::agiru::InStream &InStream,
                                        std::string_view Filename,
                                        ::agiru::Integer MinutesToExpire) {
  static_cast<void>(InStream);
  static_cast<void>(Filename);
  static_cast<void>(MinutesToExpire);
  RefuseDoor("System.ImportStreamWithUrlAccess(InStream, Text, Integer)");
}

::agiru::Boolean IsCollectingErrors() {
  RefuseDoor("System.IsCollectingErrors()");
}

::agiru::Boolean IsNull(const ::agiru::Variant &DotNet) {
  static_cast<void>(DotNet);
  RefuseDoor("System.IsNull(DotNet)");
}

::agiru::Boolean IsNullGuid(::agiru::Guid Guid) {
  static_cast<void>(Guid);
  RefuseDoor("System.IsNullGuid(Guid)");
}

::agiru::Boolean IsServiceTier() {
  RefuseDoor("System.IsServiceTier()");
}

::agiru::Date NormalDate(::agiru::Date Date) {
  static_cast<void>(Date);
  RefuseDoor("System.NormalDate(Date)");
}

::agiru::Decimal Power(::agiru::Decimal Number, ::agiru::Decimal Power) {
  static_cast<void>(Number);
  static_cast<void>(Power);
  RefuseDoor("System.Power(Decimal, Decimal)");
}

::agiru::Integer Random(::agiru::Integer MaxNumber) {
  static_cast<void>(MaxNumber);
  RefuseDoor("System.Random(Integer)");
}

void Randomize(::agiru::Integer Seed) {
  static_cast<void>(Seed);
  RefuseDoor("System.Randomize(Integer)");
}

::agiru::DateTime RoundDateTime(::agiru::DateTime Datetime,
                                ::agiru::BigInteger Precision,
                                std::string_view Direction) {
  static_cast<void>(Datetime);
  static_cast<void>(Precision);
  static_cast<void>(Direction);
  RefuseDoor("System.RoundDateTime(DateTime, BigInteger, Text)");
}

void Sleep(::agiru::Integer Duration) {
  static_cast<void>(Duration);
  RefuseDoor("System.Sleep(Integer)");
}

std::string TemporaryPath() {
  RefuseDoor("System.TemporaryPath()");
}

::agiru::Date Today() {
  RefuseDoor("System.Today()");
}

::agiru::Date Variant2Date(const ::agiru::Variant &Variant) {
  static_cast<void>(Variant);
  RefuseDoor("System.Variant2Date(Variant)");
}

::agiru::Time Variant2Time(const ::agiru::Variant &Variant) {
  static_cast<void>(Variant);
  RefuseDoor("System.Variant2Time(Variant)");
}

::agiru::Integer WindowsLanguage() {
  RefuseDoor("System.WindowsLanguage()");
}

::agiru::Date WorkDate(::agiru::Date NewDate) {
  static_cast<void>(NewDate);
  RefuseDoor("System.WorkDate(Date)");
}

std::string ConvertStr(std::string_view String,
                       std::string_view FromCharacters,
                       std::string_view ToCharacters) {
  static_cast<void>(String);
  static_cast<void>(FromCharacters);
  static_cast<void>(ToCharacters);
  RefuseDoor("Text.ConvertStr(Text, Text, Text)");
}

std::string CopyStr(std::string_view String, ::agiru::Integer Position, ::agiru::Integer Length) {
  static_cast<void>(String);
  static_cast<void>(Position);
  static_cast<void>(Length);
  RefuseDoor("Text.CopyStr(Text, Integer, Integer)");
}

std::string DelChr(std::string_view String, std::string_view Where, std::string_view Which) {
  static_cast<void>(String);
  static_cast<void>(Where);
  static_cast<void>(Which);
  RefuseDoor("Text.DelChr(Text, Text, Text)");
}

std::string DelStr(std::string_view String, ::agiru::Integer Position, ::agiru::Integer Length) {
  static_cast<void>(String);
  static_cast<void>(Position);
  static_cast<void>(Length);
  RefuseDoor("Text.DelStr(Text, Integer, Integer)");
}

std::string IncStr(std::string_view String) {
  static_cast<void>(String);
  RefuseDoor("Text.IncStr(Text)");
}

std::string IncStr(std::string_view String, ::agiru::BigInteger Increment) {
  static_cast<void>(String);
  static_cast<void>(Increment);
  RefuseDoor("Text.IncStr(Text, BigInteger)");
}

std::string InsStr(std::string_view String, std::string_view SubString, ::agiru::Integer Position) {
  static_cast<void>(String);
  static_cast<void>(SubString);
  static_cast<void>(Position);
  RefuseDoor("Text.InsStr(Text, Text, Integer)");
}

std::string LowerCase(std::string_view String) {
  static_cast<void>(String);
  RefuseDoor("Text.LowerCase(Text)");
}

std::string
PadStr(std::string_view String, ::agiru::Integer Length, std::string_view FillCharacter) {
  static_cast<void>(String);
  static_cast<void>(Length);
  static_cast<void>(FillCharacter);
  RefuseDoor("Text.PadStr(Text, Integer, Text)");
}

std::string SelectStr(::agiru::Integer Number, std::string_view CommaString) {
  static_cast<void>(Number);
  static_cast<void>(CommaString);
  RefuseDoor("Text.SelectStr(Integer, Text)");
}

::agiru::Integer
StrCheckSum(std::string_view String, std::string_view WeightString, ::agiru::Integer Modulus) {
  static_cast<void>(String);
  static_cast<void>(WeightString);
  static_cast<void>(Modulus);
  RefuseDoor("Text.StrCheckSum(Text, Text, Integer)");
}

::agiru::Integer StrPos(std::string_view String, std::string_view SubString) {
  static_cast<void>(String);
  static_cast<void>(SubString);
  RefuseDoor("Text.StrPos(Text, Text)");
}

std::string UpperCase(std::string_view String) {
  static_cast<void>(String);
  RefuseDoor("Text.UpperCase(Text)");
}

void AlterKey(const ::agiru::KeyRef &KeyRef, ::agiru::Boolean Enable) {
  static_cast<void>(KeyRef);
  static_cast<void>(Enable);
  RefuseDoor("Database.AlterKey(KeyRef, Boolean)");
}

::agiru::Boolean ChangeUserPassword(std::string_view OldPassword, std::string_view NewPassword) {
  static_cast<void>(OldPassword);
  static_cast<void>(NewPassword);
  RefuseDoor("Database.ChangeUserPassword(Text, Text)");
}

void CheckLicenseFile(::agiru::Integer KeyNumber) {
  static_cast<void>(KeyNumber);
  RefuseDoor("Database.CheckLicenseFile(Integer)");
}

std::string CompanyName() {
  RefuseDoor("Database.CompanyName()");
}

::agiru::Boolean CopyCompany(std::string_view SourceName, std::string_view DestinationName) {
  static_cast<void>(SourceName);
  static_cast<void>(DestinationName);
  RefuseDoor("Database.CopyCompany(Text, Text)");
}

::agiru::TransactionType CurrentTransactionType(const ::agiru::TransactionType &TransactionType) {
  static_cast<void>(TransactionType);
  RefuseDoor("Database.CurrentTransactionType(TransactionType)");
}

::agiru::Boolean DataFileInformation(::agiru::Boolean ShowDialog,
                                     std::string &FileName,
                                     std::string &Description,
                                     ::agiru::Boolean &HasApplication,
                                     ::agiru::Boolean &HasApplicationData,
                                     ::agiru::Boolean &HasGlobalData,
                                     std::string &tenantId,
                                     ::agiru::DateTime &exportDate,
                                     ::agiru::RecordRef &CompanyRecord) {
  static_cast<void>(ShowDialog);
  static_cast<void>(FileName);
  static_cast<void>(Description);
  static_cast<void>(HasApplication);
  static_cast<void>(HasApplicationData);
  static_cast<void>(HasGlobalData);
  static_cast<void>(tenantId);
  static_cast<void>(exportDate);
  static_cast<void>(CompanyRecord);
  RefuseDoor("Database.DataFileInformation(Boolean, Text, Text, Boolean, Boolean, Boolean, Text, "
             "DateTime, Record)");
}

::agiru::Boolean ExportData(::agiru::Boolean ShowDialog,
                            std::string &FileName,
                            std::string_view Description,
                            ::agiru::Boolean IncludeApplication,
                            ::agiru::Boolean IncludeApplicationData,
                            ::agiru::Boolean IncludeGlobalData,
                            const ::agiru::RecordRef &CompanyRecord) {
  static_cast<void>(ShowDialog);
  static_cast<void>(FileName);
  static_cast<void>(Description);
  static_cast<void>(IncludeApplication);
  static_cast<void>(IncludeApplicationData);
  static_cast<void>(IncludeGlobalData);
  static_cast<void>(CompanyRecord);
  RefuseDoor("Database.ExportData(Boolean, Text, Text, Boolean, Boolean, Boolean, Record)");
}

std::string GetDefaultTableConnection(const ::agiru::TableConnectionType &Type) {
  static_cast<void>(Type);
  RefuseDoor("Database.GetDefaultTableConnection(TableConnectionType)");
}

::agiru::Boolean HasTableConnection(const ::agiru::TableConnectionType &Type,
                                    std::string_view Name) {
  static_cast<void>(Type);
  static_cast<void>(Name);
  RefuseDoor("Database.HasTableConnection(TableConnectionType, Text)");
}

::agiru::Boolean ImportData(::agiru::Boolean ShowDialog,
                            std::string &FileName,
                            ::agiru::Boolean IncludeApplicationData,
                            ::agiru::Boolean IncludeGlobalData,
                            const ::agiru::RecordRef &CompanyRecord) {
  static_cast<void>(ShowDialog);
  static_cast<void>(FileName);
  static_cast<void>(IncludeApplicationData);
  static_cast<void>(IncludeGlobalData);
  static_cast<void>(CompanyRecord);
  RefuseDoor("Database.ImportData(Boolean, Text, Boolean, Boolean, Record)");
}

::agiru::Boolean IsInWriteTransaction() {
  RefuseDoor("Database.IsInWriteTransaction()");
}

::agiru::BigInteger LastUsedRowVersion() {
  RefuseDoor("Database.LastUsedRowVersion()");
}

::agiru::Boolean LockTimeout(::agiru::Boolean LockTimeout) {
  static_cast<void>(LockTimeout);
  RefuseDoor("Database.LockTimeout(Boolean)");
}

::agiru::Integer LockTimeoutDuration(::agiru::Integer LockTimeoutDuration) {
  static_cast<void>(LockTimeoutDuration);
  RefuseDoor("Database.LockTimeoutDuration(Integer)");
}

::agiru::BigInteger MinimumActiveRowVersion() {
  RefuseDoor("Database.MinimumActiveRowVersion()");
}

void RegisterTableConnection(const ::agiru::TableConnectionType &Type,
                             std::string_view Name,
                             std::string_view Connection) {
  static_cast<void>(Type);
  static_cast<void>(Name);
  static_cast<void>(Connection);
  RefuseDoor("Database.RegisterTableConnection(TableConnectionType, Text, Text)");
}

void SelectLatestVersion() {
  RefuseDoor("Database.SelectLatestVersion()");
}

void SelectLatestVersion(::agiru::Integer Table) {
  static_cast<void>(Table);
  RefuseDoor("Database.SelectLatestVersion(Integer)");
}

std::string SerialNumber() {
  RefuseDoor("Database.SerialNumber()");
}

::agiru::Integer ServiceInstanceId() {
  RefuseDoor("Database.ServiceInstanceId()");
}

::agiru::Integer SessionId() {
  RefuseDoor("Database.SessionId()");
}

void SetDefaultTableConnection(const ::agiru::TableConnectionType &Type,
                               std::string_view Name,
                               ::agiru::Boolean Scoped) {
  static_cast<void>(Type);
  static_cast<void>(Name);
  static_cast<void>(Scoped);
  RefuseDoor("Database.SetDefaultTableConnection(TableConnectionType, Text, Boolean)");
}

::agiru::Boolean SetUserPassword(::agiru::Guid USID, std::string_view Password) {
  static_cast<void>(USID);
  static_cast<void>(Password);
  RefuseDoor("Database.SetUserPassword(Guid, Text)");
}

std::string SID(std::string_view UserAccount) {
  static_cast<void>(UserAccount);
  RefuseDoor("Database.SID(Text)");
}

std::string TenantId() {
  RefuseDoor("Database.TenantId()");
}

void UnregisterTableConnection(const ::agiru::TableConnectionType &Type, std::string_view Name) {
  static_cast<void>(Type);
  static_cast<void>(Name);
  RefuseDoor("Database.UnregisterTableConnection(TableConnectionType, Text)");
}

std::string UserId() {
  RefuseDoor("Database.UserId()");
}

::agiru::Guid UserSecurityId() {
  RefuseDoor("Database.UserSecurityId()");
}

std::string ApplicationArea(std::string_view ApplicationArea) {
  static_cast<void>(ApplicationArea);
  RefuseDoor("Session.ApplicationArea(Text)");
}

std::string ApplicationIdentifier() {
  RefuseDoor("Session.ApplicationIdentifier()");
}

::agiru::Boolean BindSubscription(const ::agiru::Variant &Codeunit) {
  static_cast<void>(Codeunit);
  RefuseDoor("Session.BindSubscription(Codeunit)");
}

::agiru::ClientType CurrentClientType() {
  RefuseDoor("Session.CurrentClientType()");
}

::agiru::ExecutionMode CurrentExecutionMode() {
  RefuseDoor("Session.CurrentExecutionMode()");
}

::agiru::ClientType DefaultClientType() {
  RefuseDoor("Session.DefaultClientType()");
}

void EnableVerboseTelemetry(::agiru::Boolean EnableFullALFunctionTracing,
                            ::agiru::Duration Duration) {
  static_cast<void>(EnableFullALFunctionTracing);
  static_cast<void>(Duration);
  RefuseDoor("Session.EnableVerboseTelemetry(Boolean, Duration)");
}

::agiru::ExecutionContext GetCurrentModuleExecutionContext() {
  RefuseDoor("Session.GetCurrentModuleExecutionContext()");
}

::agiru::ExecutionContext GetExecutionContext() {
  RefuseDoor("Session.GetExecutionContext()");
}

::agiru::ExecutionContext GetModuleExecutionContext(::agiru::Guid AppId) {
  static_cast<void>(AppId);
  RefuseDoor("Session.GetModuleExecutionContext(Guid)");
}

::agiru::Boolean IsSessionActive(::agiru::Integer SessionID) {
  static_cast<void>(SessionID);
  RefuseDoor("Session.IsSessionActive(Integer)");
}

void LogAuditMessage(std::string_view SecurityAuditDescription,
                     const ::agiru::SecurityOperationResult &SecurityAuditOperationResult,
                     const ::agiru::AuditCategory &SecurityAuditCategory,
                     ::agiru::Integer AuditMessageOperation,
                     ::agiru::Integer AuditMessageOperationResult,
                     const ::agiru::Dictionary<std::string, std::string> &CustomDimensions) {
  static_cast<void>(SecurityAuditDescription);
  static_cast<void>(SecurityAuditOperationResult);
  static_cast<void>(SecurityAuditCategory);
  static_cast<void>(AuditMessageOperation);
  static_cast<void>(AuditMessageOperationResult);
  static_cast<void>(CustomDimensions);
  RefuseDoor("Session.LogAuditMessage(Text, SecurityOperationResult, AuditCategory, Integer, "
             "Integer, Dictionary of [Text, Text])");
}

void LogMessage(std::string_view EventId,
                std::string_view Message,
                const ::agiru::Verbosity &Verbosity,
                const ::agiru::DataClassification &DataClassification,
                const ::agiru::TelemetryScope &TelemetryScope,
                const ::agiru::Dictionary<std::string, std::string> &CustomDimensions) {
  static_cast<void>(EventId);
  static_cast<void>(Message);
  static_cast<void>(Verbosity);
  static_cast<void>(DataClassification);
  static_cast<void>(TelemetryScope);
  static_cast<void>(CustomDimensions);
  RefuseDoor("Session.LogMessage(Text, Text, Verbosity, DataClassification, TelemetryScope, "
             "Dictionary of [Text, Text])");
}

void LogMessage(std::string_view EventId,
                std::string_view Message,
                const ::agiru::Verbosity &Verbosity,
                const ::agiru::DataClassification &DataClassification,
                const ::agiru::TelemetryScope &TelemetryScope,
                std::string_view Dimension1,
                std::string_view Value1,
                std::string_view Dimension2,
                std::string_view Value2) {
  static_cast<void>(EventId);
  static_cast<void>(Message);
  static_cast<void>(Verbosity);
  static_cast<void>(DataClassification);
  static_cast<void>(TelemetryScope);
  static_cast<void>(Dimension1);
  static_cast<void>(Value1);
  static_cast<void>(Dimension2);
  static_cast<void>(Value2);
  RefuseDoor("Session.LogMessage(Text, Text, Verbosity, DataClassification, TelemetryScope, Text, "
             "Text, Text, Text)");
}

void LogSecurityAudit(std::string_view Description,
                      const ::agiru::SecurityOperationResult &Result,
                      std::string_view ResultDescription,
                      const ::agiru::AuditCategory &AuditCategory,
                      const ::agiru::Variant &TargetType,
                      const ::agiru::Variant &TargetName) {
  static_cast<void>(Description);
  static_cast<void>(Result);
  static_cast<void>(ResultDescription);
  static_cast<void>(AuditCategory);
  static_cast<void>(TargetType);
  static_cast<void>(TargetName);
  RefuseDoor("Session.LogSecurityAudit(Text, SecurityOperationResult, Text, AuditCategory, Array "
             "of [Text], Array of [Text])");
}

void SendTraceTag(std::string_view Tag,
                  std::string_view Category,
                  const ::agiru::Verbosity &Verbosity,
                  std::string_view Message,
                  const ::agiru::DataClassification &DataClassification) {
  static_cast<void>(Tag);
  static_cast<void>(Category);
  static_cast<void>(Verbosity);
  static_cast<void>(Message);
  static_cast<void>(DataClassification);
  RefuseDoor("Session.SendTraceTag(Text, Text, Verbosity, Text, DataClassification)");
}

void SetDocumentServiceToken(std::string_view Token) {
  static_cast<void>(Token);
  RefuseDoor("Session.SetDocumentServiceToken(Text)");
}

::agiru::Boolean StartSession(::agiru::Integer &SessionId,
                              ::agiru::Integer CodeunitId,
                              ::agiru::Duration Timeout,
                              std::string_view Company,
                              ::agiru::RecordRef &Record) {
  static_cast<void>(SessionId);
  static_cast<void>(CodeunitId);
  static_cast<void>(Timeout);
  static_cast<void>(Company);
  static_cast<void>(Record);
  RefuseDoor("Session.StartSession(Integer, Integer, Duration, Text, Record)");
}

::agiru::Boolean StartSession(::agiru::Integer &SessionId,
                              ::agiru::Integer CodeunitId,
                              std::string_view Company,
                              ::agiru::RecordRef &Record,
                              ::agiru::Duration Timeout) {
  static_cast<void>(SessionId);
  static_cast<void>(CodeunitId);
  static_cast<void>(Company);
  static_cast<void>(Record);
  static_cast<void>(Timeout);
  RefuseDoor("Session.StartSession(Integer, Integer, Text, Record, Duration)");
}

::agiru::Boolean StartSession(::agiru::Integer &SessionId,
                              ::agiru::Integer CodeunitId,
                              std::string_view Company,
                              ::agiru::RecordRef &Record) {
  static_cast<void>(SessionId);
  static_cast<void>(CodeunitId);
  static_cast<void>(Company);
  static_cast<void>(Record);
  RefuseDoor("Session.StartSession(Integer, Integer, Text, Record)");
}

::agiru::Boolean StopSession(::agiru::Integer SessionId, std::string_view Comment) {
  static_cast<void>(SessionId);
  static_cast<void>(Comment);
  RefuseDoor("Session.StopSession(Integer, Text)");
}

::agiru::Boolean UnbindSubscription(const ::agiru::Variant &Codeunit) {
  static_cast<void>(Codeunit);
  RefuseDoor("Session.UnbindSubscription(Codeunit)");
}

::agiru::Boolean
Confirm(std::string_view String, ::agiru::Boolean Default, const ::agiru::Variant &Value1) {
  static_cast<void>(String);
  static_cast<void>(Default);
  static_cast<void>(Value1);
  RefuseDoor("Dialog.Confirm(Text, Boolean, Any)");
}

void LogInternalError(std::string_view Message,
                      const ::agiru::DataClassification &DataClassificationInstance,
                      const ::agiru::Verbosity &VerbosityInstance) {
  static_cast<void>(Message);
  static_cast<void>(DataClassificationInstance);
  static_cast<void>(VerbosityInstance);
  RefuseDoor("Dialog.LogInternalError(Text, DataClassification, Verbosity)");
}

void LogInternalError(std::string_view Message,
                      std::string_view SubstitutionString,
                      const ::agiru::DataClassification &DataClassificationInstance,
                      const ::agiru::Verbosity &VerbosityInstance) {
  static_cast<void>(Message);
  static_cast<void>(SubstitutionString);
  static_cast<void>(DataClassificationInstance);
  static_cast<void>(VerbosityInstance);
  RefuseDoor("Dialog.LogInternalError(Text, Text, DataClassification, Verbosity)");
}

void Message(std::string_view String, const ::agiru::Variant &Value) {
  static_cast<void>(String);
  static_cast<void>(Value);
  RefuseDoor("Dialog.Message(Text, Any)");
}

::agiru::Integer StrMenu(std::string_view OptionMembers,
                         ::agiru::Integer DefaultNumber,
                         std::string_view Instruction) {
  static_cast<void>(OptionMembers);
  static_cast<void>(DefaultNumber);
  static_cast<void>(Instruction);
  RefuseDoor("Dialog.StrMenu(Text, Integer, Text)");
}

::agiru::Boolean Copy(std::string_view FromName, std::string_view ToName) {
  static_cast<void>(FromName);
  static_cast<void>(ToName);
  RefuseDoor("File.Copy(Text, Text)");
}

::agiru::Boolean Download(std::string_view FromFile,
                          std::string_view DialogTitle,
                          std::string_view ToFolder,
                          std::string_view ToFilter,
                          std::string &ToFile) {
  static_cast<void>(FromFile);
  static_cast<void>(DialogTitle);
  static_cast<void>(ToFolder);
  static_cast<void>(ToFilter);
  static_cast<void>(ToFile);
  RefuseDoor("File.Download(Text, Text, Text, Text, Text)");
}

::agiru::Boolean DownloadFromStream(const ::agiru::InStream &InStream,
                                    std::string_view DialogTitle,
                                    std::string_view ToFolder,
                                    std::string_view ToFilter,
                                    std::string &ToFile) {
  static_cast<void>(InStream);
  static_cast<void>(DialogTitle);
  static_cast<void>(ToFolder);
  static_cast<void>(ToFilter);
  static_cast<void>(ToFile);
  RefuseDoor("File.DownloadFromStream(InStream, Text, Text, Text, Text)");
}

::agiru::Boolean Erase(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("File.Erase(Text)");
}

::agiru::Boolean Exists(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("File.Exists(Text)");
}

::agiru::Boolean GetStamp(std::string_view Name, ::agiru::Date &Date, ::agiru::Time &Time) {
  static_cast<void>(Name);
  static_cast<void>(Date);
  static_cast<void>(Time);
  RefuseDoor("File.GetStamp(Text, Date, Time)");
}

::agiru::Boolean IsPathTemporary(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("File.IsPathTemporary(Text)");
}

::agiru::Boolean Rename(std::string_view OldName, std::string_view NewName) {
  static_cast<void>(OldName);
  static_cast<void>(NewName);
  RefuseDoor("File.Rename(Text, Text)");
}

::agiru::Boolean SetStamp(std::string_view Name, ::agiru::Date Date, ::agiru::Time Time) {
  static_cast<void>(Name);
  static_cast<void>(Date);
  static_cast<void>(Time);
  RefuseDoor("File.SetStamp(Text, Date, Time)");
}

::agiru::Boolean Upload(std::string_view DialogTitle,
                        std::string_view FromFolder,
                        std::string_view FromFilter,
                        std::string_view FromFile,
                        std::string &ToFile) {
  static_cast<void>(DialogTitle);
  static_cast<void>(FromFolder);
  static_cast<void>(FromFilter);
  static_cast<void>(FromFile);
  static_cast<void>(ToFile);
  RefuseDoor("File.Upload(Text, Text, Text, Text, Text)");
}

::agiru::Boolean UploadIntoStream(std::string_view FromFilter, ::agiru::InStream &InStream) {
  static_cast<void>(FromFilter);
  static_cast<void>(InStream);
  RefuseDoor("File.UploadIntoStream(Text, InStream)");
}

::agiru::Boolean UploadIntoStream(std::string_view DialogTitle,
                                  std::string_view FromFolder,
                                  std::string_view FromFilter,
                                  std::string &FromFile,
                                  ::agiru::InStream &InStream) {
  static_cast<void>(DialogTitle);
  static_cast<void>(FromFolder);
  static_cast<void>(FromFilter);
  static_cast<void>(FromFile);
  static_cast<void>(InStream);
  RefuseDoor("File.UploadIntoStream(Text, Text, Text, Text, InStream)");
}

::agiru::Boolean View(std::string_view FromFile, ::agiru::Boolean AllowDownloadAndPrint) {
  static_cast<void>(FromFile);
  static_cast<void>(AllowDownloadAndPrint);
  RefuseDoor("File.View(Text, Boolean)");
}

::agiru::Boolean ViewFromStream(const ::agiru::InStream &InStream,
                                std::string_view FileName,
                                ::agiru::Boolean AllowDownloadAndPrint) {
  static_cast<void>(InStream);
  static_cast<void>(FileName);
  static_cast<void>(AllowDownloadAndPrint);
  RefuseDoor("File.ViewFromStream(InStream, Text, Boolean)");
}

::agiru::SecretText SecretStrSubstNo(std::string_view String, const ::agiru::SecretText &Value1) {
  static_cast<void>(String);
  static_cast<void>(Value1);
  RefuseDoor("SecretText.SecretStrSubstNo(Text, SecretText)");
}

// NOLINTEND(bugprone-easily-swappable-parameters,performance-unnecessary-value-param)

} // namespace agiru
