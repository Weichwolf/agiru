#pragma once

#include "type/Boolean.h"
#include "type/Integer.h"

/// \file
/// \brief The .NET type `ALConfigSettings` -- the server instance's own settings.

namespace agiru::dotnet {

/// \brief The settings of the server instance, as AL reads them.
///
/// The BaseApp reaches them through `Codeunit "Server Setting"` ->
/// `ServerSettingImpl.InitializeConfigSettings` -> `ALConfigSettings.<Setting>()`.
///
/// \warning WITHOUT IT AN ENTIRE LAYER LIES IDLE AND NOTHING FAILS. The predecessor records the
///          cost: every subscriber of the Graph and aggregate layer opens with
///          `if Rec.IsTemporary or (not GraphMgtGeneralTools.IsApiEnabled()) then exit;` and
///          `IsApiEnabled` hangs off this type. A missing answer there is not an error, it is an
///          early `exit` -- so the layer went quiet rather than red.
///
/// \note THE ANSWERS THAT NEED NOTHING ARE STATIC, and AL does not notice: C++ lets a static member
///       be called through an instance, so `ALConfigSettings.ApiSubscriptionsEnabled()` reads and
///       compiles exactly as AL writes it. The type is stateless -- the predecessor spells that as
///       `__slots__ = ()` -- so a method that pretended to need an object would be claiming
///       something untrue about it.
///
/// \note EVERY VALUE CARRIES ITS ORIGIN. They are the documented defaults from
///       `administration/server-instance-settings.md`, column "Default", and the two that depend on
///       the environment say so.
class ALConfigSettings {
public:
  /// \brief AL `ALConfigSettings := ALConfigSettings.Instance()`.
  /// \return This object; the type carries no state of its own.
  [[nodiscard]] ALConfigSettings Instance() const { return *this; }

  /// \brief Whether API web services are enabled.
  ///
  /// \return True in the service, false on a self-hosted instance.
  ///
  /// \note THE DOCUMENTED DEFAULT IS "Not enabled" AND IT IS NOT A CONSTANT HERE, which is the one
  ///       place this type departs from the page. That default describes the SELF-HOSTED instance;
  ///       in the service the API endpoints are part of the offer, and the test libraries switch to
  ///       exactly that with `EnvironmentInfoTestLibrary.SetTestabilitySoftwareAsAService(true)`.
  ///       So it follows the environment rather than a constant.
  [[nodiscard]] static Boolean ApiServicesEnabled();

  /// \brief Whether API subscriptions are enabled.
  /// \return True -- the page's default is "Enabled".
  [[nodiscard]] static Boolean ApiSubscriptionsEnabled() { return true; }

  /// \brief How long to wait before processing notifications.
  /// \return 30 000 milliseconds, which is the page's default.
  [[nodiscard]] static Integer ApiSubscriptionDelayTime() { return kApiTimeout; }

  /// \brief How many notifications a subscription may hold.
  /// \return 30 000, which is the page's default.
  [[nodiscard]] static Integer ApiSubscriptionMaxNumberOfNotifications() { return kApiTimeout; }

  /// \brief How long the notification server has to answer.
  /// \return 30 000 milliseconds, which is the page's default.
  [[nodiscard]] static Integer ApiSubscriptionSendingNotificationTimeout() { return kApiTimeout; }

  /// \brief Whether this instance runs as the service rather than self-hosted.
  /// \return What the session says.
  [[nodiscard]] static Boolean IsSaaS();

  /// \brief Whether the Excel add-in is enabled.
  /// \return What the session says, because it is a service feature.
  [[nodiscard]] static Boolean IsSaasExcelAddinEnabled();

  /// \brief Whether extensions may be installed.
  /// \return What the session says, because it is a service feature.
  [[nodiscard]] static Boolean EnableSaasExtensionInstallConfigSetting();

  /// \brief Whether Entra groups are enabled on a self-hosted instance.
  /// \return False; the setting describes the self-hosted case and this is not one.
  [[nodiscard]] static Boolean EnableEntraGroupsOnPrem() { return false; }

  /// \brief Whether test automation is enabled.
  ///
  /// \return True.
  ///
  /// \note This tree RUNS as test automation. Answering no would be the one answer that is
  ///       demonstrably untrue of the process asking the question.
  [[nodiscard]] static Boolean TestAutomationEnabled() { return true; }

  /// \brief Whether permissions declared by extensions are used.
  /// \return True.
  [[nodiscard]] static Boolean UsePermissionsFromExtensions() { return true; }

private:
  /// The page gives 30 000 for all three API subscription settings.
  static constexpr Integer kApiTimeout = 30000;
};

}
