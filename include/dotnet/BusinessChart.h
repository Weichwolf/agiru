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
///        declared value, never a dense position. Values follow enum 484 in
///        `System Application/App/Business Chart/src/BusinessChartType.Enum.al` and
///        `Generic Chart Mgt.ChartType2GraphType`/`GraphType2ChartType`.
class DataMeasureType {
public:
  /// \brief The binder AL never calls; it marks the class as rebuilt.
  struct Binder {};

  /// \brief `DataMeasureType.Line`. \return The line type.
  [[nodiscard]] static constexpr DataMeasureType Line() { return DataMeasureType{Kind::Line}; }

  /// \brief `DataMeasureType.StackedColumn`. \return The stacked-column type.
  [[nodiscard]] static constexpr DataMeasureType StackedColumn() {
    return DataMeasureType{Kind::StackedColumn};
  }

  /// \brief The declared Point value. \return Point.
  [[nodiscard]] static constexpr DataMeasureType Point() { return DataMeasureType{Kind::Point}; }

  /// \brief The declared Bubble value. \return Bubble.
  [[nodiscard]] static constexpr DataMeasureType Bubble() { return DataMeasureType{Kind::Bubble}; }

  /// \brief The declared StepLine value. \return StepLine.
  [[nodiscard]] static constexpr DataMeasureType StepLine() {
    return DataMeasureType{Kind::StepLine};
  }

  /// \brief The declared Column value. \return Column.
  [[nodiscard]] static constexpr DataMeasureType Column() { return DataMeasureType{Kind::Column}; }

  /// \brief The declared StackedColumn100 value. \return StackedColumn100.
  [[nodiscard]] static constexpr DataMeasureType StackedColumn100() {
    return DataMeasureType{Kind::StackedColumn100};
  }

  /// \brief The declared Area value. \return Area.
  [[nodiscard]] static constexpr DataMeasureType Area() { return DataMeasureType{Kind::Area}; }

  /// \brief The declared StackedArea value. \return StackedArea.
  [[nodiscard]] static constexpr DataMeasureType StackedArea() {
    return DataMeasureType{Kind::StackedArea};
  }

  /// \brief The declared StackedArea100 value. \return StackedArea100.
  [[nodiscard]] static constexpr DataMeasureType StackedArea100() {
    return DataMeasureType{Kind::StackedArea100};
  }

  /// \brief The declared Pie value. \return Pie.
  [[nodiscard]] static constexpr DataMeasureType Pie() { return DataMeasureType{Kind::Pie}; }

  /// \brief The declared Doughnut value. \return Doughnut.
  [[nodiscard]] static constexpr DataMeasureType Doughnut() {
    return DataMeasureType{Kind::Doughnut};
  }

  /// \brief The declared Range value. \return Range.
  [[nodiscard]] static constexpr DataMeasureType Range() { return DataMeasureType{Kind::Range}; }

  /// \brief The declared Radar value. \return Radar.
  [[nodiscard]] static constexpr DataMeasureType Radar() { return DataMeasureType{Kind::Radar}; }

  /// \brief The declared Funnel value. \return Funnel.
  [[nodiscard]] static constexpr DataMeasureType Funnel() { return DataMeasureType{Kind::Funnel}; }

  /// \brief `Type := Integer`, the ordinal of `Business Chart Type`. \param ordinal The ordinal.
  /// \return This.
  constexpr DataMeasureType &operator=(Integer ordinal) {
    ordinal_ = ordinal;
    return *this;
  }

  /// \brief The ordinal. \return It.
  [[nodiscard]] constexpr Integer AsInteger() const { return ordinal_; }

  /// \brief AL's numeric read boundary for the .NET enum. \return The declared value.
  [[nodiscard]] constexpr operator Integer() const { return ordinal_; }

  /// \brief What `Format(DataMeasureType)` renders: the ordinal. \return It.
  [[nodiscard]] std::string ToText() const { return std::to_string(ordinal_); }

private:
  enum class Kind : Integer {
    Point = 0,
    Bubble = 2,
    Line = 3,
    StepLine = 5,
    Column = 10,
    StackedColumn = 11,
    StackedColumn100 = 12,
    Area = 13,
    StackedArea = 15,
    StackedArea100 = 16,
    Pie = 17,
    Doughnut = 18,
    Range = 21,
    Radar = 25,
    Funnel = 33
  };

  explicit constexpr DataMeasureType(Kind ordinal) : ordinal_(static_cast<Integer>(ordinal)) {}

public:
  /// \brief The default, `Point`, which is the enum's first member.
  constexpr DataMeasureType() = default;

private:
  Integer ordinal_ = static_cast<Integer>(Kind::Point);
};

/// \brief The platform's `BusinessChartData`: the table a chart draws, its X (and Z) dimension
///        and its measures, which `Business Chart Impl.` fills and serialises for the add-in.
///        A REFERENCE, like every .NET class.
class BusinessChartData {
public:
  /// \brief One measure: a column of the table and how it is drawn.
  struct Measure {
    std::string name;     ///< The column the measure reads.
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
    requires requires { typename R::IsAlRefusal; } ::agiru::Text<0>
  XDimension(const R &refused) {
    static_cast<void>(refused());
    throw Error("a refused member reached a rebuilt class");
  }

  /// \brief `ZDimension := AbsentObject.Member`. \see XDimension
  template <typename R>
    requires requires { typename R::IsAlRefusal; } ::agiru::Text<0>
  ZDimension(const R &refused) {
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
