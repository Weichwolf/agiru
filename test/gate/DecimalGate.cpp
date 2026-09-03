#include "type/Decimal.h"

#include "Check.h"

#include <string>

using agiru::Decimal;
using agiru::DecimalError;
using agiru::Round;
using agiru::RoundDirection;

namespace {

std::string T(const Decimal &d) {
  return d.ToInvariantString();
}

Decimal D(const char *s) {
  return Decimal::FromInvariantString(s);
}

void TextRoundTrip() {
  CHECK_TEXT("a number survives text there and back", T(D("1234.56789")), "1234.56789");
  CHECK_TEXT("a negative sign stays", T(D("-0.001")), "-0.001");
  CHECK_TEXT("a leading zero is supplied", T(D(".5")), "0.5");
  CHECK_TEXT("zero carries no sign", T(D("-0.00")), "0.00");
  // CLR parsing preserves the written scale. This is the half of the type everyone assumes wrongly.
  CHECK_TEXT("the written scale is preserved", T(D("1.2300")), "1.2300");
  // The documentation names this as the maximum calculating value: 2^96 - 1.
  CHECK_TEXT("the calculating range from the documentation holds",
             T(Decimal::MaxValue()),
             "79228162514264337593543950335");
}

void Arithmetic() {
  CHECK_TEXT("addition aligns the scales", T(D("0.1") + D("0.02")), "0.12");
  CHECK_TEXT("subtraction across zero", T(D("0.1") - D("0.3")), "-0.2");
  CHECK_TEXT("multiplication adds the scales", T(D("1.5") * D("1.5")), "2.25");
  CHECK_TEXT("multiplication keeps s1 + s2, as CLR does", T(D("2.50") * D("2")), "5.00");
  CHECK_TEXT("sign under multiplication", T(D("-2.5") * D("4")), "-10.0");
  CHECK_TEXT("division that comes out even", T(D("1") / D("2")), "0.5");
  CHECK_TEXT("division that does not, fills 28 places",
             T(D("1") / D("3")),
             "0.3333333333333333333333333333");
  CHECK_TEXT("division by a fraction", T(D("1") / D("0.25")), "4");

  // WHAT A BINARY FLOAT GETS WRONG HERE, and the reason for the whole invariant: as a double,
  // 0.1 + 0.2 is not 0.3.
  CHECK_TEXT("0.1 + 0.2 is exactly 0.3", T(D("0.1") + D("0.2")), "0.3");

  // Addition carries the scale through, so this is 100.00 and not 100. Same value, and the
  // representation is part of the value.
  Decimal cent = D("0.00");
  constexpr int kCents = 10000;
  for (int i = 0; i < kCents; ++i) { cent += D("0.01"); }
  CHECK_TEXT("ten thousand cents are exactly one hundred", T(cent), "100.00");
  CHECK_TRUE("and equal to the integer hundred", cent == Decimal(100));
}

void Comparison() {
  CHECK_TRUE("the same number written differently is equal", D("1.50") == D("1.5"));
  CHECK_TRUE("negative is less than positive", D("-0.0001") < D("0"));
  CHECK_TRUE("among negatives the order inverts", D("-5") < D("-4"));
  CHECK_TRUE("scales are aligned before comparing", D("0.30") > D("0.2999"));
}

void RoundingPerTheDocumentation() {
  // The example is written out in system-round-method.md:
  //   DecimalToRound := 1234.56789; Precision := 0.001; Direction := '>';
  CHECK_TEXT("the documentation's own example",
             T(Round(D("1234.56789"), D("0.001"), RoundDirection::Up)),
             "1234.568");
  CHECK_TEXT("'=' rounds to the nearest multiple",
             T(Round(D("1234.56789"), D("0.001"), RoundDirection::Nearest)),
             "1234.568");
  CHECK_TEXT("'<' rounds toward zero",
             T(Round(D("1234.56789"), D("0.001"), RoundDirection::Down)),
             "1234.567");
  CHECK_TEXT("exactly five rounds up", T(Round(D("0.125"), D("0.01"))), "0.13");
  CHECK_TEXT("precision is a multiple, not a digit count", T(Round(D("1.23"), D("0.05"))), "1.25");
  CHECK_TEXT("to whole numbers", T(Round(D("2.5"), D("1"))), "3");

  // THE TRAP the predecessor paid for (openerp builtins/_math.py:_al_round): '>' and '<' work on
  // the MAGNITUDE. ceil/floor invert both for negative numbers -- and negative amounts are the rule
  // in an ERP: credit memos, reversals, negative deltas.
  CHECK_TEXT("'>' on a negative goes AWAY from zero, not upward",
             T(Round(D("-1234.56789"), D("0.001"), RoundDirection::Up)),
             "-1234.568");
  CHECK_TEXT("'<' on a negative goes TOWARD zero, not downward",
             T(Round(D("-1234.56789"), D("0.001"), RoundDirection::Down)),
             "-1234.567");
}

void FailuresAreLoud() {
  bool threw = false;
  try {
    (void)(D("1") / D("0"));
  } catch (const DecimalError &) { threw = true; }
  CHECK_TRUE("division by zero throws", threw);

  threw = false;
  try {
    (void)Round(D("1"), D("0"));
  } catch (const DecimalError &) { threw = true; }
  CHECK_TRUE("rounding to a precision of zero throws", threw);

  threw = false;
  try {
    (void)D("not a number");
  } catch (const DecimalError &) { threw = true; }
  CHECK_TRUE("text that is not a number throws", threw);

  threw = false;
  try {
    Decimal big = Decimal::MaxValue();
    big += Decimal(1);
  } catch (const DecimalError &) { threw = true; }
  CHECK_TRUE("an overflow beyond 2^96-1 is loud, not silent", threw);
}

} // namespace

int main() {
  return gate::Run("Decimal", [] {
    TextRoundTrip();
    Arithmetic();
    Comparison();
    RoundingPerTheDocumentation();
    FailuresAreLoud();
  });
}
