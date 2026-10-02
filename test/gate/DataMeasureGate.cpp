#include "dotnet/BusinessChart.h"
#include "type/Integer.h"

#include "Check.h"

#include <array>
#include <type_traits>
#include <utility>

namespace {

using agiru::Integer;
using agiru::dotnet::DataMeasureType;

constexpr std::array<std::pair<DataMeasureType, Integer>, 15> kDeclaredTypes{{
    {DataMeasureType::Point(), 0},
    {DataMeasureType::Bubble(), 2},
    {DataMeasureType::Line(), 3},
    {DataMeasureType::StepLine(), 5},
    {DataMeasureType::Column(), 10},
    {DataMeasureType::StackedColumn(), 11},
    {DataMeasureType::StackedColumn100(), 12},
    {DataMeasureType::Area(), 13},
    {DataMeasureType::StackedArea(), 15},
    {DataMeasureType::StackedArea100(), 16},
    {DataMeasureType::Pie(), 17},
    {DataMeasureType::Doughnut(), 18},
    {DataMeasureType::Range(), 21},
    {DataMeasureType::Radar(), 25},
    {DataMeasureType::Funnel(), 33},
}};

static_assert(std::is_convertible_v<DataMeasureType, Integer>);
static_assert(DataMeasureType{}.AsInteger() == 0);
static_assert(DataMeasureType::Line().AsInteger() == 3);
static_assert(DataMeasureType::StackedColumn().AsInteger() == 11);

void DeclaredValuesAreNotDensePositions() {
  for (const auto &[type, expected] : kDeclaredTypes) {
    CHECK_TRUE("named type preserves the declared sparse value", type.AsInteger() == expected);
    const Integer read = type;
    CHECK_TRUE("AL numeric read uses the declared value", read == expected);
    DataMeasureType written;
    written = expected;
    CHECK_TRUE("AL numeric write roundtrips the same value", written.AsInteger() == expected);
  }
}

void ValueCopiesDoNotAliasAndUnknownNumbersArePreserved() {
  DataMeasureType original = DataMeasureType::Line();
  DataMeasureType copy = original;
  original = DataMeasureType::StackedColumn();
  CHECK_TRUE("a value copy does not observe later writes", copy.AsInteger() == 3);
  CHECK_TRUE("writing one value changes only that value", original.AsInteger() == 11);
  copy = Integer{-17};
  CHECK_TRUE("unknown numeric values are not renumbered", copy.AsInteger() == -17);
  copy = Integer{100};
  CHECK_TRUE("extension numeric values are not clamped", copy.AsInteger() == 100);
}

}

int main() {
  return gate::Run("DataMeasure", [] {
    DeclaredValuesAreNotDensePositions();
    ValueCopiesDoNotAliasAndUnknownNumbersArePreserved();
  });
}
