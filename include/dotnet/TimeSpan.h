#pragma once

#include "type/BigInteger.h"
#include "type/Decimal.h"
#include "type/Integer.h"

#include <cstdint>

namespace agiru::dotnet {

/// \brief .NET `System.TimeSpan`, rebuilt as the ticks it counts (one tick is 100 ns): `Regex
///        Impl.` hands `TimeSpan.FromTicks(MatchTimeoutInMs * 10000)` to a regex, and the
///        BaseApp constructs one and asks `FromSeconds` (measured 2026-09-10).
class TimeSpan {
public:
  /// \brief The binder behind `T := T.TimeSpan(...)`, the constructor as AL spells it.
  struct Binder {
    /// \brief `new TimeSpan()` and `new TimeSpan(ticks)`.
    /// \param ticks The ticks; zero when omitted.
    /// \return The span.
    [[nodiscard]] class TimeSpan operator()(std::int64_t ticks = 0) const;

    /// \brief `new TimeSpan(hours, minutes, seconds)`. \param hours Hours. \param minutes Minutes.
    /// \param seconds Seconds. \return The span.
    [[nodiscard]] class TimeSpan
    operator()(::agiru::Integer hours, ::agiru::Integer minutes, ::agiru::Integer seconds) const;

    /// \brief `new TimeSpan(days, hours, minutes, seconds)`. \param days Days. \param hours Hours.
    /// \param minutes Minutes. \param seconds Seconds. \return The span.
    [[nodiscard]] class TimeSpan operator()(::agiru::Integer days,
                                            ::agiru::Integer hours,
                                            ::agiru::Integer minutes,
                                            ::agiru::Integer seconds) const;

    /// \brief `new TimeSpan(days, hours, minutes, seconds, milliseconds)`. \param days Days.
    /// \param hours Hours. \param minutes Minutes. \param seconds Seconds.
    /// \param milliseconds Milliseconds. \return The span.
    [[nodiscard]] class TimeSpan operator()(::agiru::Integer days,
                                            ::agiru::Integer hours,
                                            ::agiru::Integer minutes,
                                            ::agiru::Integer seconds,
                                            ::agiru::Integer milliseconds) const;
  };

  /// \brief `T.TimeSpan(...)`, the constructor as AL calls it.
  Binder TimeSpan;

  /// \brief `TimeSpan.FromTicks(ticks)`. \param ticks The ticks. \return The span.
  [[nodiscard]] static class TimeSpan FromTicks(std::int64_t ticks);

  /// \brief `TimeSpan.FromSeconds(seconds)`. \param seconds The seconds. \return The span.
  [[nodiscard]] static class TimeSpan FromSeconds(const Decimal &seconds);

  /// \brief `TimeSpan.FromMilliseconds(milliseconds)`. \param milliseconds The count. \return The span.
  [[nodiscard]] static class TimeSpan FromMilliseconds(const Decimal &milliseconds);

  /// \brief `TimeSpan.Ticks`. \return The ticks.
  [[nodiscard]] std::int64_t Ticks() const { return ticks_; }

  /// \brief `TimeSpan.TotalMilliseconds`. \return The span in milliseconds.
  [[nodiscard]] Decimal TotalMilliseconds() const;

  /// \brief `TimeSpan.TotalSeconds`. \return The span in seconds.
  [[nodiscard]] Decimal TotalSeconds() const;

private:
  std::int64_t ticks_ = 0;
};

}
