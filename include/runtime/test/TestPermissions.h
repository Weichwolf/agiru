#pragma once

#include <cstdint>

/// \file
/// \brief AL `TestPermissions` -- Specifies a value that can be used to determine which permission
/// sets are used on tests that are run by test codunits or test functions.
///
/// The members and their order come from `methods-auto/testpermissions/testpermissions-option.md`,
/// which is the specification: an AL option is zero-based and sequential in the order the page
/// lists.

namespace agiru {

/// \brief AL `TestPermissions`. Specifies a value that can be used to determine which permission
/// sets are used on tests that are run by test codunits or test functions.
enum class TestPermissions : std::int32_t {
  InheritFromTestCodeunit, ///< Is only relevant for test methods; not test codeunits. It specifies
                           ///< that a test method uses the TestPermissions property setting of the
                           ///< test codeunit to which it belongs. If you use this value on a test
                           ///< codunit, the property will resolve to Restrictive at runtime.
  Restrictive, ///< This is the default value. Setting the Restrictive value will cause the
               ///< permission execution context of every test in the codeunit to be set by default
               ///< to 'D365 Full Access’. It is required to lower the level of permissions within
               ///< every test to any permission sets other than 'D365 Full Access’. Otherwise, it
               ///< will result in a runtime error. The change of the permission execution context
               ///< is supported by  Codeunit "Library - Lower Permissions".
  NonRestrictive, ///< Setting the NonRestrictive value will cause that the permission execution
                  ///< context of every test in the codeunit is set to 'D365 Full Access’. Opposite
                  ///< to Restrictive, setting the TestPermissions property to NonRestrictive does
                  ///< not require a change of permissions.
  Disabled, ///< Setting this value will exclude any change of the permission execution context and
            ///< all tests will be executed using SUPER.
};

}
