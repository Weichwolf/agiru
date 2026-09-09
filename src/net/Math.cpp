#include "dotnet/Math.h"

#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Decimal.h"
#include "type/Integer.h"

#include <cmath>
#include <cstdint>
#include <format>
#include <string>

namespace agiru::dotnet {

namespace {

constexpr int kSignificantDigits = 15;
constexpr int kMaximumScale = 28;

double AsDouble(const ::agiru::Decimal &value) {
  return std::stod(value.ToInvariantString());
}

::agiru::Decimal AsDecimal(double value) {
  if (!std::isfinite(value)) { throw Error("Math: the result is not a number"); }
  if (value == 0.0) { return ::agiru::Decimal{0}; }
  const int magnitude = static_cast<int>(std::floor(std::log10(std::fabs(value))));
  int scale = kSignificantDigits - 1 - magnitude;
  if (scale < 0) { scale = 0; }
  if (scale > kMaximumScale) { scale = kMaximumScale; }
  std::string text = std::format("{:.{}f}", value, scale);
  if (text.find('.') != std::string::npos) {
    while (text.ends_with('0')) { text.pop_back(); }
    if (text.ends_with('.')) { text.pop_back(); }
  }
  return ::agiru::Decimal::FromInvariantString(text);
}

}

::agiru::Decimal Math::Abs(const ::agiru::Decimal &value) {
  return value.Abs();
}

::agiru::Integer Math::Sign(const ::agiru::Decimal &value) {
  if (value.IsZero()) { return 0; }
  return value < ::agiru::Decimal{0} ? -1 : 1;
}

::agiru::Decimal Math::Min(const ::agiru::Decimal &a, const ::agiru::Decimal &b) {
  return b < a ? b : a;
}

::agiru::Decimal Math::Max(const ::agiru::Decimal &a, const ::agiru::Decimal &b) {
  return a < b ? b : a;
}

::agiru::Decimal Math::Truncate(const ::agiru::Decimal &value) {
  return Round(value, ::agiru::Decimal{1}, RoundDirection::Down);
}

::agiru::Decimal Math::Floor(const ::agiru::Decimal &value) {
  return Round(value,
               ::agiru::Decimal{1},
               value < ::agiru::Decimal{0} ? RoundDirection::Up : RoundDirection::Down);
}

::agiru::Decimal Math::Ceiling(const ::agiru::Decimal &value) {
  return Round(value,
               ::agiru::Decimal{1},
               value < ::agiru::Decimal{0} ? RoundDirection::Down : RoundDirection::Up);
}

::agiru::BigInteger Math::BigMul(::agiru::Integer a, ::agiru::Integer b) {
  return static_cast<std::int64_t>(a) * static_cast<std::int64_t>(b);
}

::agiru::Decimal Math::Sqrt(const ::agiru::Decimal &value) {
  if (value < ::agiru::Decimal{0}) { throw Error("Math.Sqrt: the value is negative"); }
  return AsDecimal(std::sqrt(AsDouble(value)));
}

::agiru::Decimal Math::Pow(const ::agiru::Decimal &x, const ::agiru::Decimal &y) {
  return AsDecimal(std::pow(AsDouble(x), AsDouble(y)));
}

::agiru::Decimal Math::Exp(const ::agiru::Decimal &value) {
  return AsDecimal(std::exp(AsDouble(value)));
}

::agiru::Decimal Math::Log(const ::agiru::Decimal &value) {
  return AsDecimal(std::log(AsDouble(value)));
}

::agiru::Decimal Math::Log(const ::agiru::Decimal &a, const ::agiru::Decimal &newBase) {
  return AsDecimal(std::log(AsDouble(a)) / std::log(AsDouble(newBase)));
}

::agiru::Decimal Math::Log10(const ::agiru::Decimal &value) {
  return AsDecimal(std::log10(AsDouble(value)));
}

::agiru::Decimal Math::IEEERemainder(const ::agiru::Decimal &x, const ::agiru::Decimal &y) {
  return AsDecimal(std::remainder(AsDouble(x), AsDouble(y)));
}

::agiru::Decimal Math::Sin(const ::agiru::Decimal &value) {
  return AsDecimal(std::sin(AsDouble(value)));
}

::agiru::Decimal Math::Cos(const ::agiru::Decimal &value) {
  return AsDecimal(std::cos(AsDouble(value)));
}

::agiru::Decimal Math::Tan(const ::agiru::Decimal &value) {
  return AsDecimal(std::tan(AsDouble(value)));
}

::agiru::Decimal Math::Asin(const ::agiru::Decimal &value) {
  return AsDecimal(std::asin(AsDouble(value)));
}

::agiru::Decimal Math::Acos(const ::agiru::Decimal &value) {
  return AsDecimal(std::acos(AsDouble(value)));
}

::agiru::Decimal Math::Atan(const ::agiru::Decimal &value) {
  return AsDecimal(std::atan(AsDouble(value)));
}

::agiru::Decimal Math::Atan2(const ::agiru::Decimal &y, const ::agiru::Decimal &x) {
  return AsDecimal(std::atan2(AsDouble(y), AsDouble(x)));
}

::agiru::Decimal Math::Sinh(const ::agiru::Decimal &value) {
  return AsDecimal(std::sinh(AsDouble(value)));
}

::agiru::Decimal Math::Cosh(const ::agiru::Decimal &value) {
  return AsDecimal(std::cosh(AsDouble(value)));
}

::agiru::Decimal Math::Tanh(const ::agiru::Decimal &value) {
  return AsDecimal(std::tanh(AsDouble(value)));
}

}
