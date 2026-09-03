#pragma once

#include "runtime/Error.h"
#include "type/Boolean.h"

#include <string_view>

/// \file
/// \brief AL `TestAction` -- one action of a page, driven by a test.
///
/// \note IT IS NOT PART OF A PAGE. A page has CONTROLS; `TestAction` is how a TEST reaches one, and
/// the
///       generated page declares its controls as a class TEMPLATE so that nothing test-shaped
///       lands in `apps/` outside the test app.

namespace agiru {

/// \brief AL `TestAction` -- one action of a page, reached by the name AL gave it.
class TestAction {
public:
  /// \brief An action with the name AL gave it.
  /// \param name The action name.
  explicit constexpr TestAction(std::string_view name) : name_(name) {}

  /// \brief The action's AL name.
  /// \return The name.
  [[nodiscard]] constexpr std::string_view Name() const { return name_; }

  /// \brief AL `TestAction.Invoke()` -- runs the action's `OnAction` trigger.
  /// \throws Error until a page can be opened.
  void Invoke();

  /// \brief AL `TestAction.Enabled()`. \return Whether the action is enabled.
  /// \throws Error until a page can be opened.
  [[nodiscard]] Boolean Enabled() const;

  /// \brief AL `TestAction.Visible()`. \return Whether the action is shown.
  /// \throws Error until a page can be opened.
  [[nodiscard]] Boolean Visible() const;

private:
  [[noreturn]] void Unbound() const;

  std::string_view name_;
};

}
