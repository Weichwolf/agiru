#pragma once

#include "meta/TableDef.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::detail {

enum class Compare : std::uint8_t {
  Equal,
  NotEqual,
  Less,
  LessOrEqual,
  Greater,
  GreaterEqual,
  Between,
  Like,
  NotLike,
};

struct Atom {
  Compare compare = Compare::Equal;
  std::string value;
  std::string upper;
  bool openLower = false;
  bool openUpper = false;
};

using All = std::vector<Atom>;

using Expression = std::vector<All>;

[[nodiscard]] Expression ParseFilter(std::string_view text);

[[nodiscard]] bool
Matches(const Expression &expression, std::string_view value, const FieldDef &def);

struct Interval {
  std::int64_t low;
  std::int64_t high;
};

[[nodiscard]] constexpr bool operator==(const Interval &a, const Interval &b) {
  return a.low == b.low && a.high == b.high;
}

using Intervals = std::vector<Interval>;

[[nodiscard]] std::optional<Intervals> IntegerIntervals(const Expression &expression,
                                                        Interval domain);

[[nodiscard]] std::int64_t CountOf(const Intervals &intervals);

}
