#include "Builtins.h"

#include "runtime/ErrorValue.h"
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
#include "type/StringValue.h"
#include "type/TableConnectionType.h"
#include "type/Time.h"
#include "type/Variant.h"
#include "type/Verbosity.h"

#include <string>
#include <string_view>

namespace agiru {

[[noreturn]] void RefuseUnimplemented(std::string_view what) {
  throw Error(std::string(what) + " is declared and not implemented yet (board:0035)");
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters,performance-unnecessary-value-param)

std::string ApplicationPath() {
  RefuseUnimplemented("System.ApplicationPath()");
}

std::string CaptionClassTranslate(std::string_view CaptionClassText) {
  static_cast<void>(CaptionClassText);
  RefuseUnimplemented("System.CaptionClassTranslate(Text)");
}

void ClearAll() {
  RefuseUnimplemented("System.ClearAll()");
}

void CodeCoverageLoad() {
  RefuseUnimplemented("System.CodeCoverageLoad()");
}

::agiru::Boolean CodeCoverageLog(::agiru::Boolean NewIsActive, ::agiru::Boolean MultiSession) {
  static_cast<void>(NewIsActive);
  static_cast<void>(MultiSession);
  RefuseUnimplemented("System.CodeCoverageLog(Boolean, Boolean)");
}

void CodeCoverageRefresh() {
  RefuseUnimplemented("System.CodeCoverageRefresh()");
}

::agiru::Integer CompressArray(const ::agiru::Variant &StringArray) {
  static_cast<void>(StringArray);
  RefuseUnimplemented("System.CompressArray(Array of [Text])");
}

::agiru::Boolean CreateEncryptionKey() {
  RefuseUnimplemented("System.CreateEncryptionKey()");
}

::agiru::Variant DaTi2Variant(::agiru::Date Date, ::agiru::Time Time) {
  static_cast<void>(Date);
  static_cast<void>(Time);
  RefuseUnimplemented("System.DaTi2Variant(Date, Time)");
}

std::string Decrypt(std::string_view EncryptedString) {
  static_cast<void>(EncryptedString);
  RefuseUnimplemented("System.Decrypt(Text)");
}

void DeleteEncryptionKey() {
  RefuseUnimplemented("System.DeleteEncryptionKey()");
}

::agiru::Date DWY2Date(::agiru::Integer WeekDay, ::agiru::Integer Week, ::agiru::Integer Year) {
  static_cast<void>(WeekDay);
  static_cast<void>(Week);
  static_cast<void>(Year);
  RefuseUnimplemented("System.DWY2Date(Integer, Integer, Integer)");
}

std::string Encrypt(std::string_view PlainTextString) {
  static_cast<void>(PlainTextString);
  RefuseUnimplemented("System.Encrypt(Text)");
}

::agiru::Boolean EncryptionKeyExists() {
  RefuseUnimplemented("System.EncryptionKeyExists()");
}

std::string ExportEncryptionKey(std::string_view Password) {
  static_cast<void>(Password);
  RefuseUnimplemented("System.ExportEncryptionKey(Text)");
}

void ExportObjects(std::string_view FileName,
                   ::agiru::RecordRef &ObjectRecord,
                   ::agiru::Integer Format) {
  static_cast<void>(FileName);
  static_cast<void>(ObjectRecord);
  static_cast<void>(Format);
  RefuseUnimplemented("System.ExportObjects(Text, Record, Integer)");
}

std::string GetDocumentUrl(::agiru::Guid ID) {
  static_cast<void>(ID);
  RefuseUnimplemented("System.GetDocumentUrl(Guid)");
}

::agiru::Variant GetDotNetType(const ::agiru::Variant &Expression) {
  static_cast<void>(Expression);
  RefuseUnimplemented("System.GetDotNetType(Any)");
}

std::string GetLastErrorCallStack() {
  RefuseUnimplemented("System.GetLastErrorCallStack()");
}

::agiru::Variant GetLastErrorObject() {
  RefuseUnimplemented("System.GetLastErrorObject()");
}

std::string GetLastErrorText(::agiru::Boolean ExcludeCustomerContent) {
  static_cast<void>(ExcludeCustomerContent);
  RefuseUnimplemented("System.GetLastErrorText(Boolean)");
}

::agiru::Boolean ImportEncryptionKey(std::string_view Path, std::string_view Password) {
  static_cast<void>(Path);
  static_cast<void>(Password);
  RefuseUnimplemented("System.ImportEncryptionKey(Text, Text)");
}

void ImportObjects(std::string_view FileName, ::agiru::Integer Format) {
  static_cast<void>(FileName);
  static_cast<void>(Format);
  RefuseUnimplemented("System.ImportObjects(Text, Integer)");
}

::agiru::Guid ImportStreamWithUrlAccess(const ::agiru::InStream &InStream,
                                        std::string_view Filename,
                                        ::agiru::Integer MinutesToExpire) {
  static_cast<void>(InStream);
  static_cast<void>(Filename);
  static_cast<void>(MinutesToExpire);
  RefuseUnimplemented("System.ImportStreamWithUrlAccess(InStream, Text, Integer)");
}

::agiru::Boolean IsCollectingErrors() {
  RefuseUnimplemented("System.IsCollectingErrors()");
}

::agiru::Boolean IsServiceTier() {
  RefuseUnimplemented("System.IsServiceTier()");
}

void Sleep(::agiru::Integer Duration) {
  static_cast<void>(Duration);
  RefuseUnimplemented("System.Sleep(Integer)");
}

std::string TemporaryPath() {
  RefuseUnimplemented("System.TemporaryPath()");
}

::agiru::Date Variant2Date(const ::agiru::Variant &Variant) {
  static_cast<void>(Variant);
  RefuseUnimplemented("System.Variant2Date(Variant)");
}

::agiru::Time Variant2Time(const ::agiru::Variant &Variant) {
  static_cast<void>(Variant);
  RefuseUnimplemented("System.Variant2Time(Variant)");
}

void AlterKey(const ::agiru::KeyRef &KeyRef, ::agiru::Boolean Enable) {
  static_cast<void>(KeyRef);
  static_cast<void>(Enable);
  RefuseUnimplemented("Database.AlterKey(KeyRef, Boolean)");
}

::agiru::Boolean ChangeUserPassword(std::string_view OldPassword, std::string_view NewPassword) {
  static_cast<void>(OldPassword);
  static_cast<void>(NewPassword);
  RefuseUnimplemented("Database.ChangeUserPassword(Text, Text)");
}

void CheckLicenseFile(::agiru::Integer KeyNumber) {
  static_cast<void>(KeyNumber);
  RefuseUnimplemented("Database.CheckLicenseFile(Integer)");
}

::agiru::Boolean CopyCompany(std::string_view SourceName, std::string_view DestinationName) {
  static_cast<void>(SourceName);
  static_cast<void>(DestinationName);
  RefuseUnimplemented("Database.CopyCompany(Text, Text)");
}

::agiru::Boolean DataFileInformation(::agiru::Boolean ShowDialog,
                                     ::agiru::Text<0> &FileName,
                                     ::agiru::Text<0> &Description,
                                     ::agiru::Boolean &HasApplication,
                                     ::agiru::Boolean &HasApplicationData,
                                     ::agiru::Boolean &HasGlobalData,
                                     ::agiru::Text<0> &tenantId,
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
  RefuseUnimplemented("Database.DataFileInformation(Boolean, Text, Text, Boolean, Boolean, "
                      "Boolean, Text, DateTime, Record)");
}

::agiru::Boolean ExportData(::agiru::Boolean ShowDialog,
                            ::agiru::Text<0> &FileName,
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
  RefuseUnimplemented(
      "Database.ExportData(Boolean, Text, Text, Boolean, Boolean, Boolean, Record)");
}

std::string GetDefaultTableConnection(const ::agiru::TableConnectionType &Type) {
  static_cast<void>(Type);
  RefuseUnimplemented("Database.GetDefaultTableConnection(TableConnectionType)");
}

::agiru::Boolean HasTableConnection(const ::agiru::TableConnectionType &Type,
                                    std::string_view Name) {
  static_cast<void>(Type);
  static_cast<void>(Name);
  RefuseUnimplemented("Database.HasTableConnection(TableConnectionType, Text)");
}

::agiru::Boolean ImportData(::agiru::Boolean ShowDialog,
                            ::agiru::Text<0> &FileName,
                            ::agiru::Boolean IncludeApplicationData,
                            ::agiru::Boolean IncludeGlobalData,
                            const ::agiru::RecordRef &CompanyRecord) {
  static_cast<void>(ShowDialog);
  static_cast<void>(FileName);
  static_cast<void>(IncludeApplicationData);
  static_cast<void>(IncludeGlobalData);
  static_cast<void>(CompanyRecord);
  RefuseUnimplemented("Database.ImportData(Boolean, Text, Boolean, Boolean, Record)");
}

::agiru::BigInteger LastUsedRowVersion() {
  RefuseUnimplemented("Database.LastUsedRowVersion()");
}

::agiru::Boolean LockTimeout(::agiru::Boolean LockTimeout) {
  static_cast<void>(LockTimeout);
  RefuseUnimplemented("Database.LockTimeout(Boolean)");
}

::agiru::Integer LockTimeoutDuration(::agiru::Integer LockTimeoutDuration) {
  static_cast<void>(LockTimeoutDuration);
  RefuseUnimplemented("Database.LockTimeoutDuration(Integer)");
}

::agiru::BigInteger MinimumActiveRowVersion() {
  RefuseUnimplemented("Database.MinimumActiveRowVersion()");
}

void RegisterTableConnection(const ::agiru::TableConnectionType &Type,
                             std::string_view Name,
                             std::string_view Connection) {
  static_cast<void>(Type);
  static_cast<void>(Name);
  static_cast<void>(Connection);
  RefuseUnimplemented("Database.RegisterTableConnection(TableConnectionType, Text, Text)");
}

void SelectLatestVersion() {
  RefuseUnimplemented("Database.SelectLatestVersion()");
}

void SelectLatestVersion(::agiru::Integer Table) {
  static_cast<void>(Table);
  RefuseUnimplemented("Database.SelectLatestVersion(Integer)");
}

std::string SerialNumber() {
  RefuseUnimplemented("Database.SerialNumber()");
}

::agiru::Boolean SetUserPassword(::agiru::Guid USID, std::string_view Password) {
  static_cast<void>(USID);
  static_cast<void>(Password);
  RefuseUnimplemented("Database.SetUserPassword(Guid, Text)");
}

std::string SID(std::string_view UserAccount) {
  static_cast<void>(UserAccount);
  RefuseUnimplemented("Database.SID(Text)");
}

std::string TenantId() {
  RefuseUnimplemented("Database.TenantId()");
}

void UnregisterTableConnection(const ::agiru::TableConnectionType &Type, std::string_view Name) {
  static_cast<void>(Type);
  static_cast<void>(Name);
  RefuseUnimplemented("Database.UnregisterTableConnection(TableConnectionType, Text)");
}

std::string ApplicationIdentifier() {
  RefuseUnimplemented("Session.ApplicationIdentifier()");
}

::agiru::Boolean BindSubscription(const ::agiru::Variant &Codeunit) {
  static_cast<void>(Codeunit);
  RefuseUnimplemented("Session.BindSubscription(Codeunit)");
}

::agiru::ExecutionMode CurrentExecutionMode() {
  RefuseUnimplemented("Session.CurrentExecutionMode()");
}

::agiru::ClientType DefaultClientType() {
  RefuseUnimplemented("Session.DefaultClientType()");
}

void EnableVerboseTelemetry(::agiru::Boolean EnableFullALFunctionTracing,
                            ::agiru::Duration Duration) {
  static_cast<void>(EnableFullALFunctionTracing);
  static_cast<void>(Duration);
  RefuseUnimplemented("Session.EnableVerboseTelemetry(Boolean, Duration)");
}

::agiru::ExecutionContext GetCurrentModuleExecutionContext() {
  RefuseUnimplemented("Session.GetCurrentModuleExecutionContext()");
}

::agiru::ExecutionContext GetModuleExecutionContext(::agiru::Guid AppId) {
  static_cast<void>(AppId);
  RefuseUnimplemented("Session.GetModuleExecutionContext(Guid)");
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
  RefuseUnimplemented("Session.LogSecurityAudit(Text, SecurityOperationResult, Text, "
                      "AuditCategory, Array of [Text], Array of [Text])");
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
  RefuseUnimplemented("Session.SendTraceTag(Text, Text, Verbosity, Text, DataClassification)");
}

void SetDocumentServiceToken(std::string_view Token) {
  static_cast<void>(Token);
  RefuseUnimplemented("Session.SetDocumentServiceToken(Text)");
}

::agiru::Boolean StopSession(::agiru::Integer SessionId, std::string_view Comment) {
  static_cast<void>(SessionId);
  static_cast<void>(Comment);
  RefuseUnimplemented("Session.StopSession(Integer, Text)");
}

::agiru::Boolean UnbindSubscription(const ::agiru::Variant &Codeunit) {
  static_cast<void>(Codeunit);
  RefuseUnimplemented("Session.UnbindSubscription(Codeunit)");
}

void LogInternalError(std::string_view Message,
                      const ::agiru::DataClassification &DataClassificationInstance,
                      const ::agiru::Verbosity &VerbosityInstance) {
  static_cast<void>(Message);
  static_cast<void>(DataClassificationInstance);
  static_cast<void>(VerbosityInstance);
  RefuseUnimplemented("Dialog.LogInternalError(Text, DataClassification, Verbosity)");
}

void LogInternalError(std::string_view Message,
                      std::string_view SubstitutionString,
                      const ::agiru::DataClassification &DataClassificationInstance,
                      const ::agiru::Verbosity &VerbosityInstance) {
  static_cast<void>(Message);
  static_cast<void>(SubstitutionString);
  static_cast<void>(DataClassificationInstance);
  static_cast<void>(VerbosityInstance);
  RefuseUnimplemented("Dialog.LogInternalError(Text, Text, DataClassification, Verbosity)");
}

::agiru::Boolean DownloadFromStream(const ::agiru::InStream &InStream,
                                    std::string_view DialogTitle,
                                    std::string_view ToFolder,
                                    std::string_view ToFilter,
                                    ::agiru::Text<0> &ToFile) {
  static_cast<void>(InStream);
  static_cast<void>(DialogTitle);
  static_cast<void>(ToFolder);
  static_cast<void>(ToFilter);
  static_cast<void>(ToFile);
  RefuseUnimplemented("File.DownloadFromStream(InStream, Text, Text, Text, Text)");
}

::agiru::Boolean GetStamp(std::string_view Name, ::agiru::Date &Date, ::agiru::Time &Time) {
  static_cast<void>(Name);
  static_cast<void>(Date);
  static_cast<void>(Time);
  RefuseUnimplemented("File.GetStamp(Text, Date, Time)");
}

::agiru::Boolean SetStamp(std::string_view Name, ::agiru::Date Date, ::agiru::Time Time) {
  static_cast<void>(Name);
  static_cast<void>(Date);
  static_cast<void>(Time);
  RefuseUnimplemented("File.SetStamp(Text, Date, Time)");
}

::agiru::Boolean Upload(std::string_view DialogTitle,
                        std::string_view FromFolder,
                        std::string_view FromFilter,
                        std::string_view FromFile,
                        ::agiru::Text<0> &ToFile) {
  static_cast<void>(DialogTitle);
  static_cast<void>(FromFolder);
  static_cast<void>(FromFilter);
  static_cast<void>(FromFile);
  static_cast<void>(ToFile);
  RefuseUnimplemented("File.Upload(Text, Text, Text, Text, Text)");
}

::agiru::Boolean UploadIntoStream(std::string_view FromFilter, ::agiru::InStream &InStream) {
  static_cast<void>(FromFilter);
  static_cast<void>(InStream);
  RefuseUnimplemented("File.UploadIntoStream(Text, InStream)");
}

::agiru::Boolean UploadIntoStream(std::string_view DialogTitle,
                                  std::string_view FromFolder,
                                  std::string_view FromFilter,
                                  ::agiru::Text<0> &FromFile,
                                  ::agiru::InStream &InStream) {
  static_cast<void>(DialogTitle);
  static_cast<void>(FromFolder);
  static_cast<void>(FromFilter);
  static_cast<void>(FromFile);
  static_cast<void>(InStream);
  RefuseUnimplemented("File.UploadIntoStream(Text, Text, Text, Text, InStream)");
}

::agiru::Boolean View(std::string_view FromFile, ::agiru::Boolean AllowDownloadAndPrint) {
  static_cast<void>(FromFile);
  static_cast<void>(AllowDownloadAndPrint);
  RefuseUnimplemented("File.View(Text, Boolean)");
}

::agiru::Boolean ViewFromStream(const ::agiru::InStream &InStream,
                                std::string_view FileName,
                                ::agiru::Boolean AllowDownloadAndPrint) {
  static_cast<void>(InStream);
  static_cast<void>(FileName);
  static_cast<void>(AllowDownloadAndPrint);
  RefuseUnimplemented("File.ViewFromStream(InStream, Text, Boolean)");
}

// NOLINTEND(bugprone-easily-swappable-parameters,performance-unnecessary-value-param)

}
