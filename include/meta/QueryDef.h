#pragma once

#include "meta/Ids.h"
#include "meta/TableDef.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

/// \file
/// \brief The static declaration of an AL query: its dataitems, how they join, and its columns.
///
/// A query is the one AL object that is already a SQL statement, and everything below is knowable
/// at translation time: the tables, the join order (the nesting), the join type, the links, the
/// column list and which columns aggregate. What is built at run time is the WHERE clause from the
/// filters, the way a record's `SetRange` is (board:0064).

namespace agiru {

/// \brief AL's `SqlJoinType` property, `devenv-sqljointype-property.md`.
///
/// \warning THE DEFAULT IS `LeftOuterJoin`, in the property page's own words: "By default, the
///          SqlJoinType property is LeftOuterJoin". The predecessor emitted INNER for an omitted
///          property and a dataitem with no match deleted the whole result (openerp WI-1227).
enum class QueryJoin : std::uint8_t {
  LeftOuter,  ///< Every upper row, the lower columns blank where nothing matched.
  Inner,      ///< Only the rows both sides have.
  RightOuter, ///< Every lower row.
  Full,       ///< Every row of either side.
  Cross,      ///< Every pair; no `DataItemLink` is allowed.
};

/// \brief AL's `Method` property on a column, `devenv-query-totals-grouping.md` and
///        `devenv-query-retrieve-date-data.md`.
///
/// \note SETTING AN AGGREGATE GROUPS IMPLICITLY: every column WITHOUT one becomes a grouping
///       column, which is what SQL's `GROUP BY` says, and AL has no clause for it.
/// \warning `Average` OVER AN INTEGER TRUNCATES: "integer division is used. The result isn't
///          rounded, and the remainder is discarded. For example, 5÷2=2 instead of 2.5." That is
///          the opposite of AL's `/`, which yields a Decimal whatever its operands (board:0088).
enum class QueryMethod : std::uint8_t {
  None,    ///< The field's own value.
  Sum,     ///< `SUM`.
  Average, ///< `AVG`, truncating over an integer field.
  Min,     ///< `MIN`.
  Max,     ///< `MAX`.
  Count,   ///< `COUNT(*)`, an Integer.
  Day,     ///< The day of a date field, an Integer.
  Month,   ///< The month of a date field, an Integer.
  Year,    ///< The year of a date field, an Integer.
};

/// \brief Whether a method aggregates, which is what decides the implicit `GROUP BY`.
/// \param method The column's method.
/// \return True for `Sum`, `Average`, `Min`, `Max` and `Count`.
[[nodiscard]] constexpr bool Aggregates(QueryMethod method) {
  return method == QueryMethod::Sum || method == QueryMethod::Average ||
         method == QueryMethod::Min || method == QueryMethod::Max || method == QueryMethod::Count;
}

/// \brief One equality of a `DataItemLink`: a field of the lower dataitem's table against a
///        field of an ANCESTOR dataitem's table (`devenv-dataitemlink-query-property.md`).
struct QueryLink {
  FieldNo field{};          ///< The lower dataitem's field.
  std::size_t dataItem{};   ///< Which ancestor, by position in `QueryDef::dataItems`.
  FieldNo reference{};      ///< The ancestor's field.
};

/// \brief One `dataitem`: a table, how it joins what is above it, and its permanent filter.
///
/// \note THE POSITION IS THE JOIN ORDER. AL nests the dataitems, the second inside the first, and
///       "lower data items are linked to the resulting dataset of the linked data items above it"
///       -- so the flat list here is the nesting flattened in order, and the first has no join.
struct QueryDataItem {
  std::string_view name{};              ///< The dataitem's AL name.
  const TableDef *table = nullptr;      ///< Its table.
  QueryJoin join = QueryJoin::LeftOuter; ///< How it joins the dataset above it.
  std::span<const QueryLink> links{};   ///< The `DataItemLink` equalities.

  /// \brief The `DataItemTableFilter` property, as AL wrote it: `Field = const(V), Field = filter(E)`.
  ///
  /// \warning IT CANNOT BE OVERWRITTEN FROM AL CODE (`devenv-dataitemtablefilter-property.md`):
  ///          a `SetRange` on the same field is ANDed with it, never put in its place. So it is
  ///          part of the query's definition and lands in the emitted `WHERE` on its own.
  std::string_view tableFilter{};
};

/// \brief One `column` or `filter` element: a field of a dataitem, read into a member of the
///        generated class -- or, for a `filter`, only filtered on and never read.
struct QueryColumn {
  std::string_view name{};    ///< The column's AL name, which `ColumnName` answers.
  std::string_view caption{}; ///< The `Caption` property, or the name.
  std::size_t offset{};       ///< `offsetof` within the generated query.
  std::size_t dataItem{};     ///< Which dataitem, by position.
  FieldNo field{};            ///< Which field of that dataitem's table.
  QueryMethod method = QueryMethod::None; ///< The `Method` property.
  bool returned = true;       ///< A `column` is read; a `filter` element is not.
  bool reverseSign = false;   ///< The `ReverseSign` property.

  /// \brief The `ColumnFilter` property, as AL wrote it. A `SetFilter` or `SetRange` from AL code
  ///        REPLACES it, which is the difference from a dataitem's table filter.
  std::string_view columnFilter{};
};

/// \brief One term of the `OrderBy` property.
struct QueryOrder {
  std::string_view column{}; ///< The column's AL name.
  bool descending = false;   ///< `descending(...)` rather than `ascending(...)`.
};

/// \brief A query's whole declaration.
struct QueryDef {
  QueryId id{};                             ///< The AL query number.
  std::string_view name{};                  ///< The AL name.
  std::string_view caption{};               ///< The `Caption` property.
  std::span<const QueryDataItem> dataItems{}; ///< In nesting order; the first is the root.
  std::span<const QueryColumn> columns{};   ///< In declaration order.
  std::span<const QueryOrder> orderBy{};    ///< The `OrderBy` property.
  std::int32_t topNumberOfRows = 0;         ///< The `TopNumberOfRows` property; 0 is no limit.
};

/// \brief Whether any column aggregates, so the rows group.
/// \param def The query.
/// \return True when one column carries an aggregating method.
[[nodiscard]] constexpr bool Groups(const QueryDef &def) {
  for (const QueryColumn &column : def.columns) {
    if (Aggregates(column.method)) { return true; }
  }
  return false;
}

}
