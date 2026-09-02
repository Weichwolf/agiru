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
/// \brief AL `NumberSequence` -- the surface the platform documentation declares.

namespace agiru {

/// \brief AL `NumberSequence`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/numbersequence/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class NumberSequence {
public:
  /// \brief AL `NumberSequence.Current(Text, Boolean)`. Gets the current value from the number
  /// sequence, without doing any increment. The value is retrieved out of transaction. The value
  /// will not be returned on transaction rollback.
  /// \param Name The AL `Text`.
  /// \param CompanySpecific The AL `Boolean`.
  /// \return The AL `BigInteger`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::BigInteger Current(std::string_view Name, ::agiru::Boolean CompanySpecific);

  /// \brief AL `NumberSequence.Delete(Text, Boolean)`. Deletes a specific number sequence.
  /// \param Name The AL `Text`.
  /// \param CompanySpecific The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static void Delete(std::string_view Name, ::agiru::Boolean CompanySpecific);

  /// \brief AL `NumberSequence.Exists(Text, Boolean)`. Checks whether a specific number sequence
  /// exists.
  /// \param Name The AL `Text`.
  /// \param CompanySpecific The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::Boolean Exists(std::string_view Name, ::agiru::Boolean CompanySpecific);

  /// \brief AL `NumberSequence.Insert(Text, BigInteger, BigInteger, Boolean)`. Creates a number
  /// sequence in the database, with the given parameters.
  /// \param Name The AL `Text`.
  /// \param Seed The AL `BigInteger`.
  /// \param Increment The AL `BigInteger`.
  /// \param CompanySpecific The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static void Insert(std::string_view Name,
                     ::agiru::BigInteger Seed,
                     ::agiru::BigInteger Increment,
                     ::agiru::Boolean CompanySpecific);

  /// \brief AL `NumberSequence.Next(Text, Boolean)`. Retrieves the next value from the number
  /// sequence.
  /// \param Name The AL `Text`.
  /// \param CompanySpecific The AL `Boolean`.
  /// \return The AL `BigInteger`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::BigInteger Next(std::string_view Name, ::agiru::Boolean CompanySpecific);

  /// \brief AL `NumberSequence.Range(Text, Integer, BigInteger, Boolean)`. Retrieves a range of
  /// values from the number sequence.
  /// \param Name The AL `Text`.
  /// \param Count The AL `Integer`.
  /// \param Increment The AL `BigInteger`.
  /// \param CompanySpecific The AL `Boolean`.
  /// \return The AL `BigInteger`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::BigInteger Range(std::string_view Name,
                                   ::agiru::Integer Count,
                                   ::agiru::BigInteger &Increment,
                                   ::agiru::Boolean CompanySpecific);

  /// \brief AL `NumberSequence.Range(Text, Integer, Boolean)`. Retrieves a range of values from the
  /// number sequence.
  /// \param Name The AL `Text`.
  /// \param Count The AL `Integer`.
  /// \param CompanySpecific The AL `Boolean`.
  /// \return The AL `BigInteger`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static ::agiru::BigInteger
  Range(std::string_view Name, ::agiru::Integer Count, ::agiru::Boolean CompanySpecific);

  /// \brief AL `NumberSequence.Restart(Text, BigInteger, Boolean)`. Restarts a number sequence.
  /// \param Name The AL `Text`.
  /// \param Seed The AL `BigInteger`.
  /// \param CompanySpecific The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  static void
  Restart(std::string_view Name, ::agiru::BigInteger Seed, ::agiru::Boolean CompanySpecific);
};

} // namespace agiru
