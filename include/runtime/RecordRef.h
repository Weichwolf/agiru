#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/Table.h"
#include "type/Integer.h"
#include "type/List.h"
#include "type/Variant.h"

#include <string>
#include <string_view>

/// \file
/// \brief AL `RecordRef` and `FieldRef` -- a record reached by NUMBER rather than by name.

namespace agiru {

/// \brief AL `FieldRef` -- one field of one record, reached without naming its type.
///
/// \note THIS IS WHAT THE FIELD TABLE WAS BUILT FOR. A generated record addresses its fields by
///       `offsetof` through `FieldDef`, and a FieldRef is that same address arrived at from the
///       other side: a number instead of a member. Nothing new is needed to hold one.
class FieldRef {
public:
  /// \brief A FieldRef over one field of one record.
  /// \param record The record.
  /// \param table  Its declaration.
  /// \param def    The field's declaration.
  FieldRef(void *record, const TableDef &table, const FieldDef &def)
      : record_(record), table_(&table), def_(&def) {}

  /// \brief AL `FieldRef.Number()`.
  /// \return The AL field number.
  [[nodiscard]] Integer Number() const { return def_->no.Value(); }

  /// \brief AL `FieldRef.Name()`.
  /// \return The AL name, spaces and all.
  [[nodiscard]] std::string_view Name() const { return def_->name; }

  /// \brief AL `FieldRef.Caption()`.
  /// \return The caption an error message quotes.
  [[nodiscard]] std::string_view Caption() const { return def_->caption; }

  /// \brief AL `FieldRef.Length()`.
  /// \return The declared length for Code and Text, 0 otherwise.
  [[nodiscard]] Integer Length() const { return def_->length; }

  /// \brief AL `FieldRef.Type()`.
  /// \return The field's AL data type.
  [[nodiscard]] FieldType Type() const { return def_->type; }

  /// \brief AL `FieldRef.IsEnum()`.
  /// \return True when the field is an enum rather than an option or anything else.
  [[nodiscard]] bool IsEnum() const { return def_->type == FieldType::Enum; }

  /// \brief AL `FieldRef.EnumValueCount()`.
  /// \return How many values the enumeration declares; 0 when the field is not one.
  [[nodiscard]] Integer EnumValueCount() const { return static_cast<Integer>(def_->values.size()); }

  /// \brief AL `FieldRef.GetEnumValueName(Index)`.
  ///
  /// \param index The ONE-BASED position in the value list, which is what the page says.
  /// \return The value's name, or empty when the index is outside the list.
  ///
  /// \note POSITION, NOT ORDINAL. The platform gives both this and
  ///       GetEnumValueNameFromOrdinalValue precisely because they are different questions:
  ///       `enum 50130 YesNo { value(0; Yes) value(10; No) }` answers `No` for index 2 and for
  ///       ordinal 10.
  [[nodiscard]] std::string_view GetEnumValueName(Integer index) const;

  /// \brief AL `FieldRef.GetEnumValueOrdinal(Index)`.
  /// \param index The one-based position.
  /// \return The declared ordinal there, or 0 when the index is outside the list.
  [[nodiscard]] Integer GetEnumValueOrdinal(Integer index) const;

  /// \brief AL `FieldRef.GetEnumValueNameFromOrdinalValue(Ordinal)`.
  /// \param ordinal The declared number.
  /// \return The value's name, or empty when the enumeration declares no such ordinal.
  [[nodiscard]] std::string_view GetEnumValueNameFromOrdinalValue(Integer ordinal) const;

  /// \brief AL `FieldRef.OptionMembers()`.
  /// \return The member names, in declaration order.
  [[nodiscard]] List<std::string> OptionMembers() const;

  /// \brief AL `FieldRef.Value()`.
  /// \return The field's value, carrying its type.
  /// \throws Error when the field's type has no Variant alternative yet.
  [[nodiscard]] Variant Value() const;

  /// \brief AL `FieldRef.Value := X` -- writes the field from text.
  /// \param text The value, as the column would hold it.
  /// \throws Error when the value does not fit the field.
  void SetValue(std::string_view text);

  /// \brief AL `FieldRef.TestField()` -- raises when the field is blank.
  /// \throws Error with the platform's own wording when the field holds its zero.
  void TestField() const;

private:
  void *record_;
  const TableDef *table_;
  const FieldDef *def_;
};

/// \brief AL `RecordRef` -- a record reached without naming its table.
///
/// \note IT DOES NOT OWN THE RECORD. `GetTable(Rec)` points a RecordRef at a record that already
///       exists, which is how the BaseApp uses it: `RecRef.GetTable(SalesLine)` and then walk the
///       fields. `Open(TableNo)` -- which makes a record out of a number alone -- needs a registry
///       from table number to declaration that this runtime does not have yet, and refuses rather
///       than handing back something empty.
class RecordRef {
public:
  /// \brief A RecordRef pointing at nothing.
  RecordRef() = default;

  /// \brief AL `RecordRef.GetTable(Record)` -- points at an existing record.
  /// \tparam T The generated table class.
  /// \param rec The record.
  template <typename T> void GetTable(T &rec) {
    record_ = &rec;
    table_ = &TableTraits<T>::kTable;
  }

  /// \brief AL `RecordRef.Open(TableNo)`.
  /// \param tableNo The AL table number.
  /// \throws Error always, for now.
  /// \warning REFUSED. Making a record from a number needs a registry from table number to
  ///          declaration, and a RecordRef that opened nothing would answer every question with a
  ///          zero rather than saying it opened nothing.
  void Open(Integer tableNo);

  /// \return True when this RecordRef points at a record.
  [[nodiscard]] bool IsOpen() const { return record_ != nullptr; }

  /// \brief AL `RecordRef.Number()`.
  /// \return The AL table number.
  /// \throws Error when the RecordRef points at nothing.
  [[nodiscard]] Integer Number() const;

  /// \brief AL `RecordRef.Name()`.
  /// \return The table's AL name.
  /// \throws Error when the RecordRef points at nothing.
  [[nodiscard]] std::string_view Name() const;

  /// \brief AL `RecordRef.FieldCount()`.
  /// \return How many fields the table declares.
  /// \throws Error when the RecordRef points at nothing.
  [[nodiscard]] Integer FieldCount() const;

  /// \brief AL `RecordRef.Field(FieldNo)`.
  /// \param fieldNo The AL field number.
  /// \return A FieldRef over it.
  /// \throws Error when the table declares no such field.
  [[nodiscard]] FieldRef Field(Integer fieldNo) const;

  /// \brief AL `RecordRef.FieldIndex(Index)`.
  /// \param index The ONE-BASED position in the field list.
  /// \return A FieldRef over the field there.
  /// \throws Error when the index is outside the list.
  [[nodiscard]] FieldRef FieldIndex(Integer index) const;

  /// \brief AL `RecordRef.FieldExist(FieldNo)`.
  /// \param fieldNo The AL field number.
  /// \return True when the table declares it.
  [[nodiscard]] bool FieldExist(Integer fieldNo) const;

  /// \brief AL `RecordRef.KeyCount()`.
  /// \return How many keys the table declares.
  /// \throws Error when the RecordRef points at nothing.
  [[nodiscard]] Integer KeyCount() const;

private:
  [[nodiscard]] const TableDef &Table() const;

  void *record_ = nullptr;
  const TableDef *table_ = nullptr;
};

} // namespace agiru
