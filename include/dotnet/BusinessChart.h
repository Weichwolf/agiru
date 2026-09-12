#pragma once

#include "dotnet/DataTable.h"
#include "dotnet/Refused.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Text.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::dotnet {

/// \brief The platform's `Microsoft.Dynamics.Nav.Client.BusinessChart.DataMeasureType`: how a
///        measure is drawn. AL assigns it from `"Business Chart Type".AsInteger()`, so it is the
///        ordinal of that enum -- Point 0, Bubble 1, Line 2, StepLine 3, Column 4,
///        StackedColumn 5, StackedColumn100 6, Area 7, StackedArea 8, StackedArea100 9, Pie 10,
///        Doughnut 11, Range 12, Radar 13, Funnel 14 -- and `Line` and `StackedColumn` are the
///        two members AL names by name (`System.Visualization."Business Chart Type"`).
class DataMeasureType {
public:
  /// \brief The binder AL never calls; it marks the class as rebuilt.
  struct Binder {};

  /// \brief `DataMeasureType.Line`. \return The line type.
  [[nodiscard]] static DataMeasureType Line() { return DataMeasureType{kLine}; }

  /// \brief `DataMeasureType.StackedColumn`. \return The stacked-column type.
  [[nodiscard]] static DataMeasureType StackedColumn() { return DataMeasureType{kStackedColumn}; }

  /// \brief `Type := Integer`, the ordinal of `Business Chart Type`. \param ordinal The ordinal.
  /// \return This.
  DataMeasureType &operator=(Integer ordinal) {
    ordinal_ = ordinal;
    return *this;
  }

  /// \brief The ordinal. \return It.
  [[nodiscard]] Integer AsInteger() const { return ordinal_; }

  /// \brief What `Format(DataMeasureType)` renders: the ordinal. \return It.
  [[nodiscard]] std::string ToText() const { return std::to_string(ordinal_); }

private:
  static constexpr std::int32_t kLine = 2;
  static constexpr std::int32_t kStackedColumn = 5;
  explicit DataMeasureType(std::int32_t ordinal) : ordinal_(ordinal) {}

public:
  /// \brief The default, `Point`, which is the enum's first member.
  DataMeasureType() = default;

private:
  std::int32_t ordinal_ = 0;
};

/// \brief The platform's `BusinessChartData`: the table a chart draws, its X (and Z) dimension
///        and its measures, which `Business Chart Impl.` fills and serialises for the add-in.
///        A REFERENCE, like every .NET class.
class BusinessChartData {
public:
  /// \brief One measure: a column of the table and how it is drawn.
  struct Measure {
    std::string name;      ///< The column the measure reads.
    DataMeasureType type; ///< How it is drawn.
  };

  /// \brief The binder behind `D := D.BusinessChartData()`.
  struct Binder {
    /// \brief `new BusinessChartData()`. \return Empty chart data.
    [[nodiscard]] class BusinessChartData operator()() const {
      class BusinessChartData made;
      made.held_ = std::make_shared<Held>();
      return made;
    }
  };

  /// \brief The constructor AL calls as a member.
  Binder BusinessChartData; // NOLINT(misc-non-private-member-variables-in-classes)

  /// \brief `XDimension` read. \return The column drawn along X.
  [[nodiscard]] ::agiru::Text<0> XDimension() const { return Held_().xDimension; }

  /// \brief `XDimension := name`. \param name The column. \return It.
  ::agiru::Text<0> XDimension(std::string_view name) {
    Held_().xDimension = std::string(name);
    return std::string(name);
  }

  /// \brief `ZDimension` read. \return The column drawn along Z, for a bubble chart.
  [[nodiscard]] ::agiru::Text<0> ZDimension() const { return Held_().zDimension; }

  /// \brief `ZDimension := name`. \param name The column. \return It.
  ::agiru::Text<0> ZDimension(std::string_view name) {
    Held_().zDimension = std::string(name);
    return std::string(name);
  }

  /// \brief `AddMeasure(name, type)`. \param name The column. \param type How it is drawn.
  void AddMeasure(std::string_view name, const DataMeasureType &type) {
    Held_().measures.push_back(Measure{.name = std::string(name), .type = type});
  }

  /// \brief `XDimension := AbsentObject.Member`, `AddMeasure(AbsentObject.Member, ...)`: a name a
  ///        .NET type this runtime does not carry would have supplied (`BusinessChartBuilder`).
  /// \tparam R The refusal. \param refused It. \return Never. \throws Error always.
  template <typename R>
    requires requires { typename R::IsAlRefusal; }
  ::agiru::Text<0> XDimension(const R &refused) {
    static_cast<void>(refused());
    throw Error("a refused member reached a rebuilt class");
  }

  /// \brief `ZDimension := AbsentObject.Member`. \see XDimension
  template <typename R>
    requires requires { typename R::IsAlRefusal; }
  ::agiru::Text<0> ZDimension(const R &refused) {
    static_cast<void>(refused());
    throw Error("a refused member reached a rebuilt class");
  }

  /// \brief `AddMeasure(AbsentObject.Member, type)`. \see XDimension
  template <typename R, typename... Rest>
    requires requires { typename R::IsAlRefusal; }
  void AddMeasure(const R &refused, Rest &&...rest) {
    (static_cast<void>(rest), ...);
    static_cast<void>(refused());
  }

  /// \brief `ClearMeasures()`: forgets every measure.
  void ClearMeasures() { Held_().measures.clear(); }

  /// \brief The measures, in the order they were added. \return Them.
  [[nodiscard]] const std::vector<Measure> &Measures() const { return Held_().measures; }

  /// \brief `DataTable := table`: the table the chart draws. \param table It. \return It.
  class DataTable DataTable(const class DataTable &table) {
    Held_().table = table;
    return table;
  }

  /// \brief `DataTable` read. \return The table the chart draws.
  [[nodiscard]] class DataTable DataTable() const { return Held_().table; }

  /// \brief `ShowChartCondensed := flag`. \param condensed The flag. \return It.
  Boolean ShowChartCondensed(Boolean condensed) {
    Held_().condensed = condensed;
    return condensed;
  }

  /// \brief `ShowChartCondensed` read. \return The flag.
  [[nodiscard]] Boolean ShowChartCondensed() const { return Held_().condensed; }

  /// \brief Whether this refers to nothing yet. \return True before the constructor ran.
  [[nodiscard]] bool IsNull() const { return held_ == nullptr; }

private:
  struct Held {
    std::string xDimension;
    std::string zDimension;
    std::vector<Measure> measures;
    class DataTable table;
    Boolean condensed = false;
  };

  Held &Held_() const {
    if (held_ == nullptr) { throw Error("BusinessChartData: it was never made"); }
    return *held_;
  }

  std::shared_ptr<Held> held_;
};

}
