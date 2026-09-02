#pragma once

#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/SecretText.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `HttpRequestMessage` -- the surface the platform documentation declares.

namespace agiru {

class Cookie;
class HttpContent;
class HttpHeaders;

/// \brief AL `HttpRequestMessage`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/httprequestmessage/` states, so a call site compiles and is CHECKED; the
///          body refuses by name rather than returning a plausible wrong answer (board:0035).
class HttpRequestMessage {
public:
  /// \brief AL `HttpRequestMessage.Content(HttpContent)`. Gets or sets the contents of the HTTP
  /// message.
  /// \param SetContent The AL `HttpContent`.
  /// \return The AL `HttpContent`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::HttpContent Content(const ::agiru::HttpContent &SetContent);

  /// \brief AL `HttpRequestMessage.GetCookie(Text, Cookie)`. Gets the specified cookie given a
  /// name.
  /// \param Name The AL `Text`.
  /// \param Cookie The AL `Cookie`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetCookie(std::string_view Name, ::agiru::Cookie &Cookie);

  /// \brief AL `HttpRequestMessage.GetCookieNames()`. Gets the list of cookie names.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void GetCookieNames();

  /// \brief AL `HttpRequestMessage.GetHeaders(HttpHeaders)`. Gets a reference to the collection of
  /// HTTP request headers.
  /// \param Headers The AL `HttpHeaders`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetHeaders(::agiru::HttpHeaders &Headers);

  /// \brief AL `HttpRequestMessage.GetRequestUri()`. Gets the URI used for the HTTP request.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string GetRequestUri();

  /// \brief AL `HttpRequestMessage.GetSecretRequestUri()`. Gets the secret URI used for the HTTP
  /// request.
  /// \return The AL `SecretText`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::SecretText GetSecretRequestUri();

  /// \brief AL `HttpRequestMessage.Method(Text)`. Gets or sets the method type as defined in the
  /// HTTP standard.
  /// \param NewMethod The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Method(std::string_view NewMethod);

  /// \brief AL `HttpRequestMessage.RemoveCookie(Text)`. Removes the specified cookie given a name.
  /// \param Name The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean RemoveCookie(std::string_view Name);

  /// \brief AL `HttpRequestMessage.SetCookie(Cookie)`. Sets the cookie given a cookie object.
  /// \param Cookie The AL `Cookie`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SetCookie(const ::agiru::Cookie &Cookie);

  /// \brief AL `HttpRequestMessage.SetCookie(Text, Text)`. Sets the cookie given a name and value.
  /// \param Name The AL `Text`.
  /// \param Value The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SetCookie(std::string_view Name, std::string_view Value);

  /// \brief AL `HttpRequestMessage.SetRequestUri(Text)`. Sets the URI used for the HTTP request.
  /// \param RequestUri The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SetRequestUri(std::string_view RequestUri);

  /// \brief AL `HttpRequestMessage.SetSecretRequestUri(SecretText)`. Sets the secret URI used for
  /// the HTTP request.
  /// \param RequestUri The AL `SecretText`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SetSecretRequestUri(const ::agiru::SecretText &RequestUri);
};

} // namespace agiru
