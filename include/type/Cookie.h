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
/// \brief AL `Cookie` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `Cookie`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/cookie/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class Cookie {
public:
  /// \brief AL `Cookie.Domain()`. The domain of the cookie. It defines to which host the cookie can
  /// be sent to.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Domain();

  /// \brief AL `Cookie.Expires()`. The expiration date of the cookie.
  /// \return The AL `DateTime`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::DateTime Expires();

  /// \brief AL `Cookie.HttpOnly()`. True if the cookie is HttpOnly, false otherwise.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean HttpOnly();

  /// \brief AL `Cookie.Name(Text)`. The name of the cookie.
  /// \param Name The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Name(std::string_view Name);

  /// \brief AL `Cookie.Path()`. The path of the cookie. It indicates the path that must exist in
  /// the request URL to send the cookie.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Path();

  /// \brief AL `Cookie.Secure()`. True if the cookie is Secure, false otherwise. It indicates that
  /// the cookie is sent only when a request is made with the https.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Secure();

  /// \brief AL `Cookie.Value(Text)`. The value of the cookie.
  /// \param Value The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Value(std::string_view Value);
};

}
