#include "platform/User.h"
#include "platform/UserLicenseType.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Language.h"
#include "type/Option.h"
#include "type/Time.h"

#include "AccountFixturePermissions.h"
#include "Check.h"
#include "OwnedDatabase.h"

#include <array>
#include <cstdint>
#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <thread>

namespace {

constexpr std::uint8_t kExpiredUser = 5;
constexpr std::uint8_t kUnknownStateUser = 6;
constexpr std::uint8_t kFutureExpiryUser = 7;
constexpr std::uint8_t kAuditTarget = 8;
constexpr std::uint8_t kTemporaryTarget = 9;
constexpr std::uint8_t kMissingUser = 99;
constexpr std::int32_t kInvalidState = 99;
constexpr auto kPolishLanguage = 1045;

agiru::Guid Identity(std::uint8_t suffix) {
  std::array<std::uint8_t, agiru::Guid::kSize> bytes{};
  bytes.back() = suffix;
  return agiru::Guid(bytes);
}

void SeedUsers(const std::string &dsn) {
  const agiru::Session seed(dsn);
  agiru::CreateTable(seed.Database(), agiru::TableTraits<agiru::platform::User>::kTable);
  for (std::uint8_t number = 1; number <= kFutureExpiryUser; ++number) {
    agiru::platform::User user;
    user.UserSecurityID = Identity(number);
    user.UserName = number == 1 ? "Alice NÄME" : "User " + std::to_string(number);
    if (number == 3) { user.State = agiru::platform::UserState::Disabled; }
    if (number == 4) { user.UserName = ""; }
    if (number == kExpiredUser) { user.ExpiryDate = agiru::DateTime::FromMilliseconds(1); }
    if (number == kUnknownStateUser) { user.State = kInvalidState; }
    if (number == kFutureExpiryUser) {
      user.ExpiryDate =
          agiru::CurrentDateTime() + agiru::Duration(agiru::Time::kMillisecondsPerDay);
    }
    user.Insert();
  }
  agiru::Commit();
}

void CheckAudit(const agiru::Connection &observer,
                const agiru::Guid &creator,
                const agiru::Guid &modifier) {
  const std::array<std::optional<std::string>, 1> parameters{
      Identity(kAuditTarget).ToStorageText()};
  const auto result = observer.Execute(
      "SELECT \"SystemCreatedBy\"::text, \"SystemModifiedBy\"::text, \"Full Name\" "
      "FROM \"User\" WHERE \"User Security ID\" = $1::uuid",
      parameters);
  CHECK_TRUE("the audit row is committed and visible to an independent connection",
             result.Rows() == 1);
  if (result.Rows() != 1) { return; }
  CHECK_TEXT("SQL creator is the original session's typed security ID",
             result.Value(0, 0).value_or(""),
             creator.ToStorageText());
  CHECK_TEXT("SQL modifier is the writing session's typed security ID",
             result.Value(0, 1).value_or(""),
             modifier.ToStorageText());
  CHECK_TEXT("SQL sees the corresponding committed business value",
             result.Value(0, 2).value_or(""),
             creator == modifier ? "Created by Alice" : "Changed by Bob");
}

void ResolvedIdentitiesOwnAuditValues(const std::string &dsn) {
  const agiru::Connection observer(dsn);
  agiru::Session alice(dsn, Identity(1));
  gate::AccountFixturePermissions(alice);
  alice.Language(kPolishLanguage);
  CHECK_TEXT("UserId comes from the Code-typed database field", alice.UserId(), "ALICE NÄME");
  CHECK_TRUE("UserSecurityId remains a Guid", alice.UserSecurityId() == Identity(1));
  agiru::platform::User row;
  row.UserSecurityID = Identity(kAuditTarget);
  row.UserName = "Audit target";
  row.FullName = "Created by Alice";
  row.Insert();
  CHECK_TRUE("insert stamps both AL audit fields with the active user",
             row.SystemCreatedBy == Identity(1) && row.SystemModifiedBy == Identity(1));
  agiru::Temporary<agiru::platform::User> temporary;
  temporary.UserSecurityID = Identity(kTemporaryTarget);
  temporary.Insert();
  CHECK_TRUE("temporary records carry the same identity without SQL persistence",
             temporary.SystemCreatedBy == Identity(1));
  agiru::Commit();
  CheckAudit(observer, Identity(1), Identity(1));
  {
    agiru::Session bob(dsn, Identity(2));
    gate::AccountFixturePermissions(bob);
    CHECK_TEXT("a nested user resolves its own name", bob.UserId(), "USER 2");
    CHECK_TRUE("a nested user has its own security ID", bob.UserSecurityId() == Identity(2));
    agiru::platform::User changed;
    changed.Get(Identity(kAuditTarget));
    changed.FullName = "Changed by Bob";
    changed.Modify();
    agiru::Commit();
  }
  CHECK_TRUE("closing a nested session restores the original identity",
             &agiru::Session::Current() == &alice && alice.UserSecurityId() == Identity(1));
  CHECK_TRUE("closing a nested session restores the original language",
             agiru::Language::Current() == kPolishLanguage);
  CheckAudit(observer, Identity(1), Identity(2));
  const auto temporaryCount =
      observer.Execute("SELECT count(*) FROM \"User\" WHERE \"User Security ID\" = "
                       "'00000000-0000-0000-0000-000000000009'::uuid");
  CHECK_TRUE("temporary user never becomes a persisted account",
             temporaryCount.Value(0, 0) == std::optional<std::string_view>{"0"});
}

void RefuseIdentity(const std::string &dsn, const agiru::Guid &identity) {
  const bool hadCurrent = agiru::Session::HasCurrent();
  agiru::Session *const previous = hadCurrent ? &agiru::Session::Current() : nullptr;
  const auto language = agiru::Language::Current();
  bool refused = false;
  try {
    const agiru::Session attempted(dsn, identity);
  } catch (const agiru::SessionError &error) {
    refused = true;
    CHECK_TEXT("identity refusals do not disclose account existence or state",
               error.what(),
               "session user is not active");
  }
  CHECK_TRUE("invalid or inactive identity refuses session construction", refused);
  CHECK_TRUE("a failed constructor preserves the current-session presence",
             agiru::Session::HasCurrent() == hadCurrent);
  if (hadCurrent && agiru::Session::HasCurrent()) {
    CHECK_TRUE("a failed constructor restores the exact parent",
               &agiru::Session::Current() == previous);
  }
  CHECK_TRUE("a failed constructor restores the active language",
             agiru::Language::Current() == language);
}

void RefusalsRestoreAmbientState(const std::string &dsn) {
  const std::array<std::uint8_t, 6> invalid{0, 3, 4, kExpiredUser, kUnknownStateUser, kMissingUser};
  for (const auto suffix : invalid) { RefuseIdentity(dsn, Identity(suffix)); }
  agiru::Session parent(dsn, Identity(1));
  parent.Language(kPolishLanguage);
  RefuseIdentity(dsn, Identity(3));
  CHECK_TRUE("failed child leaves the parent principal untouched",
             parent.UserSecurityId() == Identity(1));
  const gate::OwnedDatabase missing("identity_missing_table");
  bool databaseError = false;
  try {
    const agiru::Session attempted(missing.Dsn(), Identity(1));
  } catch (const agiru::DatabaseError &) { databaseError = true; }
  CHECK_TRUE("missing authority storage propagates its database error", databaseError);
  CHECK_TRUE("failed authority reads restore the parent and language",
             &agiru::Session::Current() == &parent &&
                 agiru::Language::Current() == kPolishLanguage);
}

void AccountsAreRecheckedWithoutLicenseGates(const std::string &dsn) {
  const agiru::Connection authority(dsn);
  authority.Run("UPDATE \"User\" SET \"State\" = 1 WHERE \"User Security ID\" = "
                "'00000000-0000-0000-0000-000000000001'::uuid");
  RefuseIdentity(dsn, Identity(1));
  authority.Run("UPDATE \"User\" SET \"State\" = 0, \"User Name\" = 'RENAMED ALICE' "
                "WHERE \"User Security ID\" = '00000000-0000-0000-0000-000000000001'::uuid");
  for (const auto &license : agiru::OptionTraits<agiru::platform::UserLicenseType>::kValues) {
    authority.Run(R"(UPDATE "User" SET "License Type" = )" + std::to_string(license.ordinal) +
                  " WHERE \"User Security ID\" = '00000000-0000-0000-0000-000000000001'::uuid");
    const agiru::Session session(dsn, Identity(1));
    CHECK_TEXT("a new session reads the renamed database account without a license gate",
               session.UserId(),
               "RENAMED ALICE");
  }
  const agiru::Session future(dsn, Identity(kFutureExpiryUser));
  CHECK_TRUE("a future expiry does not prevent sign-in",
             future.UserSecurityId() == Identity(kFutureExpiryUser));
}

void WorkerReuseDoesNotLeakIdentity(const std::string &dsn) {
  const agiru::Session parent(dsn, Identity(1));
  std::array<std::string, 2> names;
  std::array<agiru::Guid, 2> identities;
  std::string failure;
  bool cleared = false;
  std::thread worker([&] {
    try {
      for (std::uint8_t number = 1; number <= 2; ++number) {
        const agiru::Session session(dsn, Identity(number));
        names[number - 1] = session.UserId();
        identities[number - 1] = session.UserSecurityId();
      }
      cleared = !agiru::Session::HasCurrent();
    } catch (const std::exception &error) { failure = error.what(); }
  });
  worker.join();
  CHECK_SILENT("both worker sessions open successfully", failure);
  CHECK_TRUE("a reused worker clears its ambient session", cleared);
  CHECK_TRUE("a reused worker observes each distinct typed identity",
             identities[0] == Identity(1) && identities[1] == Identity(2));
  CHECK_TEXT("a reused worker observes the first name", names[0], "RENAMED ALICE");
  CHECK_TEXT("a reused worker observes the second name", names[1], "USER 2");
  CHECK_TRUE("worker activity never changes the parent's current session",
             &agiru::Session::Current() == &parent && parent.UserSecurityId() == Identity(1));
}

void HarnessIdentityRemainsExplicit(const std::string &dsn) {
  agiru::Session harness(dsn);
  CHECK_TEXT("legacy harness name remains compatible", harness.UserId(), "SYSTEM");
  CHECK_TRUE("legacy harness security ID remains blank", harness.UserSecurityId().IsNull());
  const auto explicitDate = agiru::Date::FromYmd(2026, 1, 1);
  CHECK_TRUE("explicit work date remains session-owned",
             harness.WorkDate(explicitDate) == explicitDate);
  const auto before = agiru::CurrentDateTime().Date();
  const auto restored = harness.WorkDate(agiru::Date{});
  const auto after = agiru::CurrentDateTime().Date();
  CHECK_TRUE("blank work date uses the same clock primitive as Today",
             restored == before || restored == after);
}

}

int main() {
  return gate::Run("SessionIdentity", [] {
    const gate::OwnedDatabase database("identity");
    SeedUsers(database.Dsn());
    ResolvedIdentitiesOwnAuditValues(database.Dsn());
    RefusalsRestoreAmbientState(database.Dsn());
    AccountsAreRecheckedWithoutLicenseGates(database.Dsn());
    WorkerReuseDoesNotLeakIdentity(database.Dsn());
    HarnessIdentityRemainsExplicit(database.Dsn());
    CHECK_TRUE("all accounted and harness sessions close", !agiru::Session::HasCurrent());
  });
}
