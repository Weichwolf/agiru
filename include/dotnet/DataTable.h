#pragma once

#include "dotnet/CultureInfo.h"
#include "dotnet/Refused.h"
#include "dotnet/Type.h"
#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Text.h"
#include "type/Variant.h"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::dotnet {

class DataTable;

namespace detail {
/// \brief Whether two column names are one, the way .NET compares them: without regard to case.
/// \param a One name. \param b The other. \return True when they agree.
[[nodiscard]] bool SameColumnName(std::string_view a, std::string_view b);
}

/// \brief .NET `System.Data.DataColumn`: a name, a caption and a type. A REFERENCE, the way .NET
///        hands one out: `Columns.Item(0).Caption('x')` changes the table's column.
class DataColumn {
public:
  /// \brief The binder behind `C := C.DataColumn(name)`.
  struct Binder {
    /// \brief `new DataColumn()`. \return An unnamed column.
    [[nodiscard]] class DataColumn operator()() const { return Made(""); }

    /// \brief `new DataColumn(name)`. \param name The column name. \return The column.
    [[nodiscard]] class DataColumn operator()(std::string_view name) const { return Made(name); }
  };

  /// \brief The constructor AL calls as a member.
  Binder DataColumn; // NOLINT(misc-non-private-member-variables-in-classes)

  /// \brief `DataColumn.ColumnName` read. \return The name.
  [[nodiscard]] ::agiru::Text<0> ColumnName() const { return Held_().name; }

  /// \brief `DataColumn.ColumnName := name`. \param name The name. \return It.
  ::agiru::Text<0> ColumnName(std::string_view name) {
    Held_().name = std::string(name);
    return std::string(name);
  }

  /// \brief `DataColumn.Caption` read: the caption, or the name when none was set. \return It.
  [[nodiscard]] ::agiru::Text<0> Caption() const {
    return Held_().caption.empty() ? Held_().name : Held_().caption;
  }

  /// \brief `DataColumn.Caption := caption`. \param caption The caption. \return It.
  ::agiru::Text<0> Caption(std::string_view caption) {
    Held_().caption = std::string(caption);
    return std::string(caption);
  }

  /// \brief `DataColumn.DataType` read. \return The type; `System.String` when none was set.
  [[nodiscard]] class Type DataType() const { return Held_().type; }

  /// \brief `DataColumn.DataType := type`. \param type The type. \return It.
  class Type DataType(const class Type &type) {
    Held_().type = type;
    return type;
  }

  /// \brief `DataColumn.AllowDBNull` read. \return Whether a row may leave it empty.
  [[nodiscard]] Boolean AllowDBNull() const { return Held_().allowNull; }

  /// \brief `DataColumn.AllowDBNull := allow`. \param allow The setting. \return It.
  Boolean AllowDBNull(Boolean allow) {
    Held_().allowNull = allow;
    return allow;
  }

  /// \brief Whether this refers to no column yet. \return True before a constructor ran.
  [[nodiscard]] bool IsNull() const { return held_ == nullptr; }

  /// \brief `Column := AbsentType.Member()`: the call refused before the assignment.
  /// \tparam R The refusal. \param refused It. \return This.
  template <typename R>
    requires requires { typename R::IsAlRefusal; }
  class DataColumn &operator=(const R &refused) {
    static_cast<void>(refused);
    return *this;
  }

private:
  friend class DataTable;
  friend class DataColumnCollection;
  friend class DataRow;
  struct Held {
    std::string name;
    std::string caption;
    class Type type = Type::GetType("System.String");
    Boolean allowNull = true;
  };

  static class DataColumn Made(std::string_view name) {
    class DataColumn made;
    made.held_ = std::make_shared<Held>();
    made.held_->name = std::string(name);
    return made;
  }

  Held &Held_() const {
    if (held_ == nullptr) { throw Error("DataColumn: the column was never made"); }
    return *held_;
  }

  std::shared_ptr<Held> held_;
};

/// \brief .NET `DataTable.Columns`, a `DataColumnCollection`.
class DataColumnCollection {
public:
  /// \brief The binder AL never calls; it marks the class as rebuilt.
  struct Binder {};

  /// \brief `Columns.Add(column)`. \param column The column. \return It.
  class DataColumn Add(const class DataColumn &column) {
    if (column.IsNull()) { throw Error("DataTable.Columns.Add: the column was never made"); }
    columns_->push_back(column);
    return column;
  }

  /// \brief `Columns.Add(name)`: a column of that name, of type `System.String`.
  /// \param name The name. \return The column.
  class DataColumn Add(std::string_view name) { return Add(DataColumn::Made(name)); }

  /// \brief `Columns.Add(name, type)`. \param name The name. \param type The column's type.
  /// \return The column.
  class DataColumn Add(std::string_view name, const class Type &type) {
    class DataColumn column = DataColumn::Made(name);
    column.DataType(type);
    return Add(column);
  }

  /// \brief `Columns.Add(AbsentObject.Member, ...)`: a name a .NET type this runtime does not
  ///        carry would have supplied. \tparam R The refusal. \tparam Rest The rest.
  /// \param refused The refusal. \param rest The rest, unread. \return Never.
  /// \throws Error always, the refusal's own.
  template <typename R, typename... Rest>
    requires requires { typename R::IsAlRefusal; }
  class DataColumn Add(const R &refused, Rest &&...rest) {
    (static_cast<void>(rest), ...);
    static_cast<void>(refused());
    throw Error("a refused member reached a rebuilt class");
  }

  /// \brief `Columns.Item(index)`. \param index The position, from 0. \return The column.
  /// \throws Error when the index is outside the collection.
  [[nodiscard]] class DataColumn Item(Integer index) const {
    const std::size_t at = static_cast<std::size_t>(index);
    if (index < 0 || at >= columns_->size()) {
      throw Error("DataTable.Columns: there is no column " + std::to_string(index));
    }
    return (*columns_)[at];
  }

  /// \brief `Columns.Item(name)`. \param name The column name, compared without regard to case.
  /// \return The column.
  /// \throws Error when no column carries the name.
  [[nodiscard]] class DataColumn Item(std::string_view name) const {
    const std::size_t at = IndexOf(name);
    if (at == columns_->size()) {
      throw Error("DataTable.Columns: there is no column '" + std::string(name) + "'");
    }
    return (*columns_)[at];
  }

  /// \brief `Columns.Contains(name)`. \param name The name. \return Whether a column carries it.
  [[nodiscard]] Boolean Contains(std::string_view name) const {
    return IndexOf(name) != columns_->size();
  }

  /// \brief `Columns.Count`. \return How many columns.
  [[nodiscard]] Integer Count() const { return static_cast<Integer>(columns_->size()); }

  /// \brief `Columns.Clear()`: removes every column.
  void Clear() { columns_->clear(); }

  /// \brief The position of a name, or `Count` when none. \param name The name. \return It.
  [[nodiscard]] std::size_t IndexOf(std::string_view name) const {
    for (std::size_t i = 0; i < columns_->size(); ++i) {
      if (detail::SameColumnName((*columns_)[i].Held_().name, name)) { return i; }
    }
    return columns_->size();
  }

private:
  friend class DataTable;
  friend class DataRow;
  explicit DataColumnCollection(std::shared_ptr<std::vector<class DataColumn>> columns)
      : columns_(std::move(columns)) {}
  std::shared_ptr<std::vector<class DataColumn>> columns_;
};

/// \brief .NET `System.Data.DataRow`: the values of one row, by column. A REFERENCE: a row
///        made by `NewRow()`, written through `Item(name, value)` and then handed to `Rows.Add`
///        is the row the table holds.
class DataRow {
public:
  /// \brief The binder AL never calls; it marks the class as rebuilt.
  struct Binder {};

  /// \brief `Row.Item(name)` read. \param name The column. \return The value; empty when unset.
  /// \throws Error when no column carries the name.
  [[nodiscard]] Variant Item(std::string_view name) const {
    return Held_().values[Held_().columns.IndexOfOrRefuse(name)];
  }

  /// \brief `Row.Item(index)` read. \param index The column position. \return The value.
  [[nodiscard]] Variant Item(Integer index) const { return Held_().values[Position_(index)]; }

  /// \brief `Row.Item(name) := value`. \param name The column. \param value The value.
  void Item(std::string_view name, const Variant &value) {
    Held_().values[Held_().columns.IndexOfOrRefuse(name)] = value;
  }

  /// \brief `Row.Item(index) := value`. \param index The column position. \param value The
  ///        value.
  void Item(Integer index, const Variant &value) { Held_().values[Position_(index)] = value; }

  /// \brief `Row.Item(AbsentObject.Member [, value])`: a column name a .NET type this runtime
  ///        does not carry would have supplied. \tparam R The refusal. \tparam Rest The rest.
  /// \param refused The refusal. \param rest The rest, unread. \return Never.
  /// \throws Error always, the refusal's own.
  template <typename R, typename... Rest>
    requires requires { typename R::IsAlRefusal; }
  Variant Item(const R &refused, Rest &&...rest) {
    (static_cast<void>(rest), ...);
    static_cast<void>(refused());
    throw Error("a refused member reached a rebuilt class");
  }

  /// \brief Whether this refers to no row yet. \return True before `NewRow` handed one out.
  [[nodiscard]] bool IsNull() const { return held_ == nullptr; }

  /// \brief `Row.SetParentRow(...)`: relations this runtime does not carry.
  Refused SetParentRow{{.type = "DataRow", .member = "SetParentRow"}};

  /// \brief `Row := AbsentType.Member()`: the call refused before the assignment.
  /// \tparam R The refusal. \param refused It. \return This.
  template <typename R>
    requires requires { typename R::IsAlRefusal; }
  class DataRow &operator=(const R &refused) {
    static_cast<void>(refused);
    return *this;
  }

private:
  friend class DataTable;
  friend class DataRowCollection;
  struct Columns {
    std::shared_ptr<std::vector<class DataColumn>> columns;
    [[nodiscard]] std::size_t IndexOfOrRefuse(std::string_view name) const;
  };
  struct Held {
    Columns columns;
    std::vector<Variant> values;
  };

  Held &Held_() const {
    if (held_ == nullptr) { throw Error("DataRow: the row was never made"); }
    return *held_;
  }

  [[nodiscard]] std::size_t Position_(Integer index) const {
    const std::size_t at = static_cast<std::size_t>(index);
    if (index < 0 || at >= Held_().values.size()) {
      throw Error("DataRow: there is no column " + std::to_string(index));
    }
    return at;
  }

  std::shared_ptr<Held> held_;
};

/// \brief .NET `DataTable.Rows`, a `DataRowCollection`.
class DataRowCollection {
public:
  /// \brief The binder AL never calls; it marks the class as rebuilt.
  struct Binder {};

  /// \brief `Rows.Add(row)`: attaches a row `NewRow` made. \param row The row.
  void Add(const class DataRow &row) {
    if (row.IsNull()) { throw Error("DataTable.Rows.Add: the row was never made"); }
    rows_->push_back(row);
  }

  /// \brief `Rows.Item(index)`. \param index The position, from 0. \return The row.
  /// \throws Error when the index is outside the collection.
  [[nodiscard]] class DataRow Item(Integer index) const {
    const std::size_t at = static_cast<std::size_t>(index);
    if (index < 0 || at >= rows_->size()) {
      throw Error("DataTable.Rows: there is no row " + std::to_string(index));
    }
    return (*rows_)[at];
  }

  /// \brief `Rows.Count`. \return How many rows.
  [[nodiscard]] Integer Count() const { return static_cast<Integer>(rows_->size()); }

  /// \brief `Rows.Clear()`: removes every row.
  void Clear() { rows_->clear(); }

  /// \brief AL `foreach`: the first row. \return The iterator.
  [[nodiscard]] std::vector<class DataRow>::const_iterator begin() const { return rows_->begin(); }

  /// \brief AL `foreach`: past the last row. \return The iterator.
  [[nodiscard]] std::vector<class DataRow>::const_iterator end() const { return rows_->end(); }

private:
  friend class DataTable;
  explicit DataRowCollection(std::shared_ptr<std::vector<class DataRow>> rows)
      : rows_(std::move(rows)) {}
  std::shared_ptr<std::vector<class DataRow>> rows_;
};

/// \brief .NET `System.Data.DataTable`, rebuilt as columns and rows of Variants: what `Business
///        Chart` fills and reads back (board:0035; Service Time Sheets UT, 5 cases, 2026-09-12),
///        and what a layout's `NewExtensionLayout` and the Excel copy walk.
/// \warning A REFERENCE, like every .NET class: two variables assigned from one table share it.
class DataTable {
public:
  /// \brief The binder behind `T := T.DataTable()` and `T := T.DataTable(name)`.
  struct Binder {
    /// \brief `new DataTable()`. \return An unnamed, empty table.
    [[nodiscard]] class DataTable operator()() const { return Made(""); }

    /// \brief `new DataTable(name)`. \param name The table name. \return An empty table.
    [[nodiscard]] class DataTable operator()(std::string_view name) const { return Made(name); }
  };

  /// \brief The constructor AL calls as a member.
  Binder DataTable; // NOLINT(misc-non-private-member-variables-in-classes)

  /// \brief `DataTable.TableName`. \return The name it was made with.
  [[nodiscard]] ::agiru::Text<0> TableName() const { return Held_().name; }

  /// \brief `DataTable.Columns`. \return The column collection, which writes through.
  [[nodiscard]] DataColumnCollection Columns() const {
    return DataColumnCollection{Held_().columns};
  }

  /// \brief `DataTable.Rows`. \return The row collection, which writes through.
  [[nodiscard]] DataRowCollection Rows() const { return DataRowCollection{Held_().rows}; }

  /// \brief `DataTable.NewRow()`: a detached row with one empty value per column, which
  ///        `Rows.Add` attaches. \return The row.
  [[nodiscard]] class DataRow NewRow() const {
    class DataRow row;
    row.held_ = std::make_shared<DataRow::Held>();
    row.held_->columns.columns = Held_().columns;
    row.held_->values.resize(Held_().columns->size());
    return row;
  }

  /// \brief `DataTable.Clear()`: removes every row and keeps the columns.
  void Clear() { Held_().rows->clear(); }

  /// \brief `DataTable.Locale` read. \return The culture the table was given.
  [[nodiscard]] class CultureInfo Locale() const { return Held_().locale; }

  /// \brief `DataTable.Locale := culture`. \param culture The culture. \return It.
  class CultureInfo Locale(const class CultureInfo &culture) {
    Held_().locale = culture;
    return culture;
  }

  /// \brief `DataTable.BeginLoadData()`: no constraints are checked here, so nothing to suspend.
  void BeginLoadData() { static_cast<void>(Held_()); }

  /// \brief `DataTable.EndLoadData()`. \see BeginLoadData
  void EndLoadData() { static_cast<void>(Held_()); }

  /// \brief Whether this refers to no table yet. \return True before a constructor ran.
  [[nodiscard]] bool IsNull() const { return held_ == nullptr; }

  /// \brief `DataTable.WriteXml(stream)`: the DiffGram-less XML form, not rebuilt.
  Refused WriteXml{{.type = "DataTable", .member = "WriteXml"}};

  /// \brief `Table := AbsentType.Member()`: the call refused before the assignment.
  /// \tparam R The refusal. \param refused It. \return This.
  template <typename R>
    requires requires { typename R::IsAlRefusal; }
  class DataTable &operator=(const R &refused) {
    static_cast<void>(refused);
    return *this;
  }

private:
  struct Held {
    std::string name;
    std::shared_ptr<std::vector<class DataColumn>> columns =
        std::make_shared<std::vector<class DataColumn>>();
    std::shared_ptr<std::vector<class DataRow>> rows =
        std::make_shared<std::vector<class DataRow>>();
    class CultureInfo locale = CultureInfo::InvariantCulture();
  };

  static class DataTable Made(std::string_view name) {
    class DataTable made;
    made.held_ = std::make_shared<Held>();
    made.held_->name = std::string(name);
    return made;
  }

  Held &Held_() const {
    if (held_ == nullptr) { throw Error("DataTable: the table was never made"); }
    return *held_;
  }

  std::shared_ptr<Held> held_;
};

}
