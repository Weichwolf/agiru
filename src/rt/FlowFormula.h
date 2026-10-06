#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace agiru {
struct FieldDef;

namespace detail {

std::string FlowFieldZero(const FieldDef &field);

struct FlowTerm {
  enum class How : std::uint8_t {
    Const,
    Filter,
    Field,
    FieldFilter,
    FieldUpperLimit,
    FieldUpperLimitFilter
  };
  std::string target;
  How how = How::Const;
  std::string value;
};

struct FlowFormula {
  enum class Kind : std::uint8_t { Sum, Average, Exist, Count, Min, Max, Lookup };
  Kind kind = Kind::Sum;
  bool reverseSign = false;
  std::string table;
  std::string field;
  std::vector<FlowTerm> terms;
};

}
}
