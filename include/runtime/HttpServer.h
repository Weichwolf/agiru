#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

/// \file
/// \brief Bounded native HTTP transport; ERP/authentication belong to its handler.
namespace agiru {

/// \brief One owned HTTP header, preserving its exact value.
struct ServerHttpHeader {
  std::string name;  ///< ASCII HTTP field name.
  std::string value; ///< Exact field value; response controls refuse injection.
};

/// \brief Owned request bytes, passed immutably to one bounded executor job.
/// The target is the original encoded URI, not form-decoded company/filter text.
struct ServerHttpRequest {
  std::string method;                    ///< Parsed HTTP method, without implicit ERP actions.
  std::string target;                    ///< Original encoded URI, including query text.
  std::vector<ServerHttpHeader> headers; ///< Original field values, bounded and duplicate-free.
  std::string body;                      ///< Exact body bytes, including binary uploads.

  /// \brief Case-insensitive lookup; an absent header returns an empty view.
  /// \param name ASCII header identity.
  /// \return A borrowed value while this request remains alive.
  /// \note The transport refuses repeated header identities rather than merging authority.
  [[nodiscard]] std::string_view Header(std::string_view name) const;
};

/// \brief Owned response; handlers must explicitly map AL/permission diagnostics.
struct ServerHttpResponse {
  static constexpr unsigned kOk = 200; ///< HTTP success status from RFC 9110, section 15.3.1.
  unsigned status = kOk; ///< Final HTTP response status; informational responses refuse.
  std::string contentType = "text/html; charset=utf-8"; ///< Explicit MIME type and text encoding.
  std::string body;                                     ///< Owned bounded response bytes.
  std::vector<ServerHttpHeader> headers; ///< Handler fields, validated before sending.
};

/// \brief Default native executor size from the Linux process affinity mask.
/// \return One worker per available CPU, at least one and at most the declared worker ceiling.
/// Workers are not pinned: the OS distributes ready jobs across available CPUs.
[[nodiscard]] std::size_t DefaultHttpWorkers();

/// \brief Explicit resource ceilings, not measured production scale guarantees.
struct HttpServerOptions {
  static constexpr std::size_t kMaxWorkers = 256; ///< Explicit native executor resource ceiling.
  static constexpr std::uint16_t kBackendPort = 18080;      ///< Development Caddy upstream port.
  static constexpr std::size_t kDefaultQueue = 32;          ///< Initial bounded admission profile.
  static constexpr unsigned kDefaultConnections = 256;      ///< Initial native connection ceiling.
  static constexpr unsigned kDefaultTimeoutSeconds = 15;    ///< Initial network inactivity timeout.
  static constexpr std::size_t kDefaultBodyBytes = 1048576; ///< One-MiB request safety default.
  static constexpr std::size_t kDefaultTotalBodyBytes = 33554432; ///< Thirty-two-MiB body quota.
  std::uint16_t port = kBackendPort; ///< Private native port; zero requests an ephemeral port.
  bool loopback = true;              ///< Bind loopback; false explicitly binds all IPv4 interfaces.
  std::size_t workers = DefaultHttpWorkers(); ///< Bounded executor, not per-user threads.
  std::size_t queue = kDefaultQueue; ///< Maximum pending executor jobs; overflow returns 503.
  unsigned connections = kDefaultConnections;          ///< Maximum simultaneous native connections.
  unsigned timeoutSeconds = kDefaultTimeoutSeconds;    ///< Network inactivity, not AL cancellation.
  std::size_t bodyBytes = kDefaultBodyBytes;           ///< Per-request body ceiling.
  std::size_t totalBodyBytes = kDefaultTotalBodyBytes; ///< Aggregate owned request-body ceiling.
  std::size_t responseBytes = kDefaultBodyBytes;       ///< Per-response byte ceiling.
};

/// \brief Native Linux listener with a bounded executor separate from network polling.
/// No thread/connection is allocated per ERP session. No permissions or database
/// access are granted here. Handler state must be synchronized and session-scoped.
/// Suspended requests are resumed/drained before native daemon shutdown.
/// Browser/WASM hosts need an in-process platform adapter, not this socket listener.
class HttpServer {
public:
  /// \brief Creates a listener; port zero asks the OS for an available port.
  /// \param handler Called only by bounded workers, never the network event loop.
  /// \param options Resource ceilings and native binding policy.
  /// \throws Error for invalid limits, missing handlers or failed listener setup.
  explicit HttpServer(std::function<ServerHttpResponse(const ServerHttpRequest &)> handler,
                      HttpServerOptions options = {});
  /// \brief Stops admission and drains worker jobs before releasing network state.
  ~HttpServer();
  HttpServer(const HttpServer &) = delete;
  HttpServer &operator=(const HttpServer &) = delete;
  /// \return The actual bound port, including OS-assigned ephemeral ports.
  [[nodiscard]] std::uint16_t Port() const;
  /// \return Aggregate currently owned request-body bytes, not total process memory.
  [[nodiscard]] std::size_t BufferedRequestBytes() const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}
