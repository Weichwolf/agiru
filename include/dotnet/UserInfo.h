#pragma once

#include "type/Boolean.h"
#include "type/List.h"

#include <string>
#include <string_view>

/// \file
/// \brief The .NET type `UserInfo` -- a directory user as the Graph returns it.

namespace agiru::dotnet {

/// \brief One user in the directory.
///
/// `Codeunit "Azure AD Graph"` hands these out -- `GetUser`, `GetCurrentUser`,
/// `GetUserByAuthorizationEmail` -- and the BaseApp reads properties off them. Measured over
/// BCApps: `UserPrincipalName` 9, `ObjectId` 8, `Roles` 5, `GivenName` 4, `Mail` 3, `DisplayName`
/// 2, `AccountEnabled` 2, `Surname` 1, `PreferredLanguage` 1.
///
/// \note IT IS A VALUE AND NOT A CLIENT. Whatever fetches a user from a directory belongs to the
///       host; this is what such a fetch RETURNS, so it exists and is empty until something fills
///       it. That is the difference between a type that carries no data and a type that lies: an
///       empty `UserPrincipalName` is an answer, and the BaseApp branches on it.
class UserInfo {
public:
  /// \brief A user nobody has filled in.
  UserInfo() = default;

  /// \brief The directory's immutable identifier for the user.
  /// \return The object id.
  [[nodiscard]] std::string_view ObjectId() const { return objectId_; }

  /// \brief The user's sign-in name.
  /// \return The principal name.
  [[nodiscard]] std::string_view UserPrincipalName() const { return userPrincipalName_; }

  /// \brief The user's given name.
  /// \return The first name.
  [[nodiscard]] std::string_view GivenName() const { return givenName_; }

  /// \brief The user's family name.
  /// \return The surname.
  [[nodiscard]] std::string_view Surname() const { return surname_; }

  /// \brief The name shown in a client.
  /// \return The display name.
  [[nodiscard]] std::string_view DisplayName() const { return displayName_; }

  /// \brief The user's mail address.
  /// \return The address.
  [[nodiscard]] std::string_view Mail() const { return mail_; }

  /// \brief The language the user prefers.
  /// \return The language tag.
  [[nodiscard]] std::string_view PreferredLanguage() const { return preferredLanguage_; }

  /// \brief Whether the account may sign in.
  /// \return True when it is enabled.
  [[nodiscard]] Boolean AccountEnabled() const { return accountEnabled_; }

  /// \brief The directory roles the user holds.
  /// \return The roles.
  [[nodiscard]] const List<std::string> &Roles() const { return roles_; }

  /// \brief The groups the user belongs to.
  /// \return The groups.
  [[nodiscard]] const List<std::string> &Groups() const { return groups_; }

  /// \brief Fills in what a directory returned.
  /// \param objectId          The immutable identifier.
  /// \param principalName     The sign-in name.
  /// \param displayName       The name a client shows.
  /// \param enabled           Whether the account may sign in.
  void Describe(std::string objectId,
                std::string principalName,
                std::string displayName,
                Boolean enabled) {
    objectId_ = std::move(objectId);
    userPrincipalName_ = std::move(principalName);
    displayName_ = std::move(displayName);
    accountEnabled_ = enabled;
  }

private:
  std::string objectId_;
  std::string userPrincipalName_;
  std::string givenName_;
  std::string surname_;
  std::string displayName_;
  std::string mail_;
  std::string preferredLanguage_;
  Boolean accountEnabled_ = false;
  List<std::string> roles_;
  List<std::string> groups_;
};

} // namespace agiru::dotnet
