#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "type/IsolationLevel.h"
#include "type/SecurityFilter.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

/// \file
/// \brief What belongs to a record VARIABLE rather than to the row it holds.

namespace agiru::detail {

/// \brief One field's filter, as the record variable carries it.
struct FieldFilter {
  ::agiru::FieldNo field; ///< The field it narrows.
  int group;              ///< The filter group it was set in; -1 is the one that ORs.
  std::string text;       ///< The filter expression, in AL's own language.
};

/// \brief One field of the sort order.
struct SortField {
  ::agiru::FieldNo field; ///< The field.
  bool ascending;         ///< Which way.
};

/// \brief Everything a record variable holds that is not a field value.
///
/// \note IT IS ONE LIST TAGGED BY GROUP AND NEVER A LIST PER GROUP. The predecessor kept a second
///       store for the groups and `HasFilter` went blind to it (openerp WI-1063). Every group is
///       active at once, ANDed, and group -1 is the single exception whose own fields OR together.
///
/// \note IT TRAVELS WITH `Copy` AND DIES WITH THE VARIABLE. `Rec2.Copy(Rec)` takes the filters with
///       it because they are the variable's own state, which is AL's rule.
/// \brief A cursor the runtime holds open. Defined in `src/rt`, opaque to the door.
struct OpenCursor;

/// \brief Frees a cursor `FindSet` opened.
/// \param open The cursor, which may be null.
void Close(OpenCursor *open);

/// \brief The open cursor a record's state owns.
///
/// \note A SERVER-SIDE CURSOR AND NOT A RESULT SET. `FindSet` on a table with a hundred million
///       rows must not put the rows in the session -- SQL Server declares a cursor and BC is
///       written against that, so this does the same over PostgreSQL. The type is opaque here
///       because the door may not name libpq: `runtime/Table.h` is parsed by every one of the
///       generated translation units.
///
/// \note A COPY HOLDS NOTHING, which is `Rec2 := Rec` in AL: two record variables are two
///       positions, and a shared cursor would step both at once. The copy carries the filters and
///       the key -- which the state around this does copy -- and finds its own set.
class CursorHandle {
public:
  /// \brief No cursor.
  CursorHandle() = default;

  /// \brief A copy holds no cursor of its own.
  /// \param o The other, whose cursor is not shared.
  CursorHandle(const CursorHandle &o) { static_cast<void>(o); }

  /// \brief Takes the other's cursor.
  /// \param o The other.
  CursorHandle(CursorHandle &&o) noexcept : open_(o.open_) { o.open_ = nullptr; }

  /// \brief Lets go of this one's cursor; the other's is not shared.
  /// \param o The other, whose cursor is not shared.
  /// \return This handle.
  CursorHandle &operator=(const CursorHandle &o) {
    if (this != &o) { Forget(); }
    return *this;
  }

  /// \brief Takes the other's cursor.
  /// \param o The other.
  /// \return This handle.
  CursorHandle &operator=(CursorHandle &&o) noexcept {
    if (this != &o) {
      Forget();
      open_ = o.open_;
      o.open_ = nullptr;
    }
    return *this;
  }

  /// \brief Closes the cursor, if one is open.
  ~CursorHandle() { Forget(); }

  /// \brief Takes ownership of a cursor, closing whatever stood here.
  /// \param open The cursor.
  void Hold(OpenCursor *open) {
    Forget();
    open_ = open;
  }

  /// \brief The cursor, or `nullptr`.
  [[nodiscard]] OpenCursor *Held() const { return open_; }

  /// \brief Closes the cursor, if one is open.
  void Forget() {
    Close(open_);
    open_ = nullptr;
  }

private:
  OpenCursor *open_ = nullptr;
};

/// \brief What the runtime can do with a temporary record's rows without knowing their type.
///
/// The rows are the generated table class, held in a `std::vector` the door instantiates per
/// table (`kTempOps<T>` in `runtime/Table.h`); the runtime sorts, filters and positions over
/// `const void *` rows through `CompareField` and `FieldText`, the way it already compares two
/// records of one table (board:0583).
struct TempOps {
  void *(*make)();                                                 ///< A new, empty row set.
  void (*destroy)(void *rows);                                     ///< Frees one.
  std::size_t (*count)(const void *rows);                          ///< How many rows.
  const void *(*at)(const void *rows, std::size_t index);          ///< The row at a position.
  void (*insert)(void *rows, std::size_t at, const void *record);  ///< Copies the record in.
  void (*replace)(void *rows, std::size_t at, const void *record); ///< Overwrites a row.
  void (*erase)(void *rows, std::size_t at);                       ///< Removes a row.
  void (*clear)(void *rows);                                       ///< Removes every row.
  void (*load)(void *record, const void *row);                     ///< Copies a row's FIELDS out.
};

/// \brief The rows a temporary record holds, shared by every variable that `Copy(From, true)`d
///        them, and how often they changed.
struct TempTable {
  const TempOps *ops;    ///< How to reach the rows.
  void *rows;            ///< The rows, owned here.
  std::uint64_t version; ///< Rises on every structural change, so a walk can notice.
  std::size_t held;      ///< How many records share it; the last one frees it.

  TempTable(const TempOps *ops_, void *rows_) : ops(ops_), rows(rows_), version(0), held(0) {}

  TempTable(const TempTable &) = delete;
  TempTable(TempTable &&) = delete;
  TempTable &operator=(const TempTable &) = delete;
  TempTable &operator=(TempTable &&) = delete;

  ~TempTable() { ops->destroy(rows); }
};

/// \brief A counted reference to a `TempTable`: copying shares the rows, the last holder frees
///        them. Intrusive rather than a shared pointer because `<memory>` pulls `<format>` into
///        every generated translation unit (measured 2026-09-06: ~1 s per file, board:0589).
class TempHandle {
public:
  TempHandle() = default;

  explicit TempHandle(TempTable *table) : table_(table) { Acquire(); }

  TempHandle(const TempHandle &o) : table_(o.table_) { Acquire(); }

  TempHandle(TempHandle &&o) noexcept : table_(o.table_) { o.table_ = nullptr; }

  TempHandle &operator=(const TempHandle &o) {
    if (this != &o) {
      Release();
      table_ = o.table_;
      Acquire();
    }
    return *this;
  }

  TempHandle &operator=(TempHandle &&o) noexcept {
    if (this != &o) {
      Release();
      table_ = o.table_;
      o.table_ = nullptr;
    }
    return *this;
  }

  ~TempHandle() { Release(); }

  /// \return The table, or null.
  [[nodiscard]] TempTable *get() const { return table_; }

  /// \return Whether a table is held.
  [[nodiscard]] explicit operator bool() const { return table_ != nullptr; }

  /// \param o Null.
  /// \return Whether none is held.
  [[nodiscard]] bool operator==(std::nullptr_t o) const { return table_ == o; }

private:
  void Acquire() {
    if (table_ != nullptr) { ++table_->held; }
  }

  void Release() {
    if (table_ != nullptr && --table_->held == 0) { delete table_; }
    table_ = nullptr;
  }

  TempTable *table_ = nullptr;
};

struct RecordState {
  /// \brief The temporary rows, when the record is `temporary`; null for a database record.
  ///        Temporariness is STATE and never type: a `Temporary<T>` installs it, and a `T &`
  ///        parameter bound to one keeps behaving as one (board:0583).
  TempHandle temporary;
  std::vector<std::size_t> view;        ///< The rows a `Find` selected, sorted, by index.
  std::size_t at = 0;                   ///< Where in `view` the record stands.
  std::uint64_t viewVersion = 0;        ///< The `TempTable::version` the view was built at.
  std::vector<FieldFilter> viewFilters; ///< The filters that built it -- a walk keeps its own.
  std::vector<SortField> viewKey;       ///< And the key.
  bool viewAscending = true;            ///< And the direction.

  std::vector<FieldFilter> filters; ///< AND across fields and groups.
  std::vector<SortField> key;       ///< `SetCurrentKey`; empty means the primary key.
  bool ascending = true;            ///< `Ascending()`, over the whole key.
  int group = 0;                    ///< The group `SetRange` and `SetFilter` write into.

  /// \brief The primary keys `Mark(true)` set, for this VARIABLE and no other.
  std::vector<std::string> marks;
  bool markedOnly = false; ///< `MarkedOnly(true)`.

  /// \brief The cursor `FindSet` opened, if one is open.
  CursorHandle open;

  /// \brief How many rows `Next` has stepped since `FindSet`.
  std::size_t stepped = 0;

  bool positioned = false; ///< Whether a `Find` put it anywhere.

  IsolationLevel isolation = IsolationLevel::Default; ///< `ReadIsolation`, carried (board:0012).
  SecurityFilter securityFiltering =
      SecurityFilter::Validated; ///< `SecurityFiltering`, carried (board:0313).
};

/// \brief The record variable's state, owned, copied and freed with the record.
///
/// \note IT IS THE RECORD'S FIRST DATA MEMBER, and that is what lets `Table<Derived>` reach it
///       without the generator emitting an accessor: a standard-layout object's address IS its
///       first member's address, which the language guarantees rather than a layout guess. The
///       generator asserts the offset is zero beside every table (board:0018).
///
/// \note IT IS NULL UNTIL SOMETHING FILTERS. A record that never filters costs eight bytes and no
///       allocation, which is what makes this affordable on 1 609 tables.
class StateHandle {
public:
  /// \brief A record that has not filtered.
  StateHandle() = default;

  /// \brief Copies the state, because `Rec2 := Rec` copies the variable's filters too.
  /// \param o The other.
  StateHandle(const StateHandle &o)
      : state_(o.state_ == nullptr ? nullptr : new RecordState(*o.state_)) {}

  /// \brief Takes the other's state.
  /// \param o The other.
  StateHandle(StateHandle &&o) noexcept : state_(o.state_) { o.state_ = nullptr; }

  /// \brief Copies the state, letting go of this one's.
  /// \param o The other.
  /// \return This handle.
  /// \brief AL `Rec := Other`: the filters and the position come across, the ROWS do not. A
  ///        temporary record assigned from another keeps its own rows, and a database record
  ///        assigned from a temporary one stays a database record; only `Copy(From, true)`
  ///        shares (`record-copy-method.md`).
  StateHandle &operator=(const StateHandle &o) {
    if (this != &o) {
      TempHandle keep = state_ == nullptr ? TempHandle{} : state_->temporary;
      StateHandle copy(o);
      Swap(copy);
      if (state_ != nullptr || keep != nullptr) {
        RecordState &mine = Ensure();
        mine.temporary = std::move(keep);
        mine.view.clear();
        mine.positioned = false;
      }
    }
    return *this;
  }

  /// \brief Takes the other's state, letting go of this one's.
  /// \param o The other.
  /// \return This handle.
  StateHandle &operator=(StateHandle &&o) noexcept {
    if (this != &o) { Swap(o); }
    return *this;
  }

  ~StateHandle() { delete state_; }

  /// \brief The state, made on the first call.
  /// \return It.
  RecordState &Ensure() {
    if (state_ == nullptr) { state_ = new RecordState(); }
    return *state_;
  }

  /// \brief The state, or nothing when the record has never filtered.
  /// \return It, or `nullptr`.
  [[nodiscard]] const RecordState *Peek() const { return state_; }

  /// \brief Lets go of everything the variable held, which is what `Reset` does.
  void Forget() {
    delete state_;
    state_ = nullptr;
  }

private:
  void Swap(StateHandle &o) noexcept {
    RecordState *mine = state_;
    state_ = o.state_;
    o.state_ = mine;
  }

  RecordState *state_ = nullptr;
};

/// \brief Puts a filter on one field, replacing whatever that field carried.
///
/// \param state The record variable's state.
/// \param field The field.
/// \param text  The filter expression, or empty to clear it.
///
/// \note AN EMPTY EXPRESSION CLEARS AND DOES NOT FILTER FOR THE EMPTY VALUE, which is what
///       `record-setfilter-method.md` says of `SetFilter` and what `SetRange(Field)` means.
void Narrow(RecordState &state, ::agiru::FieldNo field, const std::string &text);

/// \brief A value as a filter expression that means exactly itself.
///
/// \param value The value, already rendered as text.
/// \return It, with anything the filter language would read as an operator quoted.
///
/// \note `SetRange(No, 'A..B')` FILTERS FOR THAT LITERAL CODE and not for a range: the value came
///       from a variable and AL never re-reads it as an expression. Quoting is what says so.
[[nodiscard]] std::string Literally(const std::string &value);

/// \brief Whether the table declares a key those fields select.
///
/// \param table The table.
/// \param key   The fields, in order.
/// \return True when a declared key matches exactly or by PREFIX.
///
/// \note A KEY THAT MATCHES NOTHING STILL SORTS. `record-setcurrentkey-method.md` is explicit: the
///       order is applied either way, and what a match buys is an index.
[[nodiscard]] bool KeyMatches(const TableDef &table, const std::vector<SortField> &key);

}
