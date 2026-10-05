#pragma once

#include "type/Integer.h"

namespace agiru::dotnet {

/// \brief CLR Base64FormattingOptions ordinals; unknown values survive until conversion rejects
/// them.
/// \note None=0 and InsertLineBreaks=1 come from System.Base64FormattingOptions.
/// Null/boxing identity of .NET enumeration references is not qualified here.
class Base64FormattingOptions {
public:
  /// \brief Default conversion formatting is None.
  constexpr Base64FormattingOptions() = default;

  /// \brief Preserve an explicit CLR enumeration ordinal, including invalid values.
  /// \param value The ordinal; ToBase64String performs validation.
  constexpr explicit Base64FormattingOptions(Integer value) : value_(value) {}

  /// \brief No inserted whitespace. \return CLR ordinal zero.
  [[nodiscard]] static constexpr Base64FormattingOptions None() { return {}; }

  /// \brief CRLF between 76-column lines, never after the final line. \return CLR ordinal one.
  [[nodiscard]] static constexpr Base64FormattingOptions InsertLineBreaks() {
    return Base64FormattingOptions{1};
  }

  /// \brief Preserve AL's explicit numeric assignment to the enumeration carrier.
  /// \param value The ordinal. \return This carrier.
  Base64FormattingOptions &operator=(Integer value) {
    value_ = value;
    return *this;
  }

  /// \brief The unmodified CLR ordinal. \return Zero, one or an invalid value to be rejected.
  [[nodiscard]] constexpr Integer Ordinal() const { return value_; }

private:
  Integer value_ = 0;
};

}
