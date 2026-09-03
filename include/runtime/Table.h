#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Option.h"

#include <algorithm>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <vector>

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

/// \brief Makes a record the `xRec` of the trigger about to run.
///
/// \param record The record as it was BEFORE the change, which the caller owns and must outlive
///               the trigger.
///
/// \note THE PLATFORM PROVIDES `xRec`, NOT THE OBJECT. A table trigger takes no parameters and AL
///       still names two records inside it, so what supplies the second one is whoever invoked the
///       trigger -- which here is `Validate`, `Insert(true)`, `Modify(true)` and `Delete(true)`.
void PushBefore(const void *record);

/// \brief Ends what PushBefore began.
void PopBefore();

/// \brief The record the running trigger is changing FROM.
/// \return It, or `nullptr` outside a trigger.
[[nodiscard]] const void *CurrentBefore();

/// \brief AL `xRec` -- the record as it was before the change.
///
/// \tparam T The generated table class.
/// \return The record before the change.
/// \throws Error outside a trigger, or where the invoker has no before-image yet (board:0042).
template <typename T> const T &Before() {
  const void *before = CurrentBefore();
  if (before == nullptr) {
    throw Error("xRec is only defined inside a table trigger, and the trigger that is running was "
                "invoked without a before-image (board:0042)");
  }
  return *static_cast<const T *>(before);
}

/// \brief Holds a before-image for as long as a trigger runs.
class BeforeImage {
public:
  /// \brief Makes a record the running trigger's `xRec`.
  /// \param record The record before the change.
  explicit BeforeImage(const void *record) { PushBefore(record); }

  BeforeImage(const BeforeImage &) = delete;
  BeforeImage(BeforeImage &&) = delete;
  BeforeImage &operator=(const BeforeImage &) = delete;
  BeforeImage &operator=(BeforeImage &&) = delete;

  ~BeforeImage() { PopBefore(); }
};

/// \brief Writes the record as a new row.
/// \param record The record.
/// \param table  Its declaration.
/// \throws DatabaseError when the row cannot be written.
void RuntimeInsert(void *record, const TableDef &table);

/// \brief Overwrites the row this record's primary key selects.
/// \param record The record.
/// \param table  Its declaration.
/// \return True when a row carried that key.
bool RuntimeModify(void *record, const TableDef &table);

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

/// \brief Orders two records of the same table by their primary key.
///
/// \tparam T The generated table class.
/// \param  a One record.
/// \param  b The other.
/// \return True when `a` sorts before `b`.
///
/// The key fields are read through the field table, in the order the primary key declares them,
/// and compared BY TYPE, which is the ordering the database gives and the one AL walks in.
template <typename T> bool ByKey(const T &a, const T &b) {
  const TableDef &table = TableTraits<T>::kTable;
  if (table.keys.empty()) { return false; }
  for (const FieldNo no : table.keys[0].fields) {
    const FieldDef *def = Field(table, no);
    if (def == nullptr) { continue; }
    const std::strong_ordering order = CompareField(&a, &b, *def);
    if (order != std::strong_ordering::equal) { return order == std::strong_ordering::less; }
  }
  return false;
}

/// \brief Whether two records of the same table carry the same primary key.
/// \tparam T The generated table class.
/// \param  a One record.
/// \param  b The other.
/// \return True when neither sorts before the other.
template <typename T> bool SameKey(const T &a, const T &b) {
  return !ByKey(a, b) && !ByKey(b, a);
}

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
// The check wants a private constructor with `friend Derived`, and that makes every generated table
// a NON-INITIALISABLE AGGREGATE -- a table has no user-declared constructor, so `X Rec{}` is
// aggregate initialisation, which initialises this base from the caller's context and is refused
// there. Every page and every local record hit it. `Self()` asserts the pairing instead, on first
// use and at no run-time cost.
// NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility): see above.
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
  /// \warning IT REPLACES THE SystemId, EVEN ONE THE CALLER ASSIGNED, and that is the platform's
  ///          documented default rather than a shortcut. `record-insert-boolean-boolean-method.md`
  ///          on `InsertWithSystemId`: "If this parameter is false, the SystemId field is given a
  ///          value that is auto-generated by the platform. The default value is false." The
  ///          overload that honours an assigned one is `Insert(RunTrigger, InsertWithSystemId)`,
  ///          which needs trigger dispatch and is not here yet (board:0029).
  ///
  /// \note IT IS NOT `const`, BECAUSE THE PLATFORM WRITES INTO THE RECORD.
  ///       `devenv-table-system-fields.md`: "When a record is first inserted, the fields are
  ///       populated with actual values ... the values written to the database are always provided
  ///       by the platform." A `const` Insert could not do that, and a caller reading
  ///       `Rec.SystemId` after `Rec.Insert()` -- which the BaseApp does -- would see the blank
  ///       GUID it went in with.
  void Insert() { detail::RuntimeInsert(Self(), TableTraits<Derived>::kTable); }

  /// \brief AL `Record.Insert(RunTrigger)`.
  ///
  /// \param RunTrigger True to run the table's `OnInsert` trigger first.
  /// \throws Error when the row cannot be written, and whatever the trigger raises.
  ///
  /// \note THE TRIGGER RUNS BEFORE THE ROW IS WRITTEN, which the trigger's own page states:
  ///       "This trigger is run before default insert behavior ... The new record is not inserted
  ///       if an error occurs in the trigger code."
  /// \note WHETHER THE TABLE HAS ONE IS A COMPILE-TIME QUESTION, not a registry lookup: the
  ///       generated class declares `OnInsert` exactly when its `.al` does, so `requires` answers
  ///       it and a table without the trigger compiles to the same code `Insert()` does.
  void Insert(Boolean RunTrigger) {
    if (RunTrigger) {
      if constexpr (requires(Derived &record) { record.OnInsert(); }) { Self()->OnInsert(); }
    }
    detail::RuntimeInsert(Self(), TableTraits<Derived>::kTable);
  }

  /// \brief AL `Record.Modify()`.
  /// \throws Error when no row carries this primary key.
  /// \see Insert() for why the statement form raises.
  /// \note Not `const`, for the reason Insert() gives: SystemModifiedAt and SystemModifiedBy are
  ///       written by the platform on every modify.
  void Modify() {
    if (!detail::RuntimeModify(Self(), TableTraits<Derived>::kTable)) {
      throw Error("the record does not exist");
    }
  }

  /// \brief AL `Record.Modify(RunTrigger)`.
  /// \param RunTrigger True to run the table's `OnModify` trigger first.
  /// \throws Error when no row carries this primary key, and whatever the trigger raises.
  /// \see Insert(Boolean) for why the trigger runs first and how it is found.
  void Modify(Boolean RunTrigger) {
    if (RunTrigger) {
      if constexpr (requires(Derived &record) { record.OnModify(); }) { Self()->OnModify(); }
    }
    Modify();
  }

  /// \brief AL `Record.Delete()`.
  /// \throws Error when no row carries this primary key.
  /// \see Insert() for why the statement form raises.
  void Delete() const {
    if (!detail::RuntimeDelete(Self(), TableTraits<Derived>::kTable)) {
      throw Error("the record does not exist");
    }
  }

  /// \brief AL `Record.Delete(RunTrigger)`.
  /// \param RunTrigger True to run the table's `OnDelete` trigger first.
  /// \throws Error when no row carries this primary key, and whatever the trigger raises.
  /// \see Insert(Boolean) for why the trigger runs first and how it is found.
  /// \note NOT `const`, although the delete is: `OnDelete` is AL code that may write into the
  ///       record it is about to remove, and a great many of them do.
  void Delete(Boolean RunTrigger) {
    if (RunTrigger) {
      if constexpr (requires(Derived &record) { record.OnDelete(); }) { Self()->OnDelete(); }
    }
    Delete();
  }

  /// \brief AL `Record.Get(...)` -- assigns the primary key and reads that record.
  ///
  /// \tparam Keys The key field types, in key order.
  /// \param keys  The primary key values.
  /// \return True when the record was found; the record is unchanged otherwise beyond the key.
  /// \throws Error when the argument count does not match the primary key.
  template <typename... Keys> bool Get(const Keys &...keys) {
    AssignPrimaryKey(keys...);
    return detail::RuntimeGet(Self(), TableTraits<Derived>::kTable);
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
    requires(!std::is_same_v<FieldType, ::agiru::FieldNo>)
  [[noreturn]] void FieldError(const FieldType &member, std::string_view text = {}) const {
    ::agiru::FieldError(Self(), TableTraits<Derived>::kTable, NumberOf(&member), text);
  }

  /// \brief AL `Record.TestField(Field)`, naming the field itself.
  /// \tparam FieldType The field member's type.
  /// \param member The field.
  /// \throws Error when the field holds its type's blank.
  template <typename FieldType>
    requires(!std::is_same_v<FieldType, ::agiru::FieldNo>)
  void TestField(const FieldType &member) const {
    ::agiru::detail::TestField(Self(), TableTraits<Derived>::kTable, NumberOf(&member));
  }

  /// \brief AL `Record.TestField(Field, Value)`, naming the field itself.
  /// \tparam FieldType The field member's type.
  /// \tparam Value     The expected value's type.
  /// \param member   The field.
  /// \param expected The value it must hold, or the option member it must hold.
  /// \throws Error when the values differ.
  template <typename FieldType, typename Value>
    requires(!std::is_same_v<FieldType, ::agiru::FieldNo>)
  void TestField(const FieldType &member, const Value &expected) const {
    const ::agiru::FieldNo no = NumberOf(&member);
    if constexpr (std::is_enum_v<Value>) {
      ::agiru::TestFieldValue(Self(), TableTraits<Derived>::kTable, no, Option<Value>{expected});
    } else {
      ::agiru::TestFieldValue(Self(), TableTraits<Derived>::kTable, no, expected);
    }
  }

  /// \brief AL `Record.FieldCaption(Field)`, naming the field itself.
  /// \tparam FieldType The field member's type.
  /// \param member The field.
  /// \return The field's `Caption` property.
  template <typename FieldType>
    requires(!std::is_same_v<FieldType, ::agiru::FieldNo>)
  [[nodiscard]] std::string_view FieldCaption(const FieldType &member) const {
    return ::agiru::FieldCaption(TableTraits<Derived>::kTable, NumberOf(&member));
  }

  /// \brief AL `Record.FieldError(Field [, Text])`.
  ///
  /// \param no   The field the message is about.
  /// \param text Optional replacement for the default wording.
  /// \throws Error always.
  /// \see agiru::FieldError
  [[noreturn]] void FieldError(::agiru::FieldNo no, std::string_view text = {}) const {
    ::agiru::FieldError(Self(), TableTraits<Derived>::kTable, no, text);
  }

  /// \brief AL `Record.TestField(Field)`.
  /// \param no The field to test.
  /// \throws Error when the field holds its type's blank.
  void TestField(::agiru::FieldNo no) const {
    ::agiru::detail::TestField(Self(), TableTraits<Derived>::kTable, no);
  }

  /// \brief AL `Record.TestField(Field, Value)`.
  /// \tparam Value The field's own type.
  /// \param no       The field to test.
  /// \param expected The value it must hold.
  /// \throws Error when the values differ.
  template <typename Value>
    requires(!std::is_enum_v<Value>)
  void TestField(::agiru::FieldNo no, const Value &expected) const {
    ::agiru::TestFieldValue(Self(), TableTraits<Derived>::kTable, no, expected);
  }

  /// \brief AL `Record.TestField(Field, Value)` against a named option member.
  ///
  /// \tparam E The option's enumeration.
  /// \param no       The field to test.
  /// \param expected The member it must hold.
  /// \throws Error when the field holds another member.
  ///
  /// The overload exists so that generated code writes `TestField(Field_No::CostType,
  /// ResourceCostCostType::Fixed)` for AL's AL's two-argument TestField, instead
  /// of wrapping the member in its option type at the call site.
  template <typename E>
    requires std::is_enum_v<E>
  void TestField(::agiru::FieldNo no, E expected) const {
    ::agiru::TestFieldValue(Self(), TableTraits<Derived>::kTable, no, Option<E>{expected});
  }

  /// \brief AL `Record.FieldNo(Field)` -- the AL number of a field, named the way AL names it.
  ///
  /// \tparam Value The field's type.
  /// \param member The field itself, which is how AL writes it: `Rec.FieldNo(Code)`.
  /// \return The AL field number.
  /// \throws Error when the address is not a field of this record.
  /// \note IT RETURNS AN `Integer` BECAUSE AL'S DOES. `record-fieldno-method.md` gives the return
  ///       as Integer, and AL code hands it straight to a procedure that takes one --
  ///       `GenerateRandomCode(Rec.FieldNo(Code), DATABASE::X)`. The strong `agiru::FieldNo` stays
  ///       where the METADATA uses it, which is where a wrong number cannot be caught any other
  ///       way.
  template <typename Value> [[nodiscard]] Integer FieldNo(const Value &member) const {
    return NumberOf(&member).Value();
  }

  /// \brief AL `Record.FieldCaption(Field)`.
  /// \param no The field.
  /// \return The field's `Caption` property.
  [[nodiscard]] std::string_view FieldCaption(::agiru::FieldNo no) const {
    return ::agiru::FieldCaption(TableTraits<Derived>::kTable, no);
  }

  // WHAT A RECORD CAN DO, from `methods-auto/record/`: 111 pages over 82 names, and what a body
  // needs first is that the NAME exists and refuses rather than failing to compile. Each takes
  // whatever AL's overload set takes, because a variadic template is the one shape that accepts
  // every call site while the behaviour is still a refusal (board:0035). They go one at a time as
  // the behaviour lands, and each that does gets its documented signature and its gate case.
  /// \brief AL `Record.AddLink(...)`. Adds a link to a record.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean AddLink(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.AddLink is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.AddLoadFields(...)`. Specifies fields to be initially loaded when the record
  /// is retrieved from its data source. Subsequent calls to AddLoadFields will not overwrite fields
  /// already selected for the initial load.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean AddLoadFields(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.AddLoadFields is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.AreFieldsLoaded(...)`. Checks whether the specified fields are all initially
  /// loaded.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean AreFieldsLoaded(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.AreFieldsLoaded is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.Ascending(...)`. Gets or sets the order in which the system searches through
  /// a table.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean Ascending(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.Ascending is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.CalcFields(...)`. Calculates the FlowFields in a record. You specify which
  /// fields to calculate by using parameters.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean CalcFields(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.CalcFields is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.CalcSums(...)`. Calculates the total of a column in a table. You specify
  /// which fields to calculate by using parameters.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean CalcSums(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.CalcSums is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.ChangeCompany(...)`. Redirects references to table data from one company to
  /// another.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean ChangeCompany(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.ChangeCompany is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.ClearMarks(...)`. Removes all the marks from a record.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean ClearMarks(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.ClearMarks is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.Consistent(...)`. Marks a table as being consistent or inconsistent.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean Consistent(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.Consistent is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.CopyFilter(...)`. Copies the filter that has been set for one field and
  /// applies it to another field.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean CopyFilter(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.CopyFilter is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.CopyFilters(...)`. Copies all the filters set by the SETFILTER method
  /// (Record) or the SETRANGE method (Record) from one record to another.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean CopyFilters(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.CopyFilters is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.CopyLinks(...)`. Copies all the links from a specified record.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean CopyLinks(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.CopyLinks is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.CountApprox(...)`. Returns an approximate count of the number of records in
  /// the table, for example, for updating progress bars or displaying informational messages.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean CountApprox(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.CountApprox is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.CurrentCompany(...)`. Gets the current company of a database table record.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean CurrentCompany(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.CurrentCompany is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.CurrentKey(...)`. Gets the current key of a database table.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean CurrentKey(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.CurrentKey is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.DeleteLink(...)`. Deletes a specified link from a record in a table.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean DeleteLink(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.DeleteLink is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.DeleteLinks(...)`. Deletes all of the links that have been added to a
  /// record.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean DeleteLinks(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.DeleteLinks is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.FieldActive(...)`. Checks whether a field is enabled.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean FieldActive(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.FieldActive is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.FieldName(...)`. Gets the name of a field as a string.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean FieldName(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.FieldName is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.FilterGroup(...)`. Gets or sets the filter group that is applied to a table.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean FilterGroup(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.FilterGroup is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.Find(...)`. Finds a record in a table that is based on the values stored in
  /// keys.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean Find(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.Find is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.FindFirst(...)`. Finds the first record in a table based on the current key
  /// and filter.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean FindFirst(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.FindFirst is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.FindLast(...)`. Finds the last record in a table based on the current key
  /// and filter.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean FindLast(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.FindLast is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.FullyQualifiedName(...)`. Gets the fully qualified name of a table.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean FullyQualifiedName(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.FullyQualifiedName is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.GetAscending(...)`. Gets the sort order for the records returned. You can
  /// use GETASCENDING to identify the sort order of the specified field because fields can be
  /// sorted in ascending or descending order. For example, you can read data from an ODATA web
  /// service where the data is sorted in ascending order on the Name field but in descending order
  /// on the City field.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean GetAscending(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.GetAscending is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.GetBySystemId(...)`. Gets a record by its SystemId.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean GetBySystemId(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.GetBySystemId is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.GetFilter(...)`. Gets a list of the filters within the current filter group
  /// that are applied to a field.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean GetFilter(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.GetFilter is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.GetFilters(...)`. Gets a string that contains a list of the filters within
  /// the current filter group for all fields in a record. In addition, this method also returns the
  /// state of the MARKEDONLY method (Record).
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean GetFilters(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.GetFilters is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.GetPosition(...)`. Gets a string that contains the primary key of the
  /// current record.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean GetPosition(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.GetPosition is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.GetRangeMax(...)`. Gets the maximum value in a range for a field.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean GetRangeMax(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.GetRangeMax is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.GetRangeMin(...)`. Gets the minimum value in a range for a field.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean GetRangeMin(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.GetRangeMin is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.GetView(...)`. Gets a string that describes the current sort order, key, and
  /// filters on a table.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean GetView(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.GetView is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.HasFilter(...)`. Determines whether a filter is attached to a record within
  /// the current filter group.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean HasFilter(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.HasFilter is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.HasLinks(...)`. Determines whether a record contains any links.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean HasLinks(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.HasLinks is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.Init(...)`. Initializes a record in a table.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean Init(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.Init is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.IsTemporary(...)`. Determines whether a record refers to a temporary table.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean IsTemporary(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.IsTemporary is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.LoadFields(...)`. Accesses the table's corresponding data source and loads
  /// the values of the specified fields on the record.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean LoadFields(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.LoadFields is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.LockTable(...)`. Starts locking on a table to protect it from write
  /// transactions that conflict with each other.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean LockTable(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.LockTable is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.Mark(...)`. Marks a record. You can also use this method to determine
  /// whether a record is marked.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean Mark(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.Mark is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.MarkedOnly(...)`. Activates a special filter. After you use this function,
  /// your view of the table includes only records marked by the Mark (Record) method.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean MarkedOnly(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.MarkedOnly is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.ModifyAll(...)`. Modifies a field in all records within a range that you
  /// specify.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean ModifyAll(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.ModifyAll is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.ReadConsistency(...)`. Determines if the table supports read consistency.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean ReadConsistency(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.ReadConsistency is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.ReadIsolation(...)`. Gets or sets the read isolation level.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean ReadIsolation(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.ReadIsolation is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.ReadPermission(...)`. Determines whether a user is granted read permission
  /// to the table that contains a record. This method can test for both full read permission and
  /// partial read permission that has been granted with a security filter.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean ReadPermission(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.ReadPermission is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.RecordId(...)`. Gets the RecordId of the record that is currently selected
  /// in the table. If no table is selected, an error is generated.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean RecordId(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.RecordId is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.RecordLevelLocking(...)`. Determines whether the table supports record-level
  /// locking.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean RecordLevelLocking(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.RecordLevelLocking is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.Relation(...)`. Determines the table relationship of a given field.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean Relation(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.Relation is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.Rename(...)`. Changes the value of a primary key in a table.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean Rename(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.Rename is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.Reset(...)`. Removes all filters, including any special filters set by
  /// MarkedOnly, changes fields select for loading back to all, sets the read isolation level to
  /// the default value, and changes the current key to the primary key. Also removes any marks on
  /// the record and clears any AL variables defined on its table definition.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean Reset(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.Reset is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.SecurityFiltering(...)`. Gets or sets how security filters are applied to
  /// the record.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean SecurityFiltering(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.SecurityFiltering is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.SetAscending(...)`. Sets the sort order for the records returned. Use this
  /// method after you have set the keys to sort after, using SETCURRENTKEY. The default sort order
  /// is ascending. You can use SETASCENDING to change the sort order to descending for a specific
  /// field, while the other fields in the specified key are sorted in ascending order.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean SetAscending(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.SetAscending is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.SetAutoCalcFields(...)`. Sets the FlowFields that you specify to be
  /// automatically calculated when the record is retrieved from the database.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean SetAutoCalcFields(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.SetAutoCalcFields is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.SetBaseLoadFields(...)`. Sets that only fields for the base table to be
  /// initially loaded when the record is retrieved from its data source. This will overwrite fields
  /// previously selected for initial load.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean SetBaseLoadFields(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.SetBaseLoadFields is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.SetCurrentKey(...)`. Selects a key for a table.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean SetCurrentKey(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.SetCurrentKey is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.SetFilter(...)`. Assigns a filter to a field that you specify.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean SetFilter(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.SetFilter is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.SetLoadFields(...)`. Sets the fields to be initially loaded when the record
  /// is retrieved from its data source. This will overwrite fields previously selected for initial
  /// load.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean SetLoadFields(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.SetLoadFields is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.SetPermissionFilter(...)`. Applies the user's security filter.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean SetPermissionFilter(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.SetPermissionFilter is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.SetPosition(...)`. Sets the fields in a primary key on a record to the
  /// values specified in the supplied string. The remaining fields are not changed.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean SetPosition(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.SetPosition is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.SetRange(...)`. Sets a simple filter, such as a single range or a single
  /// value, on a field.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean SetRange(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.SetRange is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.SetRecFilter(...)`. Sets the values in the current key of the current record
  /// as a record filter.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean SetRecFilter(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.SetRecFilter is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.SetView(...)`. Sets the current sort order, key, and filters on a table.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean SetView(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.SetView is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.TableCaption(...)`. Gets the current caption of a table as a string.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean TableCaption(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.TableCaption is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.TableName(...)`. Gets the name of a table.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean TableName(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.TableName is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.TransferFields(...)`. Copies all matching fields in one record to another
  /// record.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean TransferFields(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.TransferFields is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.Truncate(...)`. Deletes all records in a table that fall within a specified
  /// range, in an efficient maner. Keep in mind that Truncate allows for less concurrency than
  /// DeleteAll, as the entire table will be locked until the transaction is committed.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean Truncate(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.Truncate is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.Validate(...)`. Calls the OnValidate trigger for the field that you specify.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean Validate(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.Validate is declared and not implemented yet (board:0035)");
  }

  /// \brief AL `Record.WritePermission(...)`. Determines whether a user can write to a table. This
  /// method can test for both full write permission and partial write permission that has been
  /// granted with a security filter. A write permission consists of Insert, Delete, and Modify
  /// permissions.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \return Never.
  /// \throws Error always -- the name is declared, the behaviour is not (board:0035).
  template <typename... Arguments> Boolean WritePermission(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Record.WritePermission is declared and not implemented yet (board:0035)");
  }

private:
  friend Derived;

  /// The AL field number of a member of this record, found by where it sits.
  [[nodiscard]] ::agiru::FieldNo NumberOf(const void *member) const {
    const auto offset = static_cast<std::size_t>(static_cast<const std::byte *>(member) -
                                                 static_cast<const std::byte *>(Self()));
    const FieldDef *def = FieldAtOffset(TableTraits<Derived>::kTable, offset);
    if (def == nullptr) { throw Error("this record declares no field at that position"); }
    return def->no;
  }

  /// \note THE CRTP PAIRING IS CHECKED HERE AND NOT BY A PRIVATE CONSTRUCTOR. A private default
  ///       constructor with `friend Derived` is the usual guard, and it makes the generated class a
  ///       NON-INITIALISABLE AGGREGATE: a table has no user-declared constructor, so `X Rec{}` is
  ///       aggregate initialisation, which initialises this base from the CALLER's context and is
  ///       refused there. Every page and every local record hit it. The assertion below catches the
  ///       same misuse -- `class Wrong : public Table<Right>` -- the moment any method is used, and
  ///       costs nothing at run time.
  [[nodiscard]] const void *Self() const {
    static_assert(std::is_base_of_v<Table, Derived>, "a table must derive from Table of itself");
    return static_cast<const Derived *>(this);
  }

protected:
  /// \brief Writes the primary key fields from the values AL's `Get` was handed.
  ///
  /// \tparam Keys The key field types, in key order.
  /// \param  keys The primary key values.
  /// \throws Error when the argument count does not match the primary key.
  ///
  /// \note Shared with the temporary store, which assigns the key the same way and then searches
  ///       its own rows rather than the database. Doing it twice was the alternative.
  template <typename... Keys> void AssignPrimaryKey(const Keys &...keys) {
    const TableDef &table = TableTraits<Derived>::kTable;
    if (table.keys.empty() || table.keys[0].fields.size() != sizeof...(Keys)) {
      throw Error("Get: the argument count does not match the primary key");
    }
    std::size_t position = 0;
    (AssignKey(table, position++, keys), ...);
  }

private:
  [[nodiscard]] void *Self() { return static_cast<Derived *>(this); }

  template <typename Key>
  void AssignKey(const TableDef &table, std::size_t position, const Key &value) {
    const FieldDef *def = Field(table, table.keys[0].fields[position]);
    if (def == nullptr) { throw Error("Get: the primary key names a field the table lacks"); }
    *reinterpret_cast<Key *>(static_cast<std::byte *>(Self()) + def->offset) = value;
  }
};

/// \brief The rows a temporary record holds, and how often they changed.
///
/// \tparam T The generated table class.
///
/// \note THE VERSION IS NOT BOOKKEEPING, IT IS THE HOT PATH. A `repeat ... until Next() = 0` loop
///       over a temporary buffer re-filters and re-sorts the whole store on every step unless a
///       cached view can tell it is still valid. The predecessor measured that as its O(n^2) case
///       and put the counter on the STORE for a second reason: AL `Copy(src, true)` makes two
///       record variables share one store, and each must see the other's mutations.
template <typename T> struct TempStore {
  std::vector<T> rows;      ///< In primary-key order, which is what AL walks.
  std::uint64_t version{0}; ///< Rises on every structural change.
  std::size_t held{0};      ///< How many records share it.
};

/// \brief AL `Record "X" temporary` -- the same table with no database behind it.
///
/// \tparam T The generated table class.
///
/// From `SetTemporary` and the `temporary` keyword: the record keeps its fields, its keys and its
/// filters, and its rows live in memory for the length of the session. AL code cannot tell the
/// difference, which is the point -- a buffer table and a real one are written the same way.
///
/// \note The rows are held SORTED BY PRIMARY KEY, because that is the order AL walks a record in
///       and the order `Get` searches. Inserting into the middle of a vector is what a buffer table
///       does rarely and reads often.
template <typename T> class Temporary : public T {
public:
  /// \brief A temporary record with a store of its own.
  Temporary() : store_(new TempStore<T>{.rows = {}, .version = 0, .held = 1}) {}

  /// \brief Copies a record, and with it whichever store that record was on.
  /// \param o The record to copy.
  Temporary(const Temporary &o) : T(o), store_(o.store_), position_(o.position_) { ++store_->held; }

  /// \brief Takes over a record's store.
  /// \param o The record to move from.
  Temporary(Temporary &&o) noexcept
      : T(std::move(static_cast<T &>(o))), store_(o.store_), position_(o.position_) {
    o.store_ = nullptr;
  }

  /// \brief Assigns a record, and with it whichever store that record is on.
  /// \param o The record to copy.
  /// \return This record.
  Temporary &operator=(const Temporary &o) {
    if (this == &o) { return *this; }
    T::operator=(o);
    Release();
    store_ = o.store_;
    position_ = o.position_;
    ++store_->held;
    return *this;
  }

  /// \brief Takes over a record's store.
  /// \param o The record to move from.
  /// \return This record.
  Temporary &operator=(Temporary &&o) noexcept {
    if (this == &o) { return *this; }
    T::operator=(std::move(static_cast<T &>(o)));
    Release();
    store_ = o.store_;
    position_ = o.position_;
    o.store_ = nullptr;
    return *this;
  }

  ~Temporary() { Release(); }

  /// \brief AL `Record.Insert()` on a temporary record.
  /// \throws Error when a row already carries this primary key, as AL does.
  void Insert() {
    const auto at = LowerBound();
    if (at != store_->rows.end() && detail::SameKey<T>(*at, *this)) {
      throw Error("the record already exists");
    }
    store_->rows.insert(at, static_cast<const T &>(*this));
    ++store_->version;
  }

  /// \brief AL `Record.Get(...)` on a temporary record.
  /// \tparam Keys The key field types, in key order.
  /// \param  keys The primary key values.
  /// \return True when a row carried that key; the record is then that row.
  template <typename... Keys> bool Get(const Keys &...keys) {
    this->AssignPrimaryKey(keys...);
    const auto at = LowerBound();
    if (at == store_->rows.end() || !detail::SameKey<T>(*at, *this)) { return false; }
    static_cast<T &>(*this) = *at;
    return true;
  }

  /// \brief AL `Record.Modify()` on a temporary record.
  /// \return True when a row carried this primary key.
  bool Modify() {
    const auto at = LowerBound();
    if (at == store_->rows.end() || !detail::SameKey<T>(*at, *this)) { return false; }
    *at = static_cast<const T &>(*this);
    ++store_->version;
    return true;
  }

  /// \brief AL `Record.Delete()` on a temporary record.
  /// \return True when a row carried this primary key.
  bool Delete() {
    const auto at = LowerBound();
    if (at == store_->rows.end() || !detail::SameKey<T>(*at, *this)) { return false; }
    store_->rows.erase(at);
    ++store_->version;
    return true;
  }

  /// \brief AL `Record.DeleteAll()` on a temporary record.
  void DeleteAll() {
    store_->rows.clear();
    ++store_->version;
  }

  /// \brief AL `Record.Count()`.
  /// \return How many rows the store holds.
  [[nodiscard]] Integer Count() const { return static_cast<Integer>(store_->rows.size()); }

  /// \brief AL `Record.IsEmpty()`.
  /// \return True when the store holds no rows.
  [[nodiscard]] bool IsEmpty() const { return store_->rows.empty(); }

  /// \brief AL `Record.FindSet()` -- positions on the first row.
  /// \return True when there is one, which is then this record.
  bool FindSet() {
    position_ = 0;
    return Fetch();
  }

  /// \brief AL `Record.Next()` -- steps to the row after this one.
  /// \return True when there was one, which is then this record.
  bool Next() {
    ++position_;
    return Fetch();
  }

  /// \brief AL `Record.Copy(From, true)` -- shares another temporary record's store.
  ///
  /// \param from  The record to share with.
  /// \param share True to share the STORE; false copies the current row only.
  ///
  /// \note Sharing is why the version rides on the store rather than on the record: after this,
  ///       two variables mutate one set of rows and each must see the other's changes.
  void Copy(const Temporary &from, bool share) {
    static_cast<T &>(*this) = static_cast<const T &>(from);
    if (!share || store_ == from.store_) { return; }
    Release();
    store_ = from.store_;
    ++store_->held;
  }

private:
  [[nodiscard]] auto LowerBound() {
    return std::ranges::lower_bound(store_->rows, static_cast<const T &>(*this), detail::ByKey<T>);
  }

  bool Fetch() {
    if (position_ >= store_->rows.size()) { return false; }
    static_cast<T &>(*this) = store_->rows[position_];
    return true;
  }

  /// A COUNT RATHER THAN A `shared_ptr`, AND THE REASON IS THE DOOR'S SIZE. `<memory>` pulls
  /// `<format>` with it in libstdc++-14 -- 143 000 preprocessed lines together -- and the door is
  /// included by all 6 398 generated files. Measured 2026-09-02: dropping the runtime half of the
  /// door takes an empty translation unit from 306 ms to 53 ms. Twenty lines of counting buy that
  /// back, and the temporary-record gate's checks on sharing are what stand behind them.
  void Release() {
    if (store_ != nullptr && --store_->held == 0) { delete store_; }
    store_ = nullptr;
  }

  TempStore<T> *store_;
  std::size_t position_{0};
};

} // namespace agiru
