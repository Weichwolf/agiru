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
/// \brief AL `KeyRef` -- the surface the platform documentation declares.

namespace agiru {

// A CIRCLE THE DECLARATIONS DO NOT HAVE. `RecordRef.KeyIndex(N)` returns a KeyRef and
// `KeyRef.Record()` returns a RecordRef, so the two headers cannot include each other -- and they
// do not need to: a declaration takes and returns a NAME.
class FieldRef;
class RecordRef;

/// \brief AL `KeyRef`.
///
/// \warning THE SURFACE IS REAL AND THE BEHAVIOUR IS NOT YET. Every signature below is the one
///          `methods-auto/keyref/` states, so a call site compiles and is CHECKED; the body
///          refuses by name rather than returning a plausible wrong answer (board:0035).
class KeyRef {
public:
  /// \brief AL `KeyRef.Active()`. Indicates whether the key is enabled.
  /// \return The AL `Boolean`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Boolean Active();

  /// \brief AL `KeyRef.FieldCount()`. Gets the number of fields that have been defined in a key.
  /// Returns an error if no key is selected.
  /// \return The AL `Integer`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::Integer FieldCount();

  /// \brief AL `KeyRef.FieldIndex(Integer)`. Gets the FieldRef of the field that has this index in
  /// the key referred to by the KeyRef variable. Returns an error if no key is selected.
  /// \param Index The AL `Integer`.
  /// \return The AL `FieldRef`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::FieldRef FieldIndex(::agiru::Integer Index);

  /// \brief AL `KeyRef.Record()`. Returns a RecordRef for the current record referred to by the
  /// key.
  /// \return The AL `RecordRef`.
  /// \throws Error always -- the surface is declared, the behaviour is not (board:0035).
  ::agiru::RecordRef Record();
};

} // namespace agiru
