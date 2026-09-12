// NOLINTBEGIN(bugprone-easily-swappable-parameters,performance-unnecessary-value-param)
#include "platform/Company.h"
#include "runtime/Error.h"
#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/TestRunner.h"
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

#include "BuiltinsWritten.h"

#include <cctype>
#include <string>
#include <string_view>
#include <utility>

namespace agiru {

namespace {

[[noreturn]] void RefuseDoor(std::string_view what) {
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
  RefuseDoor("BigText.ToText()");
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

::agiru::Integer BigText::GetSubText(::agiru::Text<0> &Variable,
                                     ::agiru::Integer Position,
                                     ::agiru::Integer Length) {
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
  const std::string name = std::string(Session::Current().CompanyName());
  platform::Company company;
  if (company.Get(name) && company.DisplayName != "") {
    return std::string(company.DisplayName.Value());
  }
  return name;
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

void Dialog::Close() {}

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
  RefuseDoor("File.Download(Text, Text, Text, Text, Text)");
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
  RefuseDoor("File.DownloadFromStream(InStream, Text, Text, Text, Text)");
}

::agiru::Boolean File::GetStamp(std::string_view Name, ::agiru::Date &Date, ::agiru::Time &Time) {
  static_cast<void>(Name);
  static_cast<void>(Date);
  static_cast<void>(Time);
  RefuseDoor("File.GetStamp(Text, Date, Time)");
}

::agiru::Boolean File::SetStamp(std::string_view Name, ::agiru::Date Date, ::agiru::Time Time) {
  static_cast<void>(Name);
  static_cast<void>(Date);
  static_cast<void>(Time);
  RefuseDoor("File.SetStamp(Text, Date, Time)");
}

void File::Trunc() {
  RefuseDoor("File.Trunc()");
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
                                        ::agiru::Text<0> &FromFile,
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

::agiru::Boolean HttpContent::ReadAs(::agiru::Text<0> &OutputString) {
  static_cast<void>(OutputString);
  RefuseDoor("HttpContent.ReadAs(Text)");
}

void HttpContent::WriteFrom(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseDoor("HttpContent.WriteFrom(InStream)");
}

void HttpContent::WriteFrom(std::string_view Text) {
  static_cast<void>(Text);
  RefuseDoor("HttpContent.WriteFrom(Text)");
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

::agiru::List<std::string> HttpHeaders::Keys() {
  RefuseDoor("HttpHeaders.Keys()");
}

::agiru::Boolean HttpHeaders::Remove(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("HttpHeaders.Remove(Text)");
}

::agiru::Boolean HttpHeaders::TryAddWithoutValidation(std::string_view Name,
                                                      std::string_view Value) {
  static_cast<void>(Name);
  static_cast<void>(Value);
  RefuseDoor("HttpHeaders.TryAddWithoutValidation(Text, Text)");
}

::agiru::HttpContent HttpRequestMessage::Content() {
  RefuseDoor("HttpRequestMessage.Content()");
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

::agiru::List<std::string> HttpRequestMessage::GetCookieNames() {
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

::agiru::List<std::string> HttpResponseMessage::GetCookieNames() {
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

std::string JsonArray::Path() {
  RefuseDoor("JsonArray.Path()");
}

::agiru::Boolean JsonArray::ReadFrom(const ::agiru::InStream &Data) {
  static_cast<void>(Data);
  RefuseDoor("JsonArray.ReadFrom(InStream)");
}

::agiru::Boolean JsonArray::SelectTokens(std::string_view Path,
                                         ::agiru::List<::agiru::JsonToken> &Result) {
  static_cast<void>(Path);
  static_cast<void>(Result);
  RefuseDoor("JsonArray.SelectTokens(Text, List of [JsonToken])");
}

::agiru::Boolean JsonArray::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("JsonArray.WriteTo(OutStream)");
}

::agiru::JsonToken JsonObject::AsToken() {
  RefuseDoor("JsonObject.AsToken()");
}

::agiru::JsonToken JsonObject::Clone() {
  RefuseDoor("JsonObject.Clone()");
}

::agiru::BigInteger JsonObject::GetBigInteger(std::string_view Key,
                                              ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetBigInteger(Text, Boolean)");
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

::agiru::Duration JsonObject::GetDuration(std::string_view Key,
                                          ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetDuration(Text, Boolean)");
}

::agiru::Integer JsonObject::GetOption(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetOption(Text, Boolean)");
}

::agiru::Time JsonObject::GetTime(std::string_view Key, ::agiru::Boolean DefaultIfNotFound) {
  static_cast<void>(Key);
  static_cast<void>(DefaultIfNotFound);
  RefuseDoor("JsonObject.GetTime(Text, Boolean)");
}

std::string JsonObject::Path() {
  RefuseDoor("JsonObject.Path()");
}

::agiru::Boolean JsonObject::ReadFrom(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseDoor("JsonObject.ReadFrom(InStream)");
}

::agiru::Boolean JsonObject::ReadFromYaml(const ::agiru::InStream &InStream) {
  static_cast<void>(InStream);
  RefuseDoor("JsonObject.ReadFromYaml(InStream)");
}

::agiru::Boolean JsonObject::ReadFromYaml(std::string_view String) {
  static_cast<void>(String);
  RefuseDoor("JsonObject.ReadFromYaml(Text)");
}

::agiru::Boolean JsonObject::SelectTokens(std::string_view Path,
                                          ::agiru::List<::agiru::JsonToken> &Result) {
  static_cast<void>(Path);
  static_cast<void>(Result);
  RefuseDoor("JsonObject.SelectTokens(Text, List of [JsonToken])");
}

::agiru::Boolean JsonObject::WriteTo(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("JsonObject.WriteTo(OutStream)");
}

::agiru::Boolean JsonObject::WriteToYaml(const ::agiru::OutStream &OutStream) {
  static_cast<void>(OutStream);
  RefuseDoor("JsonObject.WriteToYaml(OutStream)");
}

::agiru::Boolean JsonObject::WriteToYaml(::agiru::Text<0> &String) {
  static_cast<void>(String);
  RefuseDoor("JsonObject.WriteToYaml(Text)");
}

::agiru::Boolean JsonObject::WriteWithSecretsTo(
    const ::agiru::Dictionary<::agiru::Text<0>, ::agiru::SecretText> &Secrets,
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

::agiru::JsonToken JsonToken::Clone() {
  RefuseDoor("JsonToken.Clone()");
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

::agiru::Byte JsonValue::AsByte() {
  RefuseDoor("JsonValue.AsByte()");
}

::agiru::Char JsonValue::AsChar() {
  RefuseDoor("JsonValue.AsChar()");
}

::agiru::Date JsonValue::AsDate() {
  RefuseDoor("JsonValue.AsDate()");
}

::agiru::DateTime JsonValue::AsDateTime() {
  RefuseDoor("JsonValue.AsDateTime()");
}

::agiru::Duration JsonValue::AsDuration() {
  RefuseDoor("JsonValue.AsDuration()");
}

::agiru::Integer JsonValue::AsOption() {
  RefuseDoor("JsonValue.AsOption()");
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

::agiru::Boolean JsonValue::WriteTo(::agiru::Text<0> &Data) {
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

::agiru::List<::agiru::ModuleInfo> NavApp::GetCallerCallstackModuleInfos() {
  RefuseDoor("NavApp.GetCallerCallstackModuleInfos()");
}

::agiru::Boolean NavApp::GetCallerModuleInfo(::agiru::ModuleInfo &Info) {
  static_cast<void>(Info);
  RefuseDoor("NavApp.GetCallerModuleInfo(ModuleInfo)");
}

::agiru::List<::agiru::ModuleInfo> NavApp::GetCallstackModuleInfos() {
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

::agiru::List<std::string> NavApp::ListResources(std::string_view Filter) {
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
  RefuseDoor("SessionInformation.AITokensUsed()");
}

std::string SessionInformation::Callstack() {
  return {};
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

::agiru::Dictionary<::agiru::Text<0>, std::string> TestHttpRequestMessage::QueryParameters() {
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

std::string SessionSettings::Company() {
  RefuseDoor("SessionSettings.Company()");
}

::agiru::Integer SessionSettings::LanguageId() {
  RefuseDoor("SessionSettings.LanguageId()");
}

::agiru::Integer SessionSettings::LocaleId() {
  RefuseDoor("SessionSettings.LocaleId()");
}

::agiru::Guid SessionSettings::ProfileAppId() {
  RefuseDoor("SessionSettings.ProfileAppId()");
}

std::string SessionSettings::ProfileId() {
  RefuseDoor("SessionSettings.ProfileId()");
}

::agiru::Boolean SessionSettings::ProfileSystemScope() {
  RefuseDoor("SessionSettings.ProfileSystemScope()");
}

std::string SessionSettings::TimeZone() {
  RefuseDoor("SessionSettings.TimeZone()");
}

::agiru::Boolean NavApp::IsUnlicensed() {
  RefuseDoor("NavApp.IsUnlicensed()");
}

::agiru::Boolean DataTransfer::UpdateAuditFields() {
  RefuseDoor("DataTransfer.UpdateAuditFields()");
}

::agiru::Boolean Dialog::HideSubsequentDialogs() {
  RefuseDoor("Dialog.HideSubsequentDialogs()");
}

::agiru::Duration HttpClient::Timeout() {
  RefuseDoor("HttpClient.Timeout()");
}

std::string HttpRequestMessage::Method() {
  RefuseDoor("HttpRequestMessage.Method()");
}

::agiru::Boolean TestHttpResponseMessage::IsSuccessfulRequest() {
  RefuseDoor("TestHttpResponseMessage.IsSuccessfulRequest()");
}

::agiru::Integer TestHttpResponseMessage::HttpStatusCode() {
  RefuseDoor("TestHttpResponseMessage.HttpStatusCode()");
}

::agiru::Boolean TestHttpResponseMessage::IsBlockedByEnvironment() {
  RefuseDoor("TestHttpResponseMessage.IsBlockedByEnvironment()");
}

std::string TestHttpResponseMessage::ReasonPhrase() {
  RefuseDoor("TestHttpResponseMessage.ReasonPhrase()");
}

::agiru::Boolean File::GetStamp(std::string_view Name, ::agiru::Date &Date) {
  static_cast<void>(Name);
  static_cast<void>(Date);
  RefuseDoor("File.GetStamp()");
}

::agiru::Guid TaskScheduler::CreateTask(::agiru::Integer CodeunitId,
                                        ::agiru::Integer FailureCodeunitId,
                                        ::agiru::Boolean IsReady) {
  static_cast<void>(CodeunitId);
  static_cast<void>(FailureCodeunitId);
  static_cast<void>(IsReady);
  RefuseDoor("TaskScheduler.CreateTask()");
}

::agiru::Boolean File::GetStamp(std::string_view Name) {
  static_cast<void>(Name);
  RefuseDoor("File.GetStamp()");
}

}

// NOLINTEND(bugprone-easily-swappable-parameters,performance-unnecessary-value-param)
