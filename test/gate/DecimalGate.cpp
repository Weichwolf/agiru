#include "dotnet/Math.h"
#include "type/Decimal.h"

#include "Check.h"

#include <initializer_list>
#include <string>

using agiru::Decimal;
using agiru::DecimalError;
using agiru::Round;
using agiru::RoundDirection;

namespace {

void NativeMathRenderingClampsTheDecimalScale() {
  using agiru::dotnet::Math;
  CHECK_TRUE("native Math keeps large integral values at scale zero",
             Math::Pow(Decimal{10}, Decimal{20}) ==
                 Decimal::FromInvariantString("100000000000000000000"));
  CHECK_TRUE("native Math caps the scale of small representable values",
             Math::Pow(Decimal{10}, Decimal{-20}) ==
                 Decimal::FromInvariantString("0.00000000000000000001"));
  CHECK_TRUE("native Math rounds below the maximum decimal scale",
             Math::Pow(Decimal{10}, Decimal{-30}) == Decimal{0});
}

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
  CHECK_TEXT("division that does not fills CLR's twenty-eight places",
             T(D("1") / D("3")),
             "0.3333333333333333333333333333");
  CHECK_TEXT("an exactly representable product remains divisible without a SQL round trip",
             T((D("1") / D("7")) * D("44") % (D("1") / D("7"))),
             "0.0000000000000000000000000000");
  CHECK_TEXT("Trimmed drops the fractional zeros and nothing else",
             T(D("10.50000000000000000000").Trimmed()),
             "10.5");
  CHECK_TEXT("Trimmed leaves an integer bare", T(D("100.00").Trimmed()), "100");
  CHECK_TEXT("a text with more places is rounded to twenty-eight",
             T(D("0.142857142857142857142857142876")),
             "0.1428571428571428571428571429");
  CHECK_TEXT("division by a fraction", T(D("1") / D("0.25")), "4");
  // AL `mod` OVER DECIMALS, which `Item Unit of Measure` uses to check a quantity against a
  // rounding precision. Without a decimal operator the call went through the Integer conversion,
  // `0.00001` became `0`, and the process died of SIGFPE (measured 2026-09-09).
  CHECK_TEXT("mod keeps the fractional remainder", T(D("10.5") % D("3")), "1.5");
  CHECK_TEXT(
      "mod against a fine precision that divides evenly", T(D("7") % D("0.00001")), "0.00000");
  CHECK_TEXT("mod against a precision that does not", T(D("7.000015") % D("0.00001")), "0.000005");
  CHECK_TEXT("mod carries the dividend's sign", T(D("-10.5") % D("3")), "-1.5");
  CHECK_TRUE("mod by an integer", (D("10.5") % 3) == D("1.5"));

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

void ClrPrecisionAndSingleRounding() {
  CHECK_TEXT("the documented CLR scale-28 value survives parsing",
             T(D("7.9228162514264337593543950335")),
             "7.9228162514264337593543950335");
  CHECK_TEXT("a parser tie retains the even mantissa",
             T(D("0.00000000000000000000000000025")),
             "0.0000000000000000000000000002");
  CHECK_TEXT("a parser tie increments an odd mantissa",
             T(D("0.00000000000000000000000000035")),
             "0.0000000000000000000000000004");
  CHECK_TEXT("nonzero digits beyond a tie are not discarded",
             T(D("0.0000000000000000000000000002500000000001")),
             "0.0000000000000000000000000003");
  CHECK_TEXT("negative parsing uses the same magnitude rounding",
             T(D("-0.00000000000000000000000000025")),
             "-0.0000000000000000000000000002");
  CHECK_TEXT("sub-quantum parsing underflows with scale 28",
             T(D("0.000000000000000000000000000049")),
             "0.0000000000000000000000000000");
  CHECK_TEXT("fractional zeros do not overflow a full integer mantissa",
             T(D("79228162514264337593543950335.000")),
             "79228162514264337593543950335");
  CHECK_TEXT("a representable rounded fraction can start with a full mantissa",
             T(D("79228162514264337593543950335.4")),
             "79228162514264337593543950335");
  CHECK_TEXT("a parser carry reduces scale instead of exceeding 96 bits",
             T(D("7.92281625142643375935439503355")),
             "7.922816251426433759354395034");
  CHECK_TEXT("multiplication never double rounds 4.9e-29 to one quantum",
             T(D("0.0000000000000049") * D("0.00000000000001")),
             "0.0000000000000000000000000000");
  CHECK_TEXT("multiplication rounds a mantissa tie to even",
             T(D("0.000000000000025") * D("0.00000000000001")),
             "0.0000000000000000000000000002");
  CHECK_TEXT("addition respects ties when fitting the 96-bit mantissa",
             T(D("7922816251426433759354395033.4") + D("0.05")),
             "7922816251426433759354395033.4");
  CHECK_TEXT("addition carries into a coarser representable scale",
             T(D("7922816251426433759354395033.5") + D("0.05")),
             "7922816251426433759354395034");
  CHECK_TEXT(
      "division preserves natural scale when it has no remainder", T(D("1.00") / D("2")), "0.50");
  CHECK_TEXT("division rounds half a quantum to even zero",
             T(D("0.0000000000000000000000000001") / D("2")),
             "0");
  CHECK_TEXT("division rounds an odd quantum tie to even",
             T(D("0.0000000000000000000000000003") / D("2")),
             "0.0000000000000000000000000002");
  CHECK_TEXT("division normalizes a negative natural scale exactly",
             T(D("0.1") / D("0.0000000000000000000000000001")),
             "1000000000000000000000000000");
  CHECK_TEXT("division rounds the maximum mantissa half to even",
             T(Decimal::MaxValue() / D("2")),
             "39614081257132168796771975168");
  CHECK_TEXT("zero division uses the exact natural scale", T(D("0.00") / D("1.00")), "0");
  CHECK_TEXT("zero division cannot retain a negative natural scale",
             T(D("0.00") / D("0.0000000000000000000000000001")),
             "0");
  CHECK_TEXT("zero division retains a positive natural scale", T(D("0.00") / D("2")), "0.00");
  CHECK_TEXT("multiplying a zero by a wide mantissa returns CLR's unscaled zero",
             T(D("0.00") * Decimal::MaxValue()),
             "0");
  CHECK_TEXT("tiny small-mantissa products outside CLR's scale window return unscaled zero",
             T(D("0.0000000000000000000000000001") * D("0.0000000000000000000000000001")),
             "0");
}

void RemainderDoesNotRequireARepresentableQuotient() {
  CHECK_TEXT("the maximum calculating value has an exact odd remainder",
             T(Decimal::MaxValue() % D("2")),
             "1");
  CHECK_TEXT(
      "a negative dividend supplies the remainder sign", T(Decimal::MinValue() % D("2")), "-1");
  CHECK_TEXT("a negative divisor does not change the remainder sign",
             T(Decimal::MaxValue() % D("-2")),
             "1");
  CHECK_TEXT("the maximum calculating value is divisible by one cent",
             T(Decimal::MaxValue() % D("0.01")),
             "0.00");
  CHECK_TEXT("a divisor with twenty decimal places does not overflow the quotient",
             T(Decimal::MaxValue() % D("0.00000000000000000011")),
             "0.00000000000000000008");
  CHECK_TEXT("extreme mixed scales retain a negative exact remainder",
             T(Decimal::MinValue() % D("0.00000000000000000011")),
             "-0.00000000000000000008");
  CHECK_TEXT("a smaller fractional dividend is retained against the maximum divisor",
             T(D("0.00000000000000000003") % Decimal::MaxValue()),
             "0.00000000000000000003");
  CHECK_TEXT("a smaller negative dividend is retained against the maximum divisor",
             T(D("-0.00000000000000000003") % Decimal::MaxValue()),
             "-0.00000000000000000003");
  CHECK_TEXT("a smaller dividend retains its scale despite a finer divisor",
             T(D("10") % D("100.00")),
             "10");
  CHECK_TEXT(
      "a smaller negative dividend also retains its scale", T(D("-10.0") % D("-100.00")), "-10.0");
  CHECK_TEXT("equal magnitudes produce zero at the larger scale", T(D("10") % D("-10.00")), "0.00");
  CHECK_TEXT("a quotient near the mantissa boundary is truncated, not rounded",
             T(D("7922816251426433759354395033.5") % D("0.2")),
             "0.1");
  Decimal self = D("10.500");
  self %= self;
  CHECK_TEXT("self-modulo reads both magnitudes before updating the value", T(self), "0.000");
  Decimal zero = D("0.000");
  zero %= D("0.01");
  CHECK_TEXT("a zero dividend retains its existing scale", T(zero), "0.000");
  bool refused = false;
  try {
    (void)(D("0") % D("0"));
  } catch (const DecimalError &) { refused = true; }
  CHECK_TRUE("zero modulo zero remains a division error", refused);
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

  for (const char *const number : {"2.5", "-2.5"}) {
    for (const char *const direction : {"=", ">", "<"}) {
      threw = false;
      try {
        (void)Round(D(number), D("-0.01"), direction);
      } catch (const DecimalError &) { threw = true; }
      CHECK_TRUE("negative rounding precision refuses for either sign and every direction", threw);
    }
  }

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
  for (const char *text : {"79228162514264337593543950336",
                           "-79228162514264337593543950336",
                           "79228162514264337593543950335.5",
                           "0.000000000000000000000000000250000x"}) {
    threw = false;
    try {
      (void)D(text);
    } catch (const DecimalError &) { threw = true; }
    CHECK_TRUE("parsing checks the final digit against the full 96-bit mantissa", threw);
  }
}

} // namespace

int main() {
  return gate::Run("Decimal", [] {
    NativeMathRenderingClampsTheDecimalScale();
    TextRoundTrip();
    Arithmetic();
    RemainderDoesNotRequireARepresentableQuotient();
    Comparison();
    ClrPrecisionAndSingleRounding();
    RoundingPerTheDocumentation();
    FailuresAreLoud();
  });
}
