#pragma once

#include <cstdint>

/// \file
/// \brief AL `Subtype` on a codeunit.

namespace agiru {

/// \brief AL `Subtype` on a codeunit -- what the object is FOR.
///
/// `devenv-subtype-codeunit-property.md` gives five values and two of them decide how the object is
/// run: a `Test` codeunit HOLDS test methods, a `TestRunner` codeunit RUNS test codeunits and
/// carries `OnBeforeTestRun`/`OnAfterTestRun` instead.
///
/// \note IT SITS IN `meta/` BECAUSE IT IS A DECLARATION. `CodeunitDef` carries it, and `meta/`
///       may not reach `runtime/`; `runtime/Codeunit.h` includes this rather than declaring it.
enum class Subtype : std::uint8_t {
  Normal,     ///< The default: a general-purpose codeunit.
  Test,       ///< Holds `[Test]` methods.
  TestRunner, ///< Runs test codeunits.
  Upgrade,    ///< Holds data-upgrade triggers.
  Install,    ///< Holds extension-installation triggers.
};

}
