#pragma once

#include <cstdint>

/// \file
/// \brief AL `HttpRequestType` -- The different types of HTTP Requests that can be intercepted by a
/// handler
///
/// The members and their order come from `methods-auto/httprequesttype/httprequesttype-option.md`,
/// which is the specification: an AL option is zero-based and sequential in the order the page
/// lists.

namespace agiru {

/// \brief AL `HttpRequestType`. The different types of HTTP Requests that can be intercepted by a
/// handler
enum class HttpRequestType : std::int32_t {
  Unknown, ///< A request that does not match a standard HTTP method type
  Get,     ///< A get request
  Delete,  ///< A delete request
  Post,    ///< A post request
  Put,     ///< A put request
  Patch,   ///< A patch request
  Options, ///< An options request
  Head,    ///< A head request
  Connect, ///< A connect request
  Trace,   ///< A trace request
};

}
