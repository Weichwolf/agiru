// NOLINTBEGIN(bugprone-easily-swappable-parameters,performance-unnecessary-value-param)
#include "platform/Company.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/TestRunner.h"
#include "runtime/test/Handlers.h"
#include "runtime/test/TestHttpRequestMessage.h"
#include "runtime/test/TestHttpResponseMessage.h"
#include "type/BigInteger.h"
#include "type/BigText.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/CompanyProperty.h"
#include "type/Cookie.h"
#include "type/DataTransfer.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Debugger.h"
#include "type/Decimal.h"
#include "type/Dialog.h"
#include "type/Dictionary.h"
#include "type/Duration.h"
#include "type/ErrorInfo.h"
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
#include "type/JsonArray.h"
#include "type/JsonObject.h"
#include "type/JsonToken.h"
#include "type/JsonValue.h"
#include "type/KeyRef.h"
#include "type/Label.h"
#include "type/List.h"
#include "type/ModuleInfo.h"
#include "type/NavApp.h"
#include "type/ObjectType.h"
#include "type/ProductName.h"
#include "type/RecordId.h"
#include "type/SecretText.h"
#include "type/SessionInformation.h"
#include "type/SessionSettings.h"
#include "type/Stream.h"
#include "type/StringValue.h"
#include "type/TaskScheduler.h"
#include "type/TextConst.h"
#include "type/TextEncoding.h"
#include "type/Time.h"
#include "type/Variant.h"
#include "type/Verbosity.h"
#include "type/WebServiceActionContext.h"
#include "type/WebServiceActionResultCode.h"

#include "BuiltinsWritten.h"

#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace agiru {

namespace {

[[noreturn]] void RefuseUnimplemented(std::string_view what) {
  throw Error(std::string(what) + " is declared and not implemented yet (board:0035)");
}

bool SameControlName(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) { return false; }
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(a[i])) !=
        std::tolower(static_cast<unsigned char>(b[i]))) {
      return false;
    }
  }
  return true;
}

}

std::string BigText::ToText() const {
  RefuseUnimplemented("BigText.ToText()");
}

void BigText::AddText(const ::agiru::BigText &String, ::agiru::Integer Position) {
  static_cast<void>(String);
  static_cast<void>(Position);
  RefuseUnimplemented("BigText.AddText(BigText, Integer)");
}

void BigText::AddText(std::string_view String, ::agiru::Integer Position) {
  static_cast<void>(String);
  static_cast<void>(Position);
  RefuseUnimplemented("BigText.AddText(Text, Integer)");
}

::agiru::Integer BigText::GetSubText(::agiru::BigText &Variable,
                                     ::agiru::Integer Position,
                                     ::agiru::Integer Length) {
  static_cast<void>(Variable);
  static_cast<void>(Position);
  static_cast<void>(Length);
  RefuseUnimplemented("BigText.GetSubText(BigText, Integer, Integer)");
}

::agiru::Integer BigText::GetSubText(::agiru::Text<0> &Variable,
                                     ::agiru::Integer Position,
                                     ::agiru::Integer Length) {
  static_cast<void>(Variable);
  static_cast<void>(Position);
  static_cast<void>(Length);
  RefuseUnimplemented("BigText.GetSubText(Text, Integer, Integer)");
}

::agiru::Integer BigText::Length() {
  RefuseUnimplemented("BigText.Length()");
}

::agiru::Boolean BigText::Read(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseUnimplemented("BigText.Read(InStream)");
}

::agiru::Integer BigText::TextPos(std::string_view String) {
  static_cast<void>(String);
  RefuseUnimplemented("BigText.TextPos(Text)");
}

::agiru::Boolean BigText::Write(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseUnimplemented("BigText.Write(OutStream)");
}

std::string CompanyProperty::DisplayName() {
  std::string name = std::string(Session::Current().CompanyName());
  platform::Company company;
  if (company.Get(name) && company.DisplayName != "") {
    return std::string(company.DisplayName.Value());
  }
  return name;
}

::agiru::Guid CompanyProperty::ID() {
  RefuseUnimplemented("CompanyProperty.ID()");
}

std::string CompanyProperty::UrlName() {
  RefuseUnimplemented("CompanyProperty.UrlName()");
}

std::string Cookie::Domain() {
  RefuseUnimplemented("Cookie.Domain()");
}

::agiru::DateTime Cookie::Expires() {
  RefuseUnimplemented("Cookie.Expires()");
}

::agiru::Boolean Cookie::HttpOnly() {
  RefuseUnimplemented("Cookie.HttpOnly()");
}

std::string Cookie::Name(std::string_view Name) {
  static_cast<void>(Name);
  RefuseUnimplemented("Cookie.Name(Text)");
}

std::string Cookie::Path() {
  RefuseUnimplemented("Cookie.Path()");
}

::agiru::Boolean Cookie::Secure() {
  RefuseUnimplemented("Cookie.Secure()");
}

std::string Cookie::Value(std::string_view Value) {
  static_cast<void>(Value);
  RefuseUnimplemented("Cookie.Value(Text)");
}

void DataTransfer::AddConstantValue(const ::agiru::Variant &Value,
                                    ::agiru::Integer DestinationField) {
  static_cast<void>(Value);
  static_cast<void>(DestinationField);
  RefuseUnimplemented("DataTransfer.AddConstantValue(Any, Integer)");
}

void DataTransfer::AddDestinationFilter(::agiru::Integer DestinationField,
                                        std::string_view String,
                                        const ::agiru::Variant &Value) {
  static_cast<void>(DestinationField);
  static_cast<void>(String);
  static_cast<void>(Value);
  RefuseUnimplemented("DataTransfer.AddDestinationFilter(Integer, Text, Any)");
}

void DataTransfer::AddFieldValue(::agiru::Integer SourceField, ::agiru::Integer DestinationField) {
  static_cast<void>(SourceField);
  static_cast<void>(DestinationField);
  RefuseUnimplemented("DataTransfer.AddFieldValue(Integer, Integer)");
}

void DataTransfer::AddJoin(::agiru::Integer SourceField, ::agiru::Integer DestinationField) {
  static_cast<void>(SourceField);
  static_cast<void>(DestinationField);
  RefuseUnimplemented("DataTransfer.AddJoin(Integer, Integer)");
}

void DataTransfer::AddSourceFilter(::agiru::Integer SourceField,
                                   std::string_view String,
                                   const ::agiru::Variant &Value) {
  static_cast<void>(SourceField);
  static_cast<void>(String);
  static_cast<void>(Value);
  RefuseUnimplemented("DataTransfer.AddSourceFilter(Integer, Text, Any)");
}

void DataTransfer::CopyFields() {
  RefuseUnimplemented("DataTransfer.CopyFields()");
}

void DataTransfer::CopyRows() {
  RefuseUnimplemented("DataTransfer.CopyRows()");
}

void DataTransfer::SetTables(::agiru::Integer SourceTable, ::agiru::Integer DestinationTable) {
  static_cast<void>(SourceTable);
  static_cast<void>(DestinationTable);
  RefuseUnimplemented("DataTransfer.SetTables(Integer, Integer)");
}

::agiru::Boolean DataTransfer::UpdateAuditFields(::agiru::Boolean UpdateAuditFields) {
  static_cast<void>(UpdateAuditFields);
  RefuseUnimplemented("DataTransfer.UpdateAuditFields(Boolean)");
}

::agiru::Boolean Debugger::Activate() {
  RefuseUnimplemented("Debugger.Activate()");
}

::agiru::Boolean Debugger::Attach(::agiru::Integer SessionID) {
  static_cast<void>(SessionID);
  RefuseUnimplemented("Debugger.Attach(Integer)");
}

::agiru::Boolean Debugger::Break() {
  RefuseUnimplemented("Debugger.Break()");
}

::agiru::Boolean Debugger::BreakOnError(::agiru::Boolean Ok) {
  static_cast<void>(Ok);
  RefuseUnimplemented("Debugger.BreakOnError(Boolean)");
}

::agiru::Boolean Debugger::BreakOnRecordChanges(::agiru::Boolean Ok) {
  static_cast<void>(Ok);
  RefuseUnimplemented("Debugger.BreakOnRecordChanges(Boolean)");
}

::agiru::Boolean Debugger::Continue() {
  RefuseUnimplemented("Debugger.Continue()");
}

::agiru::Boolean Debugger::Deactivate() {
  RefuseUnimplemented("Debugger.Deactivate()");
}

::agiru::Integer Debugger::DebuggedSessionID() {
  RefuseUnimplemented("Debugger.DebuggedSessionID()");
}

::agiru::Integer Debugger::DebuggingSessionID() {
  RefuseUnimplemented("Debugger.DebuggingSessionID()");
}

::agiru::Boolean Debugger::EnableSqlTrace(::agiru::Integer SessionID,
                                          ::agiru::Boolean NewIsEnabled) {
  static_cast<void>(SessionID);
  static_cast<void>(NewIsEnabled);
  RefuseUnimplemented("Debugger.EnableSqlTrace(Integer, Boolean)");
}

std::string Debugger::GetLastErrorText() {
  RefuseUnimplemented("Debugger.GetLastErrorText()");
}

::agiru::Boolean Debugger::IsActive() {
  RefuseUnimplemented("Debugger.IsActive()");
}

::agiru::Boolean Debugger::IsAttached() {
  RefuseUnimplemented("Debugger.IsAttached()");
}

::agiru::Boolean Debugger::IsBreakpointHit() {
  RefuseUnimplemented("Debugger.IsBreakpointHit()");
}

::agiru::Boolean Debugger::SkipSystemTriggers(::agiru::Boolean Ok) {
  static_cast<void>(Ok);
  RefuseUnimplemented("Debugger.SkipSystemTriggers(Boolean)");
}

::agiru::Boolean Debugger::StepInto() {
  RefuseUnimplemented("Debugger.StepInto()");
}

::agiru::Boolean Debugger::StepOut() {
  RefuseUnimplemented("Debugger.StepOut()");
}

::agiru::Boolean Debugger::StepOver() {
  RefuseUnimplemented("Debugger.StepOver()");
}

::agiru::Boolean Debugger::Stop() {
  RefuseUnimplemented("Debugger.Stop()");
}

void Dialog::Close() {}

::agiru::Boolean
Dialog::Confirm(std::string_view String, ::agiru::Boolean Default, const ::agiru::Variant &Value1) {
  static_cast<void>(String);
  static_cast<void>(Default);
  static_cast<void>(Value1);
  RefuseUnimplemented("Dialog.Confirm(Text, Boolean, Any)");
}

void Dialog::Error(const ::agiru::ErrorInfo &Message) {
  static_cast<void>(Message);
  RefuseUnimplemented("Dialog.Error(ErrorInfo)");
}

void Dialog::Error(std::string_view Message, const ::agiru::Variant &Value) {
  static_cast<void>(Message);
  static_cast<void>(Value);
  RefuseUnimplemented("Dialog.Error(Text, Any)");
}

::agiru::Boolean Dialog::HideSubsequentDialogs(::agiru::Boolean HideSubsequentDialogs) {
  static_cast<void>(HideSubsequentDialogs);
  RefuseUnimplemented("Dialog.HideSubsequentDialogs(Boolean)");
}

void Dialog::LogInternalError(std::string_view Message,
                              const ::agiru::Variant &DataClassificationInstance,
                              const ::agiru::Verbosity &VerbosityInstance) {
  static_cast<void>(Message);
  static_cast<void>(DataClassificationInstance);
  static_cast<void>(VerbosityInstance);
  RefuseUnimplemented("Dialog.LogInternalError(Text, DataClassification, Verbosity)");
}

void Dialog::LogInternalError(std::string_view Message,
                              std::string_view SubstitutionString,
                              const ::agiru::Variant &DataClassificationInstance,
                              const ::agiru::Verbosity &VerbosityInstance) {
  static_cast<void>(Message);
  static_cast<void>(SubstitutionString);
  static_cast<void>(DataClassificationInstance);
  static_cast<void>(VerbosityInstance);
  RefuseUnimplemented("Dialog.LogInternalError(Text, Text, DataClassification, Verbosity)");
}

void Dialog::Message(std::string_view String, const ::agiru::Variant &Value) {
  static_cast<void>(String);
  static_cast<void>(Value);
  RefuseUnimplemented("Dialog.Message(Text, Any)");
}

void Dialog::Open(std::string_view String) {
  static_cast<void>(String);
}

void Dialog::Open(std::string_view String, ::agiru::Variant &Variable1) {
  static_cast<void>(String);
  static_cast<void>(Variable1);
}

::agiru::Integer Dialog::StrMenu(std::string_view OptionMembers,
                                 ::agiru::Integer DefaultNumber,
                                 std::string_view Instruction) {
  return ::agiru::StrMenu(OptionMembers, DefaultNumber, Instruction);
}

void Dialog::Update(::agiru::Integer Number, const ::agiru::Variant &Value) {
  static_cast<void>(Number);
  static_cast<void>(Value);
}

::agiru::Boolean File::Download(std::string_view FromFile,
                                std::string_view DialogTitle,
                                std::string_view ToFolder,
                                std::string_view ToFilter,
                                ::agiru::Text<0> &ToFile) {
  static_cast<void>(FromFile);
  static_cast<void>(DialogTitle);
  static_cast<void>(ToFolder);
  static_cast<void>(ToFilter);
  static_cast<void>(ToFile);
  RefuseUnimplemented("File.Download(Text, Text, Text, Text, Text)");
}

::agiru::Boolean File::DownloadFromStream(const ::agiru::InStream &InStream,
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

::agiru::Boolean File::GetStamp(std::string_view Name, ::agiru::Date &Date, ::agiru::Time &Time) {
  static_cast<void>(Name);
  static_cast<void>(Date);
  static_cast<void>(Time);
  RefuseUnimplemented("File.GetStamp(Text, Date, Time)");
}

::agiru::Boolean File::SetStamp(std::string_view Name, ::agiru::Date Date, ::agiru::Time Time) {
  static_cast<void>(Name);
  static_cast<void>(Date);
  static_cast<void>(Time);
  RefuseUnimplemented("File.SetStamp(Text, Date, Time)");
}

void File::Trunc() {
  RefuseUnimplemented("File.Trunc()");
}

::agiru::Boolean File::Upload(std::string_view DialogTitle,
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

::agiru::Boolean File::UploadIntoStream(std::string_view FromFilter, ::agiru::InStream &InStream) {
  static_cast<void>(FromFilter);
  static_cast<void>(InStream);
  RefuseUnimplemented("File.UploadIntoStream(Text, InStream)");
}

::agiru::Boolean File::UploadIntoStream(std::string_view DialogTitle,
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

::agiru::Boolean File::View(std::string_view FromFile, ::agiru::Boolean AllowDownloadAndPrint) {
  static_cast<void>(FromFile);
  static_cast<void>(AllowDownloadAndPrint);
  RefuseUnimplemented("File.View(Text, Boolean)");
}

::agiru::Boolean File::ViewFromStream(const ::agiru::InStream &InStream,
                                      std::string_view FileName,
                                      ::agiru::Boolean AllowDownloadAndPrint) {
  static_cast<void>(InStream);
  static_cast<void>(FileName);
  static_cast<void>(AllowDownloadAndPrint);
  RefuseUnimplemented("File.ViewFromStream(InStream, Text, Boolean)");
}

void FileUpload::CreateInStream(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseUnimplemented("FileUpload.CreateInStream(InStream)");
}

void FileUpload::CreateInStream(const ::agiru::InStream &InStream,
                                const ::agiru::TextEncoding &Encoding) {
  static_cast<void>(InStream);
  static_cast<void>(Encoding);
  RefuseUnimplemented("FileUpload.CreateInStream(InStream, TextEncoding)");
}

std::string FileUpload::FileName() {
  RefuseUnimplemented("FileUpload.FileName()");
}

FilterPageBuilder::Control_ *FilterPageBuilder::Named_(std::string_view name) {
  for (Control_ &control : controls_) {
    if (SameControlName(control.name, name)) { return &control; }
  }
  return nullptr;
}

FilterPageBuilder::Control_ &FilterPageBuilder::Require_(std::string_view name,
                                                         std::string_view method) {
  Control_ *control = Named_(name);
  if (control == nullptr) {
    throw Error("FilterPageBuilder." + std::string(method) + ": no filter control is named '" +
                std::string(name) + "'");
  }
  return *control;
}

std::string FilterPageBuilder::Add_(std::string_view name, ::agiru::RecordRef record) {
  if (Control_ *control = Named_(name); control != nullptr) {
    control->record = std::move(record);
    return control->name;
  }
  controls_.push_back(
      Control_{.name = std::string(name), .record = std::move(record), .fields = {}});
  return controls_.back().name;
}

::agiru::Boolean FilterPageBuilder::AddField(std::string_view Name,
                                             const ::agiru::FieldRef &Field,
                                             std::string_view Filter) {
  return AddFieldNo(Name, Field.Number(), Filter);
}

::agiru::Boolean FilterPageBuilder::AddField(std::string_view Name,
                                             const ::agiru::Variant &Field,
                                             std::string_view Filter) {
  if (Field.IsInteger()) { return AddFieldNo(Name, Field.Get<Integer>(), Filter); }
  throw Error("FilterPageBuilder.AddField(Any): the Variant holds no field number, and a FieldRef "
              "cannot be inside a Variant here (board:0035)");
}

::agiru::Boolean FilterPageBuilder::AddFieldNo(std::string_view Name,
                                               ::agiru::Integer FieldNo,
                                               std::string_view Filter) {
  Control_ &control = Require_(Name, "AddFieldNo");
  if (!control.record.FieldExist(FieldNo)) { return false; }
  bool listed = false;
  for (const Integer no : control.fields) { listed = listed || no == FieldNo; }
  if (!listed) { control.fields.push_back(FieldNo); }
  if (!Filter.empty()) { control.record.Field(FieldNo).SetFilter(Filter); }
  return true;
}

std::string FilterPageBuilder::AddRecord(std::string_view Name, const ::agiru::RecordRef &Record) {
  return AddRecordRef(Name, Record);
}

std::string FilterPageBuilder::AddRecordRef(std::string_view Name,
                                            const ::agiru::RecordRef &RecordRef) {
  ::agiru::RecordRef own;
  own.Copy(RecordRef);
  return Add_(Name, std::move(own));
}

std::string FilterPageBuilder::AddTable(const ::agiru::TextArgument &Name,
                                        ::agiru::Integer TableNo) {
  ::agiru::RecordRef own;
  own.Open(TableNo);
  return Add_(std::string_view(Name), std::move(own));
}

::agiru::Integer FilterPageBuilder::Count() {
  return static_cast<Integer>(controls_.size());
}

std::string FilterPageBuilder::GetView(std::string_view Name, ::agiru::Boolean UseNames) {
  return Require_(Name, "GetView").record.GetView(UseNames);
}

std::string FilterPageBuilder::Name(::agiru::Integer Index) {
  if (Index < 1 || static_cast<std::size_t>(Index) > controls_.size()) {
    throw Error("FilterPageBuilder.Name: the index " + std::to_string(Index) + " is outside 1.." +
                std::to_string(controls_.size()));
  }
  return controls_[static_cast<std::size_t>(Index) - 1].name;
}

std::string FilterPageBuilder::PageCaption() {
  return pageCaption_;
}

std::string FilterPageBuilder::PageCaption(std::string_view PageCaption) {
  pageCaption_ = std::string(PageCaption);
  return pageCaption_;
}

::agiru::Boolean FilterPageBuilder::RunModal() {
  const TestHandler *handler = HandlerTable::For(HandlerKind::FilterPage);
  if (handler == nullptr || controls_.empty()) { return false; }
  FilterPageAnswer answer{.record = controls_.front().record, .accepted = false};
  HandlerTable::Ran(*handler);
  handler->invoke(pageCaption_, &answer);
  return answer.accepted;
}

::agiru::Boolean FilterPageBuilder::SetView(std::string_view Name, std::string_view View) {
  Require_(Name, "SetView").record.SetView(View);
  return true;
}

void HttpClient::AddCertificate(const ::agiru::SecretText &Certificate,
                                const ::agiru::SecretText &Password) {
  static_cast<void>(Certificate);
  static_cast<void>(Password);
  RefuseUnimplemented("HttpClient.AddCertificate(SecretText, SecretText)");
}

void HttpClient::AddCertificate(std::string_view Certificate, std::string_view Password) {
  static_cast<void>(Certificate);
  static_cast<void>(Password);
  RefuseUnimplemented("HttpClient.AddCertificate(Text, Text)");
}

void HttpClient::Clear() {
  RefuseUnimplemented("HttpClient.Clear()");
}

::agiru::HttpHeaders HttpClient::DefaultRequestHeaders() {
  RefuseUnimplemented("HttpClient.DefaultRequestHeaders()");
}

::agiru::Boolean HttpClient::Delete(std::string_view Path, ::agiru::HttpResponseMessage &Response) {
  static_cast<void>(Path);
  static_cast<void>(Response);
  RefuseUnimplemented("HttpClient.Delete(Text, HttpResponseMessage)");
}

::agiru::Boolean HttpClient::Get(std::string_view Path, ::agiru::HttpResponseMessage &Response) {
  static_cast<void>(Path);
  static_cast<void>(Response);
  RefuseUnimplemented("HttpClient.Get(Text, HttpResponseMessage)");
}

std::string HttpClient::GetBaseAddress() {
  RefuseUnimplemented("HttpClient.GetBaseAddress()");
}

::agiru::Boolean HttpClient::Patch(std::string_view Path,
                                   const ::agiru::HttpContent &Content,
                                   ::agiru::HttpResponseMessage &Response) {
  static_cast<void>(Path);
  static_cast<void>(Content);
  static_cast<void>(Response);
  RefuseUnimplemented("HttpClient.Patch(Text, HttpContent, HttpResponseMessage)");
}

::agiru::Boolean HttpClient::Post(std::string_view Path,
                                  const ::agiru::HttpContent &Content,
                                  ::agiru::HttpResponseMessage &Response) {
  static_cast<void>(Path);
  static_cast<void>(Content);
  static_cast<void>(Response);
  RefuseUnimplemented("HttpClient.Post(Text, HttpContent, HttpResponseMessage)");
}

::agiru::Boolean HttpClient::Put(std::string_view Path,
                                 const ::agiru::HttpContent &Content,
                                 ::agiru::HttpResponseMessage &Response) {
  static_cast<void>(Path);
  static_cast<void>(Content);
  static_cast<void>(Response);
  RefuseUnimplemented("HttpClient.Put(Text, HttpContent, HttpResponseMessage)");
}

::agiru::Boolean HttpClient::Send(const ::agiru::HttpRequestMessage &Request,
                                  ::agiru::HttpResponseMessage &Response) {
  static_cast<void>(Request);
  static_cast<void>(Response);
  RefuseUnimplemented("HttpClient.Send(HttpRequestMessage, HttpResponseMessage)");
}

::agiru::Boolean HttpClient::SetBaseAddress(std::string_view NewBaseAddress) {
  static_cast<void>(NewBaseAddress);
  RefuseUnimplemented("HttpClient.SetBaseAddress(Text)");
}

::agiru::Duration HttpClient::Timeout(::agiru::Duration SetTimeout) {
  static_cast<void>(SetTimeout);
  RefuseUnimplemented("HttpClient.Timeout(Duration)");
}

::agiru::Boolean HttpClient::UseDefaultNetworkWindowsAuthentication() {
  RefuseUnimplemented("HttpClient.UseDefaultNetworkWindowsAuthentication()");
}

void HttpClient::UseResponseCookies(::agiru::Boolean UseResponseCookies) {
  static_cast<void>(UseResponseCookies);
  RefuseUnimplemented("HttpClient.UseResponseCookies(Boolean)");
}

::agiru::Boolean
HttpClient::UseServerCertificateValidation(::agiru::Boolean UseServerCertificateValidation) {
  static_cast<void>(UseServerCertificateValidation);
  RefuseUnimplemented("HttpClient.UseServerCertificateValidation(Boolean)");
}

::agiru::Boolean HttpClient::UseWindowsAuthentication(const ::agiru::SecretText &UserName,
                                                      const ::agiru::SecretText &Password,
                                                      const ::agiru::SecretText &Domain) {
  static_cast<void>(UserName);
  static_cast<void>(Password);
  static_cast<void>(Domain);
  RefuseUnimplemented("HttpClient.UseWindowsAuthentication(SecretText, SecretText, SecretText)");
}

::agiru::Boolean HttpClient::UseWindowsAuthentication(std::string_view UserName,
                                                      std::string_view Password,
                                                      std::string_view Domain) {
  static_cast<void>(UserName);
  static_cast<void>(Password);
  static_cast<void>(Domain);
  RefuseUnimplemented("HttpClient.UseWindowsAuthentication(Text, Text, Text)");
}

void HttpContent::Clear() {
  RefuseUnimplemented("HttpContent.Clear()");
}

::agiru::Boolean HttpContent::GetHeaders(::agiru::HttpHeaders &Headers) {
  static_cast<void>(Headers);
  RefuseUnimplemented("HttpContent.GetHeaders(HttpHeaders)");
}

::agiru::Boolean HttpContent::IsSecretContent() {
  RefuseUnimplemented("HttpContent.IsSecretContent()");
}

::agiru::Boolean HttpContent::ReadAs(::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseUnimplemented("HttpContent.ReadAs(InStream)");
}

::agiru::Boolean HttpContent::ReadAs(::agiru::SecretText &OutputSecretText) {
  static_cast<void>(OutputSecretText);
  RefuseUnimplemented("HttpContent.ReadAs(SecretText)");
}

::agiru::Boolean HttpContent::ReadAs(::agiru::Text<0> &OutputString) {
  static_cast<void>(OutputString);
  RefuseUnimplemented("HttpContent.ReadAs(Text)");
}

void HttpContent::WriteFrom(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseUnimplemented("HttpContent.WriteFrom(InStream)");
}

void HttpContent::WriteFrom(std::string_view Text) {
  static_cast<void>(Text);
  RefuseUnimplemented("HttpContent.WriteFrom(Text)");
}

::agiru::Boolean HttpHeaders::Add(std::string_view Name, std::string_view Value) {
  static_cast<void>(Name);
  static_cast<void>(Value);
  RefuseUnimplemented("HttpHeaders.Add(Text, Text)");
}

void HttpHeaders::Clear() {
  RefuseUnimplemented("HttpHeaders.Clear()");
}

::agiru::Boolean HttpHeaders::Contains(std::string_view Name) {
  static_cast<void>(Name);
  RefuseUnimplemented("HttpHeaders.Contains(Text)");
}

::agiru::Boolean HttpHeaders::ContainsSecret(std::string_view Key) {
  static_cast<void>(Key);
  RefuseUnimplemented("HttpHeaders.ContainsSecret(Text)");
}

::agiru::Boolean HttpHeaders::GetSecretValues(std::string_view Key,
                                              const ::agiru::List<::agiru::SecretText> &Values) {
  static_cast<void>(Key);
  static_cast<void>(Values);
  RefuseUnimplemented("HttpHeaders.GetSecretValues(Text, List of [SecretText])");
}

::agiru::Boolean HttpHeaders::GetSecretValues(std::string_view Key,
                                              const ::agiru::Variant &Values) {
  static_cast<void>(Key);
  static_cast<void>(Values);
  RefuseUnimplemented("HttpHeaders.GetSecretValues(Text, Array of [SecretText])");
}

::agiru::Boolean HttpHeaders::GetValues(std::string_view Key, const ::agiru::Variant &Values) {
  static_cast<void>(Key);
  static_cast<void>(Values);
  RefuseUnimplemented("HttpHeaders.GetValues(String, Array of [Text])");
}

::agiru::Boolean HttpHeaders::GetValues(std::string_view Key,
                                        const ::agiru::List<std::string> &Values) {
  static_cast<void>(Key);
  static_cast<void>(Values);
  RefuseUnimplemented("HttpHeaders.GetValues(Text, List of [Text])");
}

::agiru::List<std::string> HttpHeaders::Keys() {
  RefuseUnimplemented("HttpHeaders.Keys()");
}

::agiru::Boolean HttpHeaders::Remove(std::string_view Name) {
  static_cast<void>(Name);
  RefuseUnimplemented("HttpHeaders.Remove(Text)");
}

::agiru::Boolean HttpHeaders::TryAddWithoutValidation(std::string_view Name,
                                                      std::string_view Value) {
  static_cast<void>(Name);
  static_cast<void>(Value);
  RefuseUnimplemented("HttpHeaders.TryAddWithoutValidation(Text, Text)");
}

::agiru::HttpContent HttpRequestMessage::Content() {
  RefuseUnimplemented("HttpRequestMessage.Content()");
}

::agiru::HttpContent HttpRequestMessage::Content(const ::agiru::HttpContent &SetContent) {
  static_cast<void>(SetContent);
  RefuseUnimplemented("HttpRequestMessage.Content(HttpContent)");
}

::agiru::Boolean HttpRequestMessage::GetCookie(std::string_view Name, ::agiru::Cookie &Cookie) {
  static_cast<void>(Name);
  static_cast<void>(Cookie);
  RefuseUnimplemented("HttpRequestMessage.GetCookie(Text, Cookie)");
}

::agiru::List<std::string> HttpRequestMessage::GetCookieNames() {
  RefuseUnimplemented("HttpRequestMessage.GetCookieNames()");
}

::agiru::Boolean HttpRequestMessage::GetHeaders(::agiru::HttpHeaders &Headers) {
  static_cast<void>(Headers);
  RefuseUnimplemented("HttpRequestMessage.GetHeaders(HttpHeaders)");
}

std::string HttpRequestMessage::GetRequestUri() {
  RefuseUnimplemented("HttpRequestMessage.GetRequestUri()");
}

::agiru::SecretText HttpRequestMessage::GetSecretRequestUri() {
  RefuseUnimplemented("HttpRequestMessage.GetSecretRequestUri()");
}

std::string HttpRequestMessage::Method(std::string_view NewMethod) {
  static_cast<void>(NewMethod);
  RefuseUnimplemented("HttpRequestMessage.Method(Text)");
}

::agiru::Boolean HttpRequestMessage::RemoveCookie(std::string_view Name) {
  static_cast<void>(Name);
  RefuseUnimplemented("HttpRequestMessage.RemoveCookie(Text)");
}

::agiru::Boolean HttpRequestMessage::SetCookie(const ::agiru::Cookie &Cookie) {
  static_cast<void>(Cookie);
  RefuseUnimplemented("HttpRequestMessage.SetCookie(Cookie)");
}

::agiru::Boolean HttpRequestMessage::SetCookie(std::string_view Name, std::string_view Value) {
  static_cast<void>(Name);
  static_cast<void>(Value);
  RefuseUnimplemented("HttpRequestMessage.SetCookie(Text, Text)");
}

::agiru::Boolean HttpRequestMessage::SetRequestUri(std::string_view RequestUri) {
  static_cast<void>(RequestUri);
  RefuseUnimplemented("HttpRequestMessage.SetRequestUri(Text)");
}

::agiru::Boolean HttpRequestMessage::SetSecretRequestUri(const ::agiru::SecretText &RequestUri) {
  static_cast<void>(RequestUri);
  RefuseUnimplemented("HttpRequestMessage.SetSecretRequestUri(SecretText)");
}

::agiru::HttpContent HttpResponseMessage::Content() {
  RefuseUnimplemented("HttpResponseMessage.Content()");
}

::agiru::Boolean HttpResponseMessage::GetCookie(std::string_view Name, ::agiru::Cookie &Cookie) {
  static_cast<void>(Name);
  static_cast<void>(Cookie);
  RefuseUnimplemented("HttpResponseMessage.GetCookie(Text, Cookie)");
}

::agiru::List<std::string> HttpResponseMessage::GetCookieNames() {
  RefuseUnimplemented("HttpResponseMessage.GetCookieNames()");
}

::agiru::HttpHeaders HttpResponseMessage::Headers() {
  RefuseUnimplemented("HttpResponseMessage.Headers()");
}

::agiru::Integer HttpResponseMessage::HttpStatusCode() {
  RefuseUnimplemented("HttpResponseMessage.HttpStatusCode()");
}

::agiru::Boolean HttpResponseMessage::IsBlockedByEnvironment() {
  RefuseUnimplemented("HttpResponseMessage.IsBlockedByEnvironment()");
}

::agiru::Boolean HttpResponseMessage::IsSuccessStatusCode() {
  RefuseUnimplemented("HttpResponseMessage.IsSuccessStatusCode()");
}

std::string HttpResponseMessage::ReasonPhrase() {
  RefuseUnimplemented("HttpResponseMessage.ReasonPhrase()");
}

::agiru::JsonToken JsonArray::AsToken() {
  RefuseUnimplemented("JsonArray.AsToken()");
}

::agiru::JsonToken JsonArray::Clone() {
  RefuseUnimplemented("JsonArray.Clone()");
}

::agiru::JsonToken *JsonArray::begin() {
  RefuseUnimplemented("JsonArray.begin()");
}

::agiru::JsonToken *JsonArray::end() {
  RefuseUnimplemented("JsonArray.end()");
}

::agiru::JsonArray JsonArray::GetArray(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseUnimplemented("JsonArray.GetArray(Integer)");
}

::agiru::BigInteger JsonArray::GetBigInteger(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseUnimplemented("JsonArray.GetBigInteger(Integer)");
}

::agiru::Boolean JsonArray::GetBoolean(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseUnimplemented("JsonArray.GetBoolean(Integer)");
}

::agiru::Byte JsonArray::GetByte(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseUnimplemented("JsonArray.GetByte(Integer)");
}

::agiru::Char JsonArray::GetChar(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseUnimplemented("JsonArray.GetChar(Integer)");
}

::agiru::Date JsonArray::GetDate(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseUnimplemented("JsonArray.GetDate(Integer)");
}

::agiru::DateTime JsonArray::GetDateTime(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseUnimplemented("JsonArray.GetDateTime(Integer)");
}

::agiru::Decimal JsonArray::GetDecimal(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseUnimplemented("JsonArray.GetDecimal(Integer)");
}

::agiru::Integer JsonArray::GetDuration(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseUnimplemented("JsonArray.GetDuration(Integer)");
}

::agiru::Integer JsonArray::GetInteger(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseUnimplemented("JsonArray.GetInteger(Integer)");
}

::agiru::JsonObject JsonArray::GetObject(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseUnimplemented("JsonArray.GetObject(Integer)");
}

::agiru::Integer JsonArray::GetOption(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseUnimplemented("JsonArray.GetOption(Integer)");
}

std::string JsonArray::GetText(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseUnimplemented("JsonArray.GetText(Integer)");
}

::agiru::Time JsonArray::GetTime(::agiru::Integer Index) {
  static_cast<void>(Index);
  RefuseUnimplemented("JsonArray.GetTime(Integer)");
}

std::string JsonArray::Path() {
  RefuseUnimplemented("JsonArray.Path()");
}

::agiru::Boolean JsonArray::ReadFrom(const ::agiru::InStream &Data) {
  static_cast<void>(Data);
  RefuseUnimplemented("JsonArray.ReadFrom(InStream)");
}

::agiru::Boolean JsonArray::SelectTokens(std::string_view Path,
                                         ::agiru::List<::agiru::JsonToken> &Result) {
  static_cast<void>(Path);
  static_cast<void>(Result);
  RefuseUnimplemented("JsonArray.SelectTokens(Text, List of [JsonToken])");
}

::agiru::Boolean JsonArray::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseUnimplemented("JsonArray.WriteTo(OutStream)");
}

::agiru::JsonToken JsonObject::AsToken() {
  RefuseUnimplemented("JsonObject.AsToken()");
}

::agiru::JsonToken JsonObject::Clone() {
  RefuseUnimplemented("JsonObject.Clone()");
}

::agiru::BigInteger JsonObject::GetBigInteger(std::string_view Key,
                                              ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseUnimplemented("JsonObject.GetBigInteger(Text, Boolean)");
}

::agiru::Byte JsonObject::GetByte(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseUnimplemented("JsonObject.GetByte(Text, Boolean)");
}

::agiru::Char JsonObject::GetChar(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseUnimplemented("JsonObject.GetChar(Text, Boolean)");
}

::agiru::Date JsonObject::GetDate(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseUnimplemented("JsonObject.GetDate(Text, Boolean)");
}

::agiru::DateTime JsonObject::GetDateTime(std::string_view Key,
                                          ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseUnimplemented("JsonObject.GetDateTime(Text, Boolean)");
}

::agiru::Duration JsonObject::GetDuration(std::string_view Key,
                                          ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseUnimplemented("JsonObject.GetDuration(Text, Boolean)");
}

::agiru::Integer JsonObject::GetOption(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseUnimplemented("JsonObject.GetOption(Text, Boolean)");
}

::agiru::Time JsonObject::GetTime(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseUnimplemented("JsonObject.GetTime(Text, Boolean)");
}

std::string JsonObject::Path() {
  RefuseUnimplemented("JsonObject.Path()");
}

::agiru::Boolean JsonObject::ReadFrom(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseUnimplemented("JsonObject.ReadFrom(InStream)");
}

::agiru::Boolean JsonObject::ReadFromYaml(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseUnimplemented("JsonObject.ReadFromYaml(InStream)");
}

::agiru::Boolean JsonObject::ReadFromYaml(std::string_view String) {
  static_cast<void>(String);
  RefuseUnimplemented("JsonObject.ReadFromYaml(Text)");
}

::agiru::Boolean JsonObject::SelectTokens(std::string_view Path,
                                          ::agiru::List<::agiru::JsonToken> &Result) {
  static_cast<void>(Path);
  static_cast<void>(Result);
  RefuseUnimplemented("JsonObject.SelectTokens(Text, List of [JsonToken])");
}

::agiru::Boolean JsonObject::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseUnimplemented("JsonObject.WriteTo(OutStream)");
}

::agiru::Boolean JsonObject::WriteToYaml(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseUnimplemented("JsonObject.WriteToYaml(OutStream)");
}

::agiru::Boolean JsonObject::WriteToYaml(::agiru::Text<0> &String) {
  static_cast<void>(String);
  RefuseUnimplemented("JsonObject.WriteToYaml(Text)");
}

::agiru::Boolean JsonObject::WriteWithSecretsTo(
    const ::agiru::Dictionary<::agiru::Text<0>, ::agiru::SecretText> &Secrets,
    ::agiru::SecretText &Result) {
  static_cast<void>(Secrets);
  static_cast<void>(Result);
  RefuseUnimplemented(
      "JsonObject.WriteWithSecretsTo(Dictionary of [Text, SecretText], SecretText)");
}

::agiru::Boolean JsonObject::WriteWithSecretsTo(std::string_view Path,
                                                const ::agiru::SecretText &Secret,
                                                ::agiru::SecretText &Result) {
  static_cast<void>(Path);
  static_cast<void>(Secret);
  static_cast<void>(Result);
  RefuseUnimplemented("JsonObject.WriteWithSecretsTo(Text, SecretText, SecretText)");
}

::agiru::JsonToken JsonToken::Clone() {
  RefuseUnimplemented("JsonToken.Clone()");
}

std::string JsonToken::Path() {
  RefuseUnimplemented("JsonToken.Path()");
}

::agiru::Boolean JsonToken::ReadFrom(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseUnimplemented("JsonToken.ReadFrom(InStream)");
}

::agiru::Boolean JsonToken::ReadFrom(std::string_view String) {
  static_cast<void>(String);
  RefuseUnimplemented("JsonToken.ReadFrom(Text)");
}

::agiru::Boolean JsonToken::SelectToken(std::string_view Path, ::agiru::JsonToken &Result) {
  static_cast<void>(Path);
  static_cast<void>(Result);
  RefuseUnimplemented("JsonToken.SelectToken(Text, JsonToken)");
}

::agiru::Boolean JsonToken::SelectTokens(std::string_view Path,
                                         ::agiru::List<::agiru::JsonToken> &Result) {
  static_cast<void>(Path);
  static_cast<void>(Result);
  RefuseUnimplemented("JsonToken.SelectTokens(Text, List of [JsonToken])");
}

::agiru::Boolean JsonToken::WriteTo(const ::agiru::OutStream &Data) {
  static_cast<void>(Data);
  RefuseUnimplemented("JsonToken.WriteTo(OutStream)");
}

::agiru::Byte JsonValue::AsByte() {
  RefuseUnimplemented("JsonValue.AsByte()");
}

::agiru::Char JsonValue::AsChar() {
  RefuseUnimplemented("JsonValue.AsChar()");
}

::agiru::Date JsonValue::AsDate() {
  RefuseUnimplemented("JsonValue.AsDate()");
}

::agiru::DateTime JsonValue::AsDateTime() {
  RefuseUnimplemented("JsonValue.AsDateTime()");
}

::agiru::Duration JsonValue::AsDuration() {
  RefuseUnimplemented("JsonValue.AsDuration()");
}

::agiru::Integer JsonValue::AsOption() {
  RefuseUnimplemented("JsonValue.AsOption()");
}

::agiru::Time JsonValue::AsTime() {
  RefuseUnimplemented("JsonValue.AsTime()");
}

::agiru::JsonToken JsonValue::AsToken() {
  RefuseUnimplemented("JsonValue.AsToken()");
}

::agiru::JsonToken JsonValue::Clone() {
  RefuseUnimplemented("JsonValue.Clone()");
}

::agiru::Boolean JsonValue::IsUndefined() {
  RefuseUnimplemented("JsonValue.IsUndefined()");
}

std::string JsonValue::Path() {
  RefuseUnimplemented("JsonValue.Path()");
}

::agiru::Boolean JsonValue::ReadFrom(const ::agiru::InStream &Data) {
  static_cast<void>(Data);
  RefuseUnimplemented("JsonValue.ReadFrom(InStream)");
}

::agiru::Boolean JsonValue::ReadFrom(std::string_view Data) {
  static_cast<void>(Data);
  RefuseUnimplemented("JsonValue.ReadFrom(Text)");
}

::agiru::Boolean JsonValue::SelectToken(std::string_view Path, ::agiru::JsonToken &Result) {
  static_cast<void>(Path);
  static_cast<void>(Result);
  RefuseUnimplemented("JsonValue.SelectToken(Text, JsonToken)");
}

void JsonValue::SetValueToNull() {
  RefuseUnimplemented("JsonValue.SetValueToNull()");
}

void JsonValue::SetValueToUndefined() {
  RefuseUnimplemented("JsonValue.SetValueToUndefined()");
}

::agiru::Boolean JsonValue::WriteTo(const ::agiru::OutStream &Data) {
  static_cast<void>(Data);
  RefuseUnimplemented("JsonValue.WriteTo(OutStream)");
}

::agiru::Boolean JsonValue::WriteTo(::agiru::Text<0> &Data) {
  static_cast<void>(Data);
  RefuseUnimplemented("JsonValue.WriteTo(Text)");
}

::agiru::Boolean Label::Contains(std::string_view Value) {
  static_cast<void>(Value);
  RefuseUnimplemented("Label.Contains(Text)");
}

::agiru::Boolean Label::EndsWith(std::string_view Value) {
  static_cast<void>(Value);
  RefuseUnimplemented("Label.EndsWith(Text)");
}

::agiru::Integer Label::IndexOf(std::string_view Value, ::agiru::Integer StartIndex) {
  static_cast<void>(Value);
  static_cast<void>(StartIndex);
  RefuseUnimplemented("Label.IndexOf(Text, Integer)");
}

::agiru::Integer Label::IndexOfAny(const ::agiru::List<::agiru::Char> &Values,
                                   ::agiru::Integer StartIndex) {
  static_cast<void>(Values);
  static_cast<void>(StartIndex);
  RefuseUnimplemented("Label.IndexOfAny(List of [Char], Integer)");
}

::agiru::Integer Label::IndexOfAny(std::string_view Values, ::agiru::Integer StartIndex) {
  static_cast<void>(Values);
  static_cast<void>(StartIndex);
  RefuseUnimplemented("Label.IndexOfAny(Text, Integer)");
}

::agiru::Integer Label::LastIndexOf(std::string_view Value, ::agiru::Integer StartIndex) {
  static_cast<void>(Value);
  static_cast<void>(StartIndex);
  RefuseUnimplemented("Label.LastIndexOf(Text, Integer)");
}

std::string Label::PadLeft(::agiru::Integer Count, ::agiru::Char Char) {
  static_cast<void>(Count);
  static_cast<void>(Char);
  RefuseUnimplemented("Label.PadLeft(Integer, Char)");
}

std::string Label::PadRight(::agiru::Integer Count, ::agiru::Char Char) {
  static_cast<void>(Count);
  static_cast<void>(Char);
  RefuseUnimplemented("Label.PadRight(Integer, Char)");
}

std::string Label::Remove(::agiru::Integer StartIndex, ::agiru::Integer Count) {
  static_cast<void>(StartIndex);
  static_cast<void>(Count);
  RefuseUnimplemented("Label.Remove(Integer, Integer)");
}

std::string Label::Replace(std::string_view OldValue, std::string_view NewValue) {
  static_cast<void>(OldValue);
  static_cast<void>(NewValue);
  RefuseUnimplemented("Label.Replace(Text, Text)");
}

void Label::Split(const ::agiru::List<::agiru::Char> &Separators) {
  static_cast<void>(Separators);
  RefuseUnimplemented("Label.Split(List of [Char])");
}

void Label::Split(const ::agiru::List<std::string> &Separators) {
  static_cast<void>(Separators);
  RefuseUnimplemented("Label.Split(List of [Text])");
}

void Label::Split(std::string_view Separators) {
  static_cast<void>(Separators);
  RefuseUnimplemented("Label.Split(Text)");
}

::agiru::Boolean Label::StartsWith(std::string_view Value) {
  static_cast<void>(Value);
  RefuseUnimplemented("Label.StartsWith(Text)");
}

std::string Label::Substring(::agiru::Integer StartIndex, ::agiru::Integer Count) {
  static_cast<void>(StartIndex);
  static_cast<void>(Count);
  RefuseUnimplemented("Label.Substring(Integer, Integer)");
}

std::string Label::ToLower() {
  RefuseUnimplemented("Label.ToLower()");
}

std::string Label::ToUpper() {
  RefuseUnimplemented("Label.ToUpper()");
}

std::string Label::Trim() {
  RefuseUnimplemented("Label.Trim()");
}

std::string Label::TrimEnd(std::string_view Chars) {
  static_cast<void>(Chars);
  RefuseUnimplemented("Label.TrimEnd(Text)");
}

std::string Label::TrimStart(std::string_view Chars) {
  static_cast<void>(Chars);
  RefuseUnimplemented("Label.TrimStart(Text)");
}

void NavApp::DeleteArchiveData(::agiru::Integer TableNo) {
  static_cast<void>(TableNo);
  RefuseUnimplemented("NavApp.DeleteArchiveData(Integer)");
}

::agiru::Boolean NavApp::GetArchiveRecordRef(::agiru::Integer TableNo,
                                             ::agiru::RecordRef &RecordRef) {
  static_cast<void>(TableNo);
  static_cast<void>(RecordRef);
  RefuseUnimplemented("NavApp.GetArchiveRecordRef(Integer, RecordRef)");
}

std::string NavApp::GetArchiveVersion() {
  RefuseUnimplemented("NavApp.GetArchiveVersion()");
}

::agiru::List<::agiru::ModuleInfo> NavApp::GetCallerCallstackModuleInfos() {
  RefuseUnimplemented("NavApp.GetCallerCallstackModuleInfos()");
}

::agiru::Boolean NavApp::GetCallerModuleInfo(::agiru::ModuleInfo &Info) {
  static_cast<void>(Info);
  RefuseUnimplemented("NavApp.GetCallerModuleInfo(ModuleInfo)");
}

::agiru::List<::agiru::ModuleInfo> NavApp::GetCallstackModuleInfos() {
  RefuseUnimplemented("NavApp.GetCallstackModuleInfos()");
}

::agiru::Boolean NavApp::GetCurrentModuleInfo(::agiru::ModuleInfo &Info) {
  static_cast<void>(Info);
  RefuseUnimplemented("NavApp.GetCurrentModuleInfo(ModuleInfo)");
}

::agiru::Boolean NavApp::GetModuleInfo(::agiru::Guid AppId, ::agiru::ModuleInfo &Info) {
  static_cast<void>(AppId);
  static_cast<void>(Info);
  RefuseUnimplemented("NavApp.GetModuleInfo(Guid, ModuleInfo)");
}

void NavApp::GetResource(std::string_view ResourceName,
                         ::agiru::InStream &ResourceStream,
                         const ::agiru::TextEncoding &Encoding) {
  static_cast<void>(ResourceName);
  static_cast<void>(ResourceStream);
  static_cast<void>(Encoding);
  RefuseUnimplemented("NavApp.GetResource(Text, InStream, TextEncoding)");
}

::agiru::JsonObject NavApp::GetResourceAsJson(std::string_view ResourceName,
                                              const ::agiru::TextEncoding &Encoding) {
  static_cast<void>(ResourceName);
  static_cast<void>(Encoding);
  RefuseUnimplemented("NavApp.GetResourceAsJson(Text, TextEncoding)");
}

std::string NavApp::GetResourceAsText(std::string_view ResourceName,
                                      const ::agiru::TextEncoding &Encoding) {
  static_cast<void>(ResourceName);
  static_cast<void>(Encoding);
  RefuseUnimplemented("NavApp.GetResourceAsText(Text, TextEncoding)");
}

::agiru::Boolean NavApp::IsEntitled(std::string_view Id, ::agiru::Guid AppId) {
  static_cast<void>(Id);
  static_cast<void>(AppId);
  RefuseUnimplemented("NavApp.IsEntitled(Text, Guid)");
}

::agiru::Boolean NavApp::IsInstalling() {
  RefuseUnimplemented("NavApp.IsInstalling()");
}

::agiru::Boolean NavApp::IsUnlicensed(::agiru::Guid AppId) {
  static_cast<void>(AppId);
  RefuseUnimplemented("NavApp.IsUnlicensed(Guid)");
}

::agiru::List<std::string> NavApp::ListResources(std::string_view Filter) {
  static_cast<void>(Filter);
  RefuseUnimplemented("NavApp.ListResources(Text)");
}

void NavApp::LoadPackageData(::agiru::Integer TableNo) {
  static_cast<void>(TableNo);
  RefuseUnimplemented("NavApp.LoadPackageData(Integer)");
}

::agiru::Boolean NavApp::RestoreArchiveData(::agiru::Integer TableNo, ::agiru::Boolean RunTrigger) {
  static_cast<void>(TableNo);
  static_cast<void>(RunTrigger);
  RefuseUnimplemented("NavApp.RestoreArchiveData(Integer, Boolean)");
}

std::string ProductName::Full() {
  return "Dynamics 365 Business Central";
}

std::string ProductName::Marketing() {
  return "Microsoft Dynamics 365 Business Central";
}

std::string ProductName::Short() {
  return "Business Central";
}

::agiru::BigInteger SessionInformation::AITokensUsed() {
  RefuseUnimplemented("SessionInformation.AITokensUsed()");
}

std::string SessionInformation::Callstack() {
  return {};
}

::agiru::BigInteger SessionInformation::SqlRowsRead() {
  RefuseUnimplemented("SessionInformation.SqlRowsRead()");
}

::agiru::BigInteger SessionInformation::SqlStatementsExecuted() {
  RefuseUnimplemented("SessionInformation.SqlStatementsExecuted()");
}

std::string SessionSettings::Company(std::string_view NewCompanyName) {
  static_cast<void>(NewCompanyName);
  RefuseUnimplemented("SessionSettings.Company(Text)");
}

void SessionSettings::Init() {
  RefuseUnimplemented("SessionSettings.Init()");
}

::agiru::Integer SessionSettings::LanguageId(::agiru::Integer NewLanguageId) {
  static_cast<void>(NewLanguageId);
  RefuseUnimplemented("SessionSettings.LanguageId(Integer)");
}

::agiru::Integer SessionSettings::LocaleId(::agiru::Integer NewLocaleId) {
  static_cast<void>(NewLocaleId);
  RefuseUnimplemented("SessionSettings.LocaleId(Integer)");
}

::agiru::Guid SessionSettings::ProfileAppId(::agiru::Guid NewProfileAppId) {
  static_cast<void>(NewProfileAppId);
  RefuseUnimplemented("SessionSettings.ProfileAppId(Guid)");
}

std::string SessionSettings::ProfileId(std::string_view NewProfileId) {
  static_cast<void>(NewProfileId);
  RefuseUnimplemented("SessionSettings.ProfileId(Text)");
}

::agiru::Boolean SessionSettings::ProfileSystemScope(::agiru::Boolean NewProfileScope) {
  static_cast<void>(NewProfileScope);
  RefuseUnimplemented("SessionSettings.ProfileSystemScope(Boolean)");
}

void SessionSettings::RequestSessionUpdate(::agiru::Boolean saveSettings) {
  static_cast<void>(saveSettings);
  RefuseUnimplemented("SessionSettings.RequestSessionUpdate(Boolean)");
}

std::string SessionSettings::TimeZone(std::string_view NewTimeZone) {
  static_cast<void>(NewTimeZone);
  RefuseUnimplemented("SessionSettings.TimeZone(Text)");
}

::agiru::Boolean TaskScheduler::CancelTask(::agiru::Guid Task) {
  static_cast<void>(Task);
  RefuseUnimplemented("TaskScheduler.CancelTask(Guid)");
}

::agiru::Boolean TaskScheduler::CanCreateTask() {
  return true;
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
  RefuseUnimplemented(
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
  RefuseUnimplemented(
      "TaskScheduler.CreateTask(Integer, Integer, Boolean, Text, DateTime, RecordId)");
}

::agiru::Boolean TaskScheduler::SetTaskReady(::agiru::Guid Task, ::agiru::DateTime NotBefore) {
  static_cast<void>(Task);
  static_cast<void>(NotBefore);
  RefuseUnimplemented("TaskScheduler.SetTaskReady(Guid, DateTime)");
}

::agiru::Boolean TaskScheduler::TaskExists(::agiru::Guid Task) {
  static_cast<void>(Task);
  RefuseUnimplemented("TaskScheduler.TaskExists(Guid)");
}

::agiru::Boolean TestHttpRequestMessage::HasSecretUri() {
  RefuseUnimplemented("TestHttpRequestMessage.HasSecretUri()");
}

std::string TestHttpRequestMessage::Path() {
  RefuseUnimplemented("TestHttpRequestMessage.Path()");
}

::agiru::Dictionary<::agiru::Text<0>, std::string> TestHttpRequestMessage::QueryParameters() {
  RefuseUnimplemented("TestHttpRequestMessage.QueryParameters()");
}

::agiru::HttpRequestType TestHttpRequestMessage::RequestType() {
  RefuseUnimplemented("TestHttpRequestMessage.RequestType()");
}

::agiru::HttpContent TestHttpResponseMessage::Content() {
  RefuseUnimplemented("TestHttpResponseMessage.Content()");
}

::agiru::HttpHeaders TestHttpResponseMessage::Headers() {
  RefuseUnimplemented("TestHttpResponseMessage.Headers()");
}

::agiru::Integer TestHttpResponseMessage::HttpStatusCode(::agiru::Integer SetStatusCode) {
  static_cast<void>(SetStatusCode);
  RefuseUnimplemented("TestHttpResponseMessage.HttpStatusCode(Integer)");
}

::agiru::Boolean
TestHttpResponseMessage::IsBlockedByEnvironment(::agiru::Boolean SetIsBlockedByEnvironment) {
  static_cast<void>(SetIsBlockedByEnvironment);
  RefuseUnimplemented("TestHttpResponseMessage.IsBlockedByEnvironment(Boolean)");
}

::agiru::Boolean
TestHttpResponseMessage::IsSuccessfulRequest(::agiru::Boolean SetIsSuccessfulRequest) {
  static_cast<void>(SetIsSuccessfulRequest);
  RefuseUnimplemented("TestHttpResponseMessage.IsSuccessfulRequest(Boolean)");
}

std::string TestHttpResponseMessage::ReasonPhrase(std::string_view SetReasonPhrase) {
  static_cast<void>(SetReasonPhrase);
  RefuseUnimplemented("TestHttpResponseMessage.ReasonPhrase(Text)");
}

::agiru::Boolean TextConst::Contains(std::string_view Value) {
  static_cast<void>(Value);
  RefuseUnimplemented("TextConst.Contains(Text)");
}

::agiru::Boolean TextConst::EndsWith(std::string_view Value) {
  static_cast<void>(Value);
  RefuseUnimplemented("TextConst.EndsWith(Text)");
}

::agiru::Integer TextConst::IndexOf(std::string_view Value, ::agiru::Integer StartIndex) {
  static_cast<void>(Value);
  static_cast<void>(StartIndex);
  RefuseUnimplemented("TextConst.IndexOf(Text, Integer)");
}

::agiru::Integer TextConst::IndexOfAny(const ::agiru::List<::agiru::Char> &Values,
                                       ::agiru::Integer StartIndex) {
  static_cast<void>(Values);
  static_cast<void>(StartIndex);
  RefuseUnimplemented("TextConst.IndexOfAny(List of [Char], Integer)");
}

::agiru::Integer TextConst::IndexOfAny(std::string_view Values, ::agiru::Integer StartIndex) {
  static_cast<void>(Values);
  static_cast<void>(StartIndex);
  RefuseUnimplemented("TextConst.IndexOfAny(Text, Integer)");
}

::agiru::Integer TextConst::LastIndexOf(std::string_view Value, ::agiru::Integer StartIndex) {
  static_cast<void>(Value);
  static_cast<void>(StartIndex);
  RefuseUnimplemented("TextConst.LastIndexOf(Text, Integer)");
}

std::string TextConst::PadLeft(::agiru::Integer Count, ::agiru::Char Char) {
  static_cast<void>(Count);
  static_cast<void>(Char);
  RefuseUnimplemented("TextConst.PadLeft(Integer, Char)");
}

std::string TextConst::PadRight(::agiru::Integer Count, ::agiru::Char Char) {
  static_cast<void>(Count);
  static_cast<void>(Char);
  RefuseUnimplemented("TextConst.PadRight(Integer, Char)");
}

std::string TextConst::Remove(::agiru::Integer StartIndex, ::agiru::Integer Count) {
  static_cast<void>(StartIndex);
  static_cast<void>(Count);
  RefuseUnimplemented("TextConst.Remove(Integer, Integer)");
}

std::string TextConst::Replace(std::string_view OldValue, std::string_view NewValue) {
  static_cast<void>(OldValue);
  static_cast<void>(NewValue);
  RefuseUnimplemented("TextConst.Replace(Text, Text)");
}

void TextConst::Split(const ::agiru::List<::agiru::Char> &Separators) {
  static_cast<void>(Separators);
  RefuseUnimplemented("TextConst.Split(List of [Char])");
}

void TextConst::Split(const ::agiru::List<std::string> &Separators) {
  static_cast<void>(Separators);
  RefuseUnimplemented("TextConst.Split(List of [Text])");
}

void TextConst::Split(std::string_view Separators) {
  static_cast<void>(Separators);
  RefuseUnimplemented("TextConst.Split(Text)");
}

::agiru::Boolean TextConst::StartsWith(std::string_view Value) {
  static_cast<void>(Value);
  RefuseUnimplemented("TextConst.StartsWith(Text)");
}

std::string TextConst::Substring(::agiru::Integer StartIndex, ::agiru::Integer Count) {
  static_cast<void>(StartIndex);
  static_cast<void>(Count);
  RefuseUnimplemented("TextConst.Substring(Integer, Integer)");
}

std::string TextConst::ToLower() {
  RefuseUnimplemented("TextConst.ToLower()");
}

std::string TextConst::ToUpper() {
  RefuseUnimplemented("TextConst.ToUpper()");
}

std::string TextConst::Trim() {
  RefuseUnimplemented("TextConst.Trim()");
}

std::string TextConst::TrimEnd(std::string_view Chars) {
  static_cast<void>(Chars);
  RefuseUnimplemented("TextConst.TrimEnd(Text)");
}

std::string TextConst::TrimStart(std::string_view Chars) {
  static_cast<void>(Chars);
  RefuseUnimplemented("TextConst.TrimStart(Text)");
}

::agiru::Boolean WebServiceActionContext::AddEntityKey(::agiru::Integer FieldId,
                                                       const ::agiru::Variant &FieldValue) {
  static_cast<void>(FieldId);
  static_cast<void>(FieldValue);
  RefuseUnimplemented("WebServiceActionContext.AddEntityKey(Integer, Any)");
}

::agiru::Integer WebServiceActionContext::GetObjectId() {
  RefuseUnimplemented("WebServiceActionContext.GetObjectId()");
}

::agiru::ObjectType WebServiceActionContext::GetObjectType() {
  RefuseUnimplemented("WebServiceActionContext.GetObjectType()");
}

::agiru::WebServiceActionResultCode WebServiceActionContext::GetResultCode() {
  RefuseUnimplemented("WebServiceActionContext.GetResultCode()");
}

void WebServiceActionContext::SetObjectId(::agiru::Integer ObjectId) {
  static_cast<void>(ObjectId);
  RefuseUnimplemented("WebServiceActionContext.SetObjectId(Integer)");
}

void WebServiceActionContext::SetObjectType(const ::agiru::ObjectType &ObjectType) {
  static_cast<void>(ObjectType);
  RefuseUnimplemented("WebServiceActionContext.SetObjectType(ObjectType)");
}

void WebServiceActionContext::SetResultCode(const ::agiru::WebServiceActionResultCode &ResultCode) {
  static_cast<void>(ResultCode);
  RefuseUnimplemented("WebServiceActionContext.SetResultCode(WebServiceActionResultCode)");
}

std::string SessionSettings::Company() {
  RefuseUnimplemented("SessionSettings.Company()");
}

::agiru::Integer SessionSettings::LanguageId() {
  RefuseUnimplemented("SessionSettings.LanguageId()");
}

::agiru::Integer SessionSettings::LocaleId() {
  RefuseUnimplemented("SessionSettings.LocaleId()");
}

::agiru::Guid SessionSettings::ProfileAppId() {
  RefuseUnimplemented("SessionSettings.ProfileAppId()");
}

std::string SessionSettings::ProfileId() {
  RefuseUnimplemented("SessionSettings.ProfileId()");
}

::agiru::Boolean SessionSettings::ProfileSystemScope() {
  RefuseUnimplemented("SessionSettings.ProfileSystemScope()");
}

std::string SessionSettings::TimeZone() {
  RefuseUnimplemented("SessionSettings.TimeZone()");
}

::agiru::Boolean NavApp::IsUnlicensed() {
  RefuseUnimplemented("NavApp.IsUnlicensed()");
}

::agiru::Boolean DataTransfer::UpdateAuditFields() {
  RefuseUnimplemented("DataTransfer.UpdateAuditFields()");
}

::agiru::Boolean Dialog::HideSubsequentDialogs() {
  RefuseUnimplemented("Dialog.HideSubsequentDialogs()");
}

::agiru::Duration HttpClient::Timeout() {
  RefuseUnimplemented("HttpClient.Timeout()");
}

std::string HttpRequestMessage::Method() {
  RefuseUnimplemented("HttpRequestMessage.Method()");
}

::agiru::Boolean TestHttpResponseMessage::IsSuccessfulRequest() {
  RefuseUnimplemented("TestHttpResponseMessage.IsSuccessfulRequest()");
}

::agiru::Integer TestHttpResponseMessage::HttpStatusCode() {
  RefuseUnimplemented("TestHttpResponseMessage.HttpStatusCode()");
}

::agiru::Boolean TestHttpResponseMessage::IsBlockedByEnvironment() {
  RefuseUnimplemented("TestHttpResponseMessage.IsBlockedByEnvironment()");
}

std::string TestHttpResponseMessage::ReasonPhrase() {
  RefuseUnimplemented("TestHttpResponseMessage.ReasonPhrase()");
}

::agiru::Boolean File::GetStamp(std::string_view Name, ::agiru::Date &Date) {
  static_cast<void>(Name);
  static_cast<void>(Date);
  RefuseUnimplemented("File.GetStamp()");
}

::agiru::Guid TaskScheduler::CreateTask(::agiru::Integer CodeunitId,
                                        ::agiru::Integer FailureCodeunitId,
                                        ::agiru::Boolean IsReady) {
  static_cast<void>(CodeunitId);
  static_cast<void>(FailureCodeunitId);
  static_cast<void>(IsReady);
  RefuseUnimplemented("TaskScheduler.CreateTask()");
}

::agiru::Boolean File::GetStamp(std::string_view Name) {
  static_cast<void>(Name);
  RefuseUnimplemented("File.GetStamp()");
}

}

// NOLINTEND(bugprone-easily-swappable-parameters,performance-unnecessary-value-param)
