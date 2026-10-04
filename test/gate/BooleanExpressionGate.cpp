#include "type/BooleanExpression.h"

#include "Check.h"

#include <array>
#include <stdexcept>
#include <type_traits>

namespace {
using agiru::BooleanOperands;
using agiru::LogicalAnd;
using agiru::LogicalOr;
using agiru::LogicalXor;

static_assert(std::is_aggregate_v<BooleanOperands>);
static_assert(std::is_same_v<decltype(BooleanOperands::left), const bool>);
static_assert(std::is_same_v<decltype(BooleanOperands::right), const bool>);
static_assert(LogicalAnd({.left = true, .right = true}));
static_assert(!LogicalOr({.left = false, .right = false}));
static_assert(LogicalXor({.left = true, .right = false}));

void TruthTables() {
  constexpr std::array values{false, true};
  for (const bool left : values) {
    for (const bool right : values) {
      CHECK_TRUE("conjunction truth table",
                 LogicalAnd({.left = left, .right = right}) == (left && right));
      CHECK_TRUE("disjunction truth table",
                 LogicalOr({.left = left, .right = right}) == (left || right));
      CHECK_TRUE("exclusive disjunction truth table",
                 LogicalXor({.left = left, .right = right}) == (left != right));
    }
  }
}

bool Push(int &trace, int digit, bool result) {
  trace = trace * 10 + digit;
  return result;
}

void EagerOrderAndOwnership() {
  int trace = 0;
  CHECK_TRUE("false conjunction still evaluates its right operand",
             !LogicalAnd({.left = Push(trace, 1, false), .right = Push(trace, 2, true)}));
  CHECK_TRUE("conjunction evaluates left then right", trace == 12);
  trace = 0;
  CHECK_TRUE("true disjunction still evaluates its right operand",
             LogicalOr({.left = Push(trace, 1, true), .right = Push(trace, 2, false)}));
  CHECK_TRUE("disjunction evaluates left then right", trace == 12);
  trace = 0;
  CHECK_TRUE("exclusive disjunction evaluates both operands",
             LogicalXor({.left = Push(trace, 1, true), .right = Push(trace, 2, false)}));
  CHECK_TRUE("exclusive disjunction evaluates left then right", trace == 12);
  bool value = true;
  const bool result = LogicalAnd({.left = value, .right = [&value] {
                                    value = false;
                                    return true;
                                  }()});
  CHECK_TRUE("the left value is copied before right-side mutation", result);
  CHECK_TRUE("right-side mutation remains visible to the caller", !value);
  trace = 0;
  CHECK_TRUE(
      "nested operations retain grouping and eager order",
      LogicalOr({.left = LogicalAnd({.left = Push(trace, 1, false), .right = Push(trace, 2, true)}),
                 .right = Push(trace, 3, true)}));
  CHECK_TRUE("nested operands are evaluated exactly once", trace == 123);
}

bool Fail(int &trace, int digit) {
  trace = trace * 10 + digit;
  throw std::runtime_error("Boolean producer failed");
}

void ErrorBoundaries() {
  int trace = 0;
  bool refused = false;
  try {
    (void)LogicalAnd({.left = Push(trace, 1, false), .right = Fail(trace, 2)});
  } catch (const std::runtime_error &) { refused = true; }
  CHECK_TRUE("a false left operand does not hide a right-side error", refused);
  CHECK_TRUE("both operands precede the right-side error", trace == 12);
  trace = 0;
  refused = false;
  try {
    (void)LogicalOr({.left = Fail(trace, 1), .right = Push(trace, 2, true)});
  } catch (const std::runtime_error &) { refused = true; }
  CHECK_TRUE("a left-side error propagates", refused);
  CHECK_TRUE("a left-side error prevents right evaluation", trace == 1);
}
}

int main() {
  return gate::Run("BooleanExpressionGate", [] {
    TruthTables();
    EagerOrderAndOwnership();
    ErrorBoundaries();
  });
}
