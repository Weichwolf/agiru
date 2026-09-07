#pragma once

#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/DataClassification.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Dictionary.h"
#include "type/Duration.h"
#include "type/ErrorType.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/Time.h"
#include "type/Variant.h"
#include "type/Verbosity.h"

#include <string>
#include <string_view>
#include <vector>

/// \file
/// \brief AL `ErrorInfo` -- the surface the platform documentation declares.

namespace agiru {

/// \brief Declared in `runtime/Table.h`; named here so a record can reach an error.
/// \tparam T The table.
template <typename T> struct TableTraits;

}

namespace agiru {

class RecordRef;

/// \brief AL `ErrorInfo` -- an error and what it is about: its message, the record, the field,
///        the page and whether it is collected rather than raised (`methods-auto/errorinfo/`).
///
/// \note COLLECTIBILITY IS CARRIED AND NOT YET ACTED ON: `Error(ErrorInfo)` raises whatever the
///       info says, because the collecting mode itself is board:0195.
class ErrorInfo {
public:
  /// \brief AL `ErrorInfo.AddAction(Text, Integer, Text)`. Specifies an action for the error.
  /// \param Caption The AL `Text`.
  /// \param CodeunitID The AL `Integer`.
  /// \param MethodName The AL `Text`.
  void
  AddAction(std::string_view Caption, ::agiru::Integer CodeunitID, std::string_view MethodName);

  /// \brief AL `ErrorInfo.AddAction(Text, Integer, Text, Text)`. Specifies an action for the error.
  /// \param Caption The AL `Text`.
  /// \param CodeunitID The AL `Integer`.
  /// \param MethodName The AL `Text`.
  /// \param Description The AL `Text`.
  void AddAction(std::string_view Caption,
                 ::agiru::Integer CodeunitID,
                 std::string_view MethodName,
                 std::string_view Description);

  /// \brief AL `ErrorInfo.AddNavigationAction(Text)`. Adds a navigation action for the error.
  /// \param Caption The AL `Text`.
  void AddNavigationAction(std::string_view Caption = {});

  /// \brief AL `ErrorInfo.AddNavigationAction(Text, Text)`. Adds a navigation action for the error.
  /// \param Caption The AL `Text`.
  /// \param Description The AL `Text`.
  void AddNavigationAction(std::string_view Caption, std::string_view Description);

  /// \brief AL `ErrorInfo.Callstack()`. Specifies a callstack where the ErrorInfo was collected.
  /// \return The AL `Text`.
  std::string Callstack();

  /// \brief AL `ErrorInfo.Collectible(Boolean)`. Specifies if the error is collectible using
  /// ErrorBehavior.Collect.
  /// \param Collectible The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \brief AL `ErrorInfo.Collectible()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] ErrorInfo.Collectible([NewX])`.
  /// \return The value it holds.
  ::agiru::Boolean Collectible();

  ::agiru::Boolean Collectible(::agiru::Boolean Collectible);

  /// \brief AL `ErrorInfo.ControlName(Text)`. Specifies the control name that the error relates to.
  /// \param ControlName The AL `Text`.
  /// \return The AL `Text`.
  /// \brief AL `ErrorInfo.ControlName()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] ErrorInfo.ControlName([NewX])`.
  /// \return The value it holds.
  std::string ControlName();

  std::string ControlName(std::string_view ControlName);

  /// \brief AL `ErrorInfo.Create()`. Creates a new ErrorInfo object with Collectible set to true.
  /// \return The AL `ErrorInfo`.
  static ::agiru::ErrorInfo Create();

  /// \brief AL `ErrorInfo.Create(Text [, Boolean])` -- an error carrying a message
  ///        (`errorinfo-create-method.md`); the rest of the optional parameters are the overloads.
  /// \param Message     The message.
  /// \param Collectible Whether the error is collected rather than raised (board:0195).
  /// \return The AL `ErrorInfo`.
  static ::agiru::ErrorInfo Create(std::string_view Message, ::agiru::Boolean Collectible = false);

  /// \brief AL `ErrorInfo.Create(Text, Boolean, Record)`. An error carrying the record it is about.
  /// \tparam R The record's type -- any table, which is why it is a template.
  /// \param Message     The message.
  /// \param Collectible Whether the error is collected rather than raised.
  /// \param Record      The record.
  /// \return The AL `ErrorInfo`.
  template <typename R>
    requires requires { ::agiru::TableTraits<R>::kTable; }
  static ::agiru::ErrorInfo
  Create(std::string_view Message, ::agiru::Boolean Collectible, const R &Record) {
    return Create(Message, Collectible, Record, 0);
  }

  /// \brief AL `ErrorInfo.Create(Text, Boolean, Record, Integer)` -- with the field the error is
  /// about.
  /// \tparam R The record's table class.
  /// \param Message The text. \param Collectible Whether collectible. \param Record The record.
  /// \param FieldNo The field.
  /// \return The AL `ErrorInfo`.
  template <typename R>
    requires requires { ::agiru::TableTraits<R>::kTable; }
  static ::agiru::ErrorInfo Create(std::string_view Message,
                                   ::agiru::Boolean Collectible,
                                   const R &Record,
                                   ::agiru::Integer FieldNo) {
    ::agiru::ErrorInfo info = Create(Message, Collectible);
    info.tableId_ = ::agiru::TableTraits<R>::kTable.id.Value();
    if constexpr (requires { Record.SystemId; }) {
      info.systemId_ = Record.SystemId;
    } else {
      info.systemId_ = Record->SystemId;
    }
    info.fieldNo_ = FieldNo;
    return info;
  }

  /// \brief AL `ErrorInfo.Create(String, Boolean, Record, Integer, Integer, String, Verbosity,
  /// DataClassification, Dictionary of [Text, Text])`. Creates a new ErrorInfo object.
  /// \param Message The AL `String`.
  /// \param Collectible The AL `Boolean`.
  /// \param Record The AL `Record`.
  /// \param FieldNo The AL `Integer`.
  /// \param PageNo The AL `Integer`.
  /// \param ControlName The AL `String`.
  /// \param Verbosity The AL `Verbosity`.
  /// \param DataClassification The AL `DataClassification`.
  /// \param CustomDimensions The AL `Dictionary of [Text, Text]`.
  /// \return The AL `ErrorInfo`.
  static ::agiru::ErrorInfo
  Create(std::string_view Message,
         ::agiru::Boolean Collectible,
         ::agiru::RecordRef &Record,
         ::agiru::Integer FieldNo,
         ::agiru::Integer PageNo,
         std::string_view ControlName,
         const ::agiru::Verbosity &Verbosity,
         ::agiru::DataClassification DataClassification,
         const ::agiru::Dictionary<std::string, std::string> &CustomDimensions);

  /// \brief AL `ErrorInfo.CustomDimensions(Dictionary of [Text, Text])`. Set of additional
  /// dimensions, specified as a dictionary that relates to the error.
  /// \param CustomDimensions The AL `Dictionary of [Text, Text]`.
  /// \brief AL `ErrorInfo.CustomDimensions()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] ErrorInfo.CustomDimensions([NewX])`.
  /// \return The value it holds.
  ::agiru::Dictionary<std::string, std::string> CustomDimensions();

  void CustomDimensions(const ::agiru::Dictionary<std::string, std::string> &CustomDimensions);

  /// \brief AL `ErrorInfo.DataClassification(DataClassification)`. Specifies the classification of
  /// the error. Values include 'CustomerContent', 'EndUserIdentifiableInformation',
  /// 'EndUserPseudonymousIdentifiers', 'AccountData', 'OrganizationIdentifiableInformation',
  /// 'SystemMetadata', and 'ToBeClassified'
  /// \param DataClassification The AL `DataClassification`.
  /// \return The AL `DataClassification`.
  /// \brief AL `ErrorInfo.DataClassification()` -- the READING form, which the documentation's
  /// syntax block brackets: `[X := ] ErrorInfo.DataClassification([NewX])`.
  /// \return The value it holds.
  ::agiru::DataClassification DataClassification();

  ::agiru::DataClassification DataClassification(::agiru::DataClassification NewDataClassification);

  /// \brief AL `ErrorInfo.DetailedMessage(Text)`. Specifies a detailed error message.
  /// \param DetailedMessage The AL `Text`.
  /// \return The AL `Text`.
  /// \brief AL `ErrorInfo.DetailedMessage()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] ErrorInfo.DetailedMessage([NewX])`.
  /// \return The value it holds.
  std::string DetailedMessage();

  std::string DetailedMessage(std::string_view DetailedMessage);

  /// \brief AL `ErrorInfo.ErrorType(ErrorType)`. Specifies type of the error. 'Client' shows the
  /// specified message in the client and sends it to telemetry. 'Internal' shows a generic message
  /// in the client and sends the specified message to telemetry.
  /// \param ErrorType The AL `ErrorType`.
  /// \return The AL `ErrorType`.
  /// \brief AL `ErrorInfo.ErrorType()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] ErrorInfo.ErrorType([NewX])`.
  /// \return The value it holds.
  ::agiru::ErrorType ErrorType();

  ::agiru::ErrorType ErrorType(const ::agiru::ErrorType &ErrorType);

  /// \brief AL `ErrorInfo.FieldNo(Integer)`. Specifies the field ID that the error relates to.
  /// \param FieldNo The AL `Integer`.
  /// \return The AL `Integer`.
  /// \brief AL `ErrorInfo.FieldNo()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] ErrorInfo.FieldNo([NewX])`.
  /// \return The value it holds.
  ::agiru::Integer FieldNo();

  ::agiru::Integer FieldNo(::agiru::Integer FieldNo);

  /// \brief AL `ErrorInfo.Message(Text)`. Specifies the message that will be sent to telemetry. For
  /// a 'Client' error type, the message will also be appear in the client.
  /// \param Message The AL `Text`.
  /// \return The AL `Text`.
  /// \brief AL `ErrorInfo.Message()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] ErrorInfo.Message([NewMessage])`.
  /// \return The message the error carries.
  std::string Message();

  std::string Message(std::string_view Message);

  /// \brief AL `ErrorInfo.PageNo(Integer)`. Specifies the page number that the error relates to.
  /// \param PageNo The AL `Integer`.
  /// \return The AL `Integer`.
  /// \brief AL `ErrorInfo.PageNo()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] ErrorInfo.PageNo([NewX])`.
  /// \return The value it holds.
  ::agiru::Integer PageNo();

  ::agiru::Integer PageNo(::agiru::Integer PageNo);

  /// \brief AL `ErrorInfo.RecordId(RecordId)`. Specifies the record ID of the record that the error
  /// relates to.
  /// \param RecordId The AL `RecordId`.
  /// \return The AL `RecordId`.
  ::agiru::RecordId RecordId(::agiru::RecordId RecordId = {});

  /// \brief AL `ErrorInfo.SystemId(Guid)`. Specifies the system ID of the record that the error
  /// relates to.
  /// \param SystemId The AL `Guid`.
  /// \return The AL `Guid`.
  /// \brief AL `ErrorInfo.SystemId()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] ErrorInfo.SystemId([NewX])`.
  /// \return The value it holds.
  ::agiru::Guid SystemId();

  ::agiru::Guid SystemId(::agiru::Guid SystemId);

  /// \brief AL `ErrorInfo.TableId(Integer)`. Specifies the table ID that the error relates to.
  /// \param TableId The AL `Integer`.
  /// \return The AL `Integer`.
  /// \brief AL `ErrorInfo.TableId()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] ErrorInfo.TableId([NewX])`.
  /// \return The value it holds.
  ::agiru::Integer TableId();

  ::agiru::Integer TableId(::agiru::Integer TableId);

  /// \brief AL `ErrorInfo.Title(Text)`. Specifies the title of the error.
  /// \param Title The AL `Text`.
  /// \return The AL `Text`.
  /// \brief AL `ErrorInfo.Title()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] ErrorInfo.Title([NewX])`.
  /// \return The value it holds.
  std::string Title();

  std::string Title(std::string_view Title);

  /// \brief AL `ErrorInfo.Verbosity(Verbosity)`. Specifies the severity level of the error. This
  /// can determine whether the error should be sent to telemetry (which is based on the trace level
  /// setting of the server).
  /// \param Verbosity The AL `Verbosity`.
  /// \return The AL `Verbosity`.
  /// \brief AL `ErrorInfo.Verbosity()` -- the READING form, which the documentation's syntax
  /// block brackets: `[X := ] ErrorInfo.Verbosity([NewX])`.
  /// \return The value it holds.
  ::agiru::Verbosity Verbosity();

  ::agiru::Verbosity Verbosity(const ::agiru::Verbosity &Verbosity);

private:
  struct Action {
    std::string caption;
    ::agiru::Integer codeunitId{};
    std::string methodName;
    std::string description;
    bool navigation{};
  };

  std::string message_;
  std::string detailedMessage_;
  std::string title_;
  std::string controlName_;
  std::string callstack_;
  ::agiru::Boolean collectible_{};
  ::agiru::Integer fieldNo_{};
  ::agiru::Integer pageNo_{};
  ::agiru::Integer tableId_{};
  ::agiru::RecordId recordId_{};
  ::agiru::Guid systemId_{};
  ::agiru::ErrorType errorType_{};
  ::agiru::Verbosity verbosity_{};
  ::agiru::DataClassification dataClassification_{};
  ::agiru::Dictionary<std::string, std::string> customDimensions_;
  std::vector<Action> actions_;
};

}
