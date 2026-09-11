#include "dotnet/Uri.h"
#include "runtime/Error.h"
#include "type/Integer.h"
#include "type/Text.h"

#include "Check.h"

#include <string>

using agiru::dotnet::Uri;
using agiru::dotnet::UriBuilder;
using agiru::dotnet::UriKind;

namespace {

std::string T(const agiru::Text<0> &text) {
  return std::string(std::string_view(text));
}

/// `System.Uri` IS REBUILT OVER RFC 3986 (board:0707): the parts a URI is made of, the two static
/// checks `Web Request Helper` and the System Application's `Uri` module ask, and the escapes.
/// `IsWellFormedUriString` is what `IsValidUri` hangs on, and it is stricter than a parse.
void AUriIsTakenApartAndPutBackTheWayDotNetDoesIt() {
  Uri uri;
  uri = uri.Uri("HTTPS://User@Example.COM:8443/a/b%20c?x=1&y=2#top");
  CHECK_TEXT("the scheme, lower-cased", T(uri.Scheme()), "https");
  CHECK_TEXT("the host, lower-cased, without the port", T(uri.Host()), "example.com");
  CHECK_TRUE("the port given", uri.Port() == 8443);
  CHECK_TRUE("which is not the scheme's own", !uri.IsDefaultPort());
  CHECK_TEXT("the path as written", T(uri.AbsolutePath()), "/a/b%20c");
  CHECK_TEXT("the query with its question mark", T(uri.Query()), "?x=1&y=2");
  CHECK_TEXT("the fragment with its hash", T(uri.Fragment()), "#top");
  CHECK_TEXT("the authority carries a non-default port", T(uri.Authority()), "example.com:8443");
  CHECK_TEXT("the canonical text", T(uri.AbsoluteUri()),
             "https://User@example.com:8443/a/b%20c?x=1&y=2#top");

  Uri bare;
  bare = bare.Uri("http://Host");
  CHECK_TRUE("http's own port", bare.Port() == 80 && bare.IsDefaultPort());
  CHECK_TEXT("a bare host gets the root path", T(bare.AbsolutePath()), "/");
  CHECK_TEXT("and no port in the authority", T(bare.Authority()), "host");
  CHECK_TEXT("and a slash in the canonical text", T(bare.AbsoluteUri()), "http://host/");

  CHECK_TRUE("well formed: an absolute http URI",
             Uri::IsWellFormedUriString("https://example.com/path?q=1", UriKind::Absolute()));
  CHECK_TRUE("not well formed: no scheme where an absolute one is wanted",
             !Uri::IsWellFormedUriString("example.com/path", UriKind::Absolute()));
  CHECK_TRUE("not well formed: a space", !Uri::IsWellFormedUriString("http://a b", UriKind::Absolute()));
  CHECK_TRUE("not well formed: http with no host",
             !Uri::IsWellFormedUriString("http://", UriKind::Absolute()));
  CHECK_TRUE("well formed: a relative path when relative is allowed",
             Uri::IsWellFormedUriString("path/to?x=1", UriKind::RelativeOrAbsolute()));
  CHECK_TRUE("not well formed: an absolute one where relative is wanted",
             !Uri::IsWellFormedUriString("http://x/", UriKind::Relative()));

  Uri made;
  CHECK_TRUE("TryCreate takes a good one", Uri::TryCreate("ftp://files.example.org/x", UriKind::Absolute(), made));
  CHECK_TEXT("and fills it", T(made.Host()), "files.example.org");
  CHECK_TRUE("TryCreate refuses a bad one", !Uri::TryCreate("not a uri", UriKind::Absolute(), made));

  std::string said;
  try {
    Uri bad;
    bad = bad.Uri("no scheme here");
  } catch (const agiru::Error &e) { said = e.what(); }
  CHECK_TEXT("the constructor refuses with .NET's wording", said,
             "Invalid URI: The format of the URI could not be determined.");

  CHECK_TEXT("EscapeDataString keeps the unreserved set and encodes the rest as UTF-8",
             T(Uri::EscapeDataString("a b&c=d/é~")), "a%20b%26c%3Dd%2F%C3%A9~");
  CHECK_TEXT("UnescapeDataString is the way back", T(Uri::UnescapeDataString("a%20b%26c%3Dd%2F%C3%A9~")),
             "a b&c=d/é~");

  Uri base;
  base = base.Uri("http://example.com/dir/page.html?old=1");
  CHECK_TEXT("a relative reference resolves against the base's directory",
             T(base.Uri(base, "other.html?new=2").AbsoluteUri()), "http://example.com/dir/other.html?new=2");
  CHECK_TEXT("a rooted reference resolves against the authority",
             T(base.Uri(base, "/top").AbsoluteUri()), "http://example.com/top");
  Uri deeper;
  deeper = deeper.Uri("http://example.com/dir/sub/x");
  CHECK_TRUE("IsBaseOf sees the directory", base.IsBaseOf(deeper));
  Uri elsewhere;
  elsewhere = elsewhere.Uri("http://example.com/other/x");
  CHECK_TRUE("and not another one", !base.IsBaseOf(elsewhere));
}

/// `System.UriBuilder` holds the parts as properties and assembles them; a `?` or `#` handed to
/// the query or fragment is not doubled, and a missing path is `/`.
void AUriBuilderAssemblesItsParts() {
  UriBuilder builder;
  builder = builder.UriBuilder("https://example.com/api?x=1");
  CHECK_TEXT("the path from the text", T(builder.Path()), "/api");
  CHECK_TEXT("the query from the text", T(builder.Query()), "?x=1");
  static_cast<void>(builder.Query("?y=2&z=3"));
  static_cast<void>(builder.Fragment("frag"));
  static_cast<void>(builder.Port(8080));
  CHECK_TEXT("the assembled text", T(builder.ToString()), "https://example.com:8080/api?y=2&z=3#frag");
  CHECK_TEXT("and as a Uri", T(builder.Uri().AbsoluteUri()), "https://example.com:8080/api?y=2&z=3#frag");
  UriBuilder empty;
  empty = empty.UriBuilder();
  CHECK_TEXT("the empty builder is localhost over http", T(empty.ToString()), "http://localhost/");
  static_cast<void>(empty.Host("Example.org"));
  static_cast<void>(empty.Path("x/y"));
  CHECK_TEXT("a path without its slash gets one", T(empty.ToString()), "http://Example.org/x/y");
}

} // namespace

int main() {
  return gate::Run("Uri", [] {
    AUriIsTakenApartAndPutBackTheWayDotNetDoesIt();
    AUriBuilderAssemblesItsParts();
  });
}
