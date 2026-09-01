#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "type/Option.h"

#include <cstddef>
#include <string_view>
#include <type_traits>

/// \file
/// \brief The base every generated AL table stands on.

namespace agiru {

/// \brief The declaration belonging to a generated table.
///
/// The generator specialises this beside the table's field and key tables, so that the class itself
/// carries nothing but what AL wrote: fields, triggers, procedures. Everything the runtime needs to
/// work with the table is reached through here instead.
///
/// \tparam T The generated table class.
template <typename T> struct TableTraits;

/// \brief The platform half of a record operation. Not part of the door's vocabulary.
namespace detail {

/// \brief Writes the record as a new row.
/// \param record The record.
/// \param table  Its declaration.
/// \throws DatabaseError when the row cannot be written.
void RuntimeInsert(const void *record, const TableDef &table);

/// \brief Overwrites the row this record's primary key selects.
/// \param record The record.
/// \param table  Its declaration.
/// \return True when a row carried that key.
bool RuntimeModify(const void *record, const TableDef &table);

/// \brief Removes the row this record's primary key selects.
/// \param record The record.
/// \param table  Its declaration.
/// \return True when a row carried that key.
bool RuntimeDelete(const void *record, const TableDef &table);

/// \brief Reads the row this record's primary key selects into the record.
/// \param record The record, with its key fields set.
/// \param table  Its declaration.
/// \return True when a row carried that key.
bool RuntimeGet(void *record, const TableDef &table);

/// \brief Writes one field from the text a column returned.
/// \param record The record.
/// \param def    The field.
/// \param text   The column value.
/// \throws Error when the value does not fit the field, or the type has no reader yet.
void SetFieldText(void *record, const FieldDef &def, std::string_view text);

} // namespace detail

/// \brief What every AL table can do, without the generated class saying any of it.
///
/// AL CODE NEVER NAMES A CONNECTION, A ROW OR A COLUMN. It writes `Rec.Insert()` and
/// `Rec.Get(a, b)`, and the platform finds the session, builds the statement and moves the values.
/// This base is that platform half, so the generated class stays a transcription of the `.al` file.
///
/// \tparam Derived The generated table class, whose fields this reaches through
///         `TableTraits<Derived>::kTable`.
///
/// \note The base holds NO data. That is what leaves a generated record standard-layout, which is
///       what lets the field table address a field by `offsetof`.
template <typename Derived> class Table {
public:
  /// \brief AL `Record.Insert()`.
  ///
  /// \throws Error when the row cannot be written, a duplicate key included.
  ///
  /// THE STATEMENT FORM RAISES, and that is the documentation's own rule rather than a choice:
  /// `record-insert-method.md` writes the signature as `[Ok := ] Record.Insert(...)` and says of
  /// the return value, "If you omit this optional return value and the operation does not execute
  /// successfully, a runtime error will occur." AL code writes `Rec.Insert();` far more often than
  /// `if Rec.Insert() then`, so this is the form that carries the AL name. The value form belongs
  /// to the generator, which knows the context (board:0014).
  void Insert() const { detail::RuntimeInsert(Self(), TableTraits<Derived>::kTable); }

  /// \brief AL `Record.Modify()`.
  /// \throws Error when no row carries this primary key.
  /// \see Insert() for why the statement form raises.
  void Modify() const {
    if (!detail::RuntimeModify(Self(), TableTraits<Derived>::kTable)) {
      throw Error("the record does not exist");
    }
  }

  /// \brief AL `Record.Delete()`.
  /// \throws Error when no row carries this primary key.
  /// \see Insert() for why the statement form raises.
  void Delete() const {
    if (!detail::RuntimeDelete(Self(), TableTraits<Derived>::kTable)) {
      throw Error("the record does not exist");
    }
  }

  /// \brief AL `Record.Get(...)` -- assigns the primary key and reads that record.
  ///
  /// \tparam Keys The key field types, in key order.
  /// \param keys  The primary key values.
  /// \return True when the record was found; the record is unchanged otherwise beyond the key.
  /// \throws Error when the argument count does not match the primary key.
  template <typename... Keys> bool Get(const Keys &...keys) {
    const TableDef &table = TableTraits<Derived>::kTable;
    if (table.keys.empty() || table.keys[0].fields.size() != sizeof...(Keys)) {
      throw Error("Get: the argument count does not match the primary key");
    }
    std::size_t position = 0;
    (AssignKey(table, position++, keys), ...);
    return detail::RuntimeGet(Self(), table);
  }

  /// \brief AL `Record.FieldError(Field [, Text])`, naming the field itself.
  ///
  /// \tparam FieldType The field member's type.
  /// \param member The field, written as AL writes it: `FieldError(Code)`.
  /// \param text   Optional replacement for the default wording.
  /// \throws Error always.
  ///
  /// The address of the member is enough to find its declaration, so the generated line reads like
  /// the AL line instead of naming a field number.
  template <typename FieldType>
    requires(!std::is_same_v<FieldType, FieldNo>)
  [[noreturn]] void FieldError(const FieldType &member, std::string_view text = {}) const {
    ::agiru::FieldError(Self(), TableTraits<Derived>::kTable, NumberOf(&member), text);
  }

  /// \brief AL `Record.TestField(Field)`, naming the field itself.
  /// \tparam FieldType The field member's type.
  /// \param member The field.
  /// \throws Error when the field holds its type's blank.
  template <typename FieldType>
    requires(!std::is_same_v<FieldType, FieldNo>)
  void TestField(const FieldType &member) const {
    ::agiru::TestField(Self(), TableTraits<Derived>::kTable, NumberOf(&member));
  }

  /// \brief AL `Record.TestField(Field, Value)`, naming the field itself.
  /// \tparam FieldType The field member's type.
  /// \tparam Value     The expected value's type.
  /// \param member   The field.
  /// \param expected The value it must hold, or the option member it must hold.
  /// \throws Error when the values differ.
  template <typename FieldType, typename Value>
    requires(!std::is_same_v<FieldType, FieldNo>)
  void TestField(const FieldType &member, const Value &expected) const {
    const FieldNo no = NumberOf(&member);
    if constexpr (std::is_enum_v<Value>) {
      ::agiru::TestField(Self(), TableTraits<Derived>::kTable, no, Option<Value>{expected});
    } else {
      ::agiru::TestField(Self(), TableTraits<Derived>::kTable, no, expected);
    }
  }

  /// \brief AL `Record.FieldCaption(Field)`, naming the field itself.
  /// \tparam FieldType The field member's type.
  /// \param member The field.
  /// \return The field's `Caption` property.
  template <typename FieldType>
    requires(!std::is_same_v<FieldType, FieldNo>)
  [[nodiscard]] std::string_view FieldCaption(const FieldType &member) const {
    return ::agiru::FieldCaption(TableTraits<Derived>::kTable, NumberOf(&member));
  }

  /// \brief AL `Record.FieldError(Field [, Text])`.
  ///
  /// \param no   The field the message is about.
  /// \param text Optional replacement for the default wording.
  /// \throws Error always.
  /// \see agiru::FieldError
  [[noreturn]] void FieldError(FieldNo no, std::string_view text = {}) const {
    ::agiru::FieldError(Self(), TableTraits<Derived>::kTable, no, text);
  }

  /// \brief AL `Record.TestField(Field)`.
  /// \param no The field to test.
  /// \throws Error when the field holds its type's blank.
  void TestField(FieldNo no) const { ::agiru::TestField(Self(), TableTraits<Derived>::kTable, no); }

  /// \brief AL `Record.TestField(Field, Value)`.
  /// \tparam Value The field's own type.
  /// \param no       The field to test.
  /// \param expected The value it must hold.
  /// \throws Error when the values differ.
  template <typename Value>
    requires(!std::is_enum_v<Value>)
  void TestField(FieldNo no, const Value &expected) const {
    ::agiru::TestField(Self(), TableTraits<Derived>::kTable, no, expected);
  }

  /// \brief AL `Record.TestField(Field, Value)` against a named option member.
  ///
  /// \tparam E The option's enumeration.
  /// \param no       The field to test.
  /// \param expected The member it must hold.
  /// \throws Error when the field holds another member.
  ///
  /// The overload exists so that generated code writes `TestField(FieldNumber::CostType,
  /// ResourceCostCostType::Fixed)` for AL's AL's two-argument TestField, instead
  /// of wrapping the member in its option type at the call site.
  template <typename E>
    requires std::is_enum_v<E>
  void TestField(FieldNo no, E expected) const {
    ::agiru::TestField(Self(), TableTraits<Derived>::kTable, no, Option<E>{expected});
  }

  /// \brief AL `Record.FieldCaption(Field)`.
  /// \param no The field.
  /// \return The field's `Caption` property.
  [[nodiscard]] std::string_view FieldCaption(FieldNo no) const {
    return ::agiru::FieldCaption(TableTraits<Derived>::kTable, no);
  }

private:
  friend Derived;

  Table() = default;

  /// The AL field number of a member of this record, found by where it sits.
  [[nodiscard]] FieldNo NumberOf(const void *member) const {
    const auto offset = static_cast<std::size_t>(static_cast<const std::byte *>(member) -
                                                 static_cast<const std::byte *>(Self()));
    const FieldDef *def = FieldAtOffset(TableTraits<Derived>::kTable, offset);
    if (def == nullptr) { throw Error("this record declares no field at that position"); }
    return def->no;
  }

  [[nodiscard]] const void *Self() const { return static_cast<const Derived *>(this); }

  [[nodiscard]] void *Self() { return static_cast<Derived *>(this); }

  template <typename Key>
  void AssignKey(const TableDef &table, std::size_t position, const Key &value) {
    const FieldDef *def = Field(table, table.keys[0].fields[position]);
    if (def == nullptr) { throw Error("Get: the primary key names a field the table lacks"); }
    *reinterpret_cast<Key *>(static_cast<std::byte *>(Self()) + def->offset) = value;
  }
};

} // namespace agiru
