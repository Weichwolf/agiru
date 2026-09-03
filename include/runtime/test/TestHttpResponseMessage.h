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
/// \brief AL `TestHttpResponseMessage` -- the surface the platform documentation declares.

namespace agiru {

class HttpContent;
class HttpHeaders;

/// \brief AL `TestHttpResponseMessage`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/testhttpresponsemessage/` states, so a call site compiles and is CHECKED;
///          the body refuses by name rather than returning a plausible wrong answer (board:0035).
class TestHttpResponseMessage {
public:
  /// \brief AL `TestHttpResponseMessage.Content()`. Gets the contents of the HTTP response.
  /// \return The AL `HttpContent`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::HttpContent Content();

  /// \brief AL `TestHttpResponseMessage.Headers()`. Gets the HTTP response's HTTP headers.
  /// \return The AL `HttpHeaders`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::HttpHeaders Headers();

  /// \brief AL `TestHttpResponseMessage.HttpStatusCode(Integer)`. Gets or sets the status code of
  /// the HTTP response.
  /// \param SetStatusCode The AL `Integer`.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer HttpStatusCode(::agiru::Integer SetStatusCode);

  /// \brief AL `TestHttpResponseMessage.IsBlockedByEnvironment(Boolean)`. Gets or sets a value that
  /// indicates if the HTTP response is the result of the environment blocking an outgoing HTTP
  /// request.
  /// \param SetIsBlockedByEnvironment The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsBlockedByEnvironment(::agiru::Boolean SetIsBlockedByEnvironment);

  /// \brief AL `TestHttpResponseMessage.IsSuccessfulRequest(Boolean)`. Gets or sets a value that
  /// indicates if the HTTP request was successful or not. By setting this value it is possible to
  /// mock the return value of the HttpClient call.
  /// \param SetIsSuccessfulRequest The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsSuccessfulRequest(::agiru::Boolean SetIsSuccessfulRequest);

  /// \brief AL `TestHttpResponseMessage.ReasonPhrase(Text)`. Gets or sets the reason phrase which
  /// typically is sent by servers together with the status code.
  /// \param SetReasonPhrase The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string ReasonPhrase(std::string_view SetReasonPhrase);
};

}
