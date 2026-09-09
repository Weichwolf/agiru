#pragma once

#include "type/Boolean.h"

namespace agiru::dotnet {

/// \brief .NET `Microsoft.Dynamics.Nav.Runtime.NavTestExecution`, which the Environment
///        Information module asks whether a test is running.
class NavTestExecution {
public:
  /// \brief The helper; it holds nothing.
  NavTestExecution() = default;

  /// \brief `NavTestExecution.IsInTestMode()`: whether a test case is running.
  /// \return True between a case's handler table being installed and uninstalled, which is the
  ///         span the test runner gives a case -- the same signal `GuiAllowed` answers with.
  ///
  /// \note IT WAS AN ABSENT TYPE and refused 52 UT cases through `EnvironmentInformation.
  ///       IsSaaS()` alone (Match Bank Reconciliation, Price List Header UT, measured 2026-09-09).
  [[nodiscard]] static ::agiru::Boolean IsInTestMode();
};

}
