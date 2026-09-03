#pragma once

#include "runtime/Error.h"
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

#include <string>
#include <string_view>

/// \file
/// \brief AL `Dialog` -- the surface the platform documentation declares.

namespace agiru {

class ErrorInfo;

/// \brief AL `Dialog`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/dialog/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class Dialog {
public:
  /// \brief AL `Dialog.Close()`. Closes a dialog window that has been opened by the OPEN method.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Close();

  /// \brief AL `Dialog.Confirm(Text, Boolean, Any)`. Creates a dialog box that prompts the user for
  /// a yes or no answer. The dialog box is centered on the screen.
  /// \param String The AL `Text`.
  /// \param Default The AL `Boolean`.
  /// \param Value1 The AL `Any`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean
  Confirm(std::string_view String, ::agiru::Boolean Default, const ::agiru::Variant &Value1);

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
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static void Message(std::string_view String, const ::agiru::Variant &Value);

  /// \brief AL `Dialog.Open(Text, Any)`. Opens a dialog window.
  /// \param String The AL `Text`.
  /// \param Variable1 The AL `Any`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Open(std::string_view String, ::agiru::Variant &Variable1);

  /// \brief AL `Dialog.StrMenu(Text, Integer, Text)`. Creates a menu window that displays a series
  /// of options.
  /// \param OptionMembers The AL `Text`.
  /// \param DefaultNumber The AL `Integer`.
  /// \param Instruction The AL `Text`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Integer StrMenu(std::string_view OptionMembers,
                                  ::agiru::Integer DefaultNumber,
                                  std::string_view Instruction);

  /// \brief AL `Dialog.Update(Integer, Any)`. Updates the value of a '#'-or '@' field in the active
  /// window.
  /// \param Number The AL `Integer`.
  /// \param Value The AL `Any`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Update(::agiru::Integer Number, const ::agiru::Variant &Value);
};

}
