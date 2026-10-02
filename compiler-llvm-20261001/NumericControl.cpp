#include "dotnet/BusinessChart.h"
#include "type/Integer.h"
#include "Check.h"

#include <type_traits>

using agiru::Integer;
using agiru::dotnet::DataMeasureType;

template <typename Value>
Integer Read(Value value) {
  if constexpr (std::is_convertible_v<Value, Integer>) { return value; }
  return -1;
}

int main() {
  return gate::Run("DataMeasureControl", [] {
    CHECK_TRUE("AL's Integer read is available", (std::is_convertible_v<DataMeasureType, Integer>));
    CHECK_TRUE("Line is the declared value, not a dense index", DataMeasureType::Line().AsInteger() == 3);
    CHECK_TRUE("StackedColumn is the declared value", DataMeasureType::StackedColumn().AsInteger() == 11);
    CHECK_TRUE("default is Point", DataMeasureType{}.AsInteger() == 0);
    DataMeasureType value;
    value = Integer{33};
    CHECK_TRUE("numeric assignment preserves Funnel's value", value.AsInteger() == 33);
    DataMeasureType copy = DataMeasureType::Line();
    value = copy;
    value = Integer{11};
    CHECK_TRUE("value copies retain Line", copy.AsInteger() == 3);
    CHECK_TRUE("numeric read returns Line", Read(copy) == 3);
  });
}
