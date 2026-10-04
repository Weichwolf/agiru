#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "runtime/Session.h"
#include "type/Boolean.h"
#include "type/Integer.h"

#include "Check.h"
#include "fixture/codeunit/BooleanExpressionConsumer.h"

#include <stdexcept>
#include <string_view>

namespace {
using Consumer = agiru::Fixture::BooleanExpressionConsumer_Codeunit;

void EagerOperands() {
  Consumer consumer;
  agiru::Integer trace = 0;
  CHECK_TRUE("generated AL conjunction keeps its Boolean result", !consumer.OrderedAnd(trace));
  CHECK_TRUE("generated AL conjunction evaluates both operands left to right", trace == 12);
  trace = 0;
  CHECK_TRUE("generated AL disjunction keeps its Boolean result", consumer.OrderedOr(trace));
  CHECK_TRUE("generated AL disjunction evaluates both operands left to right", trace == 12);
  trace = 0;
  CHECK_TRUE("generated AL exclusive disjunction keeps its Boolean result",
             consumer.OrderedXor(trace));
  CHECK_TRUE("generated AL exclusive disjunction evaluates left to right", trace == 12);
  trace = 0;
  CHECK_TRUE("generated AL preserves left-side grouping", consumer.NestedLeft(trace));
  CHECK_TRUE("generated AL nested left operands run exactly once", trace == 123);
  trace = 0;
  CHECK_TRUE("generated AL preserves right-side grouping", consumer.NestedRight(trace));
  CHECK_TRUE("generated AL nested right operands run exactly once", trace == 123);
  trace = 0;
  CHECK_TRUE("generated AL negates the completed expression", consumer.Negated(trace));
  CHECK_TRUE("generated AL negation retains both effects", trace == 12);
  agiru::Boolean value = true;
  CHECK_TRUE("generated AL captures the left value before right mutation",
             consumer.CaptureBeforeMutation(value));
  CHECK_TRUE("generated AL right mutation reaches the caller", !value);
}

void ErrorBoundaries() {
  Consumer consumer;
  agiru::Integer trace = 0;
  bool raised = false;
  try {
    (void)consumer.RightAndError(trace);
  } catch (const agiru::Error &error) {
    raised = std::string_view(error.what()) == "Boolean producer failed";
  }
  CHECK_TRUE("generated AL false conjunction does not hide a right error", raised);
  CHECK_TRUE("generated AL evaluates both operands before the right error", trace == 12);
  trace = 0;
  raised = false;
  try {
    (void)consumer.RightOrError(trace);
  } catch (const agiru::Error &error) {
    raised = std::string_view(error.what()) == "Boolean producer failed";
  }
  CHECK_TRUE("generated AL true disjunction does not hide a right error", raised);
  CHECK_TRUE("generated AL disjunction retains effects before the error", trace == 12);
  trace = 0;
  raised = false;
  try {
    (void)consumer.LeftError(trace);
  } catch (const agiru::Error &error) {
    raised = std::string_view(error.what()) == "Boolean producer failed";
  }
  CHECK_TRUE("generated AL propagates a left error", raised);
  CHECK_TRUE("generated AL left error prevents right evaluation", trace == 1);
  trace = 0;
  agiru::ClearLastError();
  CHECK_TRUE("generated AL conjunction consumes the TryFunction result", !consumer.CatchAnd(trace));
  CHECK_TRUE("generated AL evaluates TryFunction despite a false left value", trace == 12);
  CHECK_TEXT("generated AL keeps the consumed conjunction error",
             agiru::GetLastErrorText(),
             "Boolean try failed");
  trace = 0;
  agiru::ClearLastError();
  CHECK_TRUE("generated AL disjunction consumes the TryFunction result", consumer.CatchOr(trace));
  CHECK_TRUE("generated AL evaluates TryFunction despite a true left value", trace == 12);
  CHECK_TEXT("generated AL keeps the consumed disjunction error",
             agiru::GetLastErrorText(),
             "Boolean try failed");
}

void ValueContexts() {
  Consumer consumer;
  agiru::Integer trace = 0;
  CHECK_TRUE("generated AL condition consumes the complete conjunction",
             consumer.ConditionContext(trace) == 2);
  CHECK_TRUE("generated AL condition retains right effects", trace == 12);
  trace = 0;
  CHECK_TRUE("generated AL assignment consumes the complete disjunction",
             consumer.AssignmentContext(trace));
  CHECK_TRUE("generated AL assignment retains right effects", trace == 12);
  trace = 0;
  CHECK_TRUE("generated AL ternary selects the true branch", consumer.ChosenBranch(true, trace));
  CHECK_TRUE("generated AL ternary leaves the false branch unevaluated", trace == 1);
  trace = 0;
  CHECK_TRUE("generated AL ternary selects the false branch", !consumer.ChosenBranch(false, trace));
  CHECK_TRUE("generated AL ternary leaves the true branch unevaluated", trace == 2);
}
}

int main(int argc, char **argv) {
  return gate::Run("Generated Boolean Expressions", [argc, argv] {
    if (argc != 2) { throw std::runtime_error("expected the dedicated gate database DSN"); }
    const agiru::Session session(argv[1]);
    EagerOperands();
    ErrorBoundaries();
    ValueContexts();
  });
}
