// NOLINTBEGIN(readability-convert-member-functions-to-static,bugprone-easily-swappable-parameters,performance-unnecessary-value-param)
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "runtime/test/TestHttpRequestMessage.h"
#include "runtime/test/TestHttpResponseMessage.h"
#include "type/BigInteger.h"
#include "type/BigText.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/CompanyProperty.h"
#include "type/Cookie.h"
#include "type/DataScope.h"
#include "type/DataTransfer.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Debugger.h"
#include "type/Decimal.h"
#include "type/Dialog.h"
#include "type/Dictionary.h"
#include "type/Duration.h"
#include "type/ErrorInfo.h"
#include "type/ErrorType.h"
#include "type/File.h"
#include "type/FileUpload.h"
#include "type/FilterPageBuilder.h"
#include "type/Guid.h"
#include "type/HttpClient.h"
#include "type/HttpContent.h"
#include "type/HttpHeaders.h"
#include "type/HttpRequestMessage.h"
#include "type/HttpRequestType.h"
#include "type/HttpResponseMessage.h"
#include "type/Integer.h"
#include "type/IsolatedStorage.h"
#include "type/JsonArray.h"
#include "type/JsonObject.h"
#include "type/JsonToken.h"
#include "type/JsonValue.h"
#include "type/KeyRef.h"
#include "type/Label.h"
#include "type/List.h"
#include "type/ModuleInfo.h"
#include "type/NavApp.h"
#include "type/NumberSequence.h"
#include "type/ObjectType.h"
#include "type/ProductName.h"
#include "type/RecordId.h"
#include "type/SecretText.h"
#include "type/SessionInformation.h"
#include "type/SessionSettings.h"
#include "type/Stream.h"
#include "type/TaskScheduler.h"
#include "type/TextBuilder.h"
#include "type/TextConst.h"
#include "type/TextEncoding.h"
#include "type/Time.h"
#include "type/Variant.h"
#include "type/Verbosity.h"
#include "type/WebServiceActionContext.h"
#include "type/WebServiceActionResultCode.h"
#include "type/XmlAttribute.h"
#include "type/XmlAttributeCollection.h"
#include "type/XmlCData.h"
#include "type/XmlComment.h"
#include "type/XmlDeclaration.h"
#include "type/XmlDocument.h"
#include "type/XmlDocumentType.h"
#include "type/XmlElement.h"
#include "type/XmlNameTable.h"
#include "type/XmlNamespaceManager.h"
#include "type/XmlNode.h"
#include "type/XmlNodeList.h"
#include "type/XmlProcessingInstruction.h"
#include "type/XmlReadOptions.h"
#include "type/XmlText.h"
#include "type/XmlWriteOptions.h"

#include <string>
#include <string_view>

namespace agiru {

namespace {

[[noreturn]] void RefuseDoor(std::string_view what) {
  throw Error(std::string(what) + " is declared and not implemented yet (board:0035)");
}

}

void BigText::AddText(const ::agiru::BigText &String, ::agiru::Integer Position) {
  static_cast<void>(String);
  static_cast<void>(Position);
  RefuseDoor("BigText.AddText(BigText, Integer)");
}

void BigText::AddText(std::string_view String, ::agiru::Integer Position) {
  static_cast<void>(String);
  static_cast<void>(Position);
  RefuseDoor("BigText.AddText(Text, Integer)");
}

::agiru::Integer BigText::GetSubText(::agiru::BigText &Variable,
                                     ::agiru::Integer Position,
                                     ::agiru::Integer Length) {
  static_cast<void>(Variable);
  static_cast<void>(Position);
  static_cast<void>(Length);
  RefuseDoor("BigText.GetSubText(BigText, Integer, Integer)");
}

::agiru::Integer
BigText::GetSubText(std::string &Variable, ::agiru::Integer Position, ::agiru::Integer Length) {
  static_cast<void>(Variable);
  static_cast<void>(Position);
  static_cast<void>(Length);
  RefuseDoor("BigText.GetSubText(Text, Integer, Integer)");
}

::agiru::Integer BigText::Length() {
  RefuseDoor("BigText.Length()");
}

::agiru::Boolean BigText::Read(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseDoor("BigText.Read(InStream)");
}

::agiru::Integer BigText::TextPos(std::string_view String) {
  static_cast<void>(String);
  RefuseDoor("BigText.TextPos(Text)");
}

::agiru::Boolean BigText::Write(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("BigText.Write(OutStream)");
}

std::string CompanyProperty::DisplayName() {
  RefuseDoor("CompanyProperty.DisplayName()");
}

::agiru::Guid CompanyProperty::ID() {
  RefuseDoor("CompanyProperty.ID()");
}

std::string CompanyProperty::UrlName() {
  RefuseDoor("CompanyProperty.UrlName()");
}

std::string Cookie::Domain() {
  RefuseDoor("Cookie.Domain()");
}

::agiru::DateTime Cookie::Expires() {
  RefuseDoor("Cookie.Expires()");
}

::agiru::Boolean Cookie::HttpOnly() {
  RefuseDoor("Cookie.HttpOnly()");
}

std::string Cookie::Name(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("Cookie.Name(Text)");
}

std::string Cookie::Path() {
  RefuseDoor("Cookie.Path()");
}

::agiru::Boolean Cookie::Secure() {
  RefuseDoor("Cookie.Secure()");
}

std::string Cookie::Value(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("Cookie.Value(Text)");
}

void DataTransfer::AddConstantValue(const ::agiru::Variant &Value,
                                    ::agiru::Integer DestinationField) {
  static_cast<void>(Value);
  static_cast<void>(DestinationField);
  RefuseDoor("DataTransfer.AddConstantValue(Any, Integer)");
}

void DataTransfer::AddDestinationFilter(::agiru::Integer DestinationField,
                                        std::string_view String,
                                        const ::agiru::Variant &Value) {
  static_cast<void>(DestinationField);
  static_cast<void>(String);
  static_cast<void>(Value);
  RefuseDoor("DataTransfer.AddDestinationFilter(Integer, Text, Any)");
}

void DataTransfer::AddFieldValue(::agiru::Integer SourceField, ::agiru::Integer DestinationField) {
  static_cast<void>(SourceField);
  static_cast<void>(DestinationField);
  RefuseDoor("DataTransfer.AddFieldValue(Integer, Integer)");
}

void DataTransfer::AddJoin(::agiru::Integer SourceField, ::agiru::Integer DestinationField) {
  static_cast<void>(SourceField);
  static_cast<void>(DestinationField);
  RefuseDoor("DataTransfer.AddJoin(Integer, Integer)");
}

void DataTransfer::AddSourceFilter(::agiru::Integer SourceField,
                                   std::string_view String,
                                   const ::agiru::Variant &Value) {
  static_cast<void>(SourceField);
  static_cast<void>(String);
  static_cast<void>(Value);
  RefuseDoor("DataTransfer.AddSourceFilter(Integer, Text, Any)");
}

void DataTransfer::CopyFields() {
  RefuseDoor("DataTransfer.CopyFields()");
}

void DataTransfer::CopyRows() {
  RefuseDoor("DataTransfer.CopyRows()");
}

void DataTransfer::SetTables(::agiru::Integer SourceTable, ::agiru::Integer DestinationTable) {
  static_cast<void>(SourceTable);
  static_cast<void>(DestinationTable);
  RefuseDoor("DataTransfer.SetTables(Integer, Integer)");
}

::agiru::Boolean DataTransfer::UpdateAuditFields(::agiru::Boolean UpdateAuditFields) {
  static_cast<void>(UpdateAuditFields);
  RefuseDoor("DataTransfer.UpdateAuditFields(Boolean)");
}

::agiru::Boolean Debugger::Activate() {
  RefuseDoor("Debugger.Activate()");
}

::agiru::Boolean Debugger::Attach(::agiru::Integer SessionID) {
  static_cast<void>(SessionID);
  RefuseDoor("Debugger.Attach(Integer)");
}

::agiru::Boolean Debugger::Break() {
  RefuseDoor("Debugger.Break()");
}

::agiru::Boolean Debugger::BreakOnError(::agiru::Boolean Ok) {
  static_cast<void>(Ok);
  RefuseDoor("Debugger.BreakOnError(Boolean)");
}

::agiru::Boolean Debugger::BreakOnRecordChanges(::agiru::Boolean Ok) {
  static_cast<void>(Ok);
  RefuseDoor("Debugger.BreakOnRecordChanges(Boolean)");
}

::agiru::Boolean Debugger::Continue() {
  RefuseDoor("Debugger.Continue()");
}

::agiru::Boolean Debugger::Deactivate() {
  RefuseDoor("Debugger.Deactivate()");
}

::agiru::Integer Debugger::DebuggedSessionID() {
  RefuseDoor("Debugger.DebuggedSessionID()");
}

::agiru::Integer Debugger::DebuggingSessionID() {
  RefuseDoor("Debugger.DebuggingSessionID()");
}

::agiru::Boolean Debugger::EnableSqlTrace(::agiru::Integer SessionID,
                                          ::agiru::Boolean NewIsEnabled) {
  static_cast<void>(SessionID);
  static_cast<void>(NewIsEnabled);
  RefuseDoor("Debugger.EnableSqlTrace(Integer, Boolean)");
}

std::string Debugger::GetLastErrorText() {
  RefuseDoor("Debugger.GetLastErrorText()");
}

::agiru::Boolean Debugger::IsActive() {
  RefuseDoor("Debugger.IsActive()");
}

::agiru::Boolean Debugger::IsAttached() {
  RefuseDoor("Debugger.IsAttached()");
}

::agiru::Boolean Debugger::IsBreakpointHit() {
  RefuseDoor("Debugger.IsBreakpointHit()");
}

::agiru::Boolean Debugger::SkipSystemTriggers(::agiru::Boolean Ok) {
  static_cast<void>(Ok);
  RefuseDoor("Debugger.SkipSystemTriggers(Boolean)");
}

::agiru::Boolean Debugger::StepInto() {
  RefuseDoor("Debugger.StepInto()");
}

::agiru::Boolean Debugger::StepOut() {
  RefuseDoor("Debugger.StepOut()");
}

::agiru::Boolean Debugger::StepOver() {
  RefuseDoor("Debugger.StepOver()");
}

::agiru::Boolean Debugger::Stop() {
  RefuseDoor("Debugger.Stop()");
}

void Dialog::Close() {
  RefuseDoor("Dialog.Close()");
}

::agiru::Boolean
Dialog::Confirm(std::string_view String, ::agiru::Boolean Default, const ::agiru::Variant &Value1) {
  static_cast<void>(String);
  static_cast<void>(Default);
  static_cast<void>(Value1);
  RefuseDoor("Dialog.Confirm(Text, Boolean, Any)");
}

void Dialog::Error(const ::agiru::ErrorInfo &Message) {
  static_cast<void>(Message);
  RefuseDoor("Dialog.Error(ErrorInfo)");
}

void Dialog::Error(std::string_view Message, const ::agiru::Variant &Value) {
  static_cast<void>(Message);
  static_cast<void>(Value);
  RefuseDoor("Dialog.Error(Text, Any)");
}

::agiru::Boolean Dialog::HideSubsequentDialogs(::agiru::Boolean HideSubsequentDialogs) {
  static_cast<void>(HideSubsequentDialogs);
  RefuseDoor("Dialog.HideSubsequentDialogs(Boolean)");
}

void Dialog::LogInternalError(std::string_view Message,
                              const ::agiru::Variant &DataClassificationInstance,
                              const ::agiru::Verbosity &VerbosityInstance) {
  static_cast<void>(Message);
  static_cast<void>(DataClassificationInstance);
  static_cast<void>(VerbosityInstance);
  RefuseDoor("Dialog.LogInternalError(Text, DataClassification, Verbosity)");
}

void Dialog::LogInternalError(std::string_view Message,
                              std::string_view SubstitutionString,
                              const ::agiru::Variant &DataClassificationInstance,
                              const ::agiru::Verbosity &VerbosityInstance) {
  static_cast<void>(Message);
  static_cast<void>(SubstitutionString);
  static_cast<void>(DataClassificationInstance);
  static_cast<void>(VerbosityInstance);
  RefuseDoor("Dialog.LogInternalError(Text, Text, DataClassification, Verbosity)");
}

void Dialog::Message(std::string_view String, const ::agiru::Variant &Value) {
  static_cast<void>(String);
  static_cast<void>(Value);
  RefuseDoor("Dialog.Message(Text, Any)");
}

void Dialog::Open(std::string_view String, ::agiru::Variant &Variable1) {
  static_cast<void>(String);
  static_cast<void>(Variable1);
  RefuseDoor("Dialog.Open(Text, Any)");
}

::agiru::Integer Dialog::StrMenu(std::string_view OptionMembers,
                                 ::agiru::Integer DefaultNumber,
                                 std::string_view Instruction) {
  static_cast<void>(OptionMembers);
  static_cast<void>(DefaultNumber);
  static_cast<void>(Instruction);
  RefuseDoor("Dialog.StrMenu(Text, Integer, Text)");
}

void Dialog::Update(::agiru::Integer Number, const ::agiru::Variant &Value) {
  static_cast<void>(Number);
  static_cast<void>(Value);
  RefuseDoor("Dialog.Update(Integer, Any)");
}

void ErrorInfo::AddAction(std::string_view Caption,
                          ::agiru::Integer CodeunitID,
                          std::string_view MethodName) {
  static_cast<void>(Caption);
  static_cast<void>(CodeunitID);
  static_cast<void>(MethodName);
  RefuseDoor("ErrorInfo.AddAction(Text, Integer, Text)");
}

void ErrorInfo::AddAction(std::string_view Caption,
                          ::agiru::Integer CodeunitID,
                          std::string_view MethodName,
                          std::string_view Description) {
  static_cast<void>(Caption);
  static_cast<void>(CodeunitID);
  static_cast<void>(MethodName);
  static_cast<void>(Description);
  RefuseDoor("ErrorInfo.AddAction(Text, Integer, Text, Text)");
}

void ErrorInfo::AddNavigationAction(std::string_view Caption) {
  static_cast<void>(Caption);
  RefuseDoor("ErrorInfo.AddNavigationAction(Text)");
}

void ErrorInfo::AddNavigationAction(std::string_view Caption, std::string_view Description) {
  static_cast<void>(Caption);
  static_cast<void>(Description);
  RefuseDoor("ErrorInfo.AddNavigationAction(Text, Text)");
}

std::string ErrorInfo::Callstack() {
  RefuseDoor("ErrorInfo.Callstack()");
}

::agiru::Boolean ErrorInfo::Collectible(::agiru::Boolean Collectible) {
  static_cast<void>(Collectible);
  RefuseDoor("ErrorInfo.Collectible(Boolean)");
}

std::string ErrorInfo::ControlName(std::string_view ControlName) {
  static_cast<void>(ControlName);
  RefuseDoor("ErrorInfo.ControlName(Text)");
}

::agiru::ErrorInfo ErrorInfo::Create() {
  RefuseDoor("ErrorInfo.Create()");
}

::agiru::ErrorInfo
ErrorInfo::Create(std::string_view Message,
                  ::agiru::Boolean Collectible,
                  ::agiru::RecordRef &Record,
                  ::agiru::Integer FieldNo,
                  ::agiru::Integer PageNo,
                  std::string_view ControlName,
                  const ::agiru::Verbosity &Verbosity,
                  const ::agiru::Variant &DataClassification,
                  const ::agiru::Dictionary<std::string, std::string> &CustomDimensions) {
  static_cast<void>(Message);
  static_cast<void>(Collectible);
  static_cast<void>(Record);
  static_cast<void>(FieldNo);
  static_cast<void>(PageNo);
  static_cast<void>(ControlName);
  static_cast<void>(Verbosity);
  static_cast<void>(DataClassification);
  static_cast<void>(CustomDimensions);
  RefuseDoor("ErrorInfo.Create(String, Boolean, Record, Integer, Integer, String, "
             "Verbosity, DataClassification, Dictionary of [Text, Text])");
}

void ErrorInfo::CustomDimensions(
    const ::agiru::Dictionary<std::string, std::string> &CustomDimensions) {
  static_cast<void>(CustomDimensions);
  RefuseDoor("ErrorInfo.CustomDimensions(Dictionary of [Text, Text])");
}

::agiru::Variant ErrorInfo::DataClassification(const ::agiru::Variant &DataClassification) {
  static_cast<void>(DataClassification);
  RefuseDoor("ErrorInfo.DataClassification(DataClassification)");
}

std::string ErrorInfo::DetailedMessage(std::string_view DetailedMessage) {
  static_cast<void>(DetailedMessage);
  RefuseDoor("ErrorInfo.DetailedMessage(Text)");
}

::agiru::ErrorType ErrorInfo::ErrorType(const ::agiru::ErrorType &ErrorType) {
  static_cast<void>(ErrorType);
  RefuseDoor("ErrorInfo.ErrorType(ErrorType)");
}

::agiru::Integer ErrorInfo::FieldNo(::agiru::Integer FieldNo) {
  static_cast<void>(FieldNo);
  RefuseDoor("ErrorInfo.FieldNo(Integer)");
}

std::string ErrorInfo::Message(std::string_view Message) {
  static_cast<void>(Message);
  RefuseDoor("ErrorInfo.Message(Text)");
}

::agiru::Integer ErrorInfo::PageNo(::agiru::Integer PageNo) {
  static_cast<void>(PageNo);
  RefuseDoor("ErrorInfo.PageNo(Integer)");
}

::agiru::RecordId ErrorInfo::RecordId(::agiru::RecordId RecordId) {
  static_cast<void>(RecordId);
  RefuseDoor("ErrorInfo.RecordId(RecordId)");
}

::agiru::Guid ErrorInfo::SystemId(::agiru::Guid SystemId) {
  static_cast<void>(SystemId);
  RefuseDoor("ErrorInfo.SystemId(Guid)");
}

::agiru::Integer ErrorInfo::TableId(::agiru::Integer TableId) {
  static_cast<void>(TableId);
  RefuseDoor("ErrorInfo.TableId(Integer)");
}

std::string ErrorInfo::Title(std::string_view Title) {
  static_cast<void>(Title);
  RefuseDoor("ErrorInfo.Title(Text)");
}

::agiru::Verbosity ErrorInfo::Verbosity(const ::agiru::Verbosity &Verbosity) {
  static_cast<void>(Verbosity);
  RefuseDoor("ErrorInfo.Verbosity(Verbosity)");
}

void File::Close() {
  RefuseDoor("File.Close()");
}

::agiru::Boolean File::Copy(std::string_view FromName, std::string_view ToName) {
  static_cast<void>(FromName);
  static_cast<void>(ToName);
  RefuseDoor("File.Copy(Text, Text)");
}

::agiru::Boolean File::Create(std::string_view Name, const ::agiru::TextEncoding &Encoding) {
  static_cast<void>(Name);
  static_cast<void>(Encoding);
  RefuseDoor("File.Create(Text, TextEncoding)");
}

void File::CreateInStream(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseDoor("File.CreateInStream(InStream)");
}

void File::CreateOutStream(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("File.CreateOutStream(OutStream)");
}

::agiru::Boolean File::CreateTempFile(const ::agiru::TextEncoding &Encoding) {
  static_cast<void>(Encoding);
  RefuseDoor("File.CreateTempFile(TextEncoding)");
}

::agiru::Boolean File::Download(std::string_view FromFile,
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

::agiru::Boolean File::DownloadFromStream(const ::agiru::InStream &InStream,
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

::agiru::Boolean File::Erase(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("File.Erase(Text)");
}

::agiru::Boolean File::Exists(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("File.Exists(Text)");
}

::agiru::Boolean File::GetStamp(std::string_view Name, ::agiru::Date &Date, ::agiru::Time &Time) {
  static_cast<void>(Name);
  static_cast<void>(Date);
  static_cast<void>(Time);
  RefuseDoor("File.GetStamp(Text, Date, Time)");
}

::agiru::Boolean File::IsPathTemporary(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("File.IsPathTemporary(Text)");
}

::agiru::Integer File::Len() {
  RefuseDoor("File.Len()");
}

std::string File::Name() {
  RefuseDoor("File.Name()");
}

::agiru::Boolean File::Open(std::string_view Name, const ::agiru::TextEncoding &Encoding) {
  static_cast<void>(Name);
  static_cast<void>(Encoding);
  RefuseDoor("File.Open(Text, TextEncoding)");
}

::agiru::Integer File::Pos() {
  RefuseDoor("File.Pos()");
}

::agiru::Integer File::Read(::agiru::Variant &Read) {
  static_cast<void>(Read);
  RefuseDoor("File.Read(Any)");
}

::agiru::Boolean File::Rename(std::string_view OldName, std::string_view NewName) {
  static_cast<void>(OldName);
  static_cast<void>(NewName);
  RefuseDoor("File.Rename(Text, Text)");
}

void File::Seek(::agiru::Integer Position) {
  static_cast<void>(Position);
  RefuseDoor("File.Seek(Integer)");
}

::agiru::Boolean File::SetStamp(std::string_view Name, ::agiru::Date Date, ::agiru::Time Time) {
  static_cast<void>(Name);
  static_cast<void>(Date);
  static_cast<void>(Time);
  RefuseDoor("File.SetStamp(Text, Date, Time)");
}

::agiru::Boolean File::TextMode(::agiru::Boolean Mode) {
  static_cast<void>(Mode);
  RefuseDoor("File.TextMode(Boolean)");
}

void File::Trunc() {
  RefuseDoor("File.Trunc()");
}

::agiru::Boolean File::Upload(std::string_view DialogTitle,
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

::agiru::Boolean File::UploadIntoStream(std::string_view FromFilter, ::agiru::InStream &InStream) {
  static_cast<void>(FromFilter);
  static_cast<void>(InStream);
  RefuseDoor("File.UploadIntoStream(Text, InStream)");
}

::agiru::Boolean File::UploadIntoStream(std::string_view DialogTitle,
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

::agiru::Boolean File::View(std::string_view FromFile, ::agiru::Boolean AllowDownloadAndPrint) {
  static_cast<void>(FromFile);
  static_cast<void>(AllowDownloadAndPrint);
  RefuseDoor("File.View(Text, Boolean)");
}

::agiru::Boolean File::ViewFromStream(const ::agiru::InStream &InStream,
                                      std::string_view FileName,
                                      ::agiru::Boolean AllowDownloadAndPrint) {
  static_cast<void>(InStream);
  static_cast<void>(FileName);
  static_cast<void>(AllowDownloadAndPrint);
  RefuseDoor("File.ViewFromStream(InStream, Text, Boolean)");
}

void File::Write(::agiru::BigInteger Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(BigInteger)");
}

void File::Write(const ::agiru::BigText &Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(BigText)");
}

void File::Write(::agiru::Boolean Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(Boolean)");
}

void File::Write(::agiru::Byte Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(Byte)");
}

void File::Write(::agiru::Char Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(Char)");
}

void File::Write(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(Code)");
}

void File::Write(::agiru::Date Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(Date)");
}

void File::Write(::agiru::DateFormula Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(DateFormula)");
}

void File::Write(::agiru::DateTime Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(DateTime)");
}

void File::Write(::agiru::Decimal Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(Decimal)");
}

void File::Write(::agiru::Duration Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(Duration)");
}

void File::Write(::agiru::Guid Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(Guid)");
}

void File::Write(::agiru::Integer Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(Integer)");
}

void File::Write(const ::agiru::Variant &Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(Any)");
}

void File::Write(::agiru::RecordId Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(RecordId)");
}

void File::Write(const ::agiru::RecordRef &Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(Record)");
}

void File::Write(::agiru::Time Value) {
  static_cast<void>(Value);
  RefuseDoor("File.Write(Time)");
}

::agiru::Boolean File::WriteMode(::agiru::Boolean Mode) {
  static_cast<void>(Mode);
  RefuseDoor("File.WriteMode(Boolean)");
}

void FileUpload::CreateInStream(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseDoor("FileUpload.CreateInStream(InStream)");
}

void FileUpload::CreateInStream(const ::agiru::InStream &InStream,
                                const ::agiru::TextEncoding &Encoding) {
  static_cast<void>(InStream);
  static_cast<void>(Encoding);
  RefuseDoor("FileUpload.CreateInStream(InStream, TextEncoding)");
}

std::string FileUpload::FileName() {
  RefuseDoor("FileUpload.FileName()");
}

::agiru::Boolean FilterPageBuilder::AddField(std::string_view Name,
                                             const ::agiru::FieldRef &Field,
                                             std::string_view Filter) {
  static_cast<void>(Name);
  static_cast<void>(Field);
  static_cast<void>(Filter);
  RefuseDoor("FilterPageBuilder.AddField(Text, FieldRef, Text)");
}

::agiru::Boolean FilterPageBuilder::AddField(std::string_view Name,
                                             const ::agiru::Variant &Field,
                                             std::string_view Filter) {
  static_cast<void>(Name);
  static_cast<void>(Field);
  static_cast<void>(Filter);
  RefuseDoor("FilterPageBuilder.AddField(Text, Any, Text)");
}

::agiru::Boolean FilterPageBuilder::AddFieldNo(std::string_view Name,
                                               ::agiru::Integer FieldNo,
                                               std::string_view Filter) {
  static_cast<void>(Name);
  static_cast<void>(FieldNo);
  static_cast<void>(Filter);
  RefuseDoor("FilterPageBuilder.AddFieldNo(Text, Integer, Text)");
}

std::string FilterPageBuilder::AddRecord(std::string_view Name, const ::agiru::RecordRef &Record) {
  static_cast<void>(Name);
  static_cast<void>(Record);
  RefuseDoor("FilterPageBuilder.AddRecord(Text, Record)");
}

std::string FilterPageBuilder::AddRecordRef(std::string_view Name,
                                            const ::agiru::RecordRef &RecordRef) {
  static_cast<void>(Name);
  static_cast<void>(RecordRef);
  RefuseDoor("FilterPageBuilder.AddRecordRef(Text, RecordRef)");
}

std::string FilterPageBuilder::AddTable(std::string_view Name, ::agiru::Integer TableNo) {
  static_cast<void>(Name);
  static_cast<void>(TableNo);
  RefuseDoor("FilterPageBuilder.AddTable(Text, Integer)");
}

::agiru::Integer FilterPageBuilder::Count() {
  RefuseDoor("FilterPageBuilder.Count()");
}

std::string FilterPageBuilder::GetView(std::string_view Name, ::agiru::Boolean UseNames) {
  static_cast<void>(Name);
  static_cast<void>(UseNames);
  RefuseDoor("FilterPageBuilder.GetView(Text, Boolean)");
}

std::string FilterPageBuilder::Name(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("FilterPageBuilder.Name(Integer)");
}

std::string FilterPageBuilder::PageCaption(std::string_view PageCaption) {
  static_cast<void>(PageCaption);
  RefuseDoor("FilterPageBuilder.PageCaption(Text)");
}

::agiru::Boolean FilterPageBuilder::RunModal() {
  RefuseDoor("FilterPageBuilder.RunModal()");
}

::agiru::Boolean FilterPageBuilder::SetView(std::string_view Name, std::string_view View) {
  static_cast<void>(Name);
  static_cast<void>(View);
  RefuseDoor("FilterPageBuilder.SetView(Text, Text)");
}

void HttpClient::AddCertificate(const ::agiru::SecretText &Certificate,
                                const ::agiru::SecretText &Password) {
  static_cast<void>(Certificate);
  static_cast<void>(Password);
  RefuseDoor("HttpClient.AddCertificate(SecretText, SecretText)");
}

void HttpClient::AddCertificate(std::string_view Certificate, std::string_view Password) {
  static_cast<void>(Certificate);
  static_cast<void>(Password);
  RefuseDoor("HttpClient.AddCertificate(Text, Text)");
}

void HttpClient::Clear() {
  RefuseDoor("HttpClient.Clear()");
}

::agiru::HttpHeaders HttpClient::DefaultRequestHeaders() {
  RefuseDoor("HttpClient.DefaultRequestHeaders()");
}

::agiru::Boolean HttpClient::Delete(std::string_view Path, ::agiru::HttpResponseMessage &Response) {
  static_cast<void>(Path);
  static_cast<void>(Response);
  RefuseDoor("HttpClient.Delete(Text, HttpResponseMessage)");
}

::agiru::Boolean HttpClient::Get(std::string_view Path, ::agiru::HttpResponseMessage &Response) {
  static_cast<void>(Path);
  static_cast<void>(Response);
  RefuseDoor("HttpClient.Get(Text, HttpResponseMessage)");
}

std::string HttpClient::GetBaseAddress() {
  RefuseDoor("HttpClient.GetBaseAddress()");
}

::agiru::Boolean HttpClient::Patch(std::string_view Path,
                                   const ::agiru::HttpContent &Content,
                                   ::agiru::HttpResponseMessage &Response) {
  static_cast<void>(Path);
  static_cast<void>(Content);
  static_cast<void>(Response);
  RefuseDoor("HttpClient.Patch(Text, HttpContent, HttpResponseMessage)");
}

::agiru::Boolean HttpClient::Post(std::string_view Path,
                                  const ::agiru::HttpContent &Content,
                                  ::agiru::HttpResponseMessage &Response) {
  static_cast<void>(Path);
  static_cast<void>(Content);
  static_cast<void>(Response);
  RefuseDoor("HttpClient.Post(Text, HttpContent, HttpResponseMessage)");
}

::agiru::Boolean HttpClient::Put(std::string_view Path,
                                 const ::agiru::HttpContent &Content,
                                 ::agiru::HttpResponseMessage &Response) {
  static_cast<void>(Path);
  static_cast<void>(Content);
  static_cast<void>(Response);
  RefuseDoor("HttpClient.Put(Text, HttpContent, HttpResponseMessage)");
}

::agiru::Boolean HttpClient::Send(const ::agiru::HttpRequestMessage &Request,
                                  ::agiru::HttpResponseMessage &Response) {
  static_cast<void>(Request);
  static_cast<void>(Response);
  RefuseDoor("HttpClient.Send(HttpRequestMessage, HttpResponseMessage)");
}

::agiru::Boolean HttpClient::SetBaseAddress(std::string_view NewBaseAddress) {
  static_cast<void>(NewBaseAddress);
  RefuseDoor("HttpClient.SetBaseAddress(Text)");
}

::agiru::Duration HttpClient::Timeout(::agiru::Duration SetTimeout) {
  static_cast<void>(SetTimeout);
  RefuseDoor("HttpClient.Timeout(Duration)");
}

::agiru::Boolean HttpClient::UseDefaultNetworkWindowsAuthentication() {
  RefuseDoor("HttpClient.UseDefaultNetworkWindowsAuthentication()");
}

void HttpClient::UseResponseCookies(::agiru::Boolean UseResponseCookies) {
  static_cast<void>(UseResponseCookies);
  RefuseDoor("HttpClient.UseResponseCookies(Boolean)");
}

::agiru::Boolean
HttpClient::UseServerCertificateValidation(::agiru::Boolean UseServerCertificateValidation) {
  static_cast<void>(UseServerCertificateValidation);
  RefuseDoor("HttpClient.UseServerCertificateValidation(Boolean)");
}

::agiru::Boolean HttpClient::UseWindowsAuthentication(const ::agiru::SecretText &UserName,
                                                      const ::agiru::SecretText &Password,
                                                      const ::agiru::SecretText &Domain) {
  static_cast<void>(UserName);
  static_cast<void>(Password);
  static_cast<void>(Domain);
  RefuseDoor("HttpClient.UseWindowsAuthentication(SecretText, SecretText, SecretText)");
}

::agiru::Boolean HttpClient::UseWindowsAuthentication(std::string_view UserName,
                                                      std::string_view Password,
                                                      std::string_view Domain) {
  static_cast<void>(UserName);
  static_cast<void>(Password);
  static_cast<void>(Domain);
  RefuseDoor("HttpClient.UseWindowsAuthentication(Text, Text, Text)");
}

void HttpContent::Clear() {
  RefuseDoor("HttpContent.Clear()");
}

::agiru::Boolean HttpContent::GetHeaders(::agiru::HttpHeaders &Headers) {
  static_cast<void>(Headers);
  RefuseDoor("HttpContent.GetHeaders(HttpHeaders)");
}

::agiru::Boolean HttpContent::IsSecretContent() {
  RefuseDoor("HttpContent.IsSecretContent()");
}

::agiru::Boolean HttpContent::ReadAs(::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseDoor("HttpContent.ReadAs(InStream)");
}

::agiru::Boolean HttpContent::ReadAs(::agiru::SecretText &OutputSecretText) {
  static_cast<void>(OutputSecretText);
  RefuseDoor("HttpContent.ReadAs(SecretText)");
}

::agiru::Boolean HttpContent::ReadAs(std::string &OutputString) {
  static_cast<void>(OutputString);
  RefuseDoor("HttpContent.ReadAs(Text)");
}

void HttpContent::WriteFrom(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseDoor("HttpContent.WriteFrom(InStream)");
}

void HttpContent::WriteFrom(const ::agiru::SecretText &SecretText) {
  static_cast<void>(SecretText);
  RefuseDoor("HttpContent.WriteFrom(SecretText)");
}

void HttpContent::WriteFrom(std::string_view Text) {
  static_cast<void>(Text);
  RefuseDoor("HttpContent.WriteFrom(Text)");
}

::agiru::Boolean HttpHeaders::Add(std::string_view Name, const ::agiru::SecretText &Value) {
  static_cast<void>(Name);
  static_cast<void>(Value);
  RefuseDoor("HttpHeaders.Add(Text, SecretText)");
}

::agiru::Boolean HttpHeaders::Add(std::string_view Name, std::string_view Value) {
  static_cast<void>(Name);
  static_cast<void>(Value);
  RefuseDoor("HttpHeaders.Add(Text, Text)");
}

void HttpHeaders::Clear() {
  RefuseDoor("HttpHeaders.Clear()");
}

::agiru::Boolean HttpHeaders::Contains(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("HttpHeaders.Contains(Text)");
}

::agiru::Boolean HttpHeaders::ContainsSecret(std::string_view Key) {
  static_cast<void>(Key);
  RefuseDoor("HttpHeaders.ContainsSecret(Text)");
}

::agiru::Boolean HttpHeaders::GetSecretValues(std::string_view Key,
                                              const ::agiru::List<::agiru::SecretText> &Values) {
  static_cast<void>(Key);
  static_cast<void>(Values);
  RefuseDoor("HttpHeaders.GetSecretValues(Text, List of [SecretText])");
}

::agiru::Boolean HttpHeaders::GetSecretValues(std::string_view Key,
                                              const ::agiru::Variant &Values) {
  static_cast<void>(Key);
  static_cast<void>(Values);
  RefuseDoor("HttpHeaders.GetSecretValues(Text, Array of [SecretText])");
}

::agiru::Boolean HttpHeaders::GetValues(std::string_view Key, const ::agiru::Variant &Values) {
  static_cast<void>(Key);
  static_cast<void>(Values);
  RefuseDoor("HttpHeaders.GetValues(String, Array of [Text])");
}

::agiru::Boolean HttpHeaders::GetValues(std::string_view Key,
                                        const ::agiru::List<std::string> &Values) {
  static_cast<void>(Key);
  static_cast<void>(Values);
  RefuseDoor("HttpHeaders.GetValues(Text, List of [Text])");
}

void HttpHeaders::Keys() {
  RefuseDoor("HttpHeaders.Keys()");
}

::agiru::Boolean HttpHeaders::Remove(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("HttpHeaders.Remove(Text)");
}

::agiru::Boolean HttpHeaders::TryAddWithoutValidation(std::string_view Name,
                                                      const ::agiru::SecretText &Value) {
  static_cast<void>(Name);
  static_cast<void>(Value);
  RefuseDoor("HttpHeaders.TryAddWithoutValidation(Text, SecretText)");
}

::agiru::Boolean HttpHeaders::TryAddWithoutValidation(std::string_view Name,
                                                      std::string_view Value) {
  static_cast<void>(Name);
  static_cast<void>(Value);
  RefuseDoor("HttpHeaders.TryAddWithoutValidation(Text, Text)");
}

::agiru::HttpContent HttpRequestMessage::Content(const ::agiru::HttpContent &SetContent) {
  static_cast<void>(SetContent);
  RefuseDoor("HttpRequestMessage.Content(HttpContent)");
}

::agiru::Boolean HttpRequestMessage::GetCookie(std::string_view Name, ::agiru::Cookie &Cookie) {
  static_cast<void>(Name);
  static_cast<void>(Cookie);
  RefuseDoor("HttpRequestMessage.GetCookie(Text, Cookie)");
}

void HttpRequestMessage::GetCookieNames() {
  RefuseDoor("HttpRequestMessage.GetCookieNames()");
}

::agiru::Boolean HttpRequestMessage::GetHeaders(::agiru::HttpHeaders &Headers) {
  static_cast<void>(Headers);
  RefuseDoor("HttpRequestMessage.GetHeaders(HttpHeaders)");
}

std::string HttpRequestMessage::GetRequestUri() {
  RefuseDoor("HttpRequestMessage.GetRequestUri()");
}

::agiru::SecretText HttpRequestMessage::GetSecretRequestUri() {
  RefuseDoor("HttpRequestMessage.GetSecretRequestUri()");
}

std::string HttpRequestMessage::Method(std::string_view NewMethod) {
  static_cast<void>(NewMethod);
  RefuseDoor("HttpRequestMessage.Method(Text)");
}

::agiru::Boolean HttpRequestMessage::RemoveCookie(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("HttpRequestMessage.RemoveCookie(Text)");
}

::agiru::Boolean HttpRequestMessage::SetCookie(const ::agiru::Cookie &Cookie) {
  static_cast<void>(Cookie);
  RefuseDoor("HttpRequestMessage.SetCookie(Cookie)");
}

::agiru::Boolean HttpRequestMessage::SetCookie(std::string_view Name, std::string_view Value) {
  static_cast<void>(Name);
  static_cast<void>(Value);
  RefuseDoor("HttpRequestMessage.SetCookie(Text, Text)");
}

::agiru::Boolean HttpRequestMessage::SetRequestUri(std::string_view RequestUri) {
  static_cast<void>(RequestUri);
  RefuseDoor("HttpRequestMessage.SetRequestUri(Text)");
}

::agiru::Boolean HttpRequestMessage::SetSecretRequestUri(const ::agiru::SecretText &RequestUri) {
  static_cast<void>(RequestUri);
  RefuseDoor("HttpRequestMessage.SetSecretRequestUri(SecretText)");
}

::agiru::HttpContent HttpResponseMessage::Content() {
  RefuseDoor("HttpResponseMessage.Content()");
}

::agiru::Boolean HttpResponseMessage::GetCookie(std::string_view Name, ::agiru::Cookie &Cookie) {
  static_cast<void>(Name);
  static_cast<void>(Cookie);
  RefuseDoor("HttpResponseMessage.GetCookie(Text, Cookie)");
}

void HttpResponseMessage::GetCookieNames() {
  RefuseDoor("HttpResponseMessage.GetCookieNames()");
}

::agiru::HttpHeaders HttpResponseMessage::Headers() {
  RefuseDoor("HttpResponseMessage.Headers()");
}

::agiru::Integer HttpResponseMessage::HttpStatusCode() {
  RefuseDoor("HttpResponseMessage.HttpStatusCode()");
}

::agiru::Boolean HttpResponseMessage::IsBlockedByEnvironment() {
  RefuseDoor("HttpResponseMessage.IsBlockedByEnvironment()");
}

::agiru::Boolean HttpResponseMessage::IsSuccessStatusCode() {
  RefuseDoor("HttpResponseMessage.IsSuccessStatusCode()");
}

std::string HttpResponseMessage::ReasonPhrase() {
  RefuseDoor("HttpResponseMessage.ReasonPhrase()");
}

::agiru::Boolean IsolatedStorage::Contains(std::string_view Key,
                                           const ::agiru::DataScope &DataScope,
                                           ::agiru::Boolean &isSecret) {
  static_cast<void>(Key);
  static_cast<void>(DataScope);
  static_cast<void>(isSecret);
  RefuseDoor("IsolatedStorage.Contains(Text, DataScope, Boolean)");
}

::agiru::Boolean IsolatedStorage::Contains(std::string_view Key,
                                           const ::agiru::DataScope &DataScope) {
  static_cast<void>(Key);
  static_cast<void>(DataScope);
  RefuseDoor("IsolatedStorage.Contains(Text, DataScope)");
}

::agiru::Boolean IsolatedStorage::Delete(std::string_view Key,
                                         const ::agiru::DataScope &DataScope) {
  static_cast<void>(Key);
  static_cast<void>(DataScope);
  RefuseDoor("IsolatedStorage.Delete(Text, DataScope)");
}

::agiru::Boolean IsolatedStorage::Get(std::string_view Key,
                                      const ::agiru::DataScope &DataScope,
                                      ::agiru::SecretText &Value) {
  static_cast<void>(Key);
  static_cast<void>(DataScope);
  static_cast<void>(Value);
  RefuseDoor("IsolatedStorage.Get(Text, DataScope, SecretText)");
}

::agiru::Boolean IsolatedStorage::Get(std::string_view Key,
                                      const ::agiru::DataScope &DataScope,
                                      std::string &Value) {
  static_cast<void>(Key);
  static_cast<void>(DataScope);
  static_cast<void>(Value);
  RefuseDoor("IsolatedStorage.Get(Text, DataScope, Text)");
}

::agiru::Boolean IsolatedStorage::Get(std::string_view Key, ::agiru::SecretText &Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("IsolatedStorage.Get(Text, SecretText)");
}

::agiru::Boolean IsolatedStorage::Get(std::string_view Key, std::string &Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("IsolatedStorage.Get(Text, Text)");
}

::agiru::Boolean IsolatedStorage::Set(std::string_view Key,
                                      const ::agiru::SecretText &Value,
                                      const ::agiru::DataScope &DataScope) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  static_cast<void>(DataScope);
  RefuseDoor("IsolatedStorage.Set(Text, SecretText, DataScope)");
}

::agiru::Boolean IsolatedStorage::Set(std::string_view Key,
                                      std::string_view Value,
                                      const ::agiru::DataScope &DataScope) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  static_cast<void>(DataScope);
  RefuseDoor("IsolatedStorage.Set(Text, Text, DataScope)");
}

::agiru::Boolean IsolatedStorage::SetEncrypted(std::string_view Key,
                                               const ::agiru::SecretText &Value,
                                               const ::agiru::DataScope &DataScope) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  static_cast<void>(DataScope);
  RefuseDoor("IsolatedStorage.SetEncrypted(Text, SecretText, DataScope)");
}

::agiru::Boolean IsolatedStorage::SetEncrypted(std::string_view Key,
                                               std::string_view Value,
                                               const ::agiru::DataScope &DataScope) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  static_cast<void>(DataScope);
  RefuseDoor("IsolatedStorage.SetEncrypted(Text, Text, DataScope)");
}

void JsonArray::Add(::agiru::BigInteger Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Add(BigInteger)");
}

void JsonArray::Add(::agiru::Boolean Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Add(Boolean)");
}

void JsonArray::Add(::agiru::Byte Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Add(Byte)");
}

void JsonArray::Add(::agiru::Char Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Add(Char)");
}

void JsonArray::Add(::agiru::Date Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Add(Date)");
}

void JsonArray::Add(::agiru::DateTime Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Add(DateTime)");
}

void JsonArray::Add(::agiru::Decimal Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Add(Decimal)");
}

void JsonArray::Add(::agiru::Duration Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Add(Duration)");
}

void JsonArray::Add(::agiru::Integer Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Add(Integer)");
}

void JsonArray::Add(const ::agiru::JsonArray &Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Add(JsonArray)");
}

void JsonArray::Add(const ::agiru::JsonObject &Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Add(JsonObject)");
}

void JsonArray::Add(const ::agiru::JsonToken &Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Add(JsonToken)");
}

void JsonArray::Add(const ::agiru::JsonValue &Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Add(JsonValue)");
}

void JsonArray::Add(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Add(Text)");
}

void JsonArray::Add(::agiru::Time Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Add(Time)");
}

::agiru::JsonToken JsonArray::AsToken() {
  RefuseDoor("JsonArray.AsToken()");
}

::agiru::JsonToken JsonArray::Clone() {
  RefuseDoor("JsonArray.Clone()");
}

::agiru::JsonToken *JsonArray::begin() {
  RefuseDoor("JsonArray.begin()");
}

::agiru::JsonToken *JsonArray::end() {
  RefuseDoor("JsonArray.end()");
}

::agiru::Integer JsonArray::Count() {
  RefuseDoor("JsonArray.Count()");
}

::agiru::Boolean JsonArray::Get(::agiru::Integer Index, ::agiru::JsonToken &Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Get(Integer, JsonToken)");
}

::agiru::JsonArray JsonArray::GetArray(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("JsonArray.GetArray(Integer)");
}

::agiru::BigInteger JsonArray::GetBigInteger(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("JsonArray.GetBigInteger(Integer)");
}

::agiru::Boolean JsonArray::GetBoolean(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("JsonArray.GetBoolean(Integer)");
}

::agiru::Byte JsonArray::GetByte(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("JsonArray.GetByte(Integer)");
}

::agiru::Char JsonArray::GetChar(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("JsonArray.GetChar(Integer)");
}

::agiru::Date JsonArray::GetDate(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("JsonArray.GetDate(Integer)");
}

::agiru::DateTime JsonArray::GetDateTime(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("JsonArray.GetDateTime(Integer)");
}

::agiru::Decimal JsonArray::GetDecimal(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("JsonArray.GetDecimal(Integer)");
}

::agiru::Integer JsonArray::GetDuration(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("JsonArray.GetDuration(Integer)");
}

::agiru::Integer JsonArray::GetInteger(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("JsonArray.GetInteger(Integer)");
}

::agiru::JsonObject JsonArray::GetObject(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("JsonArray.GetObject(Integer)");
}

::agiru::Integer JsonArray::GetOption(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("JsonArray.GetOption(Integer)");
}

std::string JsonArray::GetText(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("JsonArray.GetText(Integer)");
}

::agiru::Time JsonArray::GetTime(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("JsonArray.GetTime(Integer)");
}

::agiru::Integer JsonArray::IndexOf(::agiru::BigInteger Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.IndexOf(BigInteger)");
}

::agiru::Integer JsonArray::IndexOf(::agiru::Boolean Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.IndexOf(Boolean)");
}

::agiru::Integer JsonArray::IndexOf(::agiru::Byte Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.IndexOf(Byte)");
}

::agiru::Integer JsonArray::IndexOf(::agiru::Char Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.IndexOf(Char)");
}

::agiru::Integer JsonArray::IndexOf(::agiru::Date Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.IndexOf(Date)");
}

::agiru::Integer JsonArray::IndexOf(::agiru::DateTime Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.IndexOf(DateTime)");
}

::agiru::Integer JsonArray::IndexOf(::agiru::Decimal Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.IndexOf(Decimal)");
}

::agiru::Integer JsonArray::IndexOf(::agiru::Duration Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.IndexOf(Duration)");
}

::agiru::Integer JsonArray::IndexOf(::agiru::Integer Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.IndexOf(Integer)");
}

::agiru::Integer JsonArray::IndexOf(const ::agiru::JsonArray &Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.IndexOf(JsonArray)");
}

::agiru::Integer JsonArray::IndexOf(const ::agiru::JsonObject &Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.IndexOf(JsonObject)");
}

::agiru::Integer JsonArray::IndexOf(const ::agiru::JsonToken &Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.IndexOf(JsonToken)");
}

::agiru::Integer JsonArray::IndexOf(const ::agiru::JsonValue &Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.IndexOf(JsonValue)");
}

::agiru::Integer JsonArray::IndexOf(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.IndexOf(Text)");
}

::agiru::Integer JsonArray::IndexOf(::agiru::Time Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonArray.IndexOf(Time)");
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::BigInteger Value) {
  static_cast<void>(Index);
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Insert(Integer, BigInteger)");
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Boolean Value) {
  static_cast<void>(Index);
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Insert(Integer, Boolean)");
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Byte Value) {
  static_cast<void>(Index);
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Insert(Integer, Byte)");
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Char Value) {
  static_cast<void>(Index);
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Insert(Integer, Char)");
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Date Value) {
  static_cast<void>(Index);
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Insert(Integer, Date)");
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::DateTime Value) {
  static_cast<void>(Index);
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Insert(Integer, DateTime)");
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Decimal Value) {
  static_cast<void>(Index);
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Insert(Integer, Decimal)");
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Duration Value) {
  static_cast<void>(Index);
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Insert(Integer, Duration)");
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Integer Value) {
  static_cast<void>(Index);
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Insert(Integer, Integer)");
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, const ::agiru::JsonArray &Value) {
  static_cast<void>(Index);
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Insert(Integer, JsonArray)");
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, const ::agiru::JsonObject &Value) {
  static_cast<void>(Index);
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Insert(Integer, JsonObject)");
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, const ::agiru::JsonToken &Value) {
  static_cast<void>(Index);
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Insert(Integer, JsonToken)");
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, const ::agiru::JsonValue &Value) {
  static_cast<void>(Index);
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Insert(Integer, JsonValue)");
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, std::string_view Value) {
  static_cast<void>(Index);
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Insert(Integer, Text)");
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Time Value) {
  static_cast<void>(Index);
  static_cast<void>(Value);
  RefuseDoor("JsonArray.Insert(Integer, Time)");
}

std::string JsonArray::Path() {
  RefuseDoor("JsonArray.Path()");
}

::agiru::Boolean JsonArray::ReadFrom(const ::agiru::InStream &Data) {
  static_cast<void>(Data);
  RefuseDoor("JsonArray.ReadFrom(InStream)");
}

::agiru::Boolean JsonArray::ReadFrom(std::string_view String) {
  static_cast<void>(String);
  RefuseDoor("JsonArray.ReadFrom(Text)");
}

::agiru::Boolean JsonArray::RemoveAt(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("JsonArray.RemoveAt(Integer)");
}

::agiru::Boolean JsonArray::SelectToken(std::string_view Path, ::agiru::JsonToken &Result) {
  static_cast<void>(Path);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.SelectToken(Text, JsonToken)");
}

::agiru::Boolean JsonArray::SelectTokens(std::string_view Path,
                                         ::agiru::List<::agiru::JsonToken> &Result) {
  static_cast<void>(Path);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.SelectTokens(Text, List of [JsonToken])");
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::BigInteger Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Set(Integer, BigInteger)");
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Boolean Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Set(Integer, Boolean)");
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Byte Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Set(Integer, Byte)");
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Char Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Set(Integer, Char)");
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Date Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Set(Integer, Date)");
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::DateTime Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Set(Integer, DateTime)");
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Decimal Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Set(Integer, Decimal)");
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Duration Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Set(Integer, Duration)");
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Integer Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Set(Integer, Integer)");
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, const ::agiru::JsonArray &Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Set(Integer, JsonArray)");
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, const ::agiru::JsonObject &Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Set(Integer, JsonObject)");
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, const ::agiru::JsonToken &Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Set(Integer, JsonToken)");
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, const ::agiru::JsonValue &Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Set(Integer, JsonValue)");
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, std::string_view Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Set(Integer, Text)");
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Time Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.Set(Integer, Time)");
}

::agiru::Boolean JsonArray::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("JsonArray.WriteTo(OutStream)");
}

::agiru::Boolean JsonArray::WriteTo(std::string &String) {
  static_cast<void>(String);
  RefuseDoor("JsonArray.WriteTo(Text)");
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::BigInteger Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Add(Text, BigInteger)");
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Boolean Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Add(Text, Boolean)");
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Byte Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Add(Text, Byte)");
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Char Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Add(Text, Char)");
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Date Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Add(Text, Date)");
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::DateTime Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Add(Text, DateTime)");
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Decimal Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Add(Text, Decimal)");
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Duration Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Add(Text, Duration)");
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Integer Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Add(Text, Integer)");
}

::agiru::Boolean JsonObject::Add(std::string_view Key, const ::agiru::JsonArray &Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Add(Text, JsonArray)");
}

::agiru::Boolean JsonObject::Add(std::string_view Key, const ::agiru::JsonObject &Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Add(Text, JsonObject)");
}

::agiru::Boolean JsonObject::Add(std::string_view Key, const ::agiru::JsonToken &Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Add(Text, JsonToken)");
}

::agiru::Boolean JsonObject::Add(std::string_view Key, const ::agiru::JsonValue &Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Add(Text, JsonValue)");
}

::agiru::Boolean JsonObject::Add(std::string_view Key, std::string_view Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Add(Text, Text)");
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Time Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Add(Text, Time)");
}

::agiru::JsonToken JsonObject::AsToken() {
  RefuseDoor("JsonObject.AsToken()");
}

::agiru::JsonToken JsonObject::Clone() {
  RefuseDoor("JsonObject.Clone()");
}

::agiru::Boolean JsonObject::Contains(std::string_view Key) {
  static_cast<void>(Key);
  RefuseDoor("JsonObject.Contains(Text)");
}

::agiru::Boolean JsonObject::Get(std::string_view Key, ::agiru::JsonToken &Result) {
  static_cast<void>(Key);
  static_cast<void>(Result);
  RefuseDoor("JsonObject.Get(Text, JsonToken)");
}

::agiru::JsonArray JsonObject::GetArray(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetArray(Text, Boolean)");
}

::agiru::BigInteger JsonObject::GetBigInteger(std::string_view Key,
                                              ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetBigInteger(Text, Boolean)");
}

::agiru::Boolean JsonObject::GetBoolean(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetBoolean(Text, Boolean)");
}

::agiru::Byte JsonObject::GetByte(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetByte(Text, Boolean)");
}

::agiru::Char JsonObject::GetChar(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetChar(Text, Boolean)");
}

::agiru::Date JsonObject::GetDate(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetDate(Text, Boolean)");
}

::agiru::DateTime JsonObject::GetDateTime(std::string_view Key,
                                          ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetDateTime(Text, Boolean)");
}

::agiru::Decimal JsonObject::GetDecimal(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetDecimal(Text, Boolean)");
}

::agiru::Duration JsonObject::GetDuration(std::string_view Key,
                                          ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetDuration(Text, Boolean)");
}

::agiru::Integer JsonObject::GetInteger(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetInteger(Text, Boolean)");
}

::agiru::JsonObject JsonObject::GetObject(std::string_view Key,
                                          ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetObject(Text, Boolean)");
}

::agiru::Integer JsonObject::GetOption(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetOption(Text, Boolean)");
}

std::string JsonObject::GetText(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetText(Text, Boolean)");
}

::agiru::Time JsonObject::GetTime(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetTime(Text, Boolean)");
}

void JsonObject::Keys() {
  RefuseDoor("JsonObject.Keys()");
}

std::string JsonObject::Path() {
  RefuseDoor("JsonObject.Path()");
}

::agiru::Boolean JsonObject::ReadFrom(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseDoor("JsonObject.ReadFrom(InStream)");
}

::agiru::Boolean JsonObject::ReadFrom(std::string_view String) {
  static_cast<void>(String);
  RefuseDoor("JsonObject.ReadFrom(Text)");
}

::agiru::Boolean JsonObject::ReadFromYaml(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseDoor("JsonObject.ReadFromYaml(InStream)");
}

::agiru::Boolean JsonObject::ReadFromYaml(std::string_view String) {
  static_cast<void>(String);
  RefuseDoor("JsonObject.ReadFromYaml(Text)");
}

::agiru::Boolean JsonObject::Remove(std::string_view Key) {
  static_cast<void>(Key);
  RefuseDoor("JsonObject.Remove(Text)");
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::BigInteger Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Replace(Text, BigInteger)");
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Boolean Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Replace(Text, Boolean)");
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Byte Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Replace(Text, Byte)");
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Char Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Replace(Text, Char)");
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Date Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Replace(Text, Date)");
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::DateTime Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Replace(Text, DateTime)");
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Decimal Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Replace(Text, Decimal)");
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Duration Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Replace(Text, Duration)");
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Integer Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Replace(Text, Integer)");
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, const ::agiru::JsonArray &Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Replace(Text, JsonArray)");
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, const ::agiru::JsonObject &Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Replace(Text, JsonObject)");
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, const ::agiru::JsonToken &Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Replace(Text, JsonToken)");
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, const ::agiru::JsonValue &Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Replace(Text, JsonValue)");
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, std::string_view Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Replace(Text, Text)");
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Time Value) {
  static_cast<void>(Key);
  static_cast<void>(Value);
  RefuseDoor("JsonObject.Replace(Text, Time)");
}

::agiru::Boolean JsonObject::SelectToken(std::string_view Path, ::agiru::JsonToken &Result) {
  static_cast<void>(Path);
  static_cast<void>(Result);
  RefuseDoor("JsonObject.SelectToken(Text, JsonToken)");
}

::agiru::Boolean JsonObject::SelectTokens(std::string_view Path,
                                          ::agiru::List<::agiru::JsonToken> &Result) {
  static_cast<void>(Path);
  static_cast<void>(Result);
  RefuseDoor("JsonObject.SelectTokens(Text, List of [JsonToken])");
}

void JsonObject::Values() {
  RefuseDoor("JsonObject.Values()");
}

::agiru::Boolean JsonObject::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("JsonObject.WriteTo(OutStream)");
}

::agiru::Boolean JsonObject::WriteTo(std::string &String) {
  static_cast<void>(String);
  RefuseDoor("JsonObject.WriteTo(Text)");
}

::agiru::Boolean JsonObject::WriteToYaml(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("JsonObject.WriteToYaml(OutStream)");
}

::agiru::Boolean JsonObject::WriteToYaml(std::string &String) {
  static_cast<void>(String);
  RefuseDoor("JsonObject.WriteToYaml(Text)");
}

::agiru::Boolean
JsonObject::WriteWithSecretsTo(const ::agiru::Dictionary<std::string, ::agiru::SecretText> &Secrets,
                               ::agiru::SecretText &Result) {
  static_cast<void>(Secrets);
  static_cast<void>(Result);
  RefuseDoor("JsonObject.WriteWithSecretsTo(Dictionary of [Text, SecretText], SecretText)");
}

::agiru::Boolean JsonObject::WriteWithSecretsTo(std::string_view Path,
                                                const ::agiru::SecretText &Secret,
                                                ::agiru::SecretText &Result) {
  static_cast<void>(Path);
  static_cast<void>(Secret);
  static_cast<void>(Result);
  RefuseDoor("JsonObject.WriteWithSecretsTo(Text, SecretText, SecretText)");
}

::agiru::JsonArray JsonToken::AsArray() {
  RefuseDoor("JsonToken.AsArray()");
}

::agiru::JsonObject JsonToken::AsObject() {
  RefuseDoor("JsonToken.AsObject()");
}

::agiru::JsonValue JsonToken::AsValue() {
  RefuseDoor("JsonToken.AsValue()");
}

::agiru::JsonToken JsonToken::Clone() {
  RefuseDoor("JsonToken.Clone()");
}

::agiru::Boolean JsonToken::IsArray() {
  RefuseDoor("JsonToken.IsArray()");
}

::agiru::Boolean JsonToken::IsObject() {
  RefuseDoor("JsonToken.IsObject()");
}

::agiru::Boolean JsonToken::IsValue() {
  RefuseDoor("JsonToken.IsValue()");
}

std::string JsonToken::Path() {
  RefuseDoor("JsonToken.Path()");
}

::agiru::Boolean JsonToken::ReadFrom(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseDoor("JsonToken.ReadFrom(InStream)");
}

::agiru::Boolean JsonToken::ReadFrom(std::string_view String) {
  static_cast<void>(String);
  RefuseDoor("JsonToken.ReadFrom(Text)");
}

::agiru::Boolean JsonToken::SelectToken(std::string_view Path, ::agiru::JsonToken &Result) {
  static_cast<void>(Path);
  static_cast<void>(Result);
  RefuseDoor("JsonToken.SelectToken(Text, JsonToken)");
}

::agiru::Boolean JsonToken::SelectTokens(std::string_view Path,
                                         ::agiru::List<::agiru::JsonToken> &Result) {
  static_cast<void>(Path);
  static_cast<void>(Result);
  RefuseDoor("JsonToken.SelectTokens(Text, List of [JsonToken])");
}

::agiru::Boolean JsonToken::WriteTo(const ::agiru::OutStream &Data) {
  static_cast<void>(Data);
  RefuseDoor("JsonToken.WriteTo(OutStream)");
}

::agiru::Boolean JsonToken::WriteTo(std::string &String) {
  static_cast<void>(String);
  RefuseDoor("JsonToken.WriteTo(Text)");
}

::agiru::BigInteger JsonValue::AsBigInteger() {
  RefuseDoor("JsonValue.AsBigInteger()");
}

::agiru::Boolean JsonValue::AsBoolean() {
  RefuseDoor("JsonValue.AsBoolean()");
}

::agiru::Byte JsonValue::AsByte() {
  RefuseDoor("JsonValue.AsByte()");
}

::agiru::Char JsonValue::AsChar() {
  RefuseDoor("JsonValue.AsChar()");
}

std::string JsonValue::AsCode() {
  RefuseDoor("JsonValue.AsCode()");
}

::agiru::Date JsonValue::AsDate() {
  RefuseDoor("JsonValue.AsDate()");
}

::agiru::DateTime JsonValue::AsDateTime() {
  RefuseDoor("JsonValue.AsDateTime()");
}

::agiru::Decimal JsonValue::AsDecimal() {
  RefuseDoor("JsonValue.AsDecimal()");
}

::agiru::Duration JsonValue::AsDuration() {
  RefuseDoor("JsonValue.AsDuration()");
}

::agiru::Integer JsonValue::AsInteger() {
  RefuseDoor("JsonValue.AsInteger()");
}

::agiru::Integer JsonValue::AsOption() {
  RefuseDoor("JsonValue.AsOption()");
}

std::string JsonValue::AsText() {
  RefuseDoor("JsonValue.AsText()");
}

::agiru::Time JsonValue::AsTime() {
  RefuseDoor("JsonValue.AsTime()");
}

::agiru::JsonToken JsonValue::AsToken() {
  RefuseDoor("JsonValue.AsToken()");
}

::agiru::JsonToken JsonValue::Clone() {
  RefuseDoor("JsonValue.Clone()");
}

::agiru::Boolean JsonValue::IsNull() {
  RefuseDoor("JsonValue.IsNull()");
}

::agiru::Boolean JsonValue::IsUndefined() {
  RefuseDoor("JsonValue.IsUndefined()");
}

std::string JsonValue::Path() {
  RefuseDoor("JsonValue.Path()");
}

::agiru::Boolean JsonValue::ReadFrom(const ::agiru::InStream &Data) {
  static_cast<void>(Data);
  RefuseDoor("JsonValue.ReadFrom(InStream)");
}

::agiru::Boolean JsonValue::ReadFrom(std::string_view Data) {
  static_cast<void>(Data);
  RefuseDoor("JsonValue.ReadFrom(Text)");
}

::agiru::Boolean JsonValue::SelectToken(std::string_view Path, ::agiru::JsonToken &Result) {
  static_cast<void>(Path);
  static_cast<void>(Result);
  RefuseDoor("JsonValue.SelectToken(Text, JsonToken)");
}

void JsonValue::SetValue(::agiru::BigInteger Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonValue.SetValue(BigInteger)");
}

void JsonValue::SetValue(::agiru::Boolean Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonValue.SetValue(Boolean)");
}

void JsonValue::SetValue(::agiru::Byte Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonValue.SetValue(Byte)");
}

void JsonValue::SetValue(::agiru::Char Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonValue.SetValue(Char)");
}

void JsonValue::SetValue(::agiru::Date Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonValue.SetValue(Date)");
}

void JsonValue::SetValue(::agiru::DateTime Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonValue.SetValue(DateTime)");
}

void JsonValue::SetValue(::agiru::Decimal Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonValue.SetValue(Decimal)");
}

void JsonValue::SetValue(::agiru::Duration Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonValue.SetValue(Duration)");
}

void JsonValue::SetValue(::agiru::Integer Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonValue.SetValue(Integer)");
}

void JsonValue::SetValue(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonValue.SetValue(Text)");
}

void JsonValue::SetValue(::agiru::Time Value) {
  static_cast<void>(Value);
  RefuseDoor("JsonValue.SetValue(Time)");
}

void JsonValue::SetValueToNull() {
  RefuseDoor("JsonValue.SetValueToNull()");
}

void JsonValue::SetValueToUndefined() {
  RefuseDoor("JsonValue.SetValueToUndefined()");
}

::agiru::Boolean JsonValue::WriteTo(const ::agiru::OutStream &Data) {
  static_cast<void>(Data);
  RefuseDoor("JsonValue.WriteTo(OutStream)");
}

::agiru::Boolean JsonValue::WriteTo(std::string &Data) {
  static_cast<void>(Data);
  RefuseDoor("JsonValue.WriteTo(Text)");
}

::agiru::Boolean Label::Contains(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("Label.Contains(Text)");
}

::agiru::Boolean Label::EndsWith(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("Label.EndsWith(Text)");
}

::agiru::Integer Label::IndexOf(std::string_view Value, ::agiru::Integer StartIndex) {
  static_cast<void>(Value);
  static_cast<void>(StartIndex);
  RefuseDoor("Label.IndexOf(Text, Integer)");
}

::agiru::Integer Label::IndexOfAny(const ::agiru::List<::agiru::Char> &Values,
                                   ::agiru::Integer StartIndex) {
  static_cast<void>(Values);
  static_cast<void>(StartIndex);
  RefuseDoor("Label.IndexOfAny(List of [Char], Integer)");
}

::agiru::Integer Label::IndexOfAny(std::string_view Values, ::agiru::Integer StartIndex) {
  static_cast<void>(Values);
  static_cast<void>(StartIndex);
  RefuseDoor("Label.IndexOfAny(Text, Integer)");
}

::agiru::Integer Label::LastIndexOf(std::string_view Value, ::agiru::Integer StartIndex) {
  static_cast<void>(Value);
  static_cast<void>(StartIndex);
  RefuseDoor("Label.LastIndexOf(Text, Integer)");
}

std::string Label::PadLeft(::agiru::Integer Count, ::agiru::Char Char) {
  static_cast<void>(Count);
  static_cast<void>(Char);
  RefuseDoor("Label.PadLeft(Integer, Char)");
}

std::string Label::PadRight(::agiru::Integer Count, ::agiru::Char Char) {
  static_cast<void>(Count);
  static_cast<void>(Char);
  RefuseDoor("Label.PadRight(Integer, Char)");
}

std::string Label::Remove(::agiru::Integer StartIndex, ::agiru::Integer Count) {
  static_cast<void>(StartIndex);
  static_cast<void>(Count);
  RefuseDoor("Label.Remove(Integer, Integer)");
}

std::string Label::Replace(std::string_view OldValue, std::string_view NewValue) {
  static_cast<void>(OldValue);
  static_cast<void>(NewValue);
  RefuseDoor("Label.Replace(Text, Text)");
}

void Label::Split(const ::agiru::List<::agiru::Char> &Separators) {
  static_cast<void>(Separators);
  RefuseDoor("Label.Split(List of [Char])");
}

void Label::Split(const ::agiru::List<std::string> &Separators) {
  static_cast<void>(Separators);
  RefuseDoor("Label.Split(List of [Text])");
}

void Label::Split(std::string_view Separators) {
  static_cast<void>(Separators);
  RefuseDoor("Label.Split(Text)");
}

::agiru::Boolean Label::StartsWith(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("Label.StartsWith(Text)");
}

std::string Label::Substring(::agiru::Integer StartIndex, ::agiru::Integer Count) {
  static_cast<void>(StartIndex);
  static_cast<void>(Count);
  RefuseDoor("Label.Substring(Integer, Integer)");
}

std::string Label::ToLower() {
  RefuseDoor("Label.ToLower()");
}

std::string Label::ToUpper() {
  RefuseDoor("Label.ToUpper()");
}

std::string Label::Trim() {
  RefuseDoor("Label.Trim()");
}

std::string Label::TrimEnd(std::string_view Chars) {
  static_cast<void>(Chars);
  RefuseDoor("Label.TrimEnd(Text)");
}

std::string Label::TrimStart(std::string_view Chars) {
  static_cast<void>(Chars);
  RefuseDoor("Label.TrimStart(Text)");
}

void NavApp::DeleteArchiveData(::agiru::Integer TableNo) {
  static_cast<void>(TableNo);
  RefuseDoor("NavApp.DeleteArchiveData(Integer)");
}

::agiru::Boolean NavApp::GetArchiveRecordRef(::agiru::Integer TableNo,
                                             ::agiru::RecordRef &RecordRef) {
  static_cast<void>(TableNo);
  static_cast<void>(RecordRef);
  RefuseDoor("NavApp.GetArchiveRecordRef(Integer, RecordRef)");
}

std::string NavApp::GetArchiveVersion() {
  RefuseDoor("NavApp.GetArchiveVersion()");
}

void NavApp::GetCallerCallstackModuleInfos() {
  RefuseDoor("NavApp.GetCallerCallstackModuleInfos()");
}

::agiru::Boolean NavApp::GetCallerModuleInfo(::agiru::ModuleInfo &Info) {
  static_cast<void>(Info);
  RefuseDoor("NavApp.GetCallerModuleInfo(ModuleInfo)");
}

void NavApp::GetCallstackModuleInfos() {
  RefuseDoor("NavApp.GetCallstackModuleInfos()");
}

::agiru::Boolean NavApp::GetCurrentModuleInfo(::agiru::ModuleInfo &Info) {
  static_cast<void>(Info);
  RefuseDoor("NavApp.GetCurrentModuleInfo(ModuleInfo)");
}

::agiru::Boolean NavApp::GetModuleInfo(::agiru::Guid AppId, ::agiru::ModuleInfo &Info) {
  static_cast<void>(AppId);
  static_cast<void>(Info);
  RefuseDoor("NavApp.GetModuleInfo(Guid, ModuleInfo)");
}

void NavApp::GetResource(std::string_view ResourceName,
                         ::agiru::InStream &ResourceStream,
                         const ::agiru::TextEncoding &Encoding) {
  static_cast<void>(ResourceName);
  static_cast<void>(ResourceStream);
  static_cast<void>(Encoding);
  RefuseDoor("NavApp.GetResource(Text, InStream, TextEncoding)");
}

::agiru::JsonObject NavApp::GetResourceAsJson(std::string_view ResourceName,
                                              const ::agiru::TextEncoding &Encoding) {
  static_cast<void>(ResourceName);
  static_cast<void>(Encoding);
  RefuseDoor("NavApp.GetResourceAsJson(Text, TextEncoding)");
}

std::string NavApp::GetResourceAsText(std::string_view ResourceName,
                                      const ::agiru::TextEncoding &Encoding) {
  static_cast<void>(ResourceName);
  static_cast<void>(Encoding);
  RefuseDoor("NavApp.GetResourceAsText(Text, TextEncoding)");
}

::agiru::Boolean NavApp::IsEntitled(std::string_view Id, ::agiru::Guid AppId) {
  static_cast<void>(Id);
  static_cast<void>(AppId);
  RefuseDoor("NavApp.IsEntitled(Text, Guid)");
}

::agiru::Boolean NavApp::IsInstalling() {
  RefuseDoor("NavApp.IsInstalling()");
}

::agiru::Boolean NavApp::IsUnlicensed(::agiru::Guid AppId) {
  static_cast<void>(AppId);
  RefuseDoor("NavApp.IsUnlicensed(Guid)");
}

void NavApp::ListResources(std::string_view Filter) {
  static_cast<void>(Filter);
  RefuseDoor("NavApp.ListResources(Text)");
}

void NavApp::LoadPackageData(::agiru::Integer TableNo) {
  static_cast<void>(TableNo);
  RefuseDoor("NavApp.LoadPackageData(Integer)");
}

::agiru::Boolean NavApp::RestoreArchiveData(::agiru::Integer TableNo, ::agiru::Boolean RunTrigger) {
  static_cast<void>(TableNo);
  static_cast<void>(RunTrigger);
  RefuseDoor("NavApp.RestoreArchiveData(Integer, Boolean)");
}

::agiru::BigInteger NumberSequence::Current(std::string_view Name,
                                            ::agiru::Boolean CompanySpecific) {
  static_cast<void>(Name);
  static_cast<void>(CompanySpecific);
  RefuseDoor("NumberSequence.Current(Text, Boolean)");
}

void NumberSequence::Delete(std::string_view Name, ::agiru::Boolean CompanySpecific) {
  static_cast<void>(Name);
  static_cast<void>(CompanySpecific);
  RefuseDoor("NumberSequence.Delete(Text, Boolean)");
}

::agiru::Boolean NumberSequence::Exists(std::string_view Name, ::agiru::Boolean CompanySpecific) {
  static_cast<void>(Name);
  static_cast<void>(CompanySpecific);
  RefuseDoor("NumberSequence.Exists(Text, Boolean)");
}

void NumberSequence::Insert(std::string_view Name,
                            ::agiru::BigInteger Seed,
                            ::agiru::BigInteger Increment,
                            ::agiru::Boolean CompanySpecific) {
  static_cast<void>(Name);
  static_cast<void>(Seed);
  static_cast<void>(Increment);
  static_cast<void>(CompanySpecific);
  RefuseDoor("NumberSequence.Insert(Text, BigInteger, BigInteger, Boolean)");
}

::agiru::BigInteger NumberSequence::Next(std::string_view Name, ::agiru::Boolean CompanySpecific) {
  static_cast<void>(Name);
  static_cast<void>(CompanySpecific);
  RefuseDoor("NumberSequence.Next(Text, Boolean)");
}

::agiru::BigInteger NumberSequence::Range(std::string_view Name,
                                          ::agiru::Integer Count,
                                          ::agiru::BigInteger &Increment,
                                          ::agiru::Boolean CompanySpecific) {
  static_cast<void>(Name);
  static_cast<void>(Count);
  static_cast<void>(Increment);
  static_cast<void>(CompanySpecific);
  RefuseDoor("NumberSequence.Range(Text, Integer, BigInteger, Boolean)");
}

::agiru::BigInteger NumberSequence::Range(std::string_view Name,
                                          ::agiru::Integer Count,
                                          ::agiru::Boolean CompanySpecific) {
  static_cast<void>(Name);
  static_cast<void>(Count);
  static_cast<void>(CompanySpecific);
  RefuseDoor("NumberSequence.Range(Text, Integer, Boolean)");
}

void NumberSequence::Restart(std::string_view Name,
                             ::agiru::BigInteger Seed,
                             ::agiru::Boolean CompanySpecific) {
  static_cast<void>(Name);
  static_cast<void>(Seed);
  static_cast<void>(CompanySpecific);
  RefuseDoor("NumberSequence.Restart(Text, BigInteger, Boolean)");
}

std::string ProductName::Full() {
  RefuseDoor("ProductName.Full()");
}

std::string ProductName::Marketing() {
  RefuseDoor("ProductName.Marketing()");
}

std::string ProductName::Short() {
  RefuseDoor("ProductName.Short()");
}

::agiru::BigInteger SessionInformation::AITokensUsed() {
  RefuseDoor("SessionInformation.AITokensUsed()");
}

std::string SessionInformation::Callstack() {
  RefuseDoor("SessionInformation.Callstack()");
}

::agiru::BigInteger SessionInformation::SqlRowsRead() {
  RefuseDoor("SessionInformation.SqlRowsRead()");
}

::agiru::BigInteger SessionInformation::SqlStatementsExecuted() {
  RefuseDoor("SessionInformation.SqlStatementsExecuted()");
}

std::string SessionSettings::Company(std::string_view NewCompanyName) {
  static_cast<void>(NewCompanyName);
  RefuseDoor("SessionSettings.Company(Text)");
}

void SessionSettings::Init() {
  RefuseDoor("SessionSettings.Init()");
}

::agiru::Integer SessionSettings::LanguageId(::agiru::Integer NewLanguageId) {
  static_cast<void>(NewLanguageId);
  RefuseDoor("SessionSettings.LanguageId(Integer)");
}

::agiru::Integer SessionSettings::LocaleId(::agiru::Integer NewLocaleId) {
  static_cast<void>(NewLocaleId);
  RefuseDoor("SessionSettings.LocaleId(Integer)");
}

::agiru::Guid SessionSettings::ProfileAppId(::agiru::Guid NewProfileAppId) {
  static_cast<void>(NewProfileAppId);
  RefuseDoor("SessionSettings.ProfileAppId(Guid)");
}

std::string SessionSettings::ProfileId(std::string_view NewProfileId) {
  static_cast<void>(NewProfileId);
  RefuseDoor("SessionSettings.ProfileId(Text)");
}

::agiru::Boolean SessionSettings::ProfileSystemScope(::agiru::Boolean NewProfileScope) {
  static_cast<void>(NewProfileScope);
  RefuseDoor("SessionSettings.ProfileSystemScope(Boolean)");
}

void SessionSettings::RequestSessionUpdate(::agiru::Boolean saveSettings) {
  static_cast<void>(saveSettings);
  RefuseDoor("SessionSettings.RequestSessionUpdate(Boolean)");
}

std::string SessionSettings::TimeZone(std::string_view NewTimeZone) {
  static_cast<void>(NewTimeZone);
  RefuseDoor("SessionSettings.TimeZone(Text)");
}

::agiru::Boolean TaskScheduler::CancelTask(::agiru::Guid Task) {
  static_cast<void>(Task);
  RefuseDoor("TaskScheduler.CancelTask(Guid)");
}

::agiru::Boolean TaskScheduler::CanCreateTask() {
  RefuseDoor("TaskScheduler.CanCreateTask()");
}

::agiru::Guid TaskScheduler::CreateTask(::agiru::Integer CodeunitId,
                                        ::agiru::Integer FailureCodeunitId,
                                        ::agiru::Boolean IsReady,
                                        std::string_view Company,
                                        ::agiru::DateTime NotBefore,
                                        ::agiru::RecordId RecordID,
                                        ::agiru::Duration Timeout) {
  static_cast<void>(CodeunitId);
  static_cast<void>(FailureCodeunitId);
  static_cast<void>(IsReady);
  static_cast<void>(Company);
  static_cast<void>(NotBefore);
  static_cast<void>(RecordID);
  static_cast<void>(Timeout);
  RefuseDoor(
      "TaskScheduler.CreateTask(Integer, Integer, Boolean, Text, DateTime, RecordId, Duration)");
}

::agiru::Guid TaskScheduler::CreateTask(::agiru::Integer CodeunitId,
                                        ::agiru::Integer FailureCodeunitId,
                                        ::agiru::Boolean IsReady,
                                        std::string_view Company,
                                        ::agiru::DateTime NotBefore,
                                        ::agiru::RecordId RecordID) {
  static_cast<void>(CodeunitId);
  static_cast<void>(FailureCodeunitId);
  static_cast<void>(IsReady);
  static_cast<void>(Company);
  static_cast<void>(NotBefore);
  static_cast<void>(RecordID);
  RefuseDoor("TaskScheduler.CreateTask(Integer, Integer, Boolean, Text, DateTime, RecordId)");
}

::agiru::Boolean TaskScheduler::SetTaskReady(::agiru::Guid Task, ::agiru::DateTime NotBefore) {
  static_cast<void>(Task);
  static_cast<void>(NotBefore);
  RefuseDoor("TaskScheduler.SetTaskReady(Guid, DateTime)");
}

::agiru::Boolean TaskScheduler::TaskExists(::agiru::Guid Task) {
  static_cast<void>(Task);
  RefuseDoor("TaskScheduler.TaskExists(Guid)");
}

::agiru::Boolean TestHttpRequestMessage::HasSecretUri() {
  RefuseDoor("TestHttpRequestMessage.HasSecretUri()");
}

std::string TestHttpRequestMessage::Path() {
  RefuseDoor("TestHttpRequestMessage.Path()");
}

void TestHttpRequestMessage::QueryParameters() {
  RefuseDoor("TestHttpRequestMessage.QueryParameters()");
}

::agiru::HttpRequestType TestHttpRequestMessage::RequestType() {
  RefuseDoor("TestHttpRequestMessage.RequestType()");
}

::agiru::HttpContent TestHttpResponseMessage::Content() {
  RefuseDoor("TestHttpResponseMessage.Content()");
}

::agiru::HttpHeaders TestHttpResponseMessage::Headers() {
  RefuseDoor("TestHttpResponseMessage.Headers()");
}

::agiru::Integer TestHttpResponseMessage::HttpStatusCode(::agiru::Integer SetStatusCode) {
  static_cast<void>(SetStatusCode);
  RefuseDoor("TestHttpResponseMessage.HttpStatusCode(Integer)");
}

::agiru::Boolean
TestHttpResponseMessage::IsBlockedByEnvironment(::agiru::Boolean SetIsBlockedByEnvironment) {
  static_cast<void>(SetIsBlockedByEnvironment);
  RefuseDoor("TestHttpResponseMessage.IsBlockedByEnvironment(Boolean)");
}

::agiru::Boolean
TestHttpResponseMessage::IsSuccessfulRequest(::agiru::Boolean SetIsSuccessfulRequest) {
  static_cast<void>(SetIsSuccessfulRequest);
  RefuseDoor("TestHttpResponseMessage.IsSuccessfulRequest(Boolean)");
}

std::string TestHttpResponseMessage::ReasonPhrase(std::string_view SetReasonPhrase) {
  static_cast<void>(SetReasonPhrase);
  RefuseDoor("TestHttpResponseMessage.ReasonPhrase(Text)");
}

::agiru::Boolean TextBuilder::Append(std::string_view Text) {
  static_cast<void>(Text);
  RefuseDoor("TextBuilder.Append(Text)");
}

::agiru::Boolean TextBuilder::AppendLine(std::string_view Text) {
  static_cast<void>(Text);
  RefuseDoor("TextBuilder.AppendLine(Text)");
}

::agiru::Integer TextBuilder::Capacity(::agiru::Integer NewCapacity) {
  static_cast<void>(NewCapacity);
  RefuseDoor("TextBuilder.Capacity(Integer)");
}

void TextBuilder::Clear() {
  RefuseDoor("TextBuilder.Clear()");
}

::agiru::Boolean TextBuilder::EnsureCapacity(::agiru::Integer NewCapacity) {
  static_cast<void>(NewCapacity);
  RefuseDoor("TextBuilder.EnsureCapacity(Integer)");
}

::agiru::Boolean TextBuilder::Insert(::agiru::Integer Position, std::string_view Text) {
  static_cast<void>(Position);
  static_cast<void>(Text);
  RefuseDoor("TextBuilder.Insert(Integer, Text)");
}

::agiru::Integer TextBuilder::Length(::agiru::Integer NewLength) {
  static_cast<void>(NewLength);
  RefuseDoor("TextBuilder.Length(Integer)");
}

::agiru::Integer TextBuilder::MaxCapacity() {
  RefuseDoor("TextBuilder.MaxCapacity()");
}

::agiru::Boolean TextBuilder::Remove(::agiru::Integer StartIndex, ::agiru::Integer Count) {
  static_cast<void>(StartIndex);
  static_cast<void>(Count);
  RefuseDoor("TextBuilder.Remove(Integer, Integer)");
}

::agiru::Boolean TextBuilder::Replace(std::string_view OldText,
                                      std::string_view NewText,
                                      ::agiru::Integer StartIndex,
                                      ::agiru::Integer Count) {
  static_cast<void>(OldText);
  static_cast<void>(NewText);
  static_cast<void>(StartIndex);
  static_cast<void>(Count);
  RefuseDoor("TextBuilder.Replace(Text, Text, Integer, Integer)");
}

::agiru::Boolean TextBuilder::Replace(std::string_view OldText, std::string_view NewText) {
  static_cast<void>(OldText);
  static_cast<void>(NewText);
  RefuseDoor("TextBuilder.Replace(Text, Text)");
}

std::string TextBuilder::ToText() {
  RefuseDoor("TextBuilder.ToText()");
}

std::string TextBuilder::ToText(::agiru::Integer StartIndex, ::agiru::Integer Count) {
  static_cast<void>(StartIndex);
  static_cast<void>(Count);
  RefuseDoor("TextBuilder.ToText(Integer, Integer)");
}

::agiru::Boolean TextConst::Contains(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("TextConst.Contains(Text)");
}

::agiru::Boolean TextConst::EndsWith(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("TextConst.EndsWith(Text)");
}

::agiru::Integer TextConst::IndexOf(std::string_view Value, ::agiru::Integer StartIndex) {
  static_cast<void>(Value);
  static_cast<void>(StartIndex);
  RefuseDoor("TextConst.IndexOf(Text, Integer)");
}

::agiru::Integer TextConst::IndexOfAny(const ::agiru::List<::agiru::Char> &Values,
                                       ::agiru::Integer StartIndex) {
  static_cast<void>(Values);
  static_cast<void>(StartIndex);
  RefuseDoor("TextConst.IndexOfAny(List of [Char], Integer)");
}

::agiru::Integer TextConst::IndexOfAny(std::string_view Values, ::agiru::Integer StartIndex) {
  static_cast<void>(Values);
  static_cast<void>(StartIndex);
  RefuseDoor("TextConst.IndexOfAny(Text, Integer)");
}

::agiru::Integer TextConst::LastIndexOf(std::string_view Value, ::agiru::Integer StartIndex) {
  static_cast<void>(Value);
  static_cast<void>(StartIndex);
  RefuseDoor("TextConst.LastIndexOf(Text, Integer)");
}

std::string TextConst::PadLeft(::agiru::Integer Count, ::agiru::Char Char) {
  static_cast<void>(Count);
  static_cast<void>(Char);
  RefuseDoor("TextConst.PadLeft(Integer, Char)");
}

std::string TextConst::PadRight(::agiru::Integer Count, ::agiru::Char Char) {
  static_cast<void>(Count);
  static_cast<void>(Char);
  RefuseDoor("TextConst.PadRight(Integer, Char)");
}

std::string TextConst::Remove(::agiru::Integer StartIndex, ::agiru::Integer Count) {
  static_cast<void>(StartIndex);
  static_cast<void>(Count);
  RefuseDoor("TextConst.Remove(Integer, Integer)");
}

std::string TextConst::Replace(std::string_view OldValue, std::string_view NewValue) {
  static_cast<void>(OldValue);
  static_cast<void>(NewValue);
  RefuseDoor("TextConst.Replace(Text, Text)");
}

void TextConst::Split(const ::agiru::List<::agiru::Char> &Separators) {
  static_cast<void>(Separators);
  RefuseDoor("TextConst.Split(List of [Char])");
}

void TextConst::Split(const ::agiru::List<std::string> &Separators) {
  static_cast<void>(Separators);
  RefuseDoor("TextConst.Split(List of [Text])");
}

void TextConst::Split(std::string_view Separators) {
  static_cast<void>(Separators);
  RefuseDoor("TextConst.Split(Text)");
}

::agiru::Boolean TextConst::StartsWith(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("TextConst.StartsWith(Text)");
}

std::string TextConst::Substring(::agiru::Integer StartIndex, ::agiru::Integer Count) {
  static_cast<void>(StartIndex);
  static_cast<void>(Count);
  RefuseDoor("TextConst.Substring(Integer, Integer)");
}

std::string TextConst::ToLower() {
  RefuseDoor("TextConst.ToLower()");
}

std::string TextConst::ToUpper() {
  RefuseDoor("TextConst.ToUpper()");
}

std::string TextConst::Trim() {
  RefuseDoor("TextConst.Trim()");
}

std::string TextConst::TrimEnd(std::string_view Chars) {
  static_cast<void>(Chars);
  RefuseDoor("TextConst.TrimEnd(Text)");
}

std::string TextConst::TrimStart(std::string_view Chars) {
  static_cast<void>(Chars);
  RefuseDoor("TextConst.TrimStart(Text)");
}

::agiru::Boolean WebServiceActionContext::AddEntityKey(::agiru::Integer FieldId,
                                                       const ::agiru::Variant &FieldValue) {
  static_cast<void>(FieldId);
  static_cast<void>(FieldValue);
  RefuseDoor("WebServiceActionContext.AddEntityKey(Integer, Any)");
}

::agiru::Integer WebServiceActionContext::GetObjectId() {
  RefuseDoor("WebServiceActionContext.GetObjectId()");
}

::agiru::ObjectType WebServiceActionContext::GetObjectType() {
  RefuseDoor("WebServiceActionContext.GetObjectType()");
}

::agiru::WebServiceActionResultCode WebServiceActionContext::GetResultCode() {
  RefuseDoor("WebServiceActionContext.GetResultCode()");
}

void WebServiceActionContext::SetObjectId(::agiru::Integer ObjectId) {
  static_cast<void>(ObjectId);
  RefuseDoor("WebServiceActionContext.SetObjectId(Integer)");
}

void WebServiceActionContext::SetObjectType(const ::agiru::ObjectType &ObjectType) {
  static_cast<void>(ObjectType);
  RefuseDoor("WebServiceActionContext.SetObjectType(ObjectType)");
}

void WebServiceActionContext::SetResultCode(const ::agiru::WebServiceActionResultCode &ResultCode) {
  static_cast<void>(ResultCode);
  RefuseDoor("WebServiceActionContext.SetResultCode(WebServiceActionResultCode)");
}

::agiru::Boolean XmlAttribute::AddAfterSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlAttribute.AddAfterSelf(Any)");
}

::agiru::Boolean XmlAttribute::AddBeforeSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlAttribute.AddBeforeSelf(Any)");
}

::agiru::XmlNode XmlAttribute::AsXmlNode() {
  RefuseDoor("XmlAttribute.AsXmlNode()");
}

::agiru::XmlAttribute XmlAttribute::Create(std::string_view Name, std::string_view Value) {
  static_cast<void>(Name);
  static_cast<void>(Value);
  RefuseDoor("XmlAttribute.Create(Text, Text)");
}

::agiru::XmlAttribute XmlAttribute::Create(std::string_view LocalName,
                                           std::string_view NamespaceUri,
                                           std::string_view Value) {
  static_cast<void>(LocalName);
  static_cast<void>(NamespaceUri);
  static_cast<void>(Value);
  RefuseDoor("XmlAttribute.Create(Text, Text, Text)");
}

::agiru::XmlAttribute XmlAttribute::CreateNamespaceDeclaration(std::string_view Prefix,
                                                               std::string_view NamespaceUri) {
  static_cast<void>(Prefix);
  static_cast<void>(NamespaceUri);
  RefuseDoor("XmlAttribute.CreateNamespaceDeclaration(Text, Text)");
}

::agiru::Boolean XmlAttribute::GetDocument(::agiru::XmlDocument &Document) {
  static_cast<void>(Document);
  RefuseDoor("XmlAttribute.GetDocument(XmlDocument)");
}

::agiru::Boolean XmlAttribute::GetParent(::agiru::XmlElement &Parent) {
  static_cast<void>(Parent);
  RefuseDoor("XmlAttribute.GetParent(XmlElement)");
}

::agiru::Boolean XmlAttribute::IsNamespaceDeclaration() {
  RefuseDoor("XmlAttribute.IsNamespaceDeclaration()");
}

std::string XmlAttribute::LocalName() {
  RefuseDoor("XmlAttribute.LocalName()");
}

std::string XmlAttribute::Name() {
  RefuseDoor("XmlAttribute.Name()");
}

std::string XmlAttribute::NamespacePrefix() {
  RefuseDoor("XmlAttribute.NamespacePrefix()");
}

std::string XmlAttribute::NamespaceUri() {
  RefuseDoor("XmlAttribute.NamespaceUri()");
}

::agiru::Boolean XmlAttribute::Remove() {
  RefuseDoor("XmlAttribute.Remove()");
}

::agiru::Boolean XmlAttribute::ReplaceWith(const ::agiru::Variant &Node) {
  static_cast<void>(Node);
  RefuseDoor("XmlAttribute.ReplaceWith(Any)");
}

::agiru::Boolean XmlAttribute::SelectNodes(std::string_view XPath,
                                           const ::agiru::XmlNamespaceManager &NamespaceManager,
                                           ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(NodeList);
  RefuseDoor("XmlAttribute.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)");
}

::agiru::Boolean XmlAttribute::SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NodeList);
  RefuseDoor("XmlAttribute.SelectNodes(Text, XmlNodeList)");
}

::agiru::Boolean
XmlAttribute::SelectSingleNode(std::string_view XPath,
                               const ::agiru::XmlNamespaceManager &NamespaceManager,
                               ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(Node);
  RefuseDoor("XmlAttribute.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)");
}

::agiru::Boolean XmlAttribute::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(Node);
  RefuseDoor("XmlAttribute.SelectSingleNode(Text, XmlNode)");
}

std::string XmlAttribute::Value(std::string_view NewValue) {
  static_cast<void>(NewValue);
  RefuseDoor("XmlAttribute.Value(Text)");
}

::agiru::Boolean XmlAttribute::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("XmlAttribute.WriteTo(OutStream)");
}

::agiru::Boolean XmlAttribute::WriteTo(std::string &Text) {
  static_cast<void>(Text);
  RefuseDoor("XmlAttribute.WriteTo(Text)");
}

::agiru::Boolean XmlAttribute::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                       const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  static_cast<void>(OutStream);
  RefuseDoor("XmlAttribute.WriteTo(XmlWriteOptions, OutStream)");
}

::agiru::Boolean XmlAttribute::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                       std::string &Text) {
  static_cast<void>(WriteOptions);
  static_cast<void>(Text);
  RefuseDoor("XmlAttribute.WriteTo(XmlWriteOptions, Text)");
}

::agiru::Integer XmlAttributeCollection::Count() {
  RefuseDoor("XmlAttributeCollection.Count()");
}

::agiru::Boolean XmlAttributeCollection::Get(::agiru::Integer Index,
                                             ::agiru::XmlAttribute &Result) {
  static_cast<void>(Index);
  static_cast<void>(Result);
  RefuseDoor("XmlAttributeCollection.Get(Integer, XmlAttribute)");
}

::agiru::Boolean XmlAttributeCollection::Get(std::string_view LocalName,
                                             std::string_view NamespaceUri,
                                             ::agiru::XmlAttribute &Result) {
  static_cast<void>(LocalName);
  static_cast<void>(NamespaceUri);
  static_cast<void>(Result);
  RefuseDoor("XmlAttributeCollection.Get(Text, Text, XmlAttribute)");
}

::agiru::Boolean XmlAttributeCollection::Get(std::string_view Name, ::agiru::XmlAttribute &Result) {
  static_cast<void>(Name);
  static_cast<void>(Result);
  RefuseDoor("XmlAttributeCollection.Get(Text, XmlAttribute)");
}

void XmlAttributeCollection::Remove(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("XmlAttributeCollection.Remove(Text)");
}

void XmlAttributeCollection::Remove(std::string_view LocalName, std::string_view NamespaceUri) {
  static_cast<void>(LocalName);
  static_cast<void>(NamespaceUri);
  RefuseDoor("XmlAttributeCollection.Remove(Text, Text)");
}

void XmlAttributeCollection::Remove(const ::agiru::XmlAttribute &Attribute) {
  static_cast<void>(Attribute);
  RefuseDoor("XmlAttributeCollection.Remove(XmlAttribute)");
}

void XmlAttributeCollection::RemoveAll() {
  RefuseDoor("XmlAttributeCollection.RemoveAll()");
}

void XmlAttributeCollection::Set(std::string_view Name, std::string_view Value) {
  static_cast<void>(Name);
  static_cast<void>(Value);
  RefuseDoor("XmlAttributeCollection.Set(Text, Text)");
}

void XmlAttributeCollection::Set(std::string_view LocalName,
                                 std::string_view NamespaceUri,
                                 std::string_view Value) {
  static_cast<void>(LocalName);
  static_cast<void>(NamespaceUri);
  static_cast<void>(Value);
  RefuseDoor("XmlAttributeCollection.Set(Text, Text, Text)");
}

::agiru::Boolean XmlCData::AddAfterSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlCData.AddAfterSelf(Any)");
}

::agiru::Boolean XmlCData::AddBeforeSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlCData.AddBeforeSelf(Any)");
}

::agiru::XmlNode XmlCData::AsXmlNode() {
  RefuseDoor("XmlCData.AsXmlNode()");
}

::agiru::XmlCData XmlCData::Create(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("XmlCData.Create(Text)");
}

::agiru::Boolean XmlCData::GetDocument(::agiru::XmlDocument &Document) {
  static_cast<void>(Document);
  RefuseDoor("XmlCData.GetDocument(XmlDocument)");
}

::agiru::Boolean XmlCData::GetParent(::agiru::XmlElement &Parent) {
  static_cast<void>(Parent);
  RefuseDoor("XmlCData.GetParent(XmlElement)");
}

::agiru::Boolean XmlCData::Remove() {
  RefuseDoor("XmlCData.Remove()");
}

::agiru::Boolean XmlCData::ReplaceWith(const ::agiru::Variant &Node) {
  static_cast<void>(Node);
  RefuseDoor("XmlCData.ReplaceWith(Any)");
}

::agiru::Boolean XmlCData::SelectNodes(std::string_view XPath,
                                       const ::agiru::XmlNamespaceManager &NamespaceManager,
                                       ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(NodeList);
  RefuseDoor("XmlCData.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)");
}

::agiru::Boolean XmlCData::SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NodeList);
  RefuseDoor("XmlCData.SelectNodes(Text, XmlNodeList)");
}

::agiru::Boolean XmlCData::SelectSingleNode(std::string_view XPath,
                                            const ::agiru::XmlNamespaceManager &NamespaceManager,
                                            ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(Node);
  RefuseDoor("XmlCData.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)");
}

::agiru::Boolean XmlCData::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(Node);
  RefuseDoor("XmlCData.SelectSingleNode(Text, XmlNode)");
}

std::string XmlCData::Value(std::string_view NewValue) {
  static_cast<void>(NewValue);
  RefuseDoor("XmlCData.Value(Text)");
}

::agiru::Boolean XmlCData::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("XmlCData.WriteTo(OutStream)");
}

::agiru::Boolean XmlCData::WriteTo(std::string &Text) {
  static_cast<void>(Text);
  RefuseDoor("XmlCData.WriteTo(Text)");
}

::agiru::Boolean XmlCData::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                   const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  static_cast<void>(OutStream);
  RefuseDoor("XmlCData.WriteTo(XmlWriteOptions, OutStream)");
}

::agiru::Boolean XmlCData::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                   std::string &Text) {
  static_cast<void>(WriteOptions);
  static_cast<void>(Text);
  RefuseDoor("XmlCData.WriteTo(XmlWriteOptions, Text)");
}

::agiru::Boolean XmlComment::AddAfterSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlComment.AddAfterSelf(Any)");
}

::agiru::Boolean XmlComment::AddBeforeSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlComment.AddBeforeSelf(Any)");
}

::agiru::XmlNode XmlComment::AsXmlNode() {
  RefuseDoor("XmlComment.AsXmlNode()");
}

::agiru::XmlComment XmlComment::Create(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("XmlComment.Create(Text)");
}

::agiru::Boolean XmlComment::GetDocument(::agiru::XmlDocument &Document) {
  static_cast<void>(Document);
  RefuseDoor("XmlComment.GetDocument(XmlDocument)");
}

::agiru::Boolean XmlComment::GetParent(::agiru::XmlElement &Parent) {
  static_cast<void>(Parent);
  RefuseDoor("XmlComment.GetParent(XmlElement)");
}

::agiru::Boolean XmlComment::Remove() {
  RefuseDoor("XmlComment.Remove()");
}

::agiru::Boolean XmlComment::ReplaceWith(const ::agiru::Variant &Node) {
  static_cast<void>(Node);
  RefuseDoor("XmlComment.ReplaceWith(Any)");
}

::agiru::Boolean XmlComment::SelectNodes(std::string_view XPath,
                                         const ::agiru::XmlNamespaceManager &NamespaceManager,
                                         ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(NodeList);
  RefuseDoor("XmlComment.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)");
}

::agiru::Boolean XmlComment::SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NodeList);
  RefuseDoor("XmlComment.SelectNodes(Text, XmlNodeList)");
}

::agiru::Boolean XmlComment::SelectSingleNode(std::string_view XPath,
                                              const ::agiru::XmlNamespaceManager &NamespaceManager,
                                              ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(Node);
  RefuseDoor("XmlComment.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)");
}

::agiru::Boolean XmlComment::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(Node);
  RefuseDoor("XmlComment.SelectSingleNode(Text, XmlNode)");
}

std::string XmlComment::Value(std::string_view NewValue) {
  static_cast<void>(NewValue);
  RefuseDoor("XmlComment.Value(Text)");
}

::agiru::Boolean XmlComment::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("XmlComment.WriteTo(OutStream)");
}

::agiru::Boolean XmlComment::WriteTo(std::string &Text) {
  static_cast<void>(Text);
  RefuseDoor("XmlComment.WriteTo(Text)");
}

::agiru::Boolean XmlComment::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                     const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  static_cast<void>(OutStream);
  RefuseDoor("XmlComment.WriteTo(XmlWriteOptions, OutStream)");
}

::agiru::Boolean XmlComment::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                     std::string &Text) {
  static_cast<void>(WriteOptions);
  static_cast<void>(Text);
  RefuseDoor("XmlComment.WriteTo(XmlWriteOptions, Text)");
}

::agiru::Boolean XmlDeclaration::AddAfterSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlDeclaration.AddAfterSelf(Any)");
}

::agiru::Boolean XmlDeclaration::AddBeforeSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlDeclaration.AddBeforeSelf(Any)");
}

::agiru::XmlNode XmlDeclaration::AsXmlNode() {
  RefuseDoor("XmlDeclaration.AsXmlNode()");
}

::agiru::XmlDeclaration XmlDeclaration::Create(std::string_view Version,
                                               std::string_view Encoding,
                                               std::string_view Standalone) {
  static_cast<void>(Version);
  static_cast<void>(Encoding);
  static_cast<void>(Standalone);
  RefuseDoor("XmlDeclaration.Create(Text, Text, Text)");
}

std::string XmlDeclaration::Encoding(std::string_view NewValue) {
  static_cast<void>(NewValue);
  RefuseDoor("XmlDeclaration.Encoding(Text)");
}

::agiru::Boolean XmlDeclaration::GetDocument(::agiru::XmlDocument &Document) {
  static_cast<void>(Document);
  RefuseDoor("XmlDeclaration.GetDocument(XmlDocument)");
}

::agiru::Boolean XmlDeclaration::GetParent(::agiru::XmlElement &Parent) {
  static_cast<void>(Parent);
  RefuseDoor("XmlDeclaration.GetParent(XmlElement)");
}

::agiru::Boolean XmlDeclaration::Remove() {
  RefuseDoor("XmlDeclaration.Remove()");
}

::agiru::Boolean XmlDeclaration::ReplaceWith(const ::agiru::Variant &Node) {
  static_cast<void>(Node);
  RefuseDoor("XmlDeclaration.ReplaceWith(Any)");
}

::agiru::Boolean XmlDeclaration::SelectNodes(std::string_view XPath,
                                             const ::agiru::XmlNamespaceManager &NamespaceManager,
                                             ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(NodeList);
  RefuseDoor("XmlDeclaration.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)");
}

::agiru::Boolean XmlDeclaration::SelectNodes(std::string_view XPath,
                                             ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NodeList);
  RefuseDoor("XmlDeclaration.SelectNodes(Text, XmlNodeList)");
}

::agiru::Boolean
XmlDeclaration::SelectSingleNode(std::string_view XPath,
                                 const ::agiru::XmlNamespaceManager &NamespaceManager,
                                 ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(Node);
  RefuseDoor("XmlDeclaration.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)");
}

::agiru::Boolean XmlDeclaration::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(Node);
  RefuseDoor("XmlDeclaration.SelectSingleNode(Text, XmlNode)");
}

std::string XmlDeclaration::Standalone(std::string_view NewValue) {
  static_cast<void>(NewValue);
  RefuseDoor("XmlDeclaration.Standalone(Text)");
}

std::string XmlDeclaration::Version(std::string_view NewValue) {
  static_cast<void>(NewValue);
  RefuseDoor("XmlDeclaration.Version(Text)");
}

::agiru::Boolean XmlDeclaration::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("XmlDeclaration.WriteTo(OutStream)");
}

::agiru::Boolean XmlDeclaration::WriteTo(std::string &Text) {
  static_cast<void>(Text);
  RefuseDoor("XmlDeclaration.WriteTo(Text)");
}

::agiru::Boolean XmlDeclaration::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                         const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  static_cast<void>(OutStream);
  RefuseDoor("XmlDeclaration.WriteTo(XmlWriteOptions, OutStream)");
}

::agiru::Boolean XmlDeclaration::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                         std::string &Text) {
  static_cast<void>(WriteOptions);
  static_cast<void>(Text);
  RefuseDoor("XmlDeclaration.WriteTo(XmlWriteOptions, Text)");
}

::agiru::Boolean XmlDocument::Add(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlDocument.Add(Any)");
}

::agiru::Boolean XmlDocument::AddAfterSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlDocument.AddAfterSelf(Any)");
}

::agiru::Boolean XmlDocument::AddBeforeSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlDocument.AddBeforeSelf(Any)");
}

::agiru::Boolean XmlDocument::AddFirst(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlDocument.AddFirst(Any)");
}

::agiru::XmlNode XmlDocument::AsXmlNode() {
  RefuseDoor("XmlDocument.AsXmlNode()");
}

::agiru::XmlDocument XmlDocument::Create() {
  RefuseDoor("XmlDocument.Create()");
}

::agiru::XmlDocument XmlDocument::Create(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlDocument.Create(Any)");
}

::agiru::XmlNodeList XmlDocument::GetChildElements() {
  RefuseDoor("XmlDocument.GetChildElements()");
}

::agiru::XmlNodeList XmlDocument::GetChildElements(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("XmlDocument.GetChildElements(Text)");
}

::agiru::XmlNodeList XmlDocument::GetChildElements(std::string_view LocalName,
                                                   std::string_view NamespaceUri) {
  static_cast<void>(LocalName);
  static_cast<void>(NamespaceUri);
  RefuseDoor("XmlDocument.GetChildElements(Text, Text)");
}

::agiru::XmlNodeList XmlDocument::GetChildNodes() {
  RefuseDoor("XmlDocument.GetChildNodes()");
}

::agiru::Boolean XmlDocument::GetDeclaration(::agiru::XmlDeclaration &Result) {
  static_cast<void>(Result);
  RefuseDoor("XmlDocument.GetDeclaration(XmlDeclaration)");
}

::agiru::XmlNodeList XmlDocument::GetDescendantElements() {
  RefuseDoor("XmlDocument.GetDescendantElements()");
}

::agiru::XmlNodeList XmlDocument::GetDescendantElements(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("XmlDocument.GetDescendantElements(Text)");
}

::agiru::XmlNodeList XmlDocument::GetDescendantElements(std::string_view LocalName,
                                                        std::string_view NamespaceUri) {
  static_cast<void>(LocalName);
  static_cast<void>(NamespaceUri);
  RefuseDoor("XmlDocument.GetDescendantElements(Text, Text)");
}

::agiru::XmlNodeList XmlDocument::GetDescendantNodes() {
  RefuseDoor("XmlDocument.GetDescendantNodes()");
}

::agiru::Boolean XmlDocument::GetDocument(::agiru::XmlDocument &Document) {
  static_cast<void>(Document);
  RefuseDoor("XmlDocument.GetDocument(XmlDocument)");
}

::agiru::Boolean XmlDocument::GetDocumentType(::agiru::XmlDocumentType &DocumentType) {
  static_cast<void>(DocumentType);
  RefuseDoor("XmlDocument.GetDocumentType(XmlDocumentType)");
}

::agiru::Boolean XmlDocument::GetParent(::agiru::XmlElement &Parent) {
  static_cast<void>(Parent);
  RefuseDoor("XmlDocument.GetParent(XmlElement)");
}

::agiru::Boolean XmlDocument::GetRoot(::agiru::XmlElement &Result) {
  static_cast<void>(Result);
  RefuseDoor("XmlDocument.GetRoot(XmlElement)");
}

::agiru::XmlNameTable XmlDocument::NameTable() {
  RefuseDoor("XmlDocument.NameTable()");
}

::agiru::Boolean XmlDocument::ReadFrom(const ::agiru::InStream &InStream,
                                       ::agiru::XmlDocument &Result) {
  static_cast<void>(InStream);
  static_cast<void>(Result);
  RefuseDoor("XmlDocument.ReadFrom(InStream, XmlDocument)");
}

::agiru::Boolean XmlDocument::ReadFrom(const ::agiru::InStream &InStream,
                                       const ::agiru::XmlReadOptions &ReadOptions,
                                       ::agiru::XmlDocument &Result) {
  static_cast<void>(InStream);
  static_cast<void>(ReadOptions);
  static_cast<void>(Result);
  RefuseDoor("XmlDocument.ReadFrom(InStream, XmlReadOptions, XmlDocument)");
}

::agiru::Boolean XmlDocument::ReadFrom(std::string_view Text, ::agiru::XmlDocument &Result) {
  static_cast<void>(Text);
  static_cast<void>(Result);
  RefuseDoor("XmlDocument.ReadFrom(Text, XmlDocument)");
}

::agiru::Boolean XmlDocument::ReadFrom(std::string_view Text,
                                       const ::agiru::XmlReadOptions &ReadOptions,
                                       ::agiru::XmlDocument &Result) {
  static_cast<void>(Text);
  static_cast<void>(ReadOptions);
  static_cast<void>(Result);
  RefuseDoor("XmlDocument.ReadFrom(Text, XmlReadOptions, XmlDocument)");
}

::agiru::Boolean XmlDocument::Remove() {
  RefuseDoor("XmlDocument.Remove()");
}

void XmlDocument::RemoveNodes() {
  RefuseDoor("XmlDocument.RemoveNodes()");
}

::agiru::Boolean XmlDocument::ReplaceNodes(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlDocument.ReplaceNodes(Any)");
}

::agiru::Boolean XmlDocument::ReplaceWith(const ::agiru::Variant &Node) {
  static_cast<void>(Node);
  RefuseDoor("XmlDocument.ReplaceWith(Any)");
}

::agiru::Boolean XmlDocument::SelectNodes(std::string_view XPath,
                                          const ::agiru::XmlNamespaceManager &NamespaceManager,
                                          ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(NodeList);
  RefuseDoor("XmlDocument.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)");
}

::agiru::Boolean XmlDocument::SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NodeList);
  RefuseDoor("XmlDocument.SelectNodes(Text, XmlNodeList)");
}

::agiru::Boolean XmlDocument::SelectSingleNode(std::string_view XPath,
                                               const ::agiru::XmlNamespaceManager &NamespaceManager,
                                               ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(Node);
  RefuseDoor("XmlDocument.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)");
}

::agiru::Boolean XmlDocument::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(Node);
  RefuseDoor("XmlDocument.SelectSingleNode(Text, XmlNode)");
}

::agiru::Boolean XmlDocument::SetDeclaration(const ::agiru::XmlDeclaration &Declaration) {
  static_cast<void>(Declaration);
  RefuseDoor("XmlDocument.SetDeclaration(XmlDeclaration)");
}

::agiru::Boolean XmlDocument::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("XmlDocument.WriteTo(OutStream)");
}

::agiru::Boolean XmlDocument::WriteTo(std::string &Text) {
  static_cast<void>(Text);
  RefuseDoor("XmlDocument.WriteTo(Text)");
}

::agiru::Boolean XmlDocument::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                      const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  static_cast<void>(OutStream);
  RefuseDoor("XmlDocument.WriteTo(XmlWriteOptions, OutStream)");
}

::agiru::Boolean XmlDocument::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                      std::string &Text) {
  static_cast<void>(WriteOptions);
  static_cast<void>(Text);
  RefuseDoor("XmlDocument.WriteTo(XmlWriteOptions, Text)");
}

::agiru::Boolean XmlDocumentType::AddAfterSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlDocumentType.AddAfterSelf(Any)");
}

::agiru::Boolean XmlDocumentType::AddBeforeSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlDocumentType.AddBeforeSelf(Any)");
}

::agiru::XmlNode XmlDocumentType::AsXmlNode() {
  RefuseDoor("XmlDocumentType.AsXmlNode()");
}

::agiru::XmlDocumentType XmlDocumentType::Create(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("XmlDocumentType.Create(Text)");
}

::agiru::XmlDocumentType XmlDocumentType::Create(std::string_view Name, std::string_view PublicId) {
  static_cast<void>(Name);
  static_cast<void>(PublicId);
  RefuseDoor("XmlDocumentType.Create(Text, Text)");
}

::agiru::XmlDocumentType XmlDocumentType::Create(std::string_view Name,
                                                 std::string_view PublicId,
                                                 std::string_view SystemId) {
  static_cast<void>(Name);
  static_cast<void>(PublicId);
  static_cast<void>(SystemId);
  RefuseDoor("XmlDocumentType.Create(Text, Text, Text)");
}

::agiru::XmlDocumentType XmlDocumentType::Create(std::string_view Name,
                                                 std::string_view PublicId,
                                                 std::string_view SystemId,
                                                 std::string_view InternalSubSet) {
  static_cast<void>(Name);
  static_cast<void>(PublicId);
  static_cast<void>(SystemId);
  static_cast<void>(InternalSubSet);
  RefuseDoor("XmlDocumentType.Create(Text, Text, Text, Text)");
}

::agiru::Boolean XmlDocumentType::GetDocument(::agiru::XmlDocument &Document) {
  static_cast<void>(Document);
  RefuseDoor("XmlDocumentType.GetDocument(XmlDocument)");
}

::agiru::Boolean XmlDocumentType::GetInternalSubset(std::string &Result) {
  static_cast<void>(Result);
  RefuseDoor("XmlDocumentType.GetInternalSubset(Text)");
}

::agiru::Boolean XmlDocumentType::GetName(std::string &Result) {
  static_cast<void>(Result);
  RefuseDoor("XmlDocumentType.GetName(Text)");
}

::agiru::Boolean XmlDocumentType::GetParent(::agiru::XmlElement &Parent) {
  static_cast<void>(Parent);
  RefuseDoor("XmlDocumentType.GetParent(XmlElement)");
}

::agiru::Boolean XmlDocumentType::GetPublicId(std::string &Result) {
  static_cast<void>(Result);
  RefuseDoor("XmlDocumentType.GetPublicId(Text)");
}

::agiru::Boolean XmlDocumentType::GetSystemId(std::string &Result) {
  static_cast<void>(Result);
  RefuseDoor("XmlDocumentType.GetSystemId(Text)");
}

::agiru::Boolean XmlDocumentType::Remove() {
  RefuseDoor("XmlDocumentType.Remove()");
}

::agiru::Boolean XmlDocumentType::ReplaceWith(const ::agiru::Variant &Node) {
  static_cast<void>(Node);
  RefuseDoor("XmlDocumentType.ReplaceWith(Any)");
}

::agiru::Boolean XmlDocumentType::SelectNodes(std::string_view XPath,
                                              const ::agiru::XmlNamespaceManager &NamespaceManager,
                                              ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(NodeList);
  RefuseDoor("XmlDocumentType.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)");
}

::agiru::Boolean XmlDocumentType::SelectNodes(std::string_view XPath,
                                              ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NodeList);
  RefuseDoor("XmlDocumentType.SelectNodes(Text, XmlNodeList)");
}

::agiru::Boolean
XmlDocumentType::SelectSingleNode(std::string_view XPath,
                                  const ::agiru::XmlNamespaceManager &NamespaceManager,
                                  ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(Node);
  RefuseDoor("XmlDocumentType.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)");
}

::agiru::Boolean XmlDocumentType::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(Node);
  RefuseDoor("XmlDocumentType.SelectSingleNode(Text, XmlNode)");
}

::agiru::Boolean XmlDocumentType::SetInternalSubset(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("XmlDocumentType.SetInternalSubset(Text)");
}

::agiru::Boolean XmlDocumentType::SetName(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("XmlDocumentType.SetName(Text)");
}

::agiru::Boolean XmlDocumentType::SetPublicId(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("XmlDocumentType.SetPublicId(Text)");
}

::agiru::Boolean XmlDocumentType::SetSystemId(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("XmlDocumentType.SetSystemId(Text)");
}

::agiru::Boolean XmlDocumentType::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("XmlDocumentType.WriteTo(OutStream)");
}

::agiru::Boolean XmlDocumentType::WriteTo(std::string &Text) {
  static_cast<void>(Text);
  RefuseDoor("XmlDocumentType.WriteTo(Text)");
}

::agiru::Boolean XmlDocumentType::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                          const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  static_cast<void>(OutStream);
  RefuseDoor("XmlDocumentType.WriteTo(XmlWriteOptions, OutStream)");
}

::agiru::Boolean XmlDocumentType::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                          std::string &Text) {
  static_cast<void>(WriteOptions);
  static_cast<void>(Text);
  RefuseDoor("XmlDocumentType.WriteTo(XmlWriteOptions, Text)");
}

::agiru::Boolean XmlElement::Add(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlElement.Add(Any)");
}

::agiru::Boolean XmlElement::AddAfterSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlElement.AddAfterSelf(Any)");
}

::agiru::Boolean XmlElement::AddBeforeSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlElement.AddBeforeSelf(Any)");
}

::agiru::Boolean XmlElement::AddFirst(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlElement.AddFirst(Any)");
}

::agiru::XmlNode XmlElement::AsXmlNode() {
  RefuseDoor("XmlElement.AsXmlNode()");
}

::agiru::XmlAttributeCollection XmlElement::Attributes() {
  RefuseDoor("XmlElement.Attributes()");
}

::agiru::XmlElement XmlElement::Create(std::string_view Name, const ::agiru::Variant &Content) {
  static_cast<void>(Name);
  static_cast<void>(Content);
  RefuseDoor("XmlElement.Create(Text, Any)");
}

::agiru::XmlElement XmlElement::Create(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("XmlElement.Create(Text)");
}

::agiru::XmlElement XmlElement::Create(std::string_view LocalName,
                                       std::string_view NamespaceUri,
                                       const ::agiru::Variant &Content) {
  static_cast<void>(LocalName);
  static_cast<void>(NamespaceUri);
  static_cast<void>(Content);
  RefuseDoor("XmlElement.Create(Text, Text, Any)");
}

::agiru::XmlElement XmlElement::Create(std::string_view LocalName, std::string_view NamespaceUri) {
  static_cast<void>(LocalName);
  static_cast<void>(NamespaceUri);
  RefuseDoor("XmlElement.Create(Text, Text)");
}

::agiru::XmlNodeList XmlElement::GetChildElements() {
  RefuseDoor("XmlElement.GetChildElements()");
}

::agiru::XmlNodeList XmlElement::GetChildElements(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("XmlElement.GetChildElements(Text)");
}

::agiru::XmlNodeList XmlElement::GetChildElements(std::string_view LocalName,
                                                  std::string_view NamespaceUri) {
  static_cast<void>(LocalName);
  static_cast<void>(NamespaceUri);
  RefuseDoor("XmlElement.GetChildElements(Text, Text)");
}

::agiru::XmlNodeList XmlElement::GetChildNodes() {
  RefuseDoor("XmlElement.GetChildNodes()");
}

::agiru::XmlNodeList XmlElement::GetDescendantElements() {
  RefuseDoor("XmlElement.GetDescendantElements()");
}

::agiru::XmlNodeList XmlElement::GetDescendantElements(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("XmlElement.GetDescendantElements(Text)");
}

::agiru::XmlNodeList XmlElement::GetDescendantElements(std::string_view LocalName,
                                                       std::string_view NamespaceUri) {
  static_cast<void>(LocalName);
  static_cast<void>(NamespaceUri);
  RefuseDoor("XmlElement.GetDescendantElements(Text, Text)");
}

::agiru::XmlNodeList XmlElement::GetDescendantNodes() {
  RefuseDoor("XmlElement.GetDescendantNodes()");
}

::agiru::Boolean XmlElement::GetDocument(::agiru::XmlDocument &Document) {
  static_cast<void>(Document);
  RefuseDoor("XmlElement.GetDocument(XmlDocument)");
}

::agiru::Boolean XmlElement::GetNamespaceOfPrefix(std::string_view Prefix, std::string &Result) {
  static_cast<void>(Prefix);
  static_cast<void>(Result);
  RefuseDoor("XmlElement.GetNamespaceOfPrefix(Text, Text)");
}

::agiru::Boolean XmlElement::GetParent(::agiru::XmlElement &Parent) {
  static_cast<void>(Parent);
  RefuseDoor("XmlElement.GetParent(XmlElement)");
}

::agiru::Boolean XmlElement::GetPrefixOfNamespace(std::string_view Namespace, std::string &Result) {
  static_cast<void>(Namespace);
  static_cast<void>(Result);
  RefuseDoor("XmlElement.GetPrefixOfNamespace(Text, Text)");
}

::agiru::Boolean XmlElement::HasAttributes() {
  RefuseDoor("XmlElement.HasAttributes()");
}

::agiru::Boolean XmlElement::HasElements() {
  RefuseDoor("XmlElement.HasElements()");
}

std::string XmlElement::InnerText() {
  RefuseDoor("XmlElement.InnerText()");
}

std::string XmlElement::InnerXml() {
  RefuseDoor("XmlElement.InnerXml()");
}

::agiru::Boolean XmlElement::IsEmpty() {
  RefuseDoor("XmlElement.IsEmpty()");
}

std::string XmlElement::LocalName() {
  RefuseDoor("XmlElement.LocalName()");
}

std::string XmlElement::Name() {
  RefuseDoor("XmlElement.Name()");
}

std::string XmlElement::NamespaceUri() {
  RefuseDoor("XmlElement.NamespaceUri()");
}

::agiru::Boolean XmlElement::Remove() {
  RefuseDoor("XmlElement.Remove()");
}

void XmlElement::RemoveAllAttributes() {
  RefuseDoor("XmlElement.RemoveAllAttributes()");
}

void XmlElement::RemoveAttribute(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("XmlElement.RemoveAttribute(Text)");
}

void XmlElement::RemoveAttribute(std::string_view LocalName, std::string_view NamespaceUri) {
  static_cast<void>(LocalName);
  static_cast<void>(NamespaceUri);
  RefuseDoor("XmlElement.RemoveAttribute(Text, Text)");
}

void XmlElement::RemoveAttribute(const ::agiru::XmlAttribute &Attribute) {
  static_cast<void>(Attribute);
  RefuseDoor("XmlElement.RemoveAttribute(XmlAttribute)");
}

void XmlElement::RemoveNodes() {
  RefuseDoor("XmlElement.RemoveNodes()");
}

::agiru::Boolean XmlElement::ReplaceNodes(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlElement.ReplaceNodes(Any)");
}

::agiru::Boolean XmlElement::ReplaceWith(const ::agiru::Variant &Node) {
  static_cast<void>(Node);
  RefuseDoor("XmlElement.ReplaceWith(Any)");
}

::agiru::Boolean XmlElement::SelectNodes(std::string_view XPath,
                                         const ::agiru::XmlNamespaceManager &NamespaceManager,
                                         ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(NodeList);
  RefuseDoor("XmlElement.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)");
}

::agiru::Boolean XmlElement::SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NodeList);
  RefuseDoor("XmlElement.SelectNodes(Text, XmlNodeList)");
}

::agiru::Boolean XmlElement::SelectSingleNode(std::string_view XPath,
                                              const ::agiru::XmlNamespaceManager &NamespaceManager,
                                              ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(Node);
  RefuseDoor("XmlElement.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)");
}

::agiru::Boolean XmlElement::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(Node);
  RefuseDoor("XmlElement.SelectSingleNode(Text, XmlNode)");
}

void XmlElement::SetAttribute(std::string_view Name, std::string_view Value) {
  static_cast<void>(Name);
  static_cast<void>(Value);
  RefuseDoor("XmlElement.SetAttribute(Text, Text)");
}

void XmlElement::SetAttribute(std::string_view LocalName,
                              std::string_view NamespaceUri,
                              std::string_view Value) {
  static_cast<void>(LocalName);
  static_cast<void>(NamespaceUri);
  static_cast<void>(Value);
  RefuseDoor("XmlElement.SetAttribute(Text, Text, Text)");
}

::agiru::Boolean XmlElement::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("XmlElement.WriteTo(OutStream)");
}

::agiru::Boolean XmlElement::WriteTo(std::string &Text) {
  static_cast<void>(Text);
  RefuseDoor("XmlElement.WriteTo(Text)");
}

::agiru::Boolean XmlElement::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                     const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  static_cast<void>(OutStream);
  RefuseDoor("XmlElement.WriteTo(XmlWriteOptions, OutStream)");
}

::agiru::Boolean XmlElement::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                     std::string &Text) {
  static_cast<void>(WriteOptions);
  static_cast<void>(Text);
  RefuseDoor("XmlElement.WriteTo(XmlWriteOptions, Text)");
}

std::string XmlNameTable::Add(std::string_view Key) {
  static_cast<void>(Key);
  RefuseDoor("XmlNameTable.Add(Text)");
}

::agiru::Boolean XmlNameTable::Get(std::string_view Key, std::string &Result) {
  static_cast<void>(Key);
  static_cast<void>(Result);
  RefuseDoor("XmlNameTable.Get(Text, Text)");
}

void XmlNamespaceManager::AddNamespace(std::string_view Prefix, std::string_view Uri) {
  static_cast<void>(Prefix);
  static_cast<void>(Uri);
  RefuseDoor("XmlNamespaceManager.AddNamespace(Text, Text)");
}

::agiru::Boolean XmlNamespaceManager::HasNamespace(std::string_view Prefix) {
  static_cast<void>(Prefix);
  RefuseDoor("XmlNamespaceManager.HasNamespace(Text)");
}

::agiru::Boolean XmlNamespaceManager::LookupNamespace(std::string_view Prefix,
                                                      std::string &Result) {
  static_cast<void>(Prefix);
  static_cast<void>(Result);
  RefuseDoor("XmlNamespaceManager.LookupNamespace(Text, Text)");
}

::agiru::Boolean XmlNamespaceManager::LookupPrefix(std::string_view Uri, std::string &Result) {
  static_cast<void>(Uri);
  static_cast<void>(Result);
  RefuseDoor("XmlNamespaceManager.LookupPrefix(Text, Text)");
}

::agiru::XmlNameTable XmlNamespaceManager::NameTable(const ::agiru::XmlNameTable &NewValue) {
  static_cast<void>(NewValue);
  RefuseDoor("XmlNamespaceManager.NameTable(XmlNameTable)");
}

void XmlNamespaceManager::PopScope() {
  RefuseDoor("XmlNamespaceManager.PopScope()");
}

void XmlNamespaceManager::PushScope() {
  RefuseDoor("XmlNamespaceManager.PushScope()");
}

void XmlNamespaceManager::RemoveNamespace(std::string_view Prefix, std::string_view Uri) {
  static_cast<void>(Prefix);
  static_cast<void>(Uri);
  RefuseDoor("XmlNamespaceManager.RemoveNamespace(Text, Text)");
}

::agiru::Boolean XmlNode::AddAfterSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlNode.AddAfterSelf(Any)");
}

::agiru::Boolean XmlNode::AddBeforeSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlNode.AddBeforeSelf(Any)");
}

::agiru::XmlAttribute XmlNode::AsXmlAttribute() {
  RefuseDoor("XmlNode.AsXmlAttribute()");
}

::agiru::XmlCData XmlNode::AsXmlCData() {
  RefuseDoor("XmlNode.AsXmlCData()");
}

::agiru::XmlComment XmlNode::AsXmlComment() {
  RefuseDoor("XmlNode.AsXmlComment()");
}

::agiru::XmlDeclaration XmlNode::AsXmlDeclaration() {
  RefuseDoor("XmlNode.AsXmlDeclaration()");
}

::agiru::XmlDocument XmlNode::AsXmlDocument() {
  RefuseDoor("XmlNode.AsXmlDocument()");
}

::agiru::XmlDocumentType XmlNode::AsXmlDocumentType() {
  RefuseDoor("XmlNode.AsXmlDocumentType()");
}

::agiru::XmlElement XmlNode::AsXmlElement() {
  RefuseDoor("XmlNode.AsXmlElement()");
}

::agiru::XmlProcessingInstruction XmlNode::AsXmlProcessingInstruction() {
  RefuseDoor("XmlNode.AsXmlProcessingInstruction()");
}

::agiru::XmlText XmlNode::AsXmlText() {
  RefuseDoor("XmlNode.AsXmlText()");
}

::agiru::Boolean XmlNode::GetDocument(::agiru::XmlDocument &Document) {
  static_cast<void>(Document);
  RefuseDoor("XmlNode.GetDocument(XmlDocument)");
}

::agiru::Boolean XmlNode::GetParent(::agiru::XmlElement &Parent) {
  static_cast<void>(Parent);
  RefuseDoor("XmlNode.GetParent(XmlElement)");
}

::agiru::Boolean XmlNode::IsXmlAttribute() {
  RefuseDoor("XmlNode.IsXmlAttribute()");
}

::agiru::Boolean XmlNode::IsXmlCData() {
  RefuseDoor("XmlNode.IsXmlCData()");
}

::agiru::Boolean XmlNode::IsXmlComment() {
  RefuseDoor("XmlNode.IsXmlComment()");
}

::agiru::Boolean XmlNode::IsXmlDeclaration() {
  RefuseDoor("XmlNode.IsXmlDeclaration()");
}

::agiru::Boolean XmlNode::IsXmlDocument() {
  RefuseDoor("XmlNode.IsXmlDocument()");
}

::agiru::Boolean XmlNode::IsXmlDocumentType() {
  RefuseDoor("XmlNode.IsXmlDocumentType()");
}

::agiru::Boolean XmlNode::IsXmlElement() {
  RefuseDoor("XmlNode.IsXmlElement()");
}

::agiru::Boolean XmlNode::IsXmlProcessingInstruction() {
  RefuseDoor("XmlNode.IsXmlProcessingInstruction()");
}

::agiru::Boolean XmlNode::IsXmlText() {
  RefuseDoor("XmlNode.IsXmlText()");
}

::agiru::Boolean XmlNode::Remove() {
  RefuseDoor("XmlNode.Remove()");
}

::agiru::Boolean XmlNode::ReplaceWith(const ::agiru::Variant &Node) {
  static_cast<void>(Node);
  RefuseDoor("XmlNode.ReplaceWith(Any)");
}

::agiru::Boolean XmlNode::SelectNodes(std::string_view XPath,
                                      const ::agiru::XmlNamespaceManager &NamespaceManager,
                                      ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(NodeList);
  RefuseDoor("XmlNode.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)");
}

::agiru::Boolean XmlNode::SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NodeList);
  RefuseDoor("XmlNode.SelectNodes(Text, XmlNodeList)");
}

::agiru::Boolean XmlNode::SelectSingleNode(std::string_view XPath,
                                           const ::agiru::XmlNamespaceManager &NamespaceManager,
                                           ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(Node);
  RefuseDoor("XmlNode.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)");
}

::agiru::Boolean XmlNode::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(Node);
  RefuseDoor("XmlNode.SelectSingleNode(Text, XmlNode)");
}

::agiru::Boolean XmlNode::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("XmlNode.WriteTo(OutStream)");
}

::agiru::Boolean XmlNode::WriteTo(std::string &Text) {
  static_cast<void>(Text);
  RefuseDoor("XmlNode.WriteTo(Text)");
}

::agiru::Boolean XmlNode::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                  const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  static_cast<void>(OutStream);
  RefuseDoor("XmlNode.WriteTo(XmlWriteOptions, OutStream)");
}

::agiru::Boolean XmlNode::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions, std::string &Text) {
  static_cast<void>(WriteOptions);
  static_cast<void>(Text);
  RefuseDoor("XmlNode.WriteTo(XmlWriteOptions, Text)");
}

::agiru::Integer XmlNodeList::Count() {
  RefuseDoor("XmlNodeList.Count()");
}

::agiru::Boolean XmlNodeList::Get(::agiru::Integer Index, ::agiru::XmlNode &Node) {
  static_cast<void>(Index);
  static_cast<void>(Node);
  RefuseDoor("XmlNodeList.Get(Integer, XmlNode)");
}

::agiru::Boolean XmlProcessingInstruction::AddAfterSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlProcessingInstruction.AddAfterSelf(Any)");
}

::agiru::Boolean XmlProcessingInstruction::AddBeforeSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlProcessingInstruction.AddBeforeSelf(Any)");
}

::agiru::XmlNode XmlProcessingInstruction::AsXmlNode() {
  RefuseDoor("XmlProcessingInstruction.AsXmlNode()");
}

::agiru::XmlProcessingInstruction XmlProcessingInstruction::Create(std::string_view Target,
                                                                   std::string_view Data) {
  static_cast<void>(Target);
  static_cast<void>(Data);
  RefuseDoor("XmlProcessingInstruction.Create(Text, Text)");
}

::agiru::Boolean XmlProcessingInstruction::GetData(std::string &Result) {
  static_cast<void>(Result);
  RefuseDoor("XmlProcessingInstruction.GetData(Text)");
}

::agiru::Boolean XmlProcessingInstruction::GetDocument(::agiru::XmlDocument &Document) {
  static_cast<void>(Document);
  RefuseDoor("XmlProcessingInstruction.GetDocument(XmlDocument)");
}

::agiru::Boolean XmlProcessingInstruction::GetParent(::agiru::XmlElement &Parent) {
  static_cast<void>(Parent);
  RefuseDoor("XmlProcessingInstruction.GetParent(XmlElement)");
}

::agiru::Boolean XmlProcessingInstruction::GetTarget(std::string &Result) {
  static_cast<void>(Result);
  RefuseDoor("XmlProcessingInstruction.GetTarget(Text)");
}

::agiru::Boolean XmlProcessingInstruction::Remove() {
  RefuseDoor("XmlProcessingInstruction.Remove()");
}

::agiru::Boolean XmlProcessingInstruction::ReplaceWith(const ::agiru::Variant &Node) {
  static_cast<void>(Node);
  RefuseDoor("XmlProcessingInstruction.ReplaceWith(Any)");
}

::agiru::Boolean
XmlProcessingInstruction::SelectNodes(std::string_view XPath,
                                      const ::agiru::XmlNamespaceManager &NamespaceManager,
                                      ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(NodeList);
  RefuseDoor("XmlProcessingInstruction.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)");
}

::agiru::Boolean XmlProcessingInstruction::SelectNodes(std::string_view XPath,
                                                       ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NodeList);
  RefuseDoor("XmlProcessingInstruction.SelectNodes(Text, XmlNodeList)");
}

::agiru::Boolean
XmlProcessingInstruction::SelectSingleNode(std::string_view XPath,
                                           const ::agiru::XmlNamespaceManager &NamespaceManager,
                                           ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(Node);
  RefuseDoor("XmlProcessingInstruction.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)");
}

::agiru::Boolean XmlProcessingInstruction::SelectSingleNode(std::string_view XPath,
                                                            ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(Node);
  RefuseDoor("XmlProcessingInstruction.SelectSingleNode(Text, XmlNode)");
}

::agiru::Boolean XmlProcessingInstruction::SetData(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("XmlProcessingInstruction.SetData(Text)");
}

::agiru::Boolean XmlProcessingInstruction::SetTarget(std::string_view Value) {
  static_cast<void>(Value);
  RefuseDoor("XmlProcessingInstruction.SetTarget(Text)");
}

::agiru::Boolean XmlProcessingInstruction::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("XmlProcessingInstruction.WriteTo(OutStream)");
}

::agiru::Boolean XmlProcessingInstruction::WriteTo(std::string &Text) {
  static_cast<void>(Text);
  RefuseDoor("XmlProcessingInstruction.WriteTo(Text)");
}

::agiru::Boolean XmlProcessingInstruction::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                                   const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  static_cast<void>(OutStream);
  RefuseDoor("XmlProcessingInstruction.WriteTo(XmlWriteOptions, OutStream)");
}

::agiru::Boolean XmlProcessingInstruction::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                                   std::string &Text) {
  static_cast<void>(WriteOptions);
  static_cast<void>(Text);
  RefuseDoor("XmlProcessingInstruction.WriteTo(XmlWriteOptions, Text)");
}

::agiru::Boolean XmlReadOptions::PreserveWhitespace(::agiru::Boolean NewValue) {
  static_cast<void>(NewValue);
  RefuseDoor("XmlReadOptions.PreserveWhitespace(Boolean)");
}

::agiru::Boolean XmlText::AddAfterSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlText.AddAfterSelf(Any)");
}

::agiru::Boolean XmlText::AddBeforeSelf(const ::agiru::Variant &Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlText.AddBeforeSelf(Any)");
}

::agiru::XmlNode XmlText::AsXmlNode() {
  RefuseDoor("XmlText.AsXmlNode()");
}

::agiru::XmlText XmlText::Create(std::string_view Content) {
  static_cast<void>(Content);
  RefuseDoor("XmlText.Create(Text)");
}

::agiru::Boolean XmlText::GetDocument(::agiru::XmlDocument &Document) {
  static_cast<void>(Document);
  RefuseDoor("XmlText.GetDocument(XmlDocument)");
}

::agiru::Boolean XmlText::GetParent(::agiru::XmlElement &Parent) {
  static_cast<void>(Parent);
  RefuseDoor("XmlText.GetParent(XmlElement)");
}

::agiru::Boolean XmlText::Remove() {
  RefuseDoor("XmlText.Remove()");
}

::agiru::Boolean XmlText::ReplaceWith(const ::agiru::Variant &Node) {
  static_cast<void>(Node);
  RefuseDoor("XmlText.ReplaceWith(Any)");
}

::agiru::Boolean XmlText::SelectNodes(std::string_view XPath,
                                      const ::agiru::XmlNamespaceManager &NamespaceManager,
                                      ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(NodeList);
  RefuseDoor("XmlText.SelectNodes(Text, XmlNamespaceManager, XmlNodeList)");
}

::agiru::Boolean XmlText::SelectNodes(std::string_view XPath, ::agiru::XmlNodeList &NodeList) {
  static_cast<void>(XPath);
  static_cast<void>(NodeList);
  RefuseDoor("XmlText.SelectNodes(Text, XmlNodeList)");
}

::agiru::Boolean XmlText::SelectSingleNode(std::string_view XPath,
                                           const ::agiru::XmlNamespaceManager &NamespaceManager,
                                           ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(NamespaceManager);
  static_cast<void>(Node);
  RefuseDoor("XmlText.SelectSingleNode(Text, XmlNamespaceManager, XmlNode)");
}

::agiru::Boolean XmlText::SelectSingleNode(std::string_view XPath, ::agiru::XmlNode &Node) {
  static_cast<void>(XPath);
  static_cast<void>(Node);
  RefuseDoor("XmlText.SelectSingleNode(Text, XmlNode)");
}

std::string XmlText::Value(std::string_view NewValue) {
  static_cast<void>(NewValue);
  RefuseDoor("XmlText.Value(Text)");
}

::agiru::Boolean XmlText::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("XmlText.WriteTo(OutStream)");
}

::agiru::Boolean XmlText::WriteTo(std::string &Text) {
  static_cast<void>(Text);
  RefuseDoor("XmlText.WriteTo(Text)");
}

::agiru::Boolean XmlText::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions,
                                  const ::agiru::OutStream &OutStream) {
  static_cast<void>(WriteOptions);
  static_cast<void>(OutStream);
  RefuseDoor("XmlText.WriteTo(XmlWriteOptions, OutStream)");
}

::agiru::Boolean XmlText::WriteTo(const ::agiru::XmlWriteOptions &WriteOptions, std::string &Text) {
  static_cast<void>(WriteOptions);
  static_cast<void>(Text);
  RefuseDoor("XmlText.WriteTo(XmlWriteOptions, Text)");
}

::agiru::Boolean XmlWriteOptions::PreserveWhitespace(::agiru::Boolean NewValue) {
  static_cast<void>(NewValue);
  RefuseDoor("XmlWriteOptions.PreserveWhitespace(Boolean)");
}

::agiru::Boolean KeyRef::Active() {
  RefuseDoor("KeyRef.Active()");
}

::agiru::Integer KeyRef::FieldCount() {
  RefuseDoor("KeyRef.FieldCount()");
}

::agiru::FieldRef KeyRef::FieldIndex(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseDoor("KeyRef.FieldIndex(Integer)");
}

::agiru::RecordRef KeyRef::Record() {
  RefuseDoor("KeyRef.Record()");
}

}

// NOLINTEND(readability-convert-member-functions-to-static,bugprone-easily-swappable-parameters,performance-unnecessary-value-param)
