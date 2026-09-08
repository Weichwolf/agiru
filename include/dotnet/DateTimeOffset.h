#pragma once

#include "dotnet/DateTime.h"
#include "type/BigInteger.h"
#include "type/DateTime.h"
#include "type/Duration.h"

#include <string>
#include <string_view>

/// \file
/// \brief The .NET type `System.DateTimeOffset` -- an instant and the offset it was read in.

namespace agiru::dotnet {

/// \brief `System.DateTimeOffset`.
///
/// AL reaches it through `Codeunit DotNet_DateTimeOffset`, BC's own wrapper, and the BaseApp's
/// `ConvertToUtcDateTime` is what every caller wants:
///
/// ```AL
/// DotNetDateTimeOffsetSource := DotNetDateTimeOffsetSource.DateTimeOffset(DateTimeSource);
/// DotNetDateTimeOffsetNow := DotNetDateTimeOffsetNow.Now;
/// exit(DotNetDateTimeOffsetSource.LocalDateTime - DotNetDateTimeOffsetNow.Offset);
/// ```
///
/// Measured over BCApps: `ConvertToUtcDateTime` 34 call sites, `GetOffset` 6, `Parse` 6,
/// `ToString` 2, and one each of the rest.
///
/// \note THE SESSION'S OFFSET IS ZERO, AND THAT IS AN ANSWER RATHER THAN A PLACEHOLDER.
///       `devenv-about-dates.md`: BC "stores all `DateTime` fields as UTC and in the UI layer, we
///       convert these fields to the timezone, specified by the user on the User Settings page".
///       A session's zone is therefore a USER SETTING, and this runtime has no user settings yet --
///       so its sessions run in UTC, where local time IS the instant and the offset IS zero. That
///       is also the only DETERMINISTIC answer: reading the host's zone would make the same test
///       give two results on two machines (board:0618).
///
/// \note IT DECLARES NO CONSTRUCTOR, and that is a language rule rather than a preference. AL
///       spells .NET's constructor `X := X.DateTimeOffset(dt)`, so this type needs a member
///       carrying its own name -- which C++ forbids as a member FUNCTION outright, and allows as a
///       data member only in a class that declares no constructor of its own.
struct DateTimeOffset {
  /// \brief What `X.DateTimeOffset(instant)` is: .NET's constructor, spelled as AL spells it.
  struct FromInstant {
    /// \brief Builds one from an instant.
    /// \param at The instant.
    /// \return The offset value holding it.
    [[nodiscard]] DateTimeOffset operator()(const ::agiru::DateTime &at) const;

    /// \brief Builds one from an instant and an OFFSET, which .NET takes as a `TimeSpan`.
    /// \tparam Offset The offset's type, which is `TimeSpan` and is not rebuilt here.
    /// \param at     The instant.
    /// \param offset The offset, discarded.
    /// \return The offset value holding the instant.
    ///
    /// \note THE OFFSET IS DISCARDED AND THAT IS THE SESSION'S ANSWER, not an omission: a session
    ///       with no user setting runs in UTC, so the only offset it can be handed and honour is
    ///       zero. `CertificateRequestImpl` builds both of its bounds this way with a zero
    ///       `TimeSpan`, which is the same answer either way (board:0618).
    template <typename Offset>
    [[nodiscard]] DateTimeOffset operator()(const ::agiru::DateTime &at,
                                            const Offset &offset) const {
      static_cast<void>(offset);
      return (*this)(at);
    }
  };

  /// \brief `new DateTimeOffset(dt)` -- AL writes it as a member of the variable it assigns to.
  FromInstant DateTimeOffset{};

  /// \brief `DateTimeOffset.Now` -- this instant, in the session's own zone.
  /// \return The instant now.
  [[nodiscard]] struct DateTimeOffset Now() const;

  /// \brief `DateTimeOffset.UtcNow` -- this instant in UTC, which is the same instant.
  /// \return The instant now.
  [[nodiscard]] struct DateTimeOffset UtcNow() const;

  /// \brief `DateTimeOffset.Offset` -- how far the session's zone is from UTC.
  /// \return Zero, because a session with no user setting runs in UTC.
  [[nodiscard]] ::agiru::Duration Offset() const;

  /// \brief `DateTimeOffset.LocalDateTime` -- the instant as the session's zone shows it.
  /// \return The instant.
  [[nodiscard]] ::agiru::DateTime LocalDateTime() const { return at_; }

  /// \brief `DateTimeOffset.UtcDateTime` -- the instant in UTC.
  /// \return The instant.
  [[nodiscard]] ::agiru::DateTime UtcDateTime() const { return at_; }

  /// \brief `DateTimeOffset.DateTime` -- the instant without its offset.
  /// \return The instant, as the .NET type the wrapper hands it to.
  [[nodiscard]] ::agiru::dotnet::DateTime DateTime() const;

  /// \brief `DateTimeOffset.ToLocalTime()`.
  /// \return This value, because the session's zone is UTC.
  [[nodiscard]] struct DateTimeOffset ToLocalTime() const { return *this; }

  /// \brief `DateTimeOffset.ToUniversalTime()`.
  /// \return This value, because the session's zone is UTC.
  [[nodiscard]] struct DateTimeOffset ToUniversalTime() const { return *this; }

  /// \brief `DateTimeOffset.ToUnixTimeSeconds()`.
  /// \return The seconds since 1970-01-01, which may be negative.
  [[nodiscard]] ::agiru::BigInteger ToUnixTimeSeconds() const;

  /// \brief `DateTimeOffset.ToUnixTimeMilliseconds()`.
  /// \return The milliseconds since 1970-01-01, which may be negative.
  [[nodiscard]] ::agiru::BigInteger ToUnixTimeMilliseconds() const;

  /// \brief `DateTimeOffset.FromUnixTimeSeconds(seconds)`.
  /// \param seconds The seconds since 1970-01-01.
  /// \return The instant they name.
  [[nodiscard]] struct DateTimeOffset FromUnixTimeSeconds(::agiru::BigInteger seconds) const;

  /// \brief `DateTimeOffset.FromUnixTimeMilliseconds(milliseconds)`.
  /// \param milliseconds The milliseconds since 1970-01-01.
  /// \return The instant they name.
  [[nodiscard]] struct DateTimeOffset
  FromUnixTimeMilliseconds(::agiru::BigInteger milliseconds) const;

  /// \brief `DateTimeOffset.Parse(text)` -- reads the ISO 8601 form this type writes.
  /// \param text The instant, as `ToString()` renders it.
  /// \return The instant, or the undefined one when the text is not that form.
  ///
  /// \note IT READS WHAT IT WRITES AND NOT EVERY FORM .NET ACCEPTS. `DateTime::ToInvariantString`
  ///       renders `1996-05-21T09:15:24.614Z`, and that is what this reads back; a form with an
  ///       offset, a locale or a missing part is a refusal rather than a guess.
  [[nodiscard]] struct DateTimeOffset Parse(std::string_view text) const;

  /// \brief `DateTimeOffset.Parse(text, formatInfo, styles)` -- .NET's fuller form.
  /// \tparam Rest What the call carries after the text.
  /// \param text The instant.
  /// \param rest The format info and the styles, discarded.
  /// \return The instant, or the undefined one.
  ///
  /// \note THE STYLES ARE DISCARDED BECAUSE THE ONE THAT IS USED IS THE ONE THIS READS.
  ///       `UnixTimestampImpl` passes `DateTimeStyles.RoundtripKind`, which is the ISO 8601 form
  ///       `ToString()` writes; a style that asked for another form would be a refusal and is not
  ///       written anywhere in BCApps (measured 2026-09-08).
  template <typename... Rest>
  [[nodiscard]] struct DateTimeOffset Parse(std::string_view text, const Rest &...rest) const {
    (static_cast<void>(rest), ...);
    return Parse(text);
  }

  /// \brief `DateTimeOffset.ToString()` -- the ISO 8601 form.
  /// \return The instant as text.
  [[nodiscard]] std::string ToString() const;

  /// \brief `DateTimeOffset.ToString(format, culture)` -- the form a .NET specification asks for.
  /// \param format The .NET format string.
  /// \param culture The culture, which this runtime does not have.
  /// \return Never.
  /// \throws Error always -- a .NET format specification is not this runtime's `Format` (board:0007).
  template <typename... Rest>
  [[nodiscard]] std::string ToString(std::string_view format, const Rest &...rest) const {
    (static_cast<void>(rest), ...);
    if (format == kRoundTrip) { return ToString(); }
    return Refuse(format);
  }

  /// \brief The instant this value holds, which is what everything above reads.
  ::agiru::DateTime at_{};

private:
  /// \brief .NET's round-trip specifier, which IS the form `ToString()` writes.
  static constexpr std::string_view kRoundTrip = "o";

  [[noreturn]] static std::string Refuse(std::string_view format);
};

}
