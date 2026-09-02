#pragma once

#include "type/NotificationScope.h"

#include "type/Boolean.h"
#include "type/Dictionary.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/Text.h"

#include <cstdint>
#include <string>
#include <string_view>

/// \file
/// \brief AL `Notification` -- a message the client shows beside the user's work.

namespace agiru {


/// \brief AL `Notification`.
///
/// From `notification-data-type.md`: a message the client shows in the context of what the user is
/// doing, carrying data and actions.
///
/// \note IT IS BUILT WHEREVER BUSINESS LOGIC RUNS AND SHOWN WHERE A CLIENT IS, and those are not
///       the same place. The BaseApp constructs and sends notifications deep inside posting and
///       validation code -- 40 generated headers name the type -- so the type has to EXIST for that
///       code to translate even where nothing is watching.
///
/// \note `Send()` AND `Recall()` REACH A CLIENT, and there is none yet. They record what was sent
/// so
///       a test can read it back, which is what the BaseApp's own test libraries do
///       (`LibraryVariableStorage` collects them), rather than refusing and stopping code whose
///       notification is a side effect.
class Notification {
public:
  /// \brief A notification with no identifier yet.
  Notification() = default;

  /// \brief AL `Notification.Id` -- reading it.
  /// \return The identifier; a blank GUID until one is assigned.
  [[nodiscard]] const Guid &Id() const { return id_; }

  /// \brief AL `Notification.Id := X`.
  /// \param id The identifier, which `Recall` matches on.
  void SetId(const Guid &id) { id_ = id; }

  /// \brief AL `Notification.Message` -- reading it.
  /// \return The text the client shows.
  [[nodiscard]] std::string_view Message() const { return message_; }

  /// \brief AL `Notification.Message := X`.
  /// \param message The text the client shows.
  void SetMessage(std::string_view message) { message_ = message; }

  /// \brief AL `Notification.Scope` -- reading it.
  /// \return Where the notification appears.
  [[nodiscard]] NotificationScope Scope() const { return scope_; }

  /// \brief AL `Notification.Scope := X`.
  /// \param scope Where the notification appears.
  void SetScope(NotificationScope scope) { scope_ = scope; }

  /// \brief AL `Notification.SetData(Key, Value)`.
  /// \param key   The name.
  /// \param value The value.
  void SetData(std::string_view key, std::string_view value) {
    data_.Set(std::string(key), std::string(value));
  }

  /// \brief AL `Notification.GetData(Key)`.
  /// \param key The name.
  /// \return The value, or the empty string when nothing was set under it.
  [[nodiscard]] std::string GetData(std::string_view key) const;

  /// \brief AL `Notification.HasData(Key)`.
  /// \param key The name.
  /// \return True when something was set under it.
  [[nodiscard]] Boolean HasData(std::string_view key) const {
    return data_.ContainsKey(std::string(key));
  }

  /// \brief AL `Notification.AddAction(Caption, CodeunitId, MethodName [, Tooltip])`.
  ///
  /// \param caption    What the action reads as.
  /// \param codeunitId The codeunit the client calls.
  /// \param methodName The procedure it calls.
  ///
  /// \note THE ACTION IS RECORDED AND NOT WIRED. Invoking it is the client's half, and there is no
  ///       client; recording it lets a test see that the action was offered, which is what the
  ///       BaseApp's own tests assert.
  void AddAction(std::string_view caption, Integer codeunitId, std::string_view methodName);

  /// \brief AL `Notification.Send()`.
  ///
  /// \note IT RECORDS RATHER THAN REACHING A CLIENT, for the reason the type's own note gives.
  void Send();

  /// \brief AL `Notification.Recall()`.
  ///
  /// \note It withdraws what `Send` recorded, so a test sees the same sequence a client would.
  void Recall();

private:
  Guid id_;
  std::string message_;
  NotificationScope scope_ = NotificationScope::LocalScope;
  Dictionary<std::string, std::string> data_;
  Integer actions_ = 0;
};

} // namespace agiru
