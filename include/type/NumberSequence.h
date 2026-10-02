#pragma once

#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Integer.h"

#include <string_view>

/// \file
/// \brief Database-owned, nontransactionally consumed AL number sequences.

namespace agiru {

/// \brief AL `NumberSequence`, backed by PostgreSQL sequences and a persistent identity registry.
/// \note Next/Range consumption survives caller rollback. Insert/Delete/Restart participate in
/// the caller's transaction. CompanySpecific defaults to true for every applicable overload.
/// \pre ProvisionNumberSequences has installed the primitive in the current session's database.
class NumberSequence {
public:
  /// \brief AL `NumberSequence.Current(Text, Boolean)`. Gets the current value from the number
  /// sequence, without doing any increment. The value is retrieved out of transaction. The value
  /// will not be returned on transaction rollback.
  /// \param Name The AL `Text`.
  /// \param CompanySpecific The AL `Boolean`.
  /// \return The AL `BigInteger`.
  /// \throws Error when the sequence is absent or database access fails.
  static ::agiru::BigInteger Current(std::string_view Name,
                                     ::agiru::Boolean CompanySpecific = true);

  /// \brief AL `NumberSequence.Delete(Text, Boolean)`. Deletes a specific number sequence.
  /// \param Name The AL `Text`.
  /// \param CompanySpecific The AL `Boolean`.
  /// \throws Error when the sequence is absent or database access fails.
  static void Delete(std::string_view Name, ::agiru::Boolean CompanySpecific = true);

  /// \brief AL `NumberSequence.Exists(Text, Boolean)`. Checks whether a specific number sequence
  /// exists.
  /// \param Name The AL `Text`.
  /// \param CompanySpecific The AL `Boolean`.
  /// \return The AL `Boolean`.
  /// \throws Error when storage is unavailable or database access fails.
  static ::agiru::Boolean Exists(std::string_view Name, ::agiru::Boolean CompanySpecific = true);

  /// \brief AL `NumberSequence.Insert(Text, BigInteger, BigInteger, Boolean)`. Creates a number
  /// sequence in the database, with the given parameters.
  /// \param Name The AL `Text`.
  /// \param Seed The first value returned; defaults to zero.
  /// \param Increment Nonzero signed step; defaults to one.
  /// \param CompanySpecific The AL `Boolean`.
  /// \throws Error on duplicate identity, zero increment or database refusal.
  static void Insert(std::string_view Name,
                     ::agiru::BigInteger Seed = {},
                     ::agiru::BigInteger Increment = 1,
                     ::agiru::Boolean CompanySpecific = true);

  /// \brief AL `NumberSequence.Next(Text, Boolean)`. Retrieves the next value from the number
  /// sequence.
  /// \param Name The AL `Text`.
  /// \param CompanySpecific The AL `Boolean`.
  /// \return The AL `BigInteger`.
  /// \throws Error when the sequence is absent, exhausted or storage is incompatible.
  static ::agiru::BigInteger Next(std::string_view Name, ::agiru::Boolean CompanySpecific = true);

  /// \brief AL `NumberSequence.Range(Text, Integer, var BigInteger)` for the current company.
  /// Reserves Count consecutive sequence values atomically against Next and other Range calls.
  /// \param Name The AL `Text`.
  /// \param Count Positive number of values to reserve.
  /// \param Increment Receives the sequence's signed step after a successful reservation.
  /// \return The AL `BigInteger`.
  /// \throws Error on absent/exhausted sequence, invalid Count or incompatible storage.
  static ::agiru::BigInteger
  Range(std::string_view Name, ::agiru::Integer Count, ::agiru::BigInteger &Increment);

  /// \brief Atomic range reservation with explicit company scope and output step.
  /// \param Name The sequence identity.
  /// \param Count Positive number of values.
  /// \param Increment Receives the stored signed step on success; unchanged on error.
  /// \param CompanySpecific Selects current-company or database-wide identity.
  /// \return First reserved value.
  /// \throws Error on absent/exhausted sequence, invalid Count or incompatible storage.
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
  /// \throws Error on absent/exhausted sequence, invalid Count or incompatible storage.
  static ::agiru::BigInteger
  Range(std::string_view Name, ::agiru::Integer Count, ::agiru::Boolean CompanySpecific = true);

  /// \brief AL `NumberSequence.Restart(Text, BigInteger, Boolean)`. Restarts a number sequence.
  /// \param Name The AL `Text`.
  /// \param Seed First value returned after restart; defaults to zero.
  /// \param CompanySpecific The AL `Boolean`.
  /// \throws Error when the sequence is absent or database access fails.
  static void Restart(std::string_view Name,
                      ::agiru::BigInteger Seed = {},
                      ::agiru::Boolean CompanySpecific = true);
};

}
