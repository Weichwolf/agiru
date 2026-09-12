#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "type/IsolationLevel.h"
#include "type/SecurityFilter.h"

#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/// \file
/// \brief What belongs to a record VARIABLE rather than to the row it holds.

namespace agiru::detail {

/// \brief What a record variable owns as its `xRec` -- a record of its own table, type erased.
///
/// \note IT OWNS AND IT CLONES. The state is copied whenever the record variable is
///       (`Rec2 := Rec` takes the filters, and the image with them), so a shared pointer would
///       give two variables one image and a write through one would be seen by the other. AL's
///       `xRec` belongs to the variable.
class HeldImage {
public:
  /// \brief No image, which is what a record nobody has touched carries.
  HeldImage() = default;

  /// \brief Clones the other's image.
  /// \param o The other.
  HeldImage(const HeldImage &o) { Take(o); }

  /// \brief Takes the other's image.
  /// \param o The other.
  HeldImage(HeldImage &&o) noexcept : record_(o.record_), free_(o.free_), clone_(o.clone_) {
    o.record_ = nullptr;
  }

  /// \brief Clones the other's image, letting go of this one's.
  /// \param o The other.
  /// \return This.
  HeldImage &operator=(const HeldImage &o) {
    if (this != &o) {
      Reset();
      Take(o);
    }
    return *this;
  }

  /// \brief Takes the other's image, letting go of this one's.
  /// \param o The other.
  /// \return This.
  HeldImage &operator=(HeldImage &&o) noexcept {
    if (this != &o) {
      Reset();
      record_ = o.record_;
      free_ = o.free_;
      clone_ = o.clone_;
      o.record_ = nullptr;
    }
    return *this;
  }

  /// \brief Lets go of the image.
  ~HeldImage() { Reset(); }

  /// \brief Takes ownership of a record as this variable's image.
  /// \param record The record, which this now owns.
  /// \param free   How to unmake one.
  /// \param clone  How to copy one.
  void Hold(void *record, void (*free)(void *), void *(*clone)(const void *)) {
    Reset();
    record_ = record;
    free_ = free;
    clone_ = clone;
  }

  /// \return The image, or `nullptr` when there is none.
  [[nodiscard]] void *Get() const { return record_; }

private:
  void Take(const HeldImage &o) {
    if (o.record_ == nullptr || o.clone_ == nullptr) { return; }
    record_ = o.clone_(o.record_);
    free_ = o.free_;
    clone_ = o.clone_;
  }

  void Reset() {
    if (record_ != nullptr && free_ != nullptr) { free_(record_); }
    record_ = nullptr;
  }

  void *record_ = nullptr;
  void (*free_)(void *) = nullptr;
  void *(*clone_)(const void *) = nullptr;
};

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
  const TempOps *ops;       ///< How to reach the rows.
  void *rows;               ///< The rows, owned here.
  std::uint64_t version{0}; ///< Rises on every structural change, so a walk can notice.
  std::size_t held{0};      ///< How many records share it; the last one frees it.

  TempTable(const TempOps *ops_, void *rows_) : ops(ops_), rows(rows_) {}

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

  std::vector<FieldFilter> filters;       ///< AND across fields and groups.
  std::vector<::agiru::FieldNo> autoCalc; ///< `SetAutoCalcFields`: calculated after every read.
  std::vector<SortField> key;             ///< `SetCurrentKey`; empty means the primary key.
  bool ascending = true;                  ///< `Ascending()`, over the whole key.
  int group = 0;                          ///< The group `SetRange` and `SetFilter` write into.

  /// \brief The primary keys `Mark(true)` set, for this VARIABLE and no other.
  ///
  /// \note A SET AND NOT A VECTOR, and that is the shape rather than the container's convenience.
  ///       `record-mark-method.md` describes marks as a SET the variable carries: marking twice
  ///       marks once, and the question asked of it is only ever "is this key in it". A vector
  ///       answered that with a linear walk written out by hand, because `<algorithm>` may not
  ///       enter the door.
  std::set<std::string> marks;
  bool markedOnly = false; ///< `MarkedOnly(true)`.
  std::size_t viewMarks = 0; ///< How many marks the temporary view was built over (\see view).

  /// \brief AL `xRec` -- the record as it was last READ, INSERTED or MODIFIED.
  ///
  /// \note IT IS THE RECORD'S OWN STORED IMAGE AND NOT A TRIGGER-SCOPED HAND-IN, which is what
  ///       openerp WI-1078 settles: the BaseApp reads `xRec` in ordinary table PROCEDURES too.
  ///       `ProdOrderComponent.UpdateBin` writes `Comp2 := Comp; Comp2.GetDefaultBin()`, and
  ///       `GetDefaultBin` exits when quantity, item, location, variant and routing link all match
  ///       `xRec` -- dead code unless the image belongs to the record (board:0042).
  ///
  /// \note IT LIVES IN THE STATE AND NOT IN THE GENERATED CLASS, which is where board:0042 put it.
  ///       The state is already what every other per-VARIABLE fact lives in -- filters, marks, the
  ///       cursor -- it is already copied by `Copy` and by assignment, and putting the image here
  ///       changes no generated class's layout, so the field table's `offsetof` values stand
  ///       untouched. The type is erased because `RecordState` knows no table; the two function
  ///       pointers carry it back.
  ///
  /// \note NO `std::shared_ptr` AND THEREFORE NO `<memory>`, and that is a measurement rather than
  ///       a preference: this header is in the door, and `<memory>` took the door's parse from
  ///       1.19 s to 1.71 s (min of 3, measured 2026-09-08) -- half a second on every one of the
  ///       6 939 generated translation units. CLAUDE.md names the same trap with the same header.
  ///       Three pointers and a hand-written copy do the same job for nothing.
  HeldImage image;

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
  /// \brief AL `Rec := Other`: the FIELDS come across and nothing of the state does -- not the
  ///        filters, not the key, not the position, not the rows.
  ///
  /// \warning ASSIGNMENT COPIES THE RECORD BUFFER AND `Copy` COPIES THE FILTERS TOO.
  ///          `record-copy-method.md` lists "filters, views, marks, fields, and keys" as what
  ///          `Copy` brings, and the predecessor's `:=` was "full field-value copy (fields only)"
  ///          at 2 260 green. Here the filters used to come across, and `Record Set Management`
  ///          walked into it: `TempFoundRecordSetTree := RecordSetTree; TempFound.Insert()` in a
  ///          loop gave the temporary buffer the database record's `FindNode` filters, so the
  ///          `FindFirst` over the buffer saw one node of ten (Record Set UT, 27 cases,
  ///          2026-09-10). The state that a `Copy` transports is `CopyStateFrom`.
  /// \param o The other, which is left alone.
  /// \return This handle, unchanged.
  StateHandle &operator=(const StateHandle &o) {
    static_cast<void>(o);
    return *this;
  }

  /// \brief AL `Rec.Copy(Other)`: the filters, the key and the position come across, the ROWS
  ///        do not. A temporary record keeps its own rows, and a database record copied from a
  ///        temporary one stays a database record; only `Copy(From, true)` shares
  ///        (`record-copy-method.md`).
  /// \param o The other.
  void CopyStateFrom(const StateHandle &o) {
    if (this == &o) { return; }
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
/// \note A BLANK VALUE IS THE FILTER `''` AND NOT NO FILTER. `SetRange(Code, '')` selects the
///       rows whose Code is empty and `SetRange("Starting Date", 0D)` the blank dates; rendering
///       the blank as an empty filter text made `Narrow` CLEAR the field instead, and the
///       number-series line search walked past its own line (20 UT cases, 2026-09-09).
[[nodiscard]] std::string Literally(std::string_view value);

/// \brief AL `Record.GetView(UseNames)`: the sort order, direction and the current group's
///        filters as one view string, `VERSION(1) SORTING(...) ORDER(...) WHERE(...)`.
/// \param state    The record's state, or `nullptr` for one that never filtered.
/// \param table    The declaration.
/// \param useNames Captions (the name where a field has none) when true, `Field<no>` when false.
/// \return The view, which `ApplyView` reads back.
[[nodiscard]] std::string ViewOf(const RecordState *state, const TableDef &table, bool useNames);

/// \brief AL `Record.CopyFilter(From, Other.To)`: every group's filter on one field, copied onto
///        a field of another record in the same group.
/// \param from        The source record's state, or `nullptr` when it never filtered.
/// \param source      The source field.
/// \param target      The record whose field receives the filters.
/// \param destination That record's field.
void RuntimeCopyFilter(const RecordState *from, FieldNo source, void *target, FieldNo destination);

/// \brief AL `Record.SetView(String)`: the sort order, direction and filters a view string sets.
/// \param state The record's state.
/// \param table The declaration.
/// \param view  The view, in the `SourceTableView` form; empty clears every filter and returns
///              to the primary key (`record-setview-method.md`).
/// \throws Error when a field is not the table's, or a clause is not one of the four.
void ApplyView(RecordState &state, const TableDef &table, std::string_view view);

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
