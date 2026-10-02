#pragma once

#include "meta/Ids.h"
#include "type/Integer.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

/// \file
/// \brief AL's TEST HANDLERS -- the methods that stand in for the user while one case runs.

namespace agiru {

/// \brief Which dialog a handler answers.
///
/// \note THE KIND IS THE ATTRIBUTE. AL names one attribute per dialog kind
///       (`attributes/devenv-confirmhandler-attribute.md` and its siblings), and the runtime
///       reaches a handler by the kind of thing it is about to show.
enum class HandlerKind : std::uint8_t {
  Confirm,            ///< `[ConfirmHandler]` -- `Confirm(Question, var Reply)`.
  Message,            ///< `[MessageHandler]` -- `Message(Text)`.
  StrMenu,            ///< `[StrMenuHandler]` -- `StrMenu(Options, var Choice, Instruction)`.
  Hyperlink,          ///< `[HyperlinkHandler]` -- `Hyperlink(Url)`.
  Page,               ///< `[PageHandler]` -- a page run non-modally.
  ModalPage,          ///< `[ModalPageHandler]` -- a page run modally.
  RequestPage,        ///< `[RequestPageHandler]` -- a report's request page.
  Report,             ///< `[ReportHandler]` -- a report run without its request page.
  FilterPage,         ///< `[FilterPageHandler]` -- a generated filter page.
  SendNotification,   ///< `[SendNotificationHandler]` -- a notification on its way out.
  RecallNotification, ///< `[RecallNotificationHandler]` -- a notification being recalled.
  Session,            ///< `[SessionSettingsHandler]` -- a session-settings dialog.
  HttpClient,         ///< `[HttpClientHandler]` -- an outbound request.
};

/// \brief What a `[StrMenuHandler]` is handed: the instruction beside the options the thunk's
///        text carries, and where the chosen number goes.
struct StrMenuAnswer {
  std::string_view instruction; ///< The `Instruction` text.
  ::agiru::Integer choice;      ///< In: the default; out: what the handler chose.
};

/// \brief What every handler thunk looks like from the outside.
///
/// \param text  What the dialog would show -- the question, the message, the option string.
/// \param reply Where the answer goes: a `Boolean` for a confirm, an `Integer` for a menu,
///        `nullptr` where the kind has no answer.
///
/// \note ONE SIGNATURE FOR THIRTEEN KINDS, and the THUNK is where the kind's own signature is
///       known. A table of differently-typed pointers cannot be `constexpr` -- a cast between
///       function types is not a constant expression -- and this tree's metadata is `.rodata`
///       (board:0054).
using HandlerThunk = void (*)(std::string_view text, void *reply);

/// \brief One handler procedure of a test codeunit.
struct TestHandler {
  std::string_view name; ///< The procedure's AL name, as `[HandlerFunctions]` spells it.
  HandlerKind kind;      ///< Which dialog it answers.
  std::int32_t object;   ///< The page or report number it answers, 0 where the kind has none.
  HandlerThunk invoke;   ///< Calls it on a fresh codeunit with the dialog's own arguments.
  bool optional;         ///< `HandlerIsOptional` -- a notification handler AL lets a case declare
                 ///< and not reach (`attributes/devenv-sendnotificationhandler-attribute.md`);
                 ///< it is excluded from the "named and never ran" failure (openerp
                 ///< WI-1305, board:0054).
};

/// \brief The handlers of ONE test codeunit, installed while one of its cases runs.
///
/// \note THE SESSION HOLDS ONE TABLE, because AL reaches a handler only from the codeunit that
///       declares it (`devenv-test-codeunits-and-test-methods.md`).
class HandlerTable {
public:
  /// \brief Installs a codeunit's handlers for the case that is about to run.
  /// \param handlers The codeunit's whole handler table.
  /// \param declared The names the case's `[HandlerFunctions]` listed.
  static void Install(std::span<const TestHandler> handlers,
                      std::span<const std::string_view> declared);

  /// \brief Takes the table away again, and says which declared handlers never ran.
  /// \return The names that were declared and not called, in the order they were declared.
  ///
  /// \note A NAMED HANDLER THAT NEVER RAN FAILS THE CASE (board:0054). AL's own runner reports it,
  ///       and a silent pass would hide a dialog that never appeared.
  [[nodiscard]] static std::vector<std::string_view> Uninstall();

  /// \brief The handler that answers this kind, when one is installed and was declared.
  /// \param kind   The dialog kind.
  /// \param object The page or report number, 0 where the kind has none.
  /// \return The handler, or nullptr when the case declared none for it.
  [[nodiscard]] static const TestHandler *For(HandlerKind kind, std::int32_t object = 0);

  /// \brief Notes that a handler ran.
  /// \param handler The handler.
  static void Ran(const TestHandler &handler);

  /// \brief Whether a case is running with handlers installed at all.
  /// \return True between `Install` and `Uninstall`.
  [[nodiscard]] static bool Installed();
};

}
