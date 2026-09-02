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
/// \brief AL `HttpClient` -- the surface the platform documentation declares.

namespace agiru {

class HttpContent;
class HttpHeaders;
class HttpRequestMessage;
class HttpResponseMessage;

/// \brief AL `HttpClient`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/httpclient/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class HttpClient {
public:
  /// \brief AL `HttpClient.AddCertificate(SecretText, SecretText)`. Adds a certificate as a
  /// SecretText to the HttpClient class.
  /// \param Certificate The AL `SecretText`.
  /// \param Password The AL `SecretText`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void AddCertificate(const ::agiru::SecretText &Certificate, const ::agiru::SecretText &Password);

  /// \brief AL `HttpClient.AddCertificate(Text, Text)`. Adds a certificate to the HttpClient class.
  /// \param Certificate The AL `Text`.
  /// \param Password The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void AddCertificate(std::string_view Certificate, std::string_view Password);

  /// \brief AL `HttpClient.Clear()`. Sets the HttpClient variable to the default value.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void Clear();

  /// \brief AL `HttpClient.DefaultRequestHeaders()`. Gets the default request headers which should
  /// be sent with each request.
  /// \return The AL `HttpHeaders`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::HttpHeaders DefaultRequestHeaders();

  /// \brief AL `HttpClient.Delete(Text, HttpResponseMessage)`. Sends a DELETE request to delete the
  /// resource identified by the request URL.
  /// \param Path The AL `Text`.
  /// \param Response The AL `HttpResponseMessage`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Delete(std::string_view Path, ::agiru::HttpResponseMessage &Response);

  /// \brief AL `HttpClient.Get(Text, HttpResponseMessage)`. Sends a GET request to get the resource
  /// identified by the request URL.
  /// \param Path The AL `Text`.
  /// \param Response The AL `HttpResponseMessage`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Get(std::string_view Path, ::agiru::HttpResponseMessage &Response);

  /// \brief AL `HttpClient.GetBaseAddress()`. Gets the base address of Uniform Resource Identifier
  /// (URI) of the Internet resource used when sending requests.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string GetBaseAddress();

  /// \brief AL `HttpClient.Patch(Text, HttpContent, HttpResponseMessage)`. Sends a PATCH request to
  /// the specified URI as an asynchronous operation.
  /// \param Path The AL `Text`.
  /// \param Content The AL `HttpContent`.
  /// \param Response The AL `HttpResponseMessage`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Patch(std::string_view Path,
                         const ::agiru::HttpContent &Content,
                         ::agiru::HttpResponseMessage &Response);

  /// \brief AL `HttpClient.Post(Text, HttpContent, HttpResponseMessage)`. Sends a POST request to
  /// the specified URI as an asynchronous operation.
  /// \param Path The AL `Text`.
  /// \param Content The AL `HttpContent`.
  /// \param Response The AL `HttpResponseMessage`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Post(std::string_view Path,
                        const ::agiru::HttpContent &Content,
                        ::agiru::HttpResponseMessage &Response);

  /// \brief AL `HttpClient.Put(Text, HttpContent, HttpResponseMessage)`. Sends a PUT request to the
  /// specified URI as an asynchronous operation.
  /// \param Path The AL `Text`.
  /// \param Content The AL `HttpContent`.
  /// \param Response The AL `HttpResponseMessage`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Put(std::string_view Path,
                       const ::agiru::HttpContent &Content,
                       ::agiru::HttpResponseMessage &Response);

  /// \brief AL `HttpClient.Send(HttpRequestMessage, HttpResponseMessage)`. Sends an HTTP request as
  /// an asynchronous operation.
  /// \param Request The AL `HttpRequestMessage`.
  /// \param Response The AL `HttpResponseMessage`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Send(const ::agiru::HttpRequestMessage &Request,
                        ::agiru::HttpResponseMessage &Response);

  /// \brief AL `HttpClient.SetBaseAddress(Text)`. Sets the base address of Uniform Resource
  /// Identifier (URI) of the Internet resource used when sending requests.
  /// \param NewBaseAddress The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean SetBaseAddress(std::string_view NewBaseAddress);

  /// \brief AL `HttpClient.Timeout(Duration)`. Gets or sets the duration in milliseconds to wait
  /// before the request times out.
  /// \param SetTimeout The AL `Duration`.
  /// \return The AL `Duration`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Duration Timeout(::agiru::Duration SetTimeout);

  /// \brief AL `HttpClient.UseDefaultNetworkWindowsAuthentication()`. Sets the HttpClient
  /// credentials to use the default network credentials for Windows authentication. If this method
  /// is invoked after any HTTP request has started; a runtime error occurs.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean UseDefaultNetworkWindowsAuthentication();

  /// \brief AL `HttpClient.UseResponseCookies(Boolean)`. If true, the client automatically attaches
  /// cookies received in the response to all subsequent requests.
  /// \param UseResponseCookies The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  void UseResponseCookies(::agiru::Boolean UseResponseCookies);

  /// \brief AL `HttpClient.UseServerCertificateValidation(Boolean)`. If true, the client validates
  /// the server certificate for all HTTP requests. If false, it skips validation.
  /// \param UseServerCertificateValidation The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean UseServerCertificateValidation(::agiru::Boolean UseServerCertificateValidation);

  /// \brief AL `HttpClient.UseWindowsAuthentication(SecretText, SecretText, SecretText)`. Sets the
  /// HttpClient credentials to use the specified network credentials for Windows authentication. If
  /// this method is invoked after any HTTP request has started; a runtime error occurs.
  /// \param UserName The AL `SecretText`.
  /// \param Password The AL `SecretText`.
  /// \param Domain The AL `SecretText`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean UseWindowsAuthentication(const ::agiru::SecretText &UserName,
                                            const ::agiru::SecretText &Password,
                                            const ::agiru::SecretText &Domain);

  /// \brief AL `HttpClient.UseWindowsAuthentication(Text, Text, Text)`. Sets the HttpClient
  /// credentials to use the specified network credentials for Windows authentication. If this
  /// method is invoked after any HTTP request has started; a runtime error occurs.
  /// \param UserName The AL `Text`.
  /// \param Password The AL `Text`.
  /// \param Domain The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean UseWindowsAuthentication(std::string_view UserName,
                                            std::string_view Password,
                                            std::string_view Domain);
};

} // namespace agiru
