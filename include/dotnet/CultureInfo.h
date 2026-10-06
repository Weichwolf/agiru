#pragma once

#include "dotnet/Refused.h"
#include "type/Integer.h"
#include "type/StringValue.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace agiru::dotnet {

class CultureValue;
/// \brief The AL name of the culture identity value.
using CultureInfo = CultureValue;

/// \brief .NET `System.Globalization.CultureInfo`, rebuilt as a NAME AND A NUMBER: the language
///        tag (`en-US`), its Windows LCID (1033), the two-letter ISO name (`en`) and the
///        three-letter Windows name (`ENU`), from a table of the cultures Business Central ships
///        a language for. `Language.GetCultureName`, `Azure AD Graph User`'s language mapping and
///        a `DataTable`'s `Locale` read those four; nothing here formats a number or a date.
/// \warning `DateTimeFormat`, `NumberFormat` and `TextInfo` are REFUSED: a culture's patterns are
///          the formatting layer's, which is not rebuilt yet (board:0035). A culture this table
///          does not carry keeps the name or number it was made from and answers `Unknown` for
///          the rest, the way .NET answers a custom culture, rather than refusing.
class CultureValue {
public:
  /// \brief Named culture fields used by the native catalogue.
  struct Identity {
    std::int32_t lcid = 0;  ///< Windows language identifier.
    std::string_view name;  ///< Language tag.
    std::string_view two;   ///< Two-letter ISO language name.
    std::string_view three; ///< Three-letter Windows language name.
  };

  /// \brief An empty culture value.
  CultureValue() = default;

  /// \param identity Culture fields copied into owned storage.
  explicit CultureValue(Identity identity);

  /// \brief The binder behind `C := C.CultureInfo(1033)` and `C := C.CultureInfo('en-US')`.
  struct Binder {
    /// \brief `new CultureInfo(lcid)`. \param lcid The Windows language id. \return The culture.
    [[nodiscard]] CultureValue operator()(Integer lcid) const;

    /// \brief `new CultureInfo(name)`. \param name The language tag. \return The culture.
    [[nodiscard]] CultureValue operator()(std::string_view name) const;
  };

  /// \brief The constructor AL calls as a member.
  static constexpr Binder CultureInfo{};

  /// \brief `CultureInfo.InvariantCulture`: the culture with no name and LCID 127. \return It.
  [[nodiscard]] static CultureValue InvariantCulture();

  /// \brief `CultureInfo.CurrentCulture`: the session's language. \return It.
  [[nodiscard]] static CultureValue CurrentCulture();

  /// \brief `CultureInfo.GetCultureInfo(lcid)`. \param lcid The id. \return The culture.
  [[nodiscard]] static CultureValue GetCultureInfo(Integer lcid);

  /// \brief `CultureInfo.GetCultureInfo(name)`. \param name The tag. \return The culture.
  [[nodiscard]] static CultureValue GetCultureInfo(std::string_view name);

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
  [[nodiscard]] CultureValue Parent() const;

  /// \brief `CultureInfo.ToString()`: the name. \return It.
  [[nodiscard]] ::agiru::Text<0> ToString() const { return name_; }

  /// \brief What `Format(CultureInfo)` renders: the name. \return It.
  [[nodiscard]] std::string ToText() const { return name_; }

  /// \brief `CultureInfo.DateTimeFormat`: the culture's date and time patterns, not rebuilt.
  static constexpr Refused DateTimeFormat{{.type = "CultureInfo", .member = "DateTimeFormat"}};
  /// \brief `CultureInfo.NumberFormat`: the culture's number patterns, not rebuilt.
  static constexpr Refused NumberFormat{{.type = "CultureInfo", .member = "NumberFormat"}};
  /// \brief `CultureInfo.TextInfo`: the culture's casing rules, not rebuilt.
  static constexpr Refused TextInfo{{.type = "CultureInfo", .member = "TextInfo"}};

  /// \brief `Culture := AbsentType.Member()`: the call refused before the assignment.
  /// \tparam R The refusal. \param refused It. \return This.
  template <typename R>
    requires requires { typename R::IsAlRefusal; }
  CultureValue &operator=(const R &refused) {
    static_cast<void>(refused);
    return *this;
  }

private:
  std::string name_;
  std::string two_;
  std::string three_;
  std::int32_t lcid_ = 0;
};

}
