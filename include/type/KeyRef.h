#pragma once

#include "meta/TableDef.h"
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

class FieldRef;
class RecordRef;

/// \brief AL `KeyRef`.
///
/// \note IT IS A KEY'S DECLARATION AND A RECORD, the same pair `FieldRef` is. `RecordRef.KeyIndex`
///       hands one out, so what it points at is the record the RecordRef points at and the
///       `KeyDef` the table declares -- both of which are already there, the second as `constexpr`
///       data in `.rodata`.
class KeyRef {
public:
  /// \brief A KeyRef pointing at nothing, which is what `var K: KeyRef` declares.
  KeyRef() = default;

  /// \brief A KeyRef over one key of one record.
  /// \param record The record.
  /// \param table  Its declaration.
  /// \param def    The key's declaration.
  KeyRef(void *record, const TableDef &table, const KeyDef &def)
      : record_(record), table_(&table), def_(&def) {}

  /// \brief AL `KeyRef.Active()`. Indicates whether the key is enabled.
  /// \return The AL `Boolean`.
  /// \throws Error when no key is selected.
  ///
  /// \note THE `Enabled` PROPERTY AND NOT THE INDEX. A disabled key is declared and not
  ///       maintained, which is exactly what `KeyDef::enabled` carries.
  ::agiru::Boolean Active() const;

  /// \brief AL `KeyRef.FieldCount()`. Gets the number of fields that have been defined in a key.
  /// \return The AL `Integer`.
  /// \throws Error when no key is selected.
  ///
  /// \note THE KEY'S OWN FIELDS AND NOT ITS INCLUDED ONES. `IncludedFields` is a separate span on
  ///       the declaration for that reason: no key selects by them.
  ::agiru::Integer FieldCount() const;

  /// \brief AL `KeyRef.FieldIndex(Integer)`. Gets the FieldRef of the field that has this index in
  /// the key referred to by the KeyRef variable.
  /// \param Index The AL `Integer`, counting from ONE.
  /// \return The AL `FieldRef`.
  /// \throws Error when no key is selected, and when the index is outside the key.
  ::agiru::FieldRef FieldIndex(::agiru::Integer Index) const;

  /// \brief AL `KeyRef.Record()`. Returns a RecordRef for the current record referred to by the
  /// key.
  /// \return The AL `RecordRef`.
  /// \throws Error when no key is selected.
  ::agiru::RecordRef Record() const;

private:
  void *record_ = nullptr;
  const TableDef *table_ = nullptr;
  const KeyDef *def_ = nullptr;
};

}
