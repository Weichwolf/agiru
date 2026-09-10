#include "type/ErrorInfo.h"

#include "runtime/RecordRef.h"
#include "type/Boolean.h"
#include "type/DataClassification.h"
#include "type/Dictionary.h"
#include "type/ErrorType.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/Verbosity.h"

#include <string>
#include <string_view>
#include <utility>

namespace agiru {

void ErrorInfo::AddAction(std::string_view Caption,
                          ::agiru::Integer CodeunitID,
                          std::string_view MethodName) {
  AddAction(Caption, CodeunitID, MethodName, {});
}

void ErrorInfo::AddAction(std::string_view Caption,
                          ::agiru::Integer CodeunitID,
                          std::string_view MethodName,
                          std::string_view Description) {
  actions_.push_back(Action{.caption = std::string(Caption),
                            .codeunitId = CodeunitID,
                            .methodName = std::string(MethodName),
                            .description = std::string(Description),
                            .navigation = false});
}

void ErrorInfo::AddNavigationAction(std::string_view Caption) {
  AddNavigationAction(Caption, {});
}

void ErrorInfo::AddNavigationAction(std::string_view Caption, std::string_view Description) {
  actions_.push_back(Action{.caption = std::string(Caption),
                            .codeunitId = 0,
                            .methodName = {},
                            .description = std::string(Description),
                            .navigation = true});
}

std::string ErrorInfo::Callstack() {
  return callstack_;
}

::agiru::Boolean ErrorInfo::Collectible() const {
  return collectible_;
}

::agiru::Boolean ErrorInfo::Collectible(::agiru::Boolean Collectible) {
  collectible_ = Collectible;
  return collectible_;
}

std::string ErrorInfo::ControlName() {
  return controlName_;
}

std::string ErrorInfo::ControlName(std::string_view ControlName) {
  controlName_ = std::string(ControlName);
  return controlName_;
}

::agiru::ErrorInfo ErrorInfo::Create() {
  ::agiru::ErrorInfo info;
  info.collectible_ = true;
  return info;
}

::agiru::ErrorInfo ErrorInfo::Create(std::string_view Message, ::agiru::Boolean Collectible) {
  ::agiru::ErrorInfo info;
  info.message_ = std::string(Message);
  info.collectible_ = Collectible;
  return info;
}

::agiru::ErrorInfo
ErrorInfo::Create(std::string_view Message,
                  ::agiru::Boolean Collectible,
                  ::agiru::RecordRef &Record,
                  ::agiru::Integer FieldNo,
                  ::agiru::Integer PageNo,
                  std::string_view ControlName,
                  const ::agiru::Verbosity &Verbosity,
                  ::agiru::DataClassification DataClassification,
                  const ::agiru::Dictionary<::agiru::Text<0>, std::string> &CustomDimensions) {
  ::agiru::ErrorInfo info = Create(Message, Collectible);
  info.tableId_ = Record.Number();
  info.fieldNo_ = FieldNo;
  info.pageNo_ = PageNo;
  info.controlName_ = std::string(ControlName);
  info.verbosity_ = Verbosity;
  info.dataClassification_ = DataClassification;
  info.customDimensions_ = CustomDimensions;
  return info;
}

::agiru::Dictionary<::agiru::Text<0>, std::string> ErrorInfo::CustomDimensions() {
  return customDimensions_;
}

void ErrorInfo::CustomDimensions(
    const ::agiru::Dictionary<::agiru::Text<0>, std::string> &CustomDimensions) {
  customDimensions_ = CustomDimensions;
}

::agiru::DataClassification ErrorInfo::DataClassification() {
  return dataClassification_;
}

::agiru::DataClassification
ErrorInfo::DataClassification(::agiru::DataClassification NewDataClassification) {
  dataClassification_ = NewDataClassification;
  return dataClassification_;
}

std::string ErrorInfo::DetailedMessage() {
  return detailedMessage_;
}

std::string ErrorInfo::DetailedMessage(std::string_view DetailedMessage) {
  detailedMessage_ = std::string(DetailedMessage);
  return detailedMessage_;
}

::agiru::ErrorType ErrorInfo::ErrorType() {
  return errorType_;
}

::agiru::ErrorType ErrorInfo::ErrorType(const ::agiru::ErrorType &ErrorType) {
  errorType_ = ErrorType;
  return errorType_;
}

::agiru::Integer ErrorInfo::FieldNo() const {
  return fieldNo_;
}

::agiru::Integer ErrorInfo::FieldNo(::agiru::Integer FieldNo) {
  fieldNo_ = FieldNo;
  return fieldNo_;
}

std::string ErrorInfo::Message() {
  return message_;
}

std::string ErrorInfo::Message(std::string_view Message) {
  message_ = std::string(Message);
  return message_;
}

::agiru::Integer ErrorInfo::PageNo() const {
  return pageNo_;
}

::agiru::Integer ErrorInfo::PageNo(::agiru::Integer PageNo) {
  pageNo_ = PageNo;
  return pageNo_;
}

::agiru::RecordId ErrorInfo::RecordId(::agiru::RecordId RecordId) {
  if (RecordId != ::agiru::RecordId{}) { recordId_ = std::move(RecordId); }
  return recordId_;
}

::agiru::Guid ErrorInfo::SystemId() {
  return systemId_;
}

::agiru::Guid ErrorInfo::SystemId(::agiru::Guid SystemId) {
  systemId_ = SystemId;
  return systemId_;
}

::agiru::Integer ErrorInfo::TableId() const {
  return tableId_;
}

::agiru::Integer ErrorInfo::TableId(::agiru::Integer TableId) {
  tableId_ = TableId;
  return tableId_;
}

std::string ErrorInfo::Title() {
  return title_;
}

std::string ErrorInfo::Title(std::string_view Title) {
  title_ = std::string(Title);
  return title_;
}

::agiru::Verbosity ErrorInfo::Verbosity() {
  return verbosity_;
}

::agiru::Verbosity ErrorInfo::Verbosity(const ::agiru::Verbosity &Verbosity) {
  verbosity_ = Verbosity;
  return verbosity_;
}

}
