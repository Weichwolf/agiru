#pragma once

#include "type/BigInteger.h"
#include "type/Decimal.h"
#include "type/Integer.h"

/// \file
/// \brief The .NET type `System.Math`, behind AL's `Codeunit Math`.

namespace agiru::dotnet {

/// \brief .NET `System.Math`: the members `Codeunit Math` (System Application) reaches, which is
///        every member the BaseApp reaches.
///
/// \note IT COMPUTES IN `double`, AS .NET DOES. The AL codeunit passes a Decimal to a double
///       method and takes a double back, and the CLR converts the result to Decimal at FIFTEEN
///       significant digits (`Decimal(Double)` in the .NET reference) -- so this is what a BC
///       user sees from `Math.Sqrt(2)`, and not a 28-digit root. No amount goes through here: a
///       posting never calls a trigonometric function, and `Abs`, `Min`, `Max`, `Sign`,
///       `Truncate`, `Floor` and `Ceiling` are computed on the Decimal itself.
class Math {
public:
  /// \brief The helper; it holds nothing.
  Math() = default;

  /// \brief `Math.Abs`. \param value The value. \return Its magnitude.
  [[nodiscard]] static ::agiru::Decimal Abs(const ::agiru::Decimal &value);
  /// \brief `Math.Sign`. \param value The value. \return -1, 0 or 1.
  [[nodiscard]] static ::agiru::Integer Sign(const ::agiru::Decimal &value);
  /// \brief `Math.Min`. \param a One. \param b The other. \return The smaller.
  [[nodiscard]] static ::agiru::Decimal Min(const ::agiru::Decimal &a, const ::agiru::Decimal &b);
  /// \brief `Math.Max`. \param a One. \param b The other. \return The larger.
  [[nodiscard]] static ::agiru::Decimal Max(const ::agiru::Decimal &a, const ::agiru::Decimal &b);
  /// \brief `Math.Truncate`. \param value The value. \return The integral part, towards zero.
  [[nodiscard]] static ::agiru::Decimal Truncate(const ::agiru::Decimal &value);
  /// \brief `Math.Floor`. \param value The value. \return The largest integer not above it.
  [[nodiscard]] static ::agiru::Decimal Floor(const ::agiru::Decimal &value);
  /// \brief `Math.Ceiling`. \param value The value. \return The smallest integer not below it.
  [[nodiscard]] static ::agiru::Decimal Ceiling(const ::agiru::Decimal &value);
  /// \brief `Math.BigMul`. \param a One. \param b The other. \return The 64-bit product.
  [[nodiscard]] static ::agiru::BigInteger BigMul(::agiru::Integer a, ::agiru::Integer b);

  /// \brief `Math.Sqrt`. \param value The value. \return Its square root.
  /// \throws Error for a negative value, where .NET would answer NaN.
  [[nodiscard]] static ::agiru::Decimal Sqrt(const ::agiru::Decimal &value);
  /// \brief `Math.Pow`. \param x The base. \param y The exponent. \return x to the y.
  [[nodiscard]] static ::agiru::Decimal Pow(const ::agiru::Decimal &x, const ::agiru::Decimal &y);
  /// \brief `Math.Exp`. \param value The exponent. \return e to it.
  [[nodiscard]] static ::agiru::Decimal Exp(const ::agiru::Decimal &value);
  /// \brief `Math.Log`. \param value The value. \return Its natural logarithm.
  [[nodiscard]] static ::agiru::Decimal Log(const ::agiru::Decimal &value);
  /// \brief `Math.Log(a, newBase)`. \param a The value. \param newBase The base. \return log.
  [[nodiscard]] static ::agiru::Decimal Log(const ::agiru::Decimal &a,
                                            const ::agiru::Decimal &newBase);
  /// \brief `Math.Log10`. \param value The value. \return Its base-10 logarithm.
  [[nodiscard]] static ::agiru::Decimal Log10(const ::agiru::Decimal &value);
  /// \brief `Math.IEEERemainder`. \param x The dividend. \param y The divisor. \return x - y*n.
  [[nodiscard]] static ::agiru::Decimal IEEERemainder(const ::agiru::Decimal &x,
                                                      const ::agiru::Decimal &y);

  /// \brief `Math.Sin`. \param value Radians. \return The sine.
  [[nodiscard]] static ::agiru::Decimal Sin(const ::agiru::Decimal &value);
  /// \brief `Math.Cos`. \param value Radians. \return The cosine.
  [[nodiscard]] static ::agiru::Decimal Cos(const ::agiru::Decimal &value);
  /// \brief `Math.Tan`. \param value Radians. \return The tangent.
  [[nodiscard]] static ::agiru::Decimal Tan(const ::agiru::Decimal &value);
  /// \brief `Math.Asin`. \param value The sine. \return Radians.
  [[nodiscard]] static ::agiru::Decimal Asin(const ::agiru::Decimal &value);
  /// \brief `Math.Acos`. \param value The cosine. \return Radians.
  [[nodiscard]] static ::agiru::Decimal Acos(const ::agiru::Decimal &value);
  /// \brief `Math.Atan`. \param value The tangent. \return Radians.
  [[nodiscard]] static ::agiru::Decimal Atan(const ::agiru::Decimal &value);
  /// \brief `Math.Atan2`. \param y The ordinate. \param x The abscissa. \return Radians.
  [[nodiscard]] static ::agiru::Decimal Atan2(const ::agiru::Decimal &y, const ::agiru::Decimal &x);
  /// \brief `Math.Sinh`. \param value The value. \return The hyperbolic sine.
  [[nodiscard]] static ::agiru::Decimal Sinh(const ::agiru::Decimal &value);
  /// \brief `Math.Cosh`. \param value The value. \return The hyperbolic cosine.
  [[nodiscard]] static ::agiru::Decimal Cosh(const ::agiru::Decimal &value);
  /// \brief `Math.Tanh`. \param value The value. \return The hyperbolic tangent.
  [[nodiscard]] static ::agiru::Decimal Tanh(const ::agiru::Decimal &value);
};

}
