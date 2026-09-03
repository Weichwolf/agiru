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
/// \brief AL `XmlNameTable` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `XmlNameTable`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/xmlnametable/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class XmlNameTable {
public:
  /// \brief AL `XmlNameTable.Add(Text)`. Atomizes the specified string and adds it to the
  /// XmlNameTable.
  /// \param Key The AL `Text`.
  /// \return The AL `Text`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  std::string Add(std::string_view Key);

  /// \brief AL `XmlNameTable.Get(Text, Text)`. Gets the atomized string with the specified value.
  /// \param Key The AL `Text`.
  /// \param Result The AL `Text`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Get(std::string_view Key, std::string &Result);
};

}
