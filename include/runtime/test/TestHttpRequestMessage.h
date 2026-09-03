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
#include "type/HttpRequestType.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `TestHttpRequestMessage` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `TestHttpRequestMessage`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/testhttprequestmessage/` states, so a call site compiles and is CHECKED;
///          the body refuses by name rather than returning a plausible wrong answer (board:0035).
class TestHttpRequestMessage {
public:
  /// \brief AL `TestHttpRequestMessage.HasSecretUri()`. **true** if the request has a secret URI
  /// set, otherwise **false**.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean HasSecretUri();

  /// \brief AL `TestHttpRequestMessage.Path()`. Gets the path of the HTTP request unless a secret
  /// URI was set, in which case it's an empty string.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Path();

  /// \brief AL `TestHttpRequestMessage.QueryParameters()`. Gets the query parameters of the HTTP
  /// request if the request does not have a secret URI, otherwise an empty Dictionary.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void QueryParameters();

  /// \brief AL `TestHttpRequestMessage.RequestType()`. Gets the HTTP method type.
  /// \return The AL `HttpRequestType`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::HttpRequestType RequestType();
};

}
