#include "meta/Ids.h"
#include "platform/User.h"
#include "runtime/BrowserSession.h"
#include "runtime/ClientCredentials.h"
#include "runtime/Database.h"
#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/PageCommandHost.h"
#include "runtime/PageHostOptions.h"
#include "runtime/SecureToken.h"
#include "runtime/Session.h"
#include "runtime/SessionCommand.h"
#include "runtime/Storage.h"
#include "runtime/UiHost.h"
#include "type/Guid.h"

#include "Check.h"
#include "OwnedDatabase.h"
#include "PageCallAuthority.h"
#include "PageInteraction.h"

#include <array>
#include <chrono>
#include <future>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

namespace {

constexpr std::string_view kUser = "00000000-0000-0000-0000-000000000001";
constexpr agiru::PageId kFixturePage{50340};
constexpr auto kCallLifetime = std::chrono::minutes(5);

std::string Value(const agiru::Connection &connection, std::string_view sql) {
  const auto rows = connection.Execute(sql);
  if (rows.Rows() != 1) { throw std::runtime_error("missing authority gate row"); }
  const auto value = rows.Value(0, 0);
  if (!value) { throw std::runtime_error("null authority gate value"); }
  return std::string(*value);
}

template <class Body> bool Refused(Body body) {
  try {
    body();
  } catch (const agiru::Error &) { return true; }
  return false;
}

class Fixture {
public:
  explicit Fixture(bool browser = false) {
    {
      const agiru::Session seed(database.Dsn());
      agiru::CreateTable(seed.Database(), agiru::platform::kUserTable);
      agiru::platform::User user;
      user.UserSecurityID = agiru::Guid(kUser);
      user.UserName = "ACTIVE CLIENT";
      user.Insert();
      agiru::InstallClientCredentials(seed.Database());
      agiru::InstallBrowserSessions(seed.Database());
      agiru::InstallPageCommandHost(seed.Database());
      seed.Database().Run("CREATE TABLE active_effects(value integer PRIMARY KEY)");
      agiru::Commit();
    }
    const auto secret =
        agiru::IssueClientCredential(observer, agiru::Guid(kUser), std::chrono::hours(1));
    const auto found = agiru::LookupClientCredentialIdentity(observer, "Bearer " + secret);
    if (!found) { throw std::runtime_error("missing source credential"); }
    source = *found;
    call->credential = source.verifier;
    call->user = source.user;
    call->pageHandle = agiru::GenerateSecureToken();
    call->host = agiru::GenerateSecureToken();
    call->csrf = agiru::GenerateSecureToken();
    call->revision = "0";
    call->page = kFixturePage;
    call->deadline = std::chrono::steady_clock::now() + kCallLifetime;
    if (browser) {
      observer.Run("BEGIN");
      const auto grant = agiru::IssueBrowserSession(observer, source, {});
      observer.Run("COMMIT");
      const auto identity = agiru::LookupBrowserSession(observer, grant.secret);
      if (!identity) { throw std::runtime_error("missing browser credential"); }
      call->credential = identity->client.verifier;
      call->browserCsrf = identity->csrf;
    }
    const std::array<std::optional<std::string>, 4> binds{
        call->pageHandle, call->user.ToStorageText(), call->host, call->credential};
    observer.Run("INSERT INTO agiru_client.page_contexts "
                 "(handle,user_security_id,host_id,company,expires_at,credential_digest) "
                 "VALUES ($1,$2::uuid,$3,'',clock_timestamp()+interval '1 hour',$4)",
                 binds);
    options.database = database.Dsn();
    options.dialogTimeout = std::chrono::seconds(10);
  }

  gate::OwnedDatabase database{"page_call_authority"};
  agiru::Connection observer{database.Dsn()};
  agiru::ClientCredentialIdentity source;
  std::shared_ptr<agiru::detail::PageCall> call = std::make_shared<agiru::detail::PageCall>();
  agiru::PageHostOptions options;
};

void CommitFence(bool browser, std::string_view mutation) {
  const Fixture fixture(browser);
  {
    agiru::Session session(fixture.call->user);
    const agiru::detail::PageCallAuthority authority(session, fixture.call, fixture.options);
    agiru::Connection lease(fixture.database.Dsn());
    agiru::SessionCommand command(session, lease);
    lease.Run("INSERT INTO active_effects VALUES(1)");
    agiru::Commit();
    lease.Run("INSERT INTO active_effects VALUES(2)");
    fixture.observer.Run(mutation);
    CHECK_TRUE("expired or revoked active authority refuses explicit Commit",
               Refused([] { agiru::Commit(); }));
    CHECK_TRUE("swallowed cancellation also refuses command completion",
               Refused([&] { command.Keep(); }));
  }
  CHECK_TEXT("cancellation rolls back only uncommitted effects after an earlier durable Commit",
             Value(fixture.observer,
                   "SELECT string_agg(value::text,',' ORDER BY value) FROM active_effects"),
             "1");
  CHECK_TRUE("cancelled calls retain a sticky denial", fixture.call->cancelled);
}

void ExpiryContracts() {
  constexpr std::array<std::string_view, 7> agents{
      "UPDATE agiru_client.credentials SET revoked_at=clock_timestamp()",
      ("UPDATE agiru_client.credentials SET issued_at=clock_timestamp()-interval '2 hours',"
       "expires_at=clock_timestamp()-interval '1 hour'"),
      "UPDATE agiru_client.page_contexts SET invalidated=true",
      "UPDATE agiru_client.page_contexts SET expires_at=clock_timestamp()-interval '1 second'",
      R"(UPDATE "User" SET "State"=1)",
      R"(UPDATE "User" SET "User Name"='')",
      R"(UPDATE "User" SET "Expiry Date"='2000-01-01T00:00:00.000Z')"};
  for (const auto mutation : agents) { CommitFence(false, mutation); }
  constexpr std::array<std::string_view, 5> browsers{
      "UPDATE agiru_client.browser_sessions SET revoked_at=clock_timestamp()",
      ("UPDATE agiru_client.browser_sessions SET idle_expires_at=clock_timestamp()-interval '1 "
       "second'"),
      ("UPDATE agiru_client.browser_sessions SET issued_at=clock_timestamp()-interval '2 hours',"
       "idle_expires_at=clock_timestamp()-interval '1 hour',expires_at=clock_timestamp()-interval "
       "'1 hour'"),
      "UPDATE agiru_client.credentials SET revoked_at=clock_timestamp()",
      ("UPDATE agiru_client.credentials SET issued_at=clock_timestamp()-interval '2 hours',"
       "expires_at=clock_timestamp()-interval '1 hour'")};
  for (const auto mutation : browsers) { CommitFence(true, mutation); }
}

void StickyCancellation() {
  const Fixture fixture;
  {
    agiru::Session session(fixture.call->user);
    agiru::detail::PageCallAuthority authority(session, fixture.call, fixture.options);
    agiru::Connection lease(fixture.database.Dsn());
    const agiru::SessionCommand command(session, lease);
    lease.Run("INSERT INTO active_effects VALUES(1)");
    CHECK_TRUE("revocation changes the exact source",
               agiru::RevokeClientCredential(fixture.observer, fixture.source.verifier));
    CHECK_TRUE("active callback sees externally committed revocation",
               Refused([&] { authority.Check(); }));
    fixture.observer.Run("UPDATE agiru_client.credentials SET revoked_at=NULL");
    CHECK_TRUE("restoring a credential cannot resurrect the cancelled call",
               Refused([] { agiru::Commit(); }));
  }
  CHECK_TEXT("catching cancellation and restoring SQL authority never commits pending effects",
             Value(fixture.observer, "SELECT count(*) FROM active_effects"),
             "0");
}

void PendingAccountChange() {
  const Fixture fixture;
  {
    agiru::Session session(fixture.call->user);
    const agiru::detail::PageCallAuthority authority(session, fixture.call, fixture.options);
    agiru::Connection lease(fixture.database.Dsn());
    const agiru::SessionCommand command(session, lease);
    lease.Run(R"(UPDATE "User" SET "State"=1)");
    lease.Run("INSERT INTO active_effects VALUES(1)");
    CHECK_TRUE("own pending account change is not an externally committed revocation",
               !Refused([] { agiru::Commit(); }));
    lease.Run("INSERT INTO active_effects VALUES(2)");
    CHECK_TRUE("committed account disablement then refuses further commits",
               Refused([] { agiru::Commit(); }));
  }
  CHECK_TEXT("authorized account disablement remains durable",
             Value(fixture.observer, R"(SELECT "State" FROM "User")"),
             "1");
  CHECK_TEXT("later cancellation rolls back only effects after account disablement",
             Value(fixture.observer, "SELECT count(*) FROM active_effects"),
             "1");
}

void WaitingQuestion(bool browser, bool committed) {
  const Fixture fixture(browser);
  auto executing = std::async(std::launch::async, [&] {
    agiru::Session session(fixture.call->user);
    auto ui = agiru::detail::MakePageUiHost(
        fixture.call, fixture.options, [](const auto &, auto, const auto &) {});
    const agiru::detail::PageCallAuthority authority(session, fixture.call, fixture.options);
    agiru::Connection lease(fixture.database.Dsn());
    agiru::SessionCommand command(session, lease);
    lease.Run("INSERT INTO active_effects VALUES(1)");
    if (committed) {
      agiru::Commit();
      lease.Run("INSERT INTO active_effects VALUES(2)");
    }
    try {
      static_cast<void>(ui->Confirm("Explicit consent?", false));
      command.Keep();
      return std::string("unexpected success");
    } catch (const agiru::Error &error) { return std::string(error.Code()); }
  });
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (Value(fixture.observer, "SELECT count(*) FROM agiru_client.page_dialogs") != "1") {
    if (std::chrono::steady_clock::now() >= deadline) {
      throw std::runtime_error("question was not published");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  const std::string table = browser ? "browser_sessions" : "credentials";
  fixture.observer.Run("UPDATE agiru_client." + table + " SET revoked_at=clock_timestamp()");
  CHECK_TRUE("revocation wakes a suspended AL question before its ten-second timeout",
             executing.wait_for(std::chrono::seconds(2)) == std::future_status::ready);
  CHECK_TEXT(
      "revoked question aborts without a default answer", executing.get(), "UiSessionCancelled");
  CHECK_TEXT(
      "cancelled question is closed without accepted consent",
      Value(fixture.observer,
            "SELECT closed::text||':'||(answer IS NULL)::text FROM agiru_client.page_dialogs"),
      "true:true");
  CHECK_TEXT("question cancellation preserves only a prior production Commit",
             Value(fixture.observer, "SELECT count(*) FROM active_effects"),
             committed ? "1" : "0");
}

void CommitSerialization() {
  Fixture fixture;
  agiru::Session session(fixture.call->user);
  agiru::detail::PageCallAuthority authority(session, fixture.call, fixture.options);
  agiru::Connection lease(fixture.database.Dsn());
  agiru::SessionCommand command(session, lease);
  lease.Run("INSERT INTO active_effects VALUES(1)");
  authority.LockCommit(lease);
  std::promise<void> entered;
  auto started = entered.get_future();
  auto revoked = std::async(std::launch::async, [&] {
    entered.set_value();
    const agiru::Connection writer(fixture.database.Dsn());
    writer.Run("SET statement_timeout='3s'");
    writer.Run("SET application_name='agiru_call_revoke_gate'");
    return agiru::RevokeClientCredential(writer, fixture.source.verifier);
  });
  started.get();
  bool blocked = false;
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (!blocked &&
         revoked.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout &&
         std::chrono::steady_clock::now() < deadline) {
    blocked =
        Value(fixture.observer,
              "SELECT EXISTS(SELECT 1 FROM pg_stat_activity "
              "WHERE datname=current_database() AND application_name='agiru_call_revoke_gate' "
              "AND wait_event_type='Lock')") == "t";
    if (!blocked) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }
  }
  CHECK_TRUE("revocation cannot overtake an already authorized committing transaction", blocked);
  command.Keep();
  CHECK_TRUE("revocation completes after the prior commit releases its authority lock",
             revoked.get());
  CHECK_TEXT("ordered pre-revocation commit stays durable",
             Value(fixture.observer, "SELECT count(*) FROM active_effects"),
             "1");
}

}

int main() {
  return gate::Run("PageCallAuthority", [] {
    ExpiryContracts();
    StickyCancellation();
    PendingAccountChange();
    for (const bool browser : {false, true}) {
      for (const bool committed : {false, true}) { WaitingQuestion(browser, committed); }
    }
    CommitSerialization();
  });
}
