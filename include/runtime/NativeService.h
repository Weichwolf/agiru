#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

/// \file
/// \brief Native application lifecycle; transports and SQL remain behind the runtime boundary.
namespace agiru {

/// \brief Trusted single-company deployment configuration, never HTTP-controlled authority.
struct NativeServiceOptions {
  static constexpr std::uint16_t kBackendPort = 18080; ///< deploy/dev/Caddyfile upstream port.
  std::string database; ///< Explicit existing database connection, not a master/template clone.
  std::string company;  ///< Exact sole original Company.Name in the initial flat storage profile.
  std::string origin;   ///< Trusted public Caddy origin used for browser CSRF checks.
  std::uint16_t port = kBackendPort; ///< Private loopback upstream, never a public bind.
  std::size_t workers = 2;           ///< Fixed blocking execution workers, not per-session threads.
};

/// \brief Runs native HTTP/page execution until SIGINT/SIGTERM, then drains the listener.
/// \param options Trusted explicit deployment, with existing original permission/client storage.
/// \throws Error for invalid/missing company/storage, permission metadata or listener failure.
/// \note Requires original SQL permission data. Uninstalled system sets explicitly refuse;
/// neither startup nor requests create users/grants or silently install missing native tables.
/// Writes only a readiness line with PID/port, never credentials or connection strings.
void RunNativeService(const NativeServiceOptions &options);

/// \brief Explicit trusted-operator client-storage migration, never anonymous HTTP provisioning.
/// \param database Database already containing the original User table.
/// \throws Error for migration/commit failure; creates no users, roles or permissions.
void InitializeNativeClient(const std::string &database);

/// \brief Issues one durably committed native credential; does not grant ERP permissions.
/// \param database Trusted operator connection, with original User/client storage.
/// \param user Original User security GUID. \param lifetime Positive expiry, at most one day.
/// \return Plaintext once; operator must protect it and never log it or put it in URLs.
/// \throws Error for invalid/missing identity, storage, crypto or durable commit failure.
[[nodiscard]] std::string IssueNativeClientCredential(const std::string &database,
                                                      std::string_view user,
                                                      std::chrono::seconds lifetime);

}
