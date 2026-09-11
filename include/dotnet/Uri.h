#pragma once

#include "dotnet/Refused.h"
#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Text.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace agiru::dotnet {

/// \brief .NET `System.UriKind`, held by number the way .NET numbers it: `RelativeOrAbsolute` 0,
///        `Absolute` 1, `Relative` 2.
class UriKind {
public:
  /// \brief `RelativeOrAbsolute`, the .NET default.
  UriKind() = default;

  /// \brief A kind by number. \param number The number.
  explicit UriKind(std::int32_t number) : number_(number) {}

  [[nodiscard]] static UriKind RelativeOrAbsolute() { return UriKind{0}; } ///< `RelativeOrAbsolute`.
  [[nodiscard]] static UriKind Absolute() { return UriKind{1}; }           ///< `Absolute`.
  [[nodiscard]] static UriKind Relative() { return UriKind{2}; }           ///< `Relative`.

  /// \brief The number. \return It.
  [[nodiscard]] std::int32_t Number() const { return number_; }

  /// \brief `UriKind.Equals(other)`. \param other The other. \return Whether the same.
  [[nodiscard]] Boolean Equals(const UriKind &other) const { return number_ == other.number_; }

private:
  std::int32_t number_ = 0;
};

/// \brief .NET `System.UriPartial`: how much of a URI `Uri.GetLeftPart` keeps, numbered as .NET
///        numbers it: `Scheme` 0, `Authority` 1, `Path` 2, `Query` 3.
class UriPartial {
public:
  /// \brief `Scheme`, the .NET default.
  UriPartial() = default;

  /// \brief A part by number. \param number The number.
  explicit UriPartial(std::int32_t number) : number_(number) {}

  [[nodiscard]] static UriPartial Scheme() { return UriPartial{0}; }    ///< `Scheme`.
  [[nodiscard]] static UriPartial Authority() { return UriPartial{1}; } ///< `Authority`.
  [[nodiscard]] static UriPartial Path() { return UriPartial{2}; }      ///< `Path`.
  [[nodiscard]] static UriPartial Query() { return UriPartial{3}; }     ///< `Query`.

  /// \brief The number. \return It.
  [[nodiscard]] std::int32_t Number() const { return number_; }

private:
  std::int32_t number_ = 0;
};

/// \brief .NET `System.Uri`, rebuilt over RFC 3986: `scheme ":" ["//" authority] path ["?" query]
///        ["#" fragment]`, with the authority as `[userinfo "@"] host [":" port]`.
///
/// What the BaseApp asks of it is read access to the parts (`Web Request Helper` reads `Scheme`
/// and `Host`, the System Application's `Uri` codeunit every part), the two static checks
/// `IsWellFormedUriString` and `TryCreate`, and the two escapes -- and `Uri := Uri.Uri(Text)`,
/// the constructor as AL spells it, which refuses a text that is no URI with .NET's own wording.
///
/// \note `Segments` IS NOT REBUILT: it answers a .NET string array, and the AL that walks it
///       holds each element in a `DotNet String` this runtime does not carry either; it stays a
///       refusal by name (board:0035).
class Uri {
public:
  /// \brief The binder behind `Uri := Uri.Uri(...)`, the constructor as AL spells it.
  struct Binder {
    /// \brief `new Uri(uriString)`: an absolute URI.
    /// \param uriString The text.
    /// \return The URI.
    /// \throws Error when the text is not an absolute URI, with .NET's `UriFormatException` text.
    [[nodiscard]] class Uri operator()(std::string_view uriString) const;

    /// \brief `new Uri(uriString, uriKind)`.
    /// \param uriString The text. \param uriKind What the text may be.
    /// \return The URI.
    /// \throws Error when the text is not a URI of that kind.
    [[nodiscard]] class Uri operator()(std::string_view uriString, const UriKind &uriKind) const;

    /// \brief `new Uri(baseUri, relativeUri)`: the relative text resolved against the base.
    /// \param baseUri The base. \param relativeUri The relative text.
    /// \return The combined URI.
    [[nodiscard]] class Uri operator()(const class Uri &baseUri, std::string_view relativeUri) const;

    /// \brief `new Uri(x)` over a value this runtime does not carry -- a field of an absent
    ///        table, say. \param refused The value. \return Never.
    /// \throws Error naming the refused member, which is what the value does when used.
    [[nodiscard]] class Uri operator()(const Refused &refused) const;
  };

  /// \brief `Uri.Uri(...)`, the constructor as AL calls it.
  Binder Uri;

  /// \brief `Uri.IsWellFormedUriString(uriString, uriKind)`: whether the text is a URI of that
  ///        kind with nothing left to escape.
  /// \param uriString The text. \param uriKind What it must be.
  /// \return Whether it is.
  [[nodiscard]] static Boolean IsWellFormedUriString(std::string_view uriString,
                                                     const UriKind &uriKind);

  /// \brief `Uri.TryCreate(uriString, uriKind, out result)`.
  /// \param uriString The text. \param uriKind What it must be. \param result Where it lands.
  /// \return Whether the text was a URI of that kind.
  [[nodiscard]] static Boolean
  TryCreate(std::string_view uriString, const UriKind &uriKind, class Uri &result);

  /// \brief `Uri.EscapeDataString(text)`: every character outside RFC 3986's unreserved set as
  ///        percent-encoded UTF-8.
  /// \param stringToEscape The text. \return The escaped text.
  [[nodiscard]] static ::agiru::Text<0> EscapeDataString(std::string_view stringToEscape);

  /// \brief `Uri.UnescapeDataString(text)`: every `%XX` back to its byte.
  /// \param stringToUnescape The text. \return The unescaped text.
  [[nodiscard]] static ::agiru::Text<0> UnescapeDataString(std::string_view stringToUnescape);

  /// \brief `Uri.EscapeUriString(text)`: like `EscapeDataString`, but the reserved characters a
  ///        URI is made of (`:/?#[]@!$&'()*+,;=`) stay as they are.
  /// \param stringToEscape The text. \return The escaped text.
  [[nodiscard]] static ::agiru::Text<0> EscapeUriString(std::string_view stringToEscape);

  /// \brief `Uri.GetLeftPart(part)`: the URI up to and including that part -- `https://host:8443`
  ///        for `Authority`.
  /// \param part How much. \return The left part.
  [[nodiscard]] ::agiru::Text<0> GetLeftPart(const UriPartial &part) const;

  /// \brief `Uri.Scheme`, lower-cased. \return `https` of `https://x`.
  [[nodiscard]] ::agiru::Text<0> Scheme() const { return scheme_; }

  /// \brief `Uri.Host`, lower-cased. \return The host, without the port.
  [[nodiscard]] ::agiru::Text<0> Host() const { return host_; }

  /// \brief `Uri.Port`: the port given, the scheme's default when none was, -1 when unknown.
  /// \return The port.
  [[nodiscard]] Integer Port() const;

  /// \brief `Uri.IsDefaultPort`. \return Whether the port is the scheme's own.
  [[nodiscard]] Boolean IsDefaultPort() const { return port_ < 0; }

  /// \brief `Uri.AbsolutePath`: the path, `/` when the URI has none. \return The path.
  [[nodiscard]] ::agiru::Text<0> AbsolutePath() const;

  /// \brief `Uri.Query`: the query with its leading `?`, or empty. \return The query.
  [[nodiscard]] ::agiru::Text<0> Query() const { return query_.empty() ? std::string{} : "?" + query_; }

  /// \brief `Uri.Fragment`: the fragment with its leading `#`, or empty. \return The fragment.
  [[nodiscard]] ::agiru::Text<0> Fragment() const {
    return fragment_.empty() ? std::string{} : "#" + fragment_;
  }

  /// \brief `Uri.Authority`: the host, and the port when it is not the scheme's own.
  /// \return The authority.
  [[nodiscard]] ::agiru::Text<0> Authority() const;

  /// \brief `Uri.PathAndQuery`. \return The path followed by the query.
  [[nodiscard]] ::agiru::Text<0> PathAndQuery() const {
    return std::string(std::string_view(AbsolutePath())) + std::string(std::string_view(Query()));
  }

  /// \brief `Uri.AbsoluteUri`: the canonical text -- scheme and host lower-cased, a bare path
  ///        made `/`. \return The text.
  [[nodiscard]] ::agiru::Text<0> AbsoluteUri() const;

  /// \brief `Uri.OriginalString`: the text the URI was made from. \return The text.
  [[nodiscard]] ::agiru::Text<0> OriginalString() const { return original_; }

  /// \brief `Uri.IsAbsoluteUri`. \return Whether a scheme was given.
  [[nodiscard]] Boolean IsAbsoluteUri() const { return !scheme_.empty(); }

  /// \brief `Uri.IsBaseOf(uri)`: whether this URI's scheme, authority and directory contain the
  ///        other's. \param uri The other. \return Whether it does.
  [[nodiscard]] Boolean IsBaseOf(const class Uri &uri) const;

  /// \brief `Uri.ToString()`. \return The canonical text, `AbsoluteUri` for an absolute URI.
  [[nodiscard]] ::agiru::Text<0> ToString() const;

  /// \brief `Uri.Equals(other)`. \param other The other. \return Whether the same URI.
  [[nodiscard]] Boolean Equals(const class Uri &other) const {
    return std::string_view(AbsoluteUri()) == std::string_view(other.AbsoluteUri());
  }

  /// \brief `Uri.Segments`, a .NET string array; not rebuilt (board:0035).
  Refused Segments{{.type = "Uri", .member = "Segments"}};

  /// \brief `Uri.LocalPath`, a Windows file path; not rebuilt (board:0035).
  Refused LocalPath{{.type = "Uri", .member = "LocalPath"}};

  /// \brief Fills this URI from a text, which is what the binder and `TryCreate` share.
  /// \param uriString The text. \param uriKind What it may be.
  /// \return Whether the text was a URI of that kind; on false the URI is left empty.
  bool Parse_(std::string_view uriString, const UriKind &uriKind);

private:
  std::string original_;
  std::string scheme_;
  std::string userInfo_;
  std::string host_;
  std::int32_t port_ = -1;
  std::string path_;
  std::string query_;
  std::string fragment_;
};

/// \brief .NET `System.UriBuilder`: the parts of a URI as read-and-write properties, and `Uri`
///        assembling them.
class UriBuilder {
public:
  /// \brief The binder behind `UriBuilder := UriBuilder.UriBuilder(...)`.
  struct Binder {
    /// \brief `new UriBuilder()`: `http://localhost/`. \return The builder.
    [[nodiscard]] class UriBuilder operator()() const;

    /// \brief `new UriBuilder(uri)`. \param uri The text, parsed into the parts.
    /// \return The builder.
    [[nodiscard]] class UriBuilder operator()(std::string_view uri) const;

    /// \brief `new UriBuilder(uri)` over a built URI. \param uri The URI. \return The builder.
    [[nodiscard]] class UriBuilder operator()(const class Uri &uri) const;

    /// \brief `new UriBuilder(x)` over a value this runtime does not carry. \param refused The
    ///        value. \return Never. \throws Error naming the refused member.
    [[nodiscard]] class UriBuilder operator()(const Refused &refused) const;
  };

  /// \brief `UriBuilder.UriBuilder(...)`, the constructor as AL calls it.
  Binder UriBuilder;

  /// \brief `UriBuilder.Scheme` read. \return The scheme.
  [[nodiscard]] ::agiru::Text<0> Scheme() const { return scheme_; }
  /// \brief `UriBuilder.Scheme` write. \param value The scheme. \return It.
  ::agiru::Text<0> Scheme(std::string_view value);

  /// \brief `UriBuilder.Host` read. \return The host.
  [[nodiscard]] ::agiru::Text<0> Host() const { return host_; }
  /// \brief `UriBuilder.Host` write. \param value The host. \return It.
  ::agiru::Text<0> Host(std::string_view value) {
    host_ = std::string(value);
    return host_;
  }

  /// \brief `UriBuilder.Port` read. \return The port, -1 for the scheme's own.
  [[nodiscard]] Integer Port() const { return port_; }
  /// \brief `UriBuilder.Port` write. \param value The port. \return It.
  Integer Port(Integer value) {
    port_ = value;
    return port_;
  }

  /// \brief `UriBuilder.Path` read. \return The path, `/` at least.
  [[nodiscard]] ::agiru::Text<0> Path() const { return path_.empty() ? std::string("/") : path_; }
  /// \brief `UriBuilder.Path` write. \param value The path. \return It.
  ::agiru::Text<0> Path(std::string_view value);

  /// \brief `UriBuilder.Query` read: with its leading `?`, or empty. \return The query.
  [[nodiscard]] ::agiru::Text<0> Query() const { return query_.empty() ? std::string{} : "?" + query_; }
  /// \brief `UriBuilder.Query` write; a leading `?` is taken off, as .NET does since 4.5.
  /// \param value The query. \return It, with its `?`.
  ::agiru::Text<0> Query(std::string_view value);

  /// \brief `UriBuilder.Fragment` read: with its leading `#`, or empty. \return The fragment.
  [[nodiscard]] ::agiru::Text<0> Fragment() const {
    return fragment_.empty() ? std::string{} : "#" + fragment_;
  }
  /// \brief `UriBuilder.Fragment` write; a leading `#` is taken off. \param value The fragment.
  /// \return It, with its `#`.
  ::agiru::Text<0> Fragment(std::string_view value);

  /// \brief `UriBuilder.Uri`: the parts assembled. \return The URI.
  [[nodiscard]] class Uri Uri() const;

  /// \brief `UriBuilder.ToString()`. \return The assembled text.
  [[nodiscard]] ::agiru::Text<0> ToString() const;

private:
  std::string scheme_ = "http";
  std::string host_ = "localhost";
  std::int32_t port_ = -1;
  std::string path_ = "/";
  std::string query_;
  std::string fragment_;
};

}
