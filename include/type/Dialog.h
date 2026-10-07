#pragma once

#include "runtime/ErrorValue.h"
#include "runtime/UiHost.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/Time.h"
#include "type/Variant.h"
#include "type/Verbosity.h"

#include <array>
#include <string>
#include <string_view>

/// \file
/// \brief AL `Dialog` -- the surface the platform documentation declares.

namespace agiru {

class ErrorInfo;

/// \brief AL `Dialog`.
///
/// Basic message/question/progress operations use the owning session's UI host or explicit
/// AL test adapter. Other unsupported methods refuse, never pretend successful interaction.
class Dialog {
public:
  /// \brief AL `Dialog.Close()`. Closes a dialog window that has been opened by the OPEN method.
  /// \note Progress remains headless only under the explicit AL test adapter.
  /// \throws Error when no native UI host or test adapter is available.
  void Close();

  /// \brief AL `Dialog.Confirm(Text, Boolean, Any)`. Creates a dialog box that prompts the user for
  /// a yes or no answer. The dialog box is centered on the screen.
  /// \param String The AL `Text`.
  /// \param Default The AL `Boolean`.
  /// \param Value1 The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error without an available explicit answer; Default is presentation only.
  static ::agiru::Boolean Confirm(std::string_view String,
                                  ::agiru::Boolean Default = {},
                                  const ::agiru::Variant &Value1 = {});

  /// \brief AL `Dialog.Error(ErrorInfo)`. Displays an error message and ends the execution of AL
  /// code.
  /// \param Message The AL `ErrorInfo`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static void Error(const ::agiru::ErrorInfo &Message);

  /// \brief AL `Dialog.Error(Text, Any)`. Displays an error message and ends the execution of AL
  /// code.
  /// \param Message The AL `Text`.
  /// \param Value The AL `Any`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static void Error(std::string_view Message, const ::agiru::Variant &Value);

  /// \brief AL `Dialog.HideSubsequentDialogs(Boolean)`. Specifies that subsequent child dialogs are
  /// not shown.
  /// \param HideSubsequentDialogs The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  /// \brief AL `Dialog.HideSubsequentDialogs()` -- the READING form, which the documentation's
  /// syntax block brackets: `[X := ] Dialog.HideSubsequentDialogs([NewX])`.
  /// \return The value it holds.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean HideSubsequentDialogs();

  ::agiru::Boolean HideSubsequentDialogs(::agiru::Boolean HideSubsequentDialogs);

  /// \brief AL `Dialog.LogInternalError(Text, DataClassification, Verbosity)`. Log internal errors
  /// for telemetry.
  /// \param Message The AL `Text`.
  /// \param DataClassificationInstance The AL `DataClassification`.
  /// \param VerbosityInstance The AL `Verbosity`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static void LogInternalError(std::string_view Message,
                               const ::agiru::Variant &DataClassificationInstance,
                               const ::agiru::Verbosity &VerbosityInstance);

  /// \brief AL `Dialog.LogInternalError(Text, Text, DataClassification, Verbosity)`. Log internal
  /// errors for telemetry.
  /// \param Message The AL `Text`.
  /// \param SubstitutionString The AL `Text`.
  /// \param DataClassificationInstance The AL `DataClassification`.
  /// \param VerbosityInstance The AL `Verbosity`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static void LogInternalError(std::string_view Message,
                               std::string_view SubstitutionString,
                               const ::agiru::Variant &DataClassificationInstance,
                               const ::agiru::Verbosity &VerbosityInstance);

  /// \brief AL `Dialog.Message(Text, Any)`. Displays a text string in a message window.
  /// \param String The AL `Text`.
  /// \param Value The AL `Any`.
  /// \throws Error without a selected test handler or available UI host.
  static void Message(std::string_view String, const ::agiru::Variant &Value = {});

  /// \brief AL `Dialog.Open(Text, Any)`. Opens a dialog window.
  /// \param String The AL `Text`.
  /// \param Variable1 The AL `Any`.
  /// \note The host receives a live binding, not an initial-value-only copy.
  void Open(std::string_view String, ::agiru::Variant &Variable1);

  /// \brief AL `Dialog.Open(Text)` -- the same without a variable to show; `Window.Open(Msg)` is
  ///        the BaseApp's usual form.
  /// \param String The dialog text.
  void Open(std::string_view String);

  /// \brief AL `Dialog.Open(String, var Value1 [, var Value2 ...])` with typed variables or record
  ///        fields behind the placeholders. Updates refresh live, exact typed bindings.
  /// \tparam Values The variables' types. \param String The text. \param values The variables.
  template <typename... Values>
    requires(sizeof...(Values) >= 1)
  void Open(std::string_view String, Values &...values) {
    const std::array bindings{UiValueBinding::Bind(values)...};
    detail::OpenUiProgress(this, String, bindings);
  }

  /// \brief AL `Dialog.StrMenu(Text, Integer, Text)`. Creates a menu window that displays a series
  /// of options.
  /// \param OptionMembers The AL `Text`.
  /// \param DefaultNumber The AL `Integer`.
  /// \param Instruction The AL `Text`.
  /// \return The AL `Integer`.
  /// \throws Error without a selected test handler or available UI host.
  static ::agiru::Integer StrMenu(std::string_view OptionMembers,
                                  ::agiru::Integer DefaultNumber = 1,
                                  std::string_view Instruction = {});

  /// \brief AL `Dialog.Update(Integer, Any)`. Updates the value of a '#'-or '@' field in the active
  /// window.
  /// \param Number The AL `Integer`.
  /// \param Value The AL `Any`.
  /// \note Progress remains headless only under the explicit AL test adapter.
  /// \throws Error without an available native UI host or test adapter.
  void Update(::agiru::Integer Number = {}, const ::agiru::Variant &Value = {});
};

}
