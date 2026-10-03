#pragma once

#include <string_view>

namespace agiru {

/// \brief Immutable application identity emitted from app.json or the native NavxManifest.
/// One constexpr declaration per app is shared by object metadata and module-information calls.
/// This declaration carries no mutable AL state or installed-data version.
/// \note The generator passes the executing object's module, not a call-stack frame.
/// Current-module lookup is exact; cross-app caller-module lookup remains a gap (board:0638).
struct ModuleDef {
  std::string_view id;        ///< The original application GUID in text.
  std::string_view name;      ///< The application name.
  std::string_view publisher; ///< The application publisher.
  std::string_view version;   ///< The source application version.
};

}
