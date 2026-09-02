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
#include "type/Stream.h"
#include "type/Time.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `HttpContent` -- the surface the platform documentation declares.

namespace agiru {

class HttpHeaders;

/// \brief AL `HttpContent`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/httpcontent/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class HttpContent {
public:
  /// \brief AL `HttpContent.Clear()`. Sets the HttpContent object to a default value. The content
  /// contains an empty string and empty headers.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Clear();

  /// \brief AL `HttpContent.GetHeaders(HttpHeaders)`. Gets the HTTP content headers as defined in
  /// RFC 2616.
  /// \param Headers The AL `HttpHeaders`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean GetHeaders(::agiru::HttpHeaders &Headers);

  /// \brief AL `HttpContent.IsSecretContent()`. Returns if the content is secret. If it is secret
  /// it can be read only as a SecretText.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean IsSecretContent();

  /// \brief AL `HttpContent.ReadAs(InStream)`. Reads the content into the provided text.
  /// \param InStream The AL `InStream`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReadAs(::agiru::InStream &InStream);

  /// \brief AL `HttpContent.ReadAs(SecretText)`. Reads the content into the provided secure text.
  /// \param OutputSecretText The AL `SecretText`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReadAs(::agiru::SecretText &OutputSecretText);

  /// \brief AL `HttpContent.ReadAs(Text)`. Reads the content into the provided text.
  /// \param OutputString The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean ReadAs(std::string &OutputString);

  /// \brief AL `HttpContent.WriteFrom(InStream)`. Sets HttpContent content to the provided text or
  /// stream.
  /// \param InStream The AL `InStream`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void WriteFrom(const ::agiru::InStream &InStream);

  /// \brief AL `HttpContent.WriteFrom(SecretText)`. Sets HttpContent content to the provided
  /// SecretText.
  /// \param SecretText The AL `SecretText`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void WriteFrom(const ::agiru::SecretText &SecretText);

  /// \brief AL `HttpContent.WriteFrom(Text)`. Sets HttpContent content to the provided text or
  /// stream.
  /// \param Text The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void WriteFrom(std::string_view Text);
};

} // namespace agiru
