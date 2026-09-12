#pragma once

#include "dotnet/Refused.h"
#include "type/Integer.h"
#include "type/Text.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace agiru::dotnet {

/// \brief .NET `System.Globalization.CultureInfo`, rebuilt as a NAME AND A NUMBER: the language
///        tag (`en-US`), its Windows LCID (1033), the two-letter ISO name (`en`) and the
///        three-letter Windows name (`ENU`), from a table of the cultures Business Central ships
///        a language for. `Language.GetCultureName`, `Azure AD Graph User`'s language mapping and
///        a `DataTable`'s `Locale` read those four; nothing here formats a number or a date.
/// \warning `DateTimeFormat`, `NumberFormat` and `TextInfo` are REFUSED: a culture's patterns are
///          the formatting layer's, which is not rebuilt yet (board:0035). A culture this table
///          does not carry keeps the name or number it was made from and answers `Unknown` for
///          the rest, the way .NET answers a custom culture, rather than refusing.
class CultureInfo {
public:
  /// \brief The binder behind `C := C.CultureInfo(1033)` and `C := C.CultureInfo('en-US')`.
  struct Binder {
    /// \brief `new CultureInfo(lcid)`. \param lcid The Windows language id. \return The culture.
    [[nodiscard]] class CultureInfo operator()(Integer lcid) const;

    /// \brief `new CultureInfo(name)`. \param name The language tag. \return The culture.
    [[nodiscard]] class CultureInfo operator()(std::string_view name) const;
  };

  /// \brief The constructor AL calls as a member.
  Binder CultureInfo; // NOLINT(misc-non-private-member-variables-in-classes)

  /// \brief `CultureInfo.InvariantCulture`: the culture with no name and LCID 127. \return It.
  [[nodiscard]] static class CultureInfo InvariantCulture();

  /// \brief `CultureInfo.CurrentCulture`: the session's language. \return It.
  [[nodiscard]] static class CultureInfo CurrentCulture();

  /// \brief `CultureInfo.GetCultureInfo(lcid)`. \param lcid The id. \return The culture.
  [[nodiscard]] static class CultureInfo GetCultureInfo(Integer lcid);

  /// \brief `CultureInfo.GetCultureInfo(name)`. \param name The tag. \return The culture.
  [[nodiscard]] static class CultureInfo GetCultureInfo(std::string_view name);

  /// \brief `CultureInfo.Name`: the language tag, `en-US`; empty for the invariant culture.
  /// \return It.
  [[nodiscard]] ::agiru::Text<0> Name() const { return name_; }

  /// \brief `CultureInfo.LCID`. \return The Windows language id; 127 for the invariant culture.
  [[nodiscard]] Integer LCID() const { return lcid_; }

  /// \brief `CultureInfo.TwoLetterISOLanguageName`, `en`; `iv` for the invariant culture.
  /// \return It.
  [[nodiscard]] ::agiru::Text<0> TwoLetterISOLanguageName() const { return two_; }

  /// \brief `CultureInfo.ThreeLetterWindowsLanguageName`, `ENU`; `IVL` for the invariant
  ///        culture. \return It.
  [[nodiscard]] ::agiru::Text<0> ThreeLetterWindowsLanguageName() const { return three_; }

  /// \brief `CultureInfo.Parent`: the neutral culture of a specific one, `en` for `en-US`; the
  ///        invariant culture is its own parent. \return It.
  [[nodiscard]] class CultureInfo Parent() const;

  /// \brief `CultureInfo.ToString()`: the name. \return It.
  [[nodiscard]] ::agiru::Text<0> ToString() const { return name_; }

  /// \brief What `Format(CultureInfo)` renders: the name. \return It.
  [[nodiscard]] std::string ToText() const { return name_; }

  /// \brief `CultureInfo.DateTimeFormat`: the culture's date and time patterns, not rebuilt.
  Refused DateTimeFormat{{.type = "CultureInfo", .member = "DateTimeFormat"}};
  /// \brief `CultureInfo.NumberFormat`: the culture's number patterns, not rebuilt.
  Refused NumberFormat{{.type = "CultureInfo", .member = "NumberFormat"}};
  /// \brief `CultureInfo.TextInfo`: the culture's casing rules, not rebuilt.
  Refused TextInfo{{.type = "CultureInfo", .member = "TextInfo"}};

  /// \brief `Culture := AbsentType.Member()`: the call refused before the assignment.
  /// \tparam R The refusal. \param refused It. \return This.
  template <typename R>
    requires requires { typename R::IsAlRefusal; }
  class CultureInfo &operator=(const R &refused) {
    static_cast<void>(refused);
    return *this;
  }

  /// \brief Fills the four values; what the binder and the factories call. \param lcid The id.
  ///        \param name The tag. \param two The ISO name. \param three The Windows name.
  /// \return This.
  class CultureInfo &
  Fill_(std::int32_t lcid, std::string_view name, std::string_view two, std::string_view three);

private:
  std::string name_;
  std::string two_;
  std::string three_;
  std::int32_t lcid_ = 0;
};

}
