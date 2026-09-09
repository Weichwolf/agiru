#pragma once

#include "meta/Ids.h"
#include "meta/QueryDef.h"
#include "runtime/Error.h"
#include "runtime/Record.h"
#include "runtime/RecordState.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/SecurityFilter.h"
#include "type/Text.h"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

/// \file
/// \brief AL `Query`: a generated query's class derives from `Query<Derived>`, and the platform
///        object AL spells `QUERY` is `Query<void>` (board:0064).

namespace agiru {

namespace detail {

/// \brief What one query VARIABLE holds between calls: its column filters, its row limit and,
///        while it is open, its cursor.
struct QueryState;

/// \brief Makes an empty state.
/// \return The state, owned by the caller.
[[nodiscard]] QueryState *MakeQueryState();

/// \brief Frees a state, closing its cursor.
/// \param state The state, or nothing.
void FreeQueryState(QueryState *state) noexcept;

/// \brief Copies the filters and the limit and NOT the cursor, which is what `Q2 := Q1` gives AL.
/// \param state The state, or nothing.
/// \return The copy, or nothing.
[[nodiscard]] QueryState *CopyQueryState(const QueryState *state);

/// \brief AL `Query.Open()`: runs the statement and holds a cursor over its rows.
/// \param state The variable's state.
/// \param def   The query's declaration.
/// \return True; the statement's own refusal is an Error.
///
/// \warning THE READ STREAMS. The dataset is a server-side cursor fetched in blocks, the way
///          `FindSet` reads (board:0045); nothing here holds the result.
bool QueryOpen(QueryState &state, const QueryDef &def);

/// \brief AL `Query.Read()`: steps the cursor and writes the row's columns into the query.
/// \param state The variable's state.
/// \param def   The query's declaration.
/// \param self  The generated query object the columns are members of.
/// \return True when a row was read; false when the dataset is spent, which also closes it.
/// \throws Error when the query is not open.
bool QueryRead(QueryState &state, const QueryDef &def, void *self);

/// \brief AL `Query.Close()`.
/// \param state The variable's state.
void QueryClose(QueryState &state) noexcept;

/// \brief AL `Query.SetFilter`/`SetRange` on a column: replaces the filter standing on it.
/// \param state  The variable's state.
/// \param column The column, by position in `QueryDef::columns`.
/// \param text   The filter in AL's own language; empty removes it.
void QueryNarrow(QueryState &state, std::size_t column, const std::string &text);

/// \brief AL `Query.GetFilter(Column)`.
/// \param state  The variable's state.
/// \param column The column, by position.
/// \return The filter standing on it, or nothing.
[[nodiscard]] std::string QueryFilterOn(const QueryState &state, std::size_t column);

/// \brief AL `Query.GetFilters()`: every column filter as `Name: text`, comma-separated.
/// \param state The variable's state.
/// \param def   The query's declaration.
/// \return The text.
[[nodiscard]] std::string QueryFilters(const QueryState &state, const QueryDef &def);

/// \brief AL `Query.TopNumberOfRows(N)`.
/// \param state The variable's state.
/// \param rows  The limit; 0 lifts it.
/// \return The limit that stood before.
std::int32_t QueryTop(QueryState &state, std::int32_t rows);

/// \brief The state's owner, sitting FIRST in every generated query so the base reaches it.
///
/// \note THE SHAPE IS `StateHandle`'S: a generated query is standard-layout because its base
///       holds nothing, and `offsetof` on its columns stays a constant expression.
class QueryHandle {
public:
  /// \brief A query that has not filtered.
  QueryHandle() = default;

  /// \brief Copies the filters; the cursor stays with the original.
  /// \param o The other.
  QueryHandle(const QueryHandle &o) : state_(CopyQueryState(o.state_)) {}

  /// \brief Takes the other's state.
  /// \param o The other.
  QueryHandle(QueryHandle &&o) noexcept : state_(o.state_) { o.state_ = nullptr; }

  /// \brief Copies the filters, letting go of this one's state.
  /// \param o The other.
  /// \return This handle.
  QueryHandle &operator=(const QueryHandle &o) {
    if (this != &o) {
      QueryState *next = CopyQueryState(o.state_);
      FreeQueryState(state_);
      state_ = next;
    }
    return *this;
  }

  /// \brief Takes the other's state.
  /// \param o The other.
  /// \return This handle.
  QueryHandle &operator=(QueryHandle &&o) noexcept {
    if (this != &o) {
      FreeQueryState(state_);
      state_ = o.state_;
      o.state_ = nullptr;
    }
    return *this;
  }

  ~QueryHandle() { FreeQueryState(state_); }

  /// \brief The state, made on first use.
  /// \return The state.
  [[nodiscard]] QueryState &Ensure() {
    if (state_ == nullptr) { state_ = MakeQueryState(); }
    return *state_;
  }

private:
  QueryState *state_ = nullptr;
};

}

/// \brief What the runtime knows about a generated query: its number, its name and its
///        declaration.
/// \tparam T The query's generated class.
template <typename T> struct QueryTraits;

/// \brief AL's `Query` object: a generated query's own class derives from this.
///
/// \tparam Derived The query's generated class, or `void` for the platform object AL spells
///         `QUERY`.
///
/// \note A COLUMN IS NAMED BY ITS MEMBER, the way a record's field is: `SetRange(Q.Code, v)` is
///       a reference to the member, and the base finds the column by the member's offset. So a
///       member of another object refuses rather than filtering the wrong column.
template <typename Derived = void> class Query {
public:
  /// \brief The trigger a query owes (board:0299).
  ///
  /// \warning `OnBeforeOpen` is the only one a query declares (`triggers-auto/`), and it runs
  ///          before the query reads its first row -- which is where a filter set in AL still
  ///          reaches the SQL rather than the result.
  static constexpr std::string_view kTriggerOrder = "OnBeforeOpen";

  /// \brief The query's AL number.
  /// \return The number AL declared.
  [[nodiscard]] static constexpr QueryId Id() { return QueryTraits<Derived>::kId; }

  /// \brief AL `Query.Open()`. Generates the dataset.
  /// \return True.
  ::agiru::Boolean Open() {
    if constexpr (requires { Self()->OnBeforeOpen(); }) { Self()->OnBeforeOpen(); }
    return detail::QueryOpen(State(), Def());
  }

  /// \brief AL `Query.Read()`. Reads the next row of the dataset into the columns.
  /// \return True when a row was read.
  /// \throws Error when the query is not open.
  ::agiru::Boolean Read() { return detail::QueryRead(State(), Def(), Self()); }

  /// \brief AL `Query.Close()`. Closes the dataset.
  void Close() { detail::QueryClose(State()); }

  /// \brief AL `Query.SetRange(Column)`: removes the filter on a column.
  /// \tparam Column The member's type.
  /// \param member The column member.
  template <typename Column> void SetRange(const Column &member) {
    detail::QueryNarrow(State(), IndexOf(&member), {});
  }

  /// \brief AL `Query.SetRange(Column, Value)`.
  /// \tparam Column The member's type. \tparam Value The value's type.
  /// \param member The column member. \param value The one value the column must equal.
  template <typename Column, typename Value>
  void SetRange(const Column &member, const Value &value) {
    detail::QueryNarrow(State(), IndexOf(&member), detail::Literally(FilterText(value)));
  }

  /// \brief AL `Query.SetRange(Column, FromValue, ToValue)`.
  /// \tparam Column The member's type. \tparam From The lower bound's type.
  /// \tparam To The upper bound's type.
  /// \param member The column member. \param from The lower bound. \param to The upper bound.
  template <typename Column, typename From, typename To>
  void SetRange(const Column &member, const From &from, const To &to) {
    detail::QueryNarrow(State(),
                        IndexOf(&member),
                        detail::Literally(FilterText(from)) + ".." +
                            detail::Literally(FilterText(to)));
  }

  /// \brief AL `Query.SetFilter(Column, String, Value, ...)`: `%1` placeholders substituted.
  /// \tparam Column The member's type. \tparam Arguments The values' types.
  /// \param member The column member. \param expression The filter, in AL's own language.
  /// \param arguments The values the placeholders stand for.
  template <typename Column, typename... Arguments>
  void SetFilter(const Column &member, std::string_view expression, const Arguments &...arguments) {
    detail::QueryNarrow(State(), IndexOf(&member), StrSubstNo(expression, arguments...));
  }

  /// \brief AL `Query.GetFilter(Column)`.
  /// \tparam Column The member's type.
  /// \param member The column member.
  /// \return The filter standing on it, or nothing.
  template <typename Column> [[nodiscard]] ::agiru::Text<0> GetFilter(const Column &member) const {
    return ::agiru::Text<0>(detail::QueryFilterOn(State(), IndexOf(&member)));
  }

  /// \brief AL `Query.GetFilters()`.
  /// \return Every column filter as text.
  [[nodiscard]] ::agiru::Text<0> GetFilters() const {
    return ::agiru::Text<0>(detail::QueryFilters(State(), Def()));
  }

  /// \brief AL `Query.TopNumberOfRows([NewRows])`: the most rows the dataset returns.
  /// \param NewRows The limit; 0 lifts it.
  /// \return The limit that stood before.
  ::agiru::Integer TopNumberOfRows(::agiru::Integer NewRows) {
    return detail::QueryTop(State(), NewRows);
  }

  /// \brief AL `Query.ColumnName(ColumnNo)`. A column's AL name.
  /// \param ColumnNo The column's one-based number.
  /// \return The name.
  /// \throws Error when no column has that number.
  [[nodiscard]] ::agiru::Text<0> ColumnName(::agiru::Integer ColumnNo) const {
    return ::agiru::Text<0>(ColumnAt(ColumnNo).name);
  }

  /// \brief AL `Query.ColumnCaption(ColumnNo)`. A column's caption.
  /// \param ColumnNo The column's one-based number.
  /// \return The caption.
  /// \throws Error when no column has that number.
  [[nodiscard]] ::agiru::Text<0> ColumnCaption(::agiru::Integer ColumnNo) const {
    return ::agiru::Text<0>(ColumnAt(ColumnNo).caption);
  }

  /// \brief AL `Query.ColumnName(Column)` where AL names the column by its member, which the
  ///        compiler turns into the number; the member's offset finds it here.
  /// \tparam Column The member's type.
  /// \param member The column member.
  /// \return The AL name.
  template <typename Column>
    requires(!std::integral<Column>)
  [[nodiscard]] ::agiru::Text<0> ColumnName(const Column &member) const {
    return ::agiru::Text<0>(Def().columns[IndexOf(&member)].name);
  }

  /// \brief AL `Query.ColumnCaption(Column)` by member. \see ColumnName
  /// \tparam Column The member's type.
  /// \param member The column member.
  /// \return The caption.
  template <typename Column>
    requires(!std::integral<Column>)
  [[nodiscard]] ::agiru::Text<0> ColumnCaption(const Column &member) const {
    return ::agiru::Text<0>(Def().columns[IndexOf(&member)].caption);
  }

  /// \brief AL `Query.ColumnNo(ColumnName)`. A column's number.
  /// \param ColumnName The column's AL name.
  /// \return Its one-based number, or 0 when no column has that name.
  [[nodiscard]] ::agiru::Integer ColumnNo(std::string_view ColumnName) const {
    const QueryDef &def = Def();
    for (std::size_t i = 0; i < def.columns.size(); ++i) {
      if (SameName(def.columns[i].name, ColumnName)) {
        return static_cast<::agiru::Integer>(i + 1);
      }
    }
    return 0;
  }

  /// \brief AL `Query.SaveAsCsv(...)`. Writes the dataset as CSV.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- the export shapes are not written yet (board:0064).
  template <typename... Arguments>::agiru::Boolean SaveAsCsv(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.SaveAsCsv is declared and not implemented yet (board:0064)");
  }

  /// \brief AL `Query.SaveAsJson(...)`. Writes the dataset as JSON.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- the export shapes are not written yet (board:0064).
  template <typename... Arguments>::agiru::Boolean SaveAsJson(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.SaveAsJson is declared and not implemented yet (board:0064)");
  }

  /// \brief AL `Query.SaveAsXml(...)`. Writes the dataset as XML.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- the export shapes are not written yet (board:0064).
  template <typename... Arguments>::agiru::Boolean SaveAsXml(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.SaveAsXml is declared and not implemented yet (board:0064)");
  }

  /// \brief AL `Query.SecurityFiltering([Mode])`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param arguments The arguments, read only to be discarded.
  /// \throws Error always -- security filtering is board:0055's.
  template <typename... Arguments>
  ::agiru::SecurityFilter SecurityFiltering(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    throw Error("Query.SecurityFiltering is declared and not implemented yet (board:0064)");
  }

private:
  [[nodiscard]] Derived *Self() {
    static_assert(offsetof(Derived, State_Block) == 0,
                  "a query's State_Block is its first member: the runtime reaches the state at "
                  "the object's own address");
    return static_cast<Derived *>(this);
  }

  [[nodiscard]] const Derived *Self() const { return static_cast<const Derived *>(this); }

  [[nodiscard]] static const QueryDef &Def() { return QueryTraits<Derived>::kQuery; }

  [[nodiscard]] detail::QueryState &State() const {
    return reinterpret_cast<detail::QueryHandle *>(const_cast<Derived *>(Self()))->Ensure();
  }

  [[nodiscard]] static bool SameName(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) { return false; }
    for (std::size_t i = 0; i < a.size(); ++i) {
      const auto fold = [](char c) {
        return c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : c;
      };
      if (fold(a[i]) != fold(b[i])) { return false; }
    }
    return true;
  }

  [[nodiscard]] const QueryColumn &ColumnAt(::agiru::Integer no) const {
    const QueryDef &def = Def();
    if (no < 1 || static_cast<std::size_t>(no) > def.columns.size()) {
      throw Error("the query " + std::string(def.name) + " has no column " + std::to_string(no));
    }
    return def.columns[static_cast<std::size_t>(no) - 1];
  }

  [[nodiscard]] std::size_t IndexOf(const void *member) const {
    const auto offset = static_cast<std::size_t>(static_cast<const std::byte *>(member) -
                                                 reinterpret_cast<const std::byte *>(Self()));
    const QueryDef &def = Def();
    for (std::size_t i = 0; i < def.columns.size(); ++i) {
      if (def.columns[i].offset == offset) { return i; }
    }
    throw Error("this query, " + std::string(def.name) + ", declares no column at byte " +
                std::to_string(offset) + ": the member belongs to another object");
  }
};

/// \brief AL `QUERY` reached by NUMBER.
template <> class Query<void> {
public:
  /// \brief AL `QUERY.SaveAsCsv(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The query's number.
  /// \param arguments The rest, read only to be discarded.
  /// \throws Error always -- the export shapes are not written yet (board:0064).
  template <typename... Arguments>
  static ::agiru::Boolean SaveAsCsv(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Query.SaveAsCsv(" + std::to_string(Number) +
                ") is declared and not implemented yet (board:0064)");
  }

  /// \brief AL `QUERY.SaveAsJson(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The query's number.
  /// \param arguments The rest, read only to be discarded.
  /// \throws Error always -- the export shapes are not written yet (board:0064).
  template <typename... Arguments>
  static ::agiru::Boolean SaveAsJson(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Query.SaveAsJson(" + std::to_string(Number) +
                ") is declared and not implemented yet (board:0064)");
  }

  /// \brief AL `QUERY.SaveAsXml(Number, ...)`.
  /// \tparam Arguments Whatever AL's overload set takes.
  /// \param Number    The query's number.
  /// \param arguments The rest, read only to be discarded.
  /// \throws Error always -- the export shapes are not written yet (board:0064).
  template <typename... Arguments>
  static ::agiru::Boolean SaveAsXml(::agiru::Integer Number, Arguments &&...arguments) {
    (static_cast<void>(arguments), ...);
    throw Error("Query.SaveAsXml(" + std::to_string(Number) +
                ") is declared and not implemented yet (board:0064)");
  }
};

}
