#pragma once

#include "type/Guid.h"
#include "type/List.h"
#include "type/Text.h"
#include "type/Version.h"

#include <string>

/// \file
/// \brief AL `ModuleInfo` and `ModuleDependencyInfo` -- what an installed app says about itself.

namespace agiru {

/// \brief AL `ModuleDependencyInfo` -- one app another app depends on.
///
/// `moduledependencyinfo-data-type.md` gives it three properties and no more.
class ModuleDependencyInfo {
public:
  /// \brief No dependency.
  ModuleDependencyInfo() = default;

  /// \brief A dependency on a named app.
  /// \param id        The app's identifier.
  /// \param name      Its name.
  /// \param publisher Its publisher.
  ModuleDependencyInfo(const Guid &id, std::string name, std::string publisher)
      : id_(id), name_(std::move(name)), publisher_(std::move(publisher)) {}

  /// \brief AL `ModuleDependencyInfo.Id`. \return The app's identifier.
  [[nodiscard]] const Guid &Id() const { return id_; }

  /// \brief AL `ModuleDependencyInfo.Name`. \return The app's name.
  [[nodiscard]] std::string_view Name() const { return name_; }

  /// \brief AL `ModuleDependencyInfo.Publisher`. \return The publisher.
  [[nodiscard]] std::string_view Publisher() const { return publisher_; }

private:
  Guid id_;
  std::string name_;
  std::string publisher_;
};

/// \brief What an installed app says about itself, as the transpiler emits it from `app.json`:
///        one `constexpr` per app, in `apps/<app>/<App>Module.h`.
///
/// \note THE CALLER IS THE OBJECT, NOT A FRAME. `NavApp.GetCurrentModuleInfo` is documented against
///       the call stack; this runtime keeps no per-frame module, and the transpiler knows the app
///       of every object it writes -- so a call site passes its own app's module. That is exact
///       for `GetCurrentModuleInfo` and, for `GetCallerModuleInfo`, exact within one app and the
///       CALLEE's app across a boundary (board:0638).
struct ModuleDef {
  std::string_view id;        ///< The app id, a Guid in text.
  std::string_view name;      ///< The app name.
  std::string_view publisher; ///< The publisher.
  std::string_view version;   ///< The app version, `30.0.0.0`.
};

/// \brief AL `ModuleInfo` -- the app a piece of code belongs to.
///
/// `moduleinfo-data-type.md` gives seven properties: `Id`, `Name`, `Publisher`, `AppVersion`,
/// `DataVersion`, `PackageId` and `Dependencies`. The BaseApp reads `Id` 50 times and `DataVersion`
/// 34, measured over BCApps -- the second because every upgrade codeunit compares the version the
/// DATA was written by against the version of the code reading it.
///
/// \note `NavApp.GetCurrentModuleInfo(Info)` is how AL fills one, and that is a platform call
///       rather than a property of this type (board:0028).
class ModuleInfo {
public:
  /// \brief An empty module info.
  ModuleInfo() = default;

  /// \brief AL `ModuleInfo.Id`. \return The app's identifier.
  [[nodiscard]] const Guid &Id() const { return id_; }

  /// \brief AL `ModuleInfo.Name`. \return The app's name.
  [[nodiscard]] std::string_view Name() const { return name_; }

  /// \brief AL `ModuleInfo.Publisher`. \return The publisher.
  [[nodiscard]] std::string_view Publisher() const { return publisher_; }

  /// \brief AL `ModuleInfo.AppVersion`. \return The version of the code.
  [[nodiscard]] const Version &AppVersion() const { return appVersion_; }

  /// \brief AL `ModuleInfo.DataVersion`. \return The version the data was written by.
  [[nodiscard]] const Version &DataVersion() const { return dataVersion_; }

  /// \brief AL `ModuleInfo.PackageId`. \return The package's identifier.
  [[nodiscard]] const Guid &PackageId() const { return packageId_; }

  /// \brief AL `ModuleInfo.Dependencies`. \return The apps this one depends on.
  [[nodiscard]] const List<ModuleDependencyInfo> &Dependencies() const { return dependencies_; }

  /// \brief Fills in what an app says about itself.
  /// \param id          The app's identifier.
  /// \param name        Its name.
  /// \param publisher   Its publisher.
  /// \param appVersion  The version of the code.
  /// \param dataVersion The version the data was written by.
  void Describe(const Guid &id,
                std::string name,
                std::string publisher,
                const Version &appVersion,
                const Version &dataVersion) {
    id_ = id;
    name_ = std::move(name);
    publisher_ = std::move(publisher);
    appVersion_ = appVersion;
    dataVersion_ = dataVersion;
  }

private:
  Guid id_;
  Guid packageId_;
  std::string name_;
  std::string publisher_;
  Version appVersion_;
  Version dataVersion_;
  List<ModuleDependencyInfo> dependencies_;
};

}
