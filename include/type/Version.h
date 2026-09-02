#pragma once

#include "type/Integer.h"

#include <array>
#include <compare>
#include <string>
#include <string_view>

/// \file
/// \brief AL `Version` -- four numbers that order a build.

namespace agiru {

/// \brief AL `Version`.
///
/// From `version-data-type.md`: four parts, `Major.Minor.Build.Revision`, and it is what
/// `ModuleInfo.AppVersion` and `ModuleInfo.DataVersion` carry.
///
/// \note THE ORDER IS THE FOUR NUMBERS IN ORDER, not the text. `10.0` sorts before `9.0` as text
///       and after it as a version, and an upgrade codeunit that compared the wrong one would run
///       the wrong migrations.
class Version {
public:
  /// \brief The zero version, `0.0.0.0`.
  constexpr Version() = default;

  /// \brief AL `Version.Create(Major, Minor, Build, Revision)`.
  /// \param major    The first part.
  /// \param minor    The second.
  /// \param build    The third.
  /// \param revision The fourth.
  constexpr Version(Integer major, Integer minor, Integer build, Integer revision)
      : parts_{major, minor, build, revision} {}

  /// \brief AL `Version.Create(Major, Minor)` -- the other two are zero.
  /// \param major The first part.
  /// \param minor The second.
  constexpr Version(Integer major, Integer minor) : parts_{major, minor, 0, 0} {}

  /// \brief AL `Version.Create(Text)` -- `"1.2.3.4"`, and shorter forms.
  /// \param text The version, dot-separated.
  /// \return The version; missing parts are zero.
  /// \throws Error when a part is not a number.
  [[nodiscard]] static Version FromText(std::string_view text);

  /// \brief AL `Version.Major()`. \return The first part.
  [[nodiscard]] constexpr Integer Major() const { return parts_[0]; }

  /// \brief AL `Version.Minor()`. \return The second part.
  [[nodiscard]] constexpr Integer Minor() const { return parts_[1]; }

  /// \brief AL `Version.Build()`. \return The third part.
  [[nodiscard]] constexpr Integer Build() const { return parts_[2]; }

  /// \brief AL `Version.Revision()`. \return The fourth part.
  [[nodiscard]] constexpr Integer Revision() const { return parts_[3]; }

  /// \brief AL `Version.ToText()`.
  /// \return `Major.Minor.Build.Revision`.
  [[nodiscard]] std::string ToText() const;

  /// \brief Orders two versions by their four numbers.
  /// \param o The other.
  /// \return The ordering.
  [[nodiscard]] constexpr std::strong_ordering operator<=>(const Version &o) const = default;

  /// \brief Compares two versions.
  /// \param o The other.
  /// \return True when all four parts agree.
  [[nodiscard]] constexpr bool operator==(const Version &o) const = default;

private:
  /// THE FOUR NUMBERS IN ONE PLACE, which is what `<=>` needs to order a version the way a version
  /// orders: `10.0` after `9.0` and not before it, as the text would have it. It also makes the
  /// constructor use its four parameters in ONE expression, which is what tells a reader -- and the
  /// swappable-parameter check -- that they belong together rather than being four loose numbers.
  std::array<Integer, 4> parts_{};
};

} // namespace agiru
