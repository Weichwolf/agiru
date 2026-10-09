#pragma once

#include "runtime/HttpServerOptions.h"
#include "runtime/PageHostOptions.h"

#include <chrono>
#include <cstddef>
#include <string>
#include <string_view>

/// \file
/// \brief Native application lifecycle; transports and SQL remain behind the runtime boundary.
namespace agiru {

/// \brief Trusted single-company deployment configuration, never HTTP-controlled authority.
struct NativeServiceOptions {
  static constexpr std::size_t kConfigBytes = 65536; ///< Bounded trusted startup configuration.
  PageHostOptions pages{};  ///< Single authoritative company/session/retained-page settings.
  HttpServerOptions http{}; ///< Single authoritative private listener/admission settings.
};

/// \brief Parses the complete version-one server JSON schema, never AL or request authority.
/// \param text Bounded UTF-8 JSON with all fields shown in deploy/dev/agiru.json.
/// \return Validated configuration, retaining exact strings and integer limits.
/// \throws Error with ServerConfiguration for missing/unknown/duplicate keys or invalid values.
[[nodiscard]] NativeServiceOptions ParseNativeServiceOptions(std::string_view text);

/// \brief Reads one bounded regular file without following a final-component symbolic link.
/// \param path Explicit trusted operator path; FIFO/device/directory input refuses.
/// \return Validated immutable-startup settings. No connection or listener is opened.
/// \throws Error with ServerConfiguration for file/schema failures; values are never echoed.
[[nodiscard]] NativeServiceOptions LoadNativeServiceOptions(std::string_view path);

/// \brief Runs native HTTP/page execution until SIGINT/SIGTERM, then drains the listener.
/// \param options Trusted explicit deployment, with existing original permission/client storage.
/// \throws Error for invalid/missing company/storage, permission metadata or listener failure.
/// \note Requires original SQL permission data. Uninstalled system sets explicitly refuse;
/// neither startup nor requests create users/grants or silently install missing native tables.
/// Writes only a readiness line with PID/port, never credentials or connection strings.
void RunNativeService(const NativeServiceOptions &options);

/// \brief Explicit trusted-operator client-storage migration, never anonymous HTTP provisioning.
/// \param database Database already containing the original User table.
/// \note Installs private own-write metadata on existing registered versioned ERP tables;
///       preserves their business/audit/rowversion values and creates no absent ERP tables.
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
