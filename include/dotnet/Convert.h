#pragma once

#include "dotnet/Base64FormattingOptions.h"
#include "dotnet/Refused.h"
#include "type/Integer.h"

#include <concepts>
#include <string>
#include <string_view>

namespace agiru {
class BigText;
}

namespace agiru::dotnet {

class Array;

/// \brief CLR System.Convert byte-array Base64 methods over the shared runtime codec.
/// Static calls need no bound instance. Byte cells must be Integers in [0,255].
/// \warning Numeric conversions remain named refusals. Complete System.Array null/type/
/// reference identity and exact CLR diagnostic text remain separate gaps.
class Convert {
public:
  /// \brief Encode all bytes without text conversion, BOM or implicit line breaks.
  /// \param bytes Byte cells. \param options None or InsertLineBreaks.
  /// \return Encoded ASCII. \throws Error for invalid cells/options.
  [[nodiscard]] static std::string
  ToBase64String(const Array &bytes,
                 Base64FormattingOptions options = Base64FormattingOptions::None());

  /// \brief Encode an exact zero-based byte region; empty end-position regions are valid.
  /// \param bytes Byte cells. \param offset First cell. \param length Number of cells.
  /// \param options None or InsertLineBreaks. \return Encoded ASCII.
  /// \throws Error for invalid regions, cells or options, without modifying the input.
  [[nodiscard]] static std::string
  ToBase64String(const Array &bytes,
                 Integer offset,
                 Integer length,
                 Base64FormattingOptions options = Base64FormattingOptions::None());

  /// \brief Decode the CLR Convert profile: only ASCII space, tab, CR and LF are ignored.
  /// \param text Encoded text; unused padding bits are permitted, incomplete groups are not.
  /// \return Independently owned Integer byte cells, including zero and high bytes.
  /// \throws Error for invalid alphabet/padding; never returns partial decoded data.
  [[nodiscard]] static Array FromBase64String(std::string_view text);

  /// \brief AL BigText input is materialized through its owned ToText adapter at the call site.
  /// \tparam Carrier Exactly AL BigText, not an arbitrary text-convertible object.
  /// \param text The AL value. \return Owned decoded byte cells.
  /// \throws Error when BigText.ToText remains unimplemented or Base64 is invalid.
  /// \note Caller-side instantiation preserves the net→rt dependency boundary.
  template <typename Carrier>
    requires std::same_as<Carrier, ::agiru::BigText>
  [[nodiscard]] static auto FromBase64String(const Carrier &text) {
    return FromBase64String(text.ToText());
  }

  /// \brief Unsupported numeric methods keep their original type/member refusal identities.
  static constexpr Refused ToInt16{{.type = "Convert", .member = "ToInt16"}};
  /// \copydoc ToInt16
  static constexpr Refused ToInt32{{.type = "Convert", .member = "ToInt32"}};
  /// \copydoc ToInt16
  static constexpr Refused ToUInt16{{.type = "Convert", .member = "ToUInt16"}};
  /// \copydoc ToInt16
  static constexpr Refused ToUInt32{{.type = "Convert", .member = "ToUInt32"}};
};

}
