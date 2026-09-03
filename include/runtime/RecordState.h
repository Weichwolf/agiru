#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"

#include <cstddef>
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

struct RecordState {
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
  StateHandle &operator=(const StateHandle &o) {
    if (this != &o) {
      StateHandle copy(o);
      Swap(copy);
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
