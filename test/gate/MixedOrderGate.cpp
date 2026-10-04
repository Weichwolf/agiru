#include "runtime/RecordRef.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/TableDefinition.h"
#include "type/Decimal.h"

#include "Check.h"
#include "ResourceCost.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>
#include <string_view>

namespace {

using Cost = agiru::Projects::Resources::Pricing::ResourceCost_Table;

struct Expected {
  std::string_view code;
  std::string_view direct;
  std::string_view unit;
};

constexpr std::array population{Expected{.code = "A", .direct = "10.00", .unit = "3.00"},
                                Expected{.code = "B", .direct = "10.00", .unit = "2.00"},
                                Expected{.code = "C", .direct = "10.00", .unit = "2.00"},
                                Expected{.code = "D", .direct = "20.00", .unit = "5.00"},
                                Expected{.code = "E", .direct = "20.00", .unit = "4.00"},
                                Expected{.code = "F", .direct = "20.00", .unit = "4.00"},
                                Expected{.code = "G", .direct = "30.00", .unit = "1.00"},
                                Expected{.code = "H", .direct = "30.00", .unit = "0.00"}};

void Values(Cost &row, const Expected &value) {
  row.Code = value.code;
  row.WorkTypeCode = "hours";
  row.DirectUnitCost = agiru::Decimal::FromInvariantString(value.direct);
  row.UnitCost = agiru::Decimal::FromInvariantString(value.unit);
}

void Fill(Cost &row) {
  for (const Expected &item : population | std::views::reverse) {
    row.Init();
    Values(row, item);
    row.Insert();
  }
}

template <bool Reflected> class Reader {
public:
  explicit Reader(Cost &row) : row_(row) {
    if constexpr (Reflected) { reference_.GetTable(row_); }
  }

  bool Find(std::string_view which) {
    if constexpr (Reflected) { return reference_.Find(which); }
    return row_.Find(which);
  }

  bool FindSet() {
    if constexpr (Reflected) { return reference_.FindSet(); }
    return row_.FindSet();
  }

  std::int32_t Next(std::int32_t steps = 1) {
    if constexpr (Reflected) { return reference_.Next(steps); }
    return row_.Next(steps);
  }

  void Seek(const Expected &value) {
    Observe();
    Values(row_, value);
    if constexpr (Reflected) { reference_.GetTable(row_); }
  }

  void Check(const Expected &expected) {
    Observe();
    CHECK_TEXT(
        "current/primary keys retain declared mixed order", row_.Code.Value(), expected.code);
    CHECK_TRUE("exact first Decimal value is retained",
               row_.DirectUnitCost == agiru::Decimal::FromInvariantString(expected.direct));
    CHECK_TRUE("exact second Decimal value is retained",
               row_.UnitCost == agiru::Decimal::FromInvariantString(expected.unit));
    CHECK_TEXT("remaining primary-key value is retained", row_.WorkTypeCode.Value(), "HOURS");
  }

private:
  void Observe() {
    if constexpr (Reflected) { reference_.SetTable(row_); }
  }

  Cost &row_;
  agiru::RecordRef reference_;
};

template <typename Reading> void Walk(Reading &reading, std::span<const Expected> expected) {
  CHECK_TRUE("mixed FindSet finds rows", reading.FindSet());
  reading.Check(expected.front());
  for (std::size_t i = 1; i < expected.size(); ++i) {
    CHECK_TRUE("cursor forward steps preserve order", reading.Next() == 1);
    reading.Check(expected[i]);
  }
  CHECK_TRUE("forward exhaustion reports zero", reading.Next() == 0);
  reading.Check(expected.back());
  for (std::size_t i = expected.size() - 1; i > 0; --i) {
    CHECK_TRUE("reverse keyset steps preserve order", reading.Next(-1) == -1);
    reading.Check(expected[i - 1]);
  }
  CHECK_TRUE("reverse exhaustion reports zero", reading.Next(-1) == 0);
  reading.Check(expected.front());
  CHECK_TRUE("keyset forward resumes after direction reversal", reading.Next() == 1);
  reading.Check(expected[1]);
  CHECK_TRUE("FindLast lands at the endpoint", reading.Find("+"));
  reading.Check(expected.back());
  CHECK_TRUE("FindFirst lands at the starting point", reading.Find("-"));
  reading.Check(expected.front());
  CHECK_TRUE("zero movement does not replace position", reading.Next(0) == 0);
  reading.Check(expected.front());
  CHECK_TRUE("positive overshoot reports actual distance",
             reading.Next(100) == static_cast<std::int32_t>(expected.size() - 1));
  reading.Check(expected.back());
  CHECK_TRUE("negative overshoot reports actual distance",
             reading.Next(-100) == -static_cast<std::int32_t>(expected.size() - 1));
  reading.Check(expected.front());
}

template <typename Reading> void Relative(Reading &reading, std::span<const Expected> expected) {
  for (std::size_t i = 0; i < expected.size(); ++i) {
    reading.Seek(expected[i]);
    CHECK_TRUE("relative equality matches complete current/primary values", reading.Find("="));
    reading.Check(expected[i]);
    reading.Seek(expected[i]);
    CHECK_TRUE("relative greater respects every direction",
               reading.Find(">") == (i + 1 < expected.size()));
    reading.Check(expected[i + (i + 1 < expected.size() ? 1 : 0)]);
    reading.Seek(expected[i]);
    CHECK_TRUE("relative less respects every direction", reading.Find("<") == (i != 0));
    reading.Check(expected[i == 0 ? 0 : i - 1]);
  }
}

template <bool Reflected> void Matrix(Cost &row, bool ascending, bool filtered, bool mixed) {
  row.Reset();
  CHECK_TRUE("the mixed unindexed key is selected",
             row.SetCurrentKey(row.DirectUnitCost, row.UnitCost));
  row.SetAscending(row.UnitCost, !mixed);
  row.Ascending(ascending);
  if (filtered) { row.SetRange(row.DirectUnitCost, agiru::Decimal::FromInvariantString("20.00")); }
  std::array<Expected, population.size()> ordered{};
  const std::size_t first = filtered ? 3 : 0;
  const std::size_t count = filtered ? 3 : population.size();
  const std::string_view codes = mixed ? "ABCDEFGH" : "BCAEFDHG";
  for (std::size_t i = 0; i < count; ++i) {
    const char code = codes[first + (ascending ? i : count - i - 1)];
    ordered[i] = population[static_cast<std::size_t>(code - 'A')];
  }
  Reader<Reflected> reading(row);
  const std::span<const Expected> expected(ordered.data(), count);
  Walk(reading, expected);
  Relative(reading, expected);
  const Expected missing{.code = filtered ? "EE" : "BB",
                         .direct = filtered ? "20.00" : "10.00",
                         .unit = filtered ? "4.00" : "2.00"};
  reading.Seek(missing);
  CHECK_TRUE("missing equality reports false", !reading.Find("="));
  reading.Check(missing);
  CHECK_TRUE("combined seek advances according to direction", reading.Find("=><"));
  reading.Check(
      population[filtered ? (ascending ? first + count - 1 : first + 1) : (ascending ? 2 : 1)]);
}

void Variants(Cost &row) {
  for (const bool ascending : {true, false}) {
    for (const bool filtered : {false, true}) {
      for (const bool mixed : {true, false}) {
        Matrix<false>(row, ascending, filtered, mixed);
        Matrix<true>(row, ascending, filtered, mixed);
      }
    }
  }
}

}

int main() {
  return gate::Run("MixedOrder", [] {
    agiru::Temporary<Cost> temporary;
    Fill(temporary);
    Variants(temporary);
    const agiru::Session session(AGIRU_TEST_DSN);
    const auto &table = agiru::TableDefinition<Cost>();
    agiru::DropTable(agiru::Session::Current().Database(), table);
    agiru::CreateTable(agiru::Session::Current().Database(), table);
    Cost stored;
    Fill(stored);
    Variants(stored);
  });
}
