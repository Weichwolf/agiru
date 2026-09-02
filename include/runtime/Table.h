#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
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
    ::agiru::detail::TestField(Self(), TableTraits<Derived>::kTable, NumberOf(&member));
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
  void TestField(FieldNo no) const {
    ::agiru::detail::TestField(Self(), TableTraits<Derived>::kTable, no);
  }

  /// \brief AL `Record.TestField(Field, Value)`.
  /// \tparam Value The field's own type.
  /// \param no       The field to test.
  /// \param expected The value it must hold.
  /// \throws Error when the values differ.
  template <typename Value>
    requires(!std::is_enum_v<Value>)
  void TestField(FieldNo no, const Value &expected) const {
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
  void TestField(FieldNo no, E expected) const {
    ::agiru::TestFieldValue(Self(), TableTraits<Derived>::kTable, no, Option<E>{expected});
  }

  /// \brief AL `Record.FieldCaption(Field)`.
  /// \param no The field.
  /// \return The field's `Caption` property.
  [[nodiscard]] std::string_view FieldCaption(FieldNo no) const {
    return ::agiru::FieldCaption(TableTraits<Derived>::kTable, no);
  }

private:
  friend Derived;

  /// The AL field number of a member of this record, found by where it sits.
  [[nodiscard]] FieldNo NumberOf(const void *member) const {
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
