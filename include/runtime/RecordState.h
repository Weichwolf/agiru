#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"

#include <cstddef>
#include <string>
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
struct RecordState {
  std::vector<FieldFilter> filters; ///< AND across fields and groups.
  std::vector<SortField> key;       ///< `SetCurrentKey`; empty means the primary key.
  bool ascending = true;            ///< `Ascending()`, over the whole key.
  int group = 0;                    ///< The group `SetRange` and `SetFilter` write into.

  /// \brief The primary keys `Mark(true)` set, for this VARIABLE and no other.
  std::vector<std::string> marks;
  bool markedOnly = false; ///< `MarkedOnly(true)`.

  /// \brief What the last `FindSet` read, and where `Next` stands in it.
  ///
  /// \note THE WHOLE SET, because `FindSet` requests all matching rows in one query --
  ///       `record-findset-boolean-method.md` says so, and it is what separates it from `Find`,
  ///       which pages. A set too large for memory is the scaling gap the predecessor never closed
  ///       either (openerp WI-889), and it is named rather than pretended away.
  std::vector<std::string> rows;
  std::size_t cursor = 0;  ///< Where `Next` stands.
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
