#pragma once

#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/StringValue.h"

#include <memory>
#include <string_view>

/// \file
/// \brief Runtime-18 test-hook identity, outcome and scoped skip request.
namespace agiru {

namespace detail {
struct TestContextState;
class TestContextScope;
}

/// \brief AL TestHandlerContext; value copies share only their own hook's skip control.
/// \note Identity/outcome are immutable snapshots. Retaining a copy cannot keep a completed
///       callback's skip authority alive. There is no process-global current context.
class TestHandlerContext {
public:
  /// \brief An unbound variable; methods refuse until a hook context is assigned.
  /// \note Unbound-variable behaviour is conservative, not a measured BC default-value claim.
  TestHandlerContext() = default;

  /// \return The test codeunit's AL ID in every callback.
  /// \throws Error If this variable is unbound.
  [[nodiscard]] Integer CodeunitId() const;
  /// \return The fully qualified AL codeunit name supplied by its declaration.
  /// \throws Error If this variable is unbound.
  [[nodiscard]] Text<0> CodeunitName() const;
  /// \return The procedure name, empty for codeunit-level callbacks.
  /// \throws Error If this variable is unbound.
  [[nodiscard]] Text<0> ProcedureName() const;
  /// \return The stable case identifier, empty outside data-driven case callbacks.
  /// \throws Error If this variable is unbound.
  [[nodiscard]] Text<0> TestCaseName() const;
  /// \return The completed outcome in an after callback; false in a before callback.
  /// \throws Error If this variable is unbound.
  [[nodiscard]] Boolean Success() const;

  /// \brief Requests a skip only during a live before-procedure or before-case callback.
  /// \param reason The owned diagnostic to report with the skipped procedure/case.
  /// \note After/codeunit callbacks and expired copies cannot request skips. An empty reason
  ///       still requests a skip. Repeated calls retain the last supplied reason.
  /// \throws Error If this variable is unbound.
  void Skip(std::string_view reason) const;

private:
  friend class detail::TestContextScope;
  explicit TestHandlerContext(std::shared_ptr<detail::TestContextState> state);
  [[nodiscard]] detail::TestContextState &state() const;
  std::shared_ptr<detail::TestContextState> state_;
};

}
