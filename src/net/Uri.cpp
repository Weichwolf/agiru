#include "dotnet/Uri.h"

#include "runtime/Error.h"

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace agiru::dotnet {

namespace {

constexpr std::string_view kInvalidUri = "Invalid URI: The format of the URI could not be determined.";
constexpr std::int32_t kHttpPort = 80;
constexpr std::int32_t kHttpsPort = 443;
constexpr std::int32_t kFtpPort = 21;
constexpr std::int32_t kMaxPort = 65535;
constexpr int kDecimal = 10;
constexpr int kHexadecimal = 16;
constexpr unsigned kHighNibble = 4U;
constexpr unsigned kLowNibbleMask = 0x0FU;
constexpr unsigned char kAscii = 0x80;

std::string Lowered(std::string_view text) {
  std::string out(text);
  for (char &c : out) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
  return out;
}

bool SchemeChar(char c, bool first) {
  const auto u = static_cast<unsigned char>(c);
  if (std::isalpha(u) != 0) { return true; }
  return !first && (std::isdigit(u) != 0 || c == '+' || c == '-' || c == '.');
}

bool Unreserved(unsigned char c) {
  return std::isalnum(c) != 0 || c == '-' || c == '.' || c == '_' || c == '~';
}

bool UriChar(unsigned char c) {
  static constexpr std::string_view kReserved = ":/?#[]@!$&'()*+,;=%";
  return Unreserved(c) || kReserved.find(static_cast<char>(c)) != std::string_view::npos;
}

bool HexDigit(char c) {
  return std::isxdigit(static_cast<unsigned char>(c)) != 0;
}

int HexValue(char c) {
  const auto u = static_cast<unsigned char>(c);
  if (std::isdigit(u) != 0) { return c - '0'; }
  return static_cast<int>(std::tolower(u)) - 'a' + kDecimal;
}

std::int32_t DefaultPort(std::string_view scheme) {
  if (scheme == "http" || scheme == "ws") { return kHttpPort; }
  if (scheme == "https" || scheme == "wss") { return kHttpsPort; }
  if (scheme == "ftp") { return kFtpPort; }
  return -1;
}

bool WellFormedText(std::string_view text) {
  if (text.empty()) { return false; }
  for (std::size_t i = 0; i < text.size(); ++i) {
    const auto c = static_cast<unsigned char>(text[i]);
    if (c >= kAscii || !UriChar(c)) { return false; }
    if (c == '%' && (i + 2 >= text.size() || !HexDigit(text[i + 1]) || !HexDigit(text[i + 2]))) {
      return false;
    }
  }
  return true;
}

std::size_t SchemeLength(std::string_view text) {
  std::size_t i = 0;
  while (i < text.size() && SchemeChar(text[i], i == 0)) { ++i; }
  return i > 0 && i < text.size() && text[i] == ':' ? i : 0;
}

}

bool Uri::Parse_(std::string_view uriString, const UriKind &uriKind) {
  const class Uri empty;
  *this = empty;
  const std::size_t schemeLength = SchemeLength(uriString);
  const bool absolute = schemeLength > 0;
  if (uriKind.Number() == UriKind::Absolute().Number() && !absolute) { return false; }
  if (uriKind.Number() == UriKind::Relative().Number() && absolute) { return false; }
  if (!absolute) {
    if (uriString.empty()) { return false; }
    original_ = std::string(uriString);
    std::string_view rest = uriString;
    if (const std::size_t hash = rest.find('#'); hash != std::string_view::npos) {
      fragment_ = std::string(rest.substr(hash + 1));
      rest = rest.substr(0, hash);
    }
    if (const std::size_t question = rest.find('?'); question != std::string_view::npos) {
      query_ = std::string(rest.substr(question + 1));
      rest = rest.substr(0, question);
    }
    path_ = std::string(rest);
    return true;
  }
  original_ = std::string(uriString);
  scheme_ = Lowered(uriString.substr(0, schemeLength));
  std::string_view rest = uriString.substr(schemeLength + 1);
  if (const std::size_t hash = rest.find('#'); hash != std::string_view::npos) {
    fragment_ = std::string(rest.substr(hash + 1));
    rest = rest.substr(0, hash);
  }
  if (const std::size_t question = rest.find('?'); question != std::string_view::npos) {
    query_ = std::string(rest.substr(question + 1));
    rest = rest.substr(0, question);
  }
  if (rest.starts_with("//")) {
    rest.remove_prefix(2);
    const std::size_t slash = rest.find('/');
    std::string_view authority = slash == std::string_view::npos ? rest : rest.substr(0, slash);
    rest = slash == std::string_view::npos ? std::string_view{} : rest.substr(slash);
    if (const std::size_t at = authority.rfind('@'); at != std::string_view::npos) {
      userInfo_ = std::string(authority.substr(0, at));
      authority = authority.substr(at + 1);
    }
    std::size_t colon = authority.rfind(':');
    if (authority.starts_with('[')) {
      const std::size_t close = authority.find(']');
      colon = close == std::string_view::npos || close + 1 >= authority.size() ||
                      authority[close + 1] != ':'
                  ? std::string_view::npos
                  : close + 1;
    }
    if (colon != std::string_view::npos) {
      const std::string_view digits = authority.substr(colon + 1);
      std::int32_t port = 0;
      for (const char c : digits) {
        if (std::isdigit(static_cast<unsigned char>(c)) == 0) { return false; }
        port = port * kDecimal + (c - '0');
        if (port > kMaxPort) { return false; }
      }
      if (!digits.empty()) { port_ = port == DefaultPort(scheme_) ? -1 : port; }
      authority = authority.substr(0, colon);
    }
    host_ = Lowered(authority);
    const bool hostNeeded = scheme_ == "http" || scheme_ == "https" || scheme_ == "ftp" ||
                            scheme_ == "ws" || scheme_ == "wss";
    if (hostNeeded && host_.empty()) { return false; }
    for (const char c : host_) {
      const auto u = static_cast<unsigned char>(c);
      if (u >= kAscii || std::isspace(u) != 0 || c == '/' || c == '?' || c == '#') { return false; }
    }
  }
  path_ = std::string(rest);
  return true;
}

class Uri Uri::Binder::operator()(std::string_view uriString) const {
  return (*this)(uriString, UriKind::Absolute());
}

class Uri Uri::Binder::operator()(std::string_view uriString, const UriKind &uriKind) const {
  class Uri made;
  if (!made.Parse_(uriString, uriKind)) { throw ::agiru::Error(std::string(kInvalidUri)); }
  return made;
}

class Uri Uri::Binder::operator()(const class Uri &baseUri, std::string_view relativeUri) const {
  const std::size_t schemeLength = SchemeLength(relativeUri);
  if (schemeLength > 0) { return (*this)(relativeUri, UriKind::Absolute()); }
  std::string base = std::string(std::string_view(baseUri.AbsoluteUri()));
  if (relativeUri.starts_with('/')) {
    const std::string authority = std::string(std::string_view(baseUri.Scheme())) + "://" +
                                  std::string(std::string_view(baseUri.Authority()));
    return (*this)(authority + std::string(relativeUri), UriKind::Absolute());
  }
  if (const std::size_t hash = base.find('#'); hash != std::string::npos) { base.resize(hash); }
  if (const std::size_t question = base.find('?'); question != std::string::npos) {
    base.resize(question);
  }
  if (relativeUri.starts_with('?') || relativeUri.starts_with('#')) {
    return (*this)(base + std::string(relativeUri), UriKind::Absolute());
  }
  const std::size_t lastSlash = base.rfind('/');
  const std::size_t authorityEnd = base.find("://");
  if (lastSlash != std::string::npos && (authorityEnd == std::string::npos || lastSlash > authorityEnd + 2)) {
    base.resize(lastSlash + 1);
  } else {
    base += '/';
  }
  return (*this)(base + std::string(relativeUri), UriKind::Absolute());
}

class Uri Uri::Binder::operator()(const Refused &refused) const {
  static_cast<void>(refused());
  throw ::agiru::Error(std::string(kInvalidUri));
}

Boolean Uri::IsWellFormedUriString(std::string_view uriString, const UriKind &uriKind) {
  if (!WellFormedText(uriString)) { return false; }
  class Uri made;
  return made.Parse_(uriString, uriKind);
}

Boolean Uri::TryCreate(std::string_view uriString, const UriKind &uriKind, class Uri &result) {
  class Uri made;
  if (!made.Parse_(uriString, uriKind)) { return false; }
  result = made;
  return true;
}

::agiru::Text<0> Uri::EscapeDataString(std::string_view stringToEscape) {
  static constexpr std::string_view kHex = "0123456789ABCDEF";
  std::string out;
  for (const char c : stringToEscape) {
    const auto u = static_cast<unsigned char>(c);
    if (Unreserved(u) && u < kAscii) {
      out += c;
      continue;
    }
    out += '%';
    out += kHex[u >> kHighNibble];
    out += kHex[u & kLowNibbleMask];
  }
  return out;
}

::agiru::Text<0> Uri::UnescapeDataString(std::string_view stringToUnescape) {
  std::string out;
  for (std::size_t i = 0; i < stringToUnescape.size(); ++i) {
    const char c = stringToUnescape[i];
    if (c == '%' && i + 2 < stringToUnescape.size() && HexDigit(stringToUnescape[i + 1]) &&
        HexDigit(stringToUnescape[i + 2])) {
      out += static_cast<char>(HexValue(stringToUnescape[i + 1]) * kHexadecimal +
                               HexValue(stringToUnescape[i + 2]));
      i += 2;
      continue;
    }
    out += c;
  }
  return out;
}

::agiru::Text<0> Uri::EscapeUriString(std::string_view stringToEscape) {
  static constexpr std::string_view kHex = "0123456789ABCDEF";
  std::string out;
  for (const char c : stringToEscape) {
    const auto u = static_cast<unsigned char>(c);
    if (u < kAscii && UriChar(u) && c != '%') {
      out += c;
      continue;
    }
    out += '%';
    out += kHex[u >> kHighNibble];
    out += kHex[u & kLowNibbleMask];
  }
  return out;
}

::agiru::Text<0> Uri::GetLeftPart(const UriPartial &part) const {
  if (scheme_.empty()) { return std::string{}; }
  std::string out = scheme_ + ":";
  if (part.Number() == UriPartial::Scheme().Number()) { return out; }
  if (!host_.empty() || original_.find("//") == scheme_.size() + 1) {
    out += "//";
    if (!userInfo_.empty()) { out += userInfo_ + "@"; }
    out += std::string(std::string_view(Authority()));
  }
  if (part.Number() == UriPartial::Authority().Number()) { return out; }
  out += std::string(std::string_view(AbsolutePath()));
  if (part.Number() == UriPartial::Path().Number()) { return out; }
  out += std::string(std::string_view(Query()));
  return out;
}

Integer Uri::Port() const {
  return port_ >= 0 ? port_ : DefaultPort(scheme_);
}

::agiru::Text<0> Uri::AbsolutePath() const {
  if (path_.empty() && !host_.empty()) { return std::string("/"); }
  return path_;
}

::agiru::Text<0> Uri::Authority() const {
  if (port_ < 0) { return host_; }
  return host_ + ":" + std::to_string(port_);
}

::agiru::Text<0> Uri::AbsoluteUri() const {
  if (scheme_.empty()) { return original_; }
  std::string out = scheme_ + ":";
  if (!host_.empty() || original_.find("//") == scheme_.size() + 1) {
    out += "//";
    if (!userInfo_.empty()) { out += userInfo_ + "@"; }
    out += std::string(std::string_view(Authority()));
  }
  out += std::string(std::string_view(AbsolutePath()));
  out += std::string(std::string_view(Query()));
  out += std::string(std::string_view(Fragment()));
  return out;
}

Boolean Uri::IsBaseOf(const class Uri &uri) const {
  if (scheme_ != uri.scheme_ || host_ != uri.host_ || Port() != uri.Port()) { return false; }
  const std::string mine = std::string(std::string_view(AbsolutePath()));
  const std::string other = std::string(std::string_view(uri.AbsolutePath()));
  const std::size_t lastSlash = mine.rfind('/');
  const std::string directory = lastSlash == std::string::npos ? std::string("/") : mine.substr(0, lastSlash + 1);
  return other.starts_with(directory);
}

::agiru::Text<0> Uri::ToString() const {
  return scheme_.empty() ? ::agiru::Text<0>{original_}
                         : ::agiru::Text<0>{std::string(std::string_view(
                               UnescapeDataString(std::string_view(AbsoluteUri()))))};
}

class UriBuilder UriBuilder::Binder::operator()() const {
  const class UriBuilder made;
  return made;
}

class UriBuilder UriBuilder::Binder::operator()(std::string_view uri) const {
  class Uri parsed;
  if (!parsed.Parse_(uri, UriKind::Absolute())) {
    if (!parsed.Parse_("http://" + std::string(uri), UriKind::Absolute())) {
      throw ::agiru::Error(std::string(kInvalidUri));
    }
  }
  return (*this)(parsed);
}

class UriBuilder UriBuilder::Binder::operator()(const Refused &refused) const {
  static_cast<void>(refused());
  throw ::agiru::Error(std::string(kInvalidUri));
}

class UriBuilder UriBuilder::Binder::operator()(const class Uri &uri) const {
  class UriBuilder made;
  static_cast<void>(made.Scheme(std::string_view(uri.Scheme())));
  static_cast<void>(made.Host(std::string_view(uri.Host())));
  static_cast<void>(made.Port(uri.IsDefaultPort() ? Integer{-1} : uri.Port()));
  static_cast<void>(made.Path(std::string_view(uri.AbsolutePath())));
  static_cast<void>(made.Query(std::string_view(uri.Query())));
  static_cast<void>(made.Fragment(std::string_view(uri.Fragment())));
  return made;
}

::agiru::Text<0> UriBuilder::Scheme(std::string_view value) {
  scheme_ = Lowered(value);
  if (const std::size_t colon = scheme_.find(':'); colon != std::string::npos) {
    scheme_.resize(colon);
  }
  return scheme_;
}

::agiru::Text<0> UriBuilder::Path(std::string_view value) {
  path_ = value.empty() ? std::string("/") : std::string(value);
  if (!path_.starts_with('/')) { path_.insert(0, "/"); }
  return path_;
}

::agiru::Text<0> UriBuilder::Query(std::string_view value) {
  query_ = std::string(value.starts_with('?') ? value.substr(1) : value);
  return Query();
}

::agiru::Text<0> UriBuilder::Fragment(std::string_view value) {
  fragment_ = std::string(value.starts_with('#') ? value.substr(1) : value);
  return Fragment();
}

::agiru::Text<0> UriBuilder::ToString() const {
  std::string out = scheme_ + "://" + host_;
  if (port_ >= 0 && port_ != DefaultPort(scheme_)) { out += ":" + std::to_string(port_); }
  out += path_.empty() ? std::string("/") : path_;
  out += std::string(std::string_view(Query()));
  out += std::string(std::string_view(Fragment()));
  return out;
}

class Uri UriBuilder::Uri() const {
  class Uri made;
  if (!made.Parse_(std::string_view(ToString()), UriKind::Absolute())) {
    throw ::agiru::Error(std::string(kInvalidUri));
  }
  return made;
}

}
