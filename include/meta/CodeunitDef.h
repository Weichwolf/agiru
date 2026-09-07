#pragma once

#include "meta/Ids.h"
#include "meta/Subtype.h"

#include <string_view>

/// \file
/// \brief The static declaration of an AL codeunit: what it may reach, and how long it lives.

namespace agiru {

/// \brief One codeunit's declaration, as static const data.
///
/// A codeunit has no field table and no control tree, so this is short -- but it is the same SHAPE
/// as `TableDef` and `PageDef`, because a permission check takes an object's declaration and
/// should not have to know which of the three kinds it is holding.
struct CodeunitDef {
  CodeunitId id{};            ///< The AL codeunit number.
  std::string_view name{};    ///< The AL name: `"Sales-Post"`.
  ::agiru::Subtype subtype{}; ///< The `Subtype` property (board:0472).

  /// \brief The `TableNo` property: the table `Run` takes, 0 when the codeunit declares none.
  TableId tableNo{};

  /// \brief The `Permissions` property, as AL wrote it: what the codeunit's own code may reach
  ///        beyond what its caller may (board:0376). 675 declarations.
  std::string_view permissions{};

  /// \brief The `InherentPermissions` and `InherentEntitlements` properties, as AL wrote them: an
  ///        object granted its own access, for the duration of the call (board:0378, board:0499).
  std::string_view inherentPermissions{};
  std::string_view inherentEntitlements{}; ///< \see inherentPermissions

  /// \brief The `SingleInstance` property: ONE object per session rather than one per variable
  ///        (board:0471). 147 declarations.
  bool singleInstance = false;

  /// \brief The `EventSubscriberInstance` property, as AL wrote it: `StaticAutomatic` or
  ///        `Manual`, which decides whether the runtime binds the subscriber itself (board:0057).
  std::string_view eventSubscriberInstance{};

  /// \brief The `TestPermissions` property, as AL wrote it (board:0473).
  std::string_view testPermissions{};

  /// \brief The `Access` property, as AL wrote it (board:0359).
  std::string_view access{};

  /// \brief The `ObsoleteState` property, as AL wrote it.
  std::string_view obsoleteState{};
};

}
