#pragma once

#include "meta/Ids.h"

#include <cstdint>
#include <string>
#include <string_view>

/// \file
/// \brief Presentation-independent, authorized commands over an existing page control adapter.

namespace agiru {

class PageCore;
struct PageDef;
struct ControlDef;

/// \brief Client control operations; page lifecycle and modal answers are separate contracts.
enum class PageControlOperation : std::uint8_t {
  Unknown, ///< Unrecognized or unspecified operation; always refused.
  Read,
  Set,
  Filter,
  Action,
  Lookup,
  DrillDown,
  AssistEdit,
};

/// \brief One client command using the declared AL control name, never a localized caption.
struct PageControlCommand {
  PageControlOperation operation{}; ///< The requested operation.
  std::string_view control{};       ///< Exact identity within the owning page declaration.
  std::string_view text{};          ///< Input text for Set or an AL expression for Filter.
};

/// \brief Display text and Option/Enum ordinal, kept distinct; not a general typed wire value.
struct PageControlResult {
  std::string text{};    ///< Formatted display text, including unchanged Unicode content.
  std::string ordinal{}; ///< Exact Option/Enum ordinal text, or empty for other fields.
};

/// \brief Mandatory authorization boundary supplied by the server's authenticated session.
/// This interface is not a permission implementation; visibility does not grant access.
class PageAuthorization {
public:
  virtual ~PageAuthorization() = default;

  /// \brief Reauthorize the page/control operation in the caller's user/company context.
  /// \param page The declared page identity.
  /// \param command The command, including its requested control identity and input.
  /// \throws Error when the session or its permissions do not permit the operation.
  virtual void Require(PageId page, const PageControlCommand &command) = 0;
};

/// \brief Shared command validation before typed page reads, validation and triggers.
/// The caller serializes its session and provides transaction/error boundaries. This
/// class does not own a page, authenticate users, manage revisions or suspend AL execution.
class PageDispatcher {
public:
  /// \brief Borrows one immutable declaration, its live adapter and authorization authority.
  /// \param declaration The declaration belonging to the supplied adapter.
  /// \param page The existing typed page control adapter; not a second business implementation.
  /// \param authorization The session's mandatory permission check, called on every command.
  /// \note All three arguments must outlive the dispatcher.
  PageDispatcher(const PageDef &declaration, PageCore &page, PageAuthorization &authorization);

  /// \brief Reauthorizes, checks the declared kind and current UI state, then executes once.
  /// Read returns display/ordinal text; other commands return an empty result. Unknown
  /// controls, unsupported kinds, hidden/disabled controls and read-only edits refuse.
  /// Filtering remains available for read-only or disabled fields. AL errors propagate intact.
  /// \param command An operation using an exact declared control identity.
  /// \return The operation's result; transport adapters must not reinterpret display as numbers.
  /// \throws Error for command refusals or errors from authorization, bindings and AL triggers.
  [[nodiscard]] PageControlResult Execute(const PageControlCommand &command);

private:
  [[nodiscard]] const ControlDef &RequireControl(const PageControlCommand &command);

  const PageDef &declaration_;
  PageCore &page_;
  PageAuthorization &authorization_;
};

}
