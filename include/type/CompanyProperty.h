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
/// \brief AL `CompanyProperty` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `CompanyProperty`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/companyproperty/` states, so a call site compiles and is CHECKED; the
///          body refuses by name rather than returning a plausible wrong answer (board:0035).
class CompanyProperty {
public:
  /// \brief AL `CompanyProperty.DisplayName()`. Gets the current company display name.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static std::string DisplayName();

  /// \brief AL `CompanyProperty.ID()`. Gets the current company ID.
  /// \return The AL `Guid`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Guid ID();

  /// \brief AL `CompanyProperty.UrlName()`. Gets the string that represents the company name in a
  /// URL.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static std::string UrlName();
};

}
