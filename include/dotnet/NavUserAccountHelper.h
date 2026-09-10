#pragma once

#include "dotnet/Refused.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Text.h"

#include <string_view>
#include <vector>

namespace agiru::dotnet {

/// \brief The platform's `NavUserAccountHelper`, rebuilt for one tier without Entra ID or
///        Windows: every authentication question answers "user name and password", every
///        delegated or Azure question "no", and the key and claim lookups report nothing found.
/// \warning A member that would have to reach an identity provider is `Refused` and raises when
///          called, rather than answering with an invented identity.
class NavUserAccountHelper {
public:
  /// \brief `IsAzure()`. \return `false`; this tier is not a SaaS tenant.
  [[nodiscard]] static Boolean IsAzure() { return false; }

  /// \brief `IsWindowsAuthentication()`. \return `false`.
  [[nodiscard]] static Boolean IsWindowsAuthentication() { return false; }

  /// \brief `IsUserNamePasswordAuthentication()`. \return `true`, the one authentication here.
  [[nodiscard]] static Boolean IsUserNamePasswordAuthentication() { return true; }

  /// \brief `IsAccessControlServiceAuthentication()`. \return `false`.
  [[nodiscard]] static Boolean IsAccessControlServiceAuthentication() { return false; }

  /// \brief `IsSessionAdminSession()`. \return `true`.
  [[nodiscard]] static Boolean IsSessionAdminSession() { return true; }

  /// \brief `IsUserSuperInAllCompanies(...)`. \return `true`.
  template <typename... Arguments>
  [[nodiscard]] static Boolean IsUserSuperInAllCompanies(Arguments &&...) {
    return true;
  }

  /// \brief `IsPermissionSetAssigned(...)`. \return `false`.
  template <typename... Arguments>
  [[nodiscard]] static Boolean IsPermissionSetAssigned(Arguments &&...) {
    return false;
  }

  /// \brief `IsPermissionSetValid(RoleId, AppId, Scope, var Errors)`. \return `true`.
  template <typename... Arguments>
  [[nodiscard]] static Boolean IsPermissionSetValid(Arguments &&...) {
    return true;
  }

  /// \brief `IsUserDelegatedAdmin()`. \return `false`.
  [[nodiscard]] static Boolean IsUserDelegatedAdmin() { return false; }

  /// \brief `IsUserDelegatedHelpdesk()`. \return `false`.
  [[nodiscard]] static Boolean IsUserDelegatedHelpdesk() { return false; }

  /// \brief `IsDelegatedUser()`. \return `false`.
  [[nodiscard]] static Boolean IsDelegatedUser() { return false; }

  /// \brief `IsPasswordSet(UserSecurityId)`. \return `false`; passwords are not kept here.
  template <typename... Arguments> [[nodiscard]] static Boolean IsPasswordSet(Arguments &&...) {
    return false;
  }

  /// \brief `UserName()`. \return The session's user name.
  [[nodiscard]] static ::agiru::Text<0> UserName();

  /// \brief `UserName(Sid)`. \param Sid A Windows security id or Entra object id.
  /// \return Empty: no directory answers here.
  template <typename Sid> [[nodiscard]] static ::agiru::Text<0> UserName(const Sid &) {
    return {};
  }

  /// \brief `GetAllowedCompanies([UserSecurityId])`. \return None listed: the caller then
  ///        falls back to the companies it can read itself.
  template <typename... Arguments>
  [[nodiscard]] static std::vector<::agiru::Text<0>> GetAllowedCompanies(Arguments &&...) {
    return {};
  }

  /// \brief `GetAuthenticationStatus(UserSecurityId)`. \return 0, the platform's "none".
  template <typename... Arguments>
  [[nodiscard]] static ::agiru::Integer GetAuthenticationStatus(Arguments &&...) {
    return 0;
  }

  /// \brief `GetPuid()`. \return Empty; there is no Microsoft account behind a session.
  [[nodiscard]] static ::agiru::Text<0> GetPuid() { return {}; }

  /// \brief `GetCurrentUserTokenClaim(Claim)`. \return Empty; there is no token.
  template <typename... Arguments>
  [[nodiscard]] static ::agiru::Text<0> GetCurrentUserTokenClaim(Arguments &&...) {
    return {};
  }

  /// \brief `GetEffectivePermissionForObject(User, Company, ObjectType, ObjectId)`.
  /// \return The five permissions as `SelectStr` reads them, all granted.
  template <typename... Arguments>
  [[nodiscard]] static ::agiru::Text<0> GetEffectivePermissionForObject(Arguments &&...) {
    return ::agiru::Text<0>{kAllGranted};
  }

  /// \brief `GetEntitlementPermissionForObject(User, ObjectType, ObjectId)`. \return All granted.
  template <typename... Arguments>
  [[nodiscard]] static ::agiru::Text<0> GetEntitlementPermissionForObject(Arguments &&...) {
    return ::agiru::Text<0>{kAllGranted};
  }

  /// \brief `GetEntitlementPermissionForObjectAndPlan(Plan, ObjectType, ObjectId)`.
  /// \return All granted.
  template <typename... Arguments>
  [[nodiscard]] static ::agiru::Text<0> GetEntitlementPermissionForObjectAndPlan(Arguments &&...) {
    return ::agiru::Text<0>{kAllGranted};
  }

  /// \brief `SetAuthenticationObjectId(UserSecurityId, ObjectId)`. Kept nowhere.
  template <typename... Arguments> static void SetAuthenticationObjectId(Arguments &&...) {}

  /// \brief `TrySetAuthenticationKey(UserSecurityId, Key)`. \return `true`.
  template <typename... Arguments>
  [[nodiscard]] static Boolean TrySetAuthenticationKey(Arguments &&...) {
    return true;
  }

  /// \brief `TrySetAuthenticationEmail(UserSecurityId, Email)`. \return `true`.
  template <typename... Arguments>
  [[nodiscard]] static Boolean TrySetAuthenticationEmail(Arguments &&...) {
    return true;
  }

  /// \brief `TryGetAuthenticationKey(UserSecurityId, var Key)`. \return `false`: none kept.
  template <typename... Arguments>
  [[nodiscard]] static Boolean TryGetAuthenticationKey(Arguments &&...) {
    return false;
  }

  /// \brief `TryGetAuthenticationObjectId(UserSecurityId, var ObjectId)`. \return `false`.
  template <typename... Arguments>
  [[nodiscard]] static Boolean TryGetAuthenticationObjectId(Arguments &&...) {
    return false;
  }

  /// \brief `TryGetNameIdentifier(UserSecurityId, var NameId)`. \return `false`.
  template <typename... Arguments>
  [[nodiscard]] static Boolean TryGetNameIdentifier(Arguments &&...) {
    return false;
  }

  /// \brief `TryCreateWebServicesKey(UserSecurityId, Expiry, var Key)`. \return `false`.
  template <typename... Arguments>
  [[nodiscard]] static Boolean TryCreateWebServicesKey(Arguments &&...) {
    return false;
  }

  /// \brief `TryClearWebServicesKey(UserSecurityId)`. \return `true`; there was none.
  template <typename... Arguments>
  [[nodiscard]] static Boolean TryClearWebServicesKey(Arguments &&...) {
    return true;
  }

  /// \brief `TryGetWebServicesKey(UserSecurityId, var Key, var Expiry)`. \return `false`.
  template <typename... Arguments>
  [[nodiscard]] static Boolean TryGetWebServicesKey(Arguments &&...) {
    return false;
  }

  Refused CreateUserFromAzureADObjectId{{.type = "NavUserAccountHelper", .member = "CreateUserFromAzureADObjectId"}};
  Refused CreateUserFromAAdGroupObjectId{{.type = "NavUserAccountHelper", .member = "CreateUserFromAAdGroupObjectId"}};
  Refused GetWindowsGroupMembersByName{{.type = "NavUserAccountHelper", .member = "GetWindowsGroupMembersByName"}};
  Refused GetTokenAuthorityEndpointServerSetting{{.type = "NavUserAccountHelper", .member = "GetTokenAuthorityEndpointServerSetting"}};
  Refused GetLocalWindowsGroups{{.type = "NavUserAccountHelper", .member = "GetLocalWindowsGroups"}};
  Refused GetPermissionSetRelations{{.type = "NavUserAccountHelper", .member = "GetPermissionSetRelations"}};
  Refused CreateApplicationRegistration{{.type = "NavUserAccountHelper", .member = "CreateApplicationRegistration"}};

private:
  static constexpr std::string_view kAllGranted = "1,1,1,1,1";
};

}
