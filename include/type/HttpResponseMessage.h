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
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `HttpResponseMessage` -- the surface the platform documentation declares.

namespace agiru {

class Cookie;
class HttpContent;
class HttpHeaders;

/// \brief AL `HttpResponseMessage`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/httpresponsemessage/` states, so a call site compiles and is CHECKED; the
///          body refuses by name rather than returning a plausible wrong answer (board:0035).
class HttpResponseMessage {
public:
  /// \brief AL `HttpResponseMessage.Content()`. Gets the contents of the HTTP response.
  /// \return The AL `HttpContent`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::HttpContent Content();

  /// \brief AL `HttpResponseMessage.GetCookie(Text, Cookie)`. Gets the specified cookie given a
  /// name.
  /// \param Name The AL `Text`.
  /// \param Cookie The AL `Cookie`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetCookie(std::string_view Name, ::agiru::Cookie &Cookie);

  /// \brief AL `HttpResponseMessage.GetCookieNames()`. Gets the list of cookie names.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void GetCookieNames();

  /// \brief AL `HttpResponseMessage.Headers()`. Gets the HTTP response's HTTP headers.
  /// \return The AL `HttpHeaders`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::HttpHeaders Headers();

  /// \brief AL `HttpResponseMessage.HttpStatusCode()`. Gets the status code of the HTTP response.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer HttpStatusCode();

  /// \brief AL `HttpResponseMessage.IsBlockedByEnvironment()`. Gets a value that indicates if the
  /// HTTP response is the result of the environment blocking an outgoing HTTP request.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsBlockedByEnvironment();

  /// \brief AL `HttpResponseMessage.IsSuccessStatusCode()`. Gets a value that indicates if the HTTP
  /// response was successful.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsSuccessStatusCode();

  /// \brief AL `HttpResponseMessage.ReasonPhrase()`. Gets the reason phrase which typically is sent
  /// by servers together with the status code.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string ReasonPhrase();
};

} // namespace agiru
