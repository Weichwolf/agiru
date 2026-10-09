#include "PageCallAuthority.h"

#include "runtime/ErrorValue.h"
#include "runtime/PageHostOptions.h"
#include "runtime/SecureToken.h"
#include "runtime/Session.h"

#include "PageInteraction.h"
#include "SessionState.h"
#include "SessionUser.h"

#include <array>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

namespace agiru::detail {

PageCallAuthority::PageCallAuthority(Session &session,
                                     std::shared_ptr<PageCall> call,
                                     const PageHostOptions &options)
    : state_(SessionState::For(session)),
      call_(std::move(call)),
      authority_(options.database),
      company_(options.company) {
  if (state_.commandAuthority != nullptr || state_.commandActive.test() || !call_) {
    throw SessionError("client authority requires an idle session");
  }
  Check();
  state_.commandAuthority = this;
}

PageCallAuthority::~PageCallAuthority() {
  state_.commandAuthority = nullptr;
}

bool PageCallAuthority::Live(const Connection &connection, bool lock) const {
  const std::array<std::optional<std::string>, 6> binds{
      call_->credential,
      call_->user.ToStorageText(),
      call_->pageHandle,
      call_->host,
      call_->browserCsrf.empty() ? std::string{} : SecureTokenDigest(call_->browserCsrf),
      company_};
  std::string sql;
  if (call_->browserCsrf.empty()) {
    sql = "SELECT c.digest FROM agiru_client.credentials c JOIN agiru_client.page_contexts p "
          "ON p.credential_digest=c.digest WHERE c.digest=$1 AND c.user_security_id=$2::uuid "
          "AND c.revoked_at IS NULL AND c.expires_at>clock_timestamp() AND $5::text=''";
  } else {
    sql = "SELECT b.digest FROM agiru_client.browser_sessions b JOIN agiru_client.credentials c "
          "ON c.digest=b.source_digest JOIN agiru_client.page_contexts p "
          "ON p.credential_digest=b.digest WHERE b.digest=$1 AND b.user_security_id=$2::uuid "
          "AND b.csrf_digest=$5 AND b.revoked_at IS NULL AND b.expires_at>clock_timestamp() "
          "AND b.idle_expires_at>clock_timestamp() AND c.revoked_at IS NULL "
          "AND c.expires_at>clock_timestamp() AND c.user_security_id=b.user_security_id";
  }
  sql += " AND p.handle=$3 AND p.host_id=$4 AND p.user_security_id=$2::uuid "
         "AND p.company=$6 AND NOT p.invalidated AND p.expires_at>clock_timestamp()";
  if (lock) { sql += call_->browserCsrf.empty() ? " FOR SHARE OF c,p" : " FOR SHARE OF b,c,p"; }
  return connection.Execute(sql, binds).Rows() == 1;
}

void PageCallAuthority::Cancel() {
  const std::lock_guard lock(call_->mutex);
  call_->cancelled = true;
  call_->ready.notify_all();
}

void PageCallAuthority::Check() {
  try {
    {
      const std::lock_guard lock(call_->mutex);
      if (call_->cancelled || call_->deadline <= std::chrono::steady_clock::now()) {
        throw Error("client session ended", "UiSessionCancelled");
      }
    }
    if (!Live(authority_, false)) { throw Error("client session ended", "UiSessionCancelled"); }
    static_cast<void>(RequireActiveUser(authority_, call_->user));
  } catch (...) {
    Cancel();
    throw;
  }
}

void PageCallAuthority::LockCommit(const Connection &connection) {
  Check();
  try {
    if (!connection.InTransaction()) { throw Error("client session ended", "UiSessionCancelled"); }
    const std::array<std::optional<std::string>, 1> binds{call_->user.ToStorageText()};
    if (connection
            .Execute(R"(SELECT 1 FROM "User" WHERE "User Security ID"=$1::uuid FOR SHARE)", binds)
            .Rows() != 1) {
      throw Error("client session ended", "UiSessionCancelled");
    }
    static_cast<void>(RequireActiveUser(authority_, call_->user));
    if (!Live(connection, true)) { throw Error("client session ended", "UiSessionCancelled"); }
  } catch (...) {
    Cancel();
    throw;
  }
}

}
