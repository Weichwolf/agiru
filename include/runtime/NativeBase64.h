#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace agiru {

class OutStream;
enum class TextEncoding : std::int32_t;

/// \brief Original BC29 converter's default in-memory/transform threshold, in UTF-16 characters.
/// \note Qualified by the original NavBase64Converter constructor on CLR 10.0.12
/// (29.0.54011.55407).
inline constexpr std::size_t kNativeBase64BufferCharacters = 10485760;

/// \brief Native Base64 text encoding without a BOM, using the original AL encoding option.
/// \param text UTF-8/WTF-8 AL text. \param lines Insert 76-column CRLF separators.
/// \param encoding Original TextEncoding. \param codepage Explicit single-byte encoding page.
/// \return Encoded ASCII. \throws Error for unsupported pages, locale defaults or encoding options.
/// \note Empty input returns empty before encoding validation, matching the native text core.
[[nodiscard]] std::string
NativeToBase64(std::string_view text, bool lines, TextEncoding encoding, std::int32_t codepage);

/// \brief Appends native encoded text as raw ASCII, without output transcoding or a terminator.
/// \param text Input text. \param lines Insert CRLF separators. \param encoding Input encoding.
/// \param codepage Explicit codepage. \param output Destination; existing bytes remain intact.
/// \throws Error for an unbound output or the not-yet-qualified above-threshold transform branch.
void NativeToBase64(std::string_view text,
                    bool lines,
                    TextEncoding encoding,
                    std::int32_t codepage,
                    OutStream &output);

/// \brief Decodes native Base64 text using the original AL encoding option, retaining BOM
/// characters.
/// \param text Base64 input. \param encoding Decoded bytes' encoding. \param codepage Explicit
/// page.
/// \return AL text. \throws Error for invalid Base64 or an unsupported encoding/default page.
/// \note Empty input returns empty before encoding validation; unknown options decode as UTF-8.
[[nodiscard]] std::string
NativeFromBase64(std::string_view text, TextEncoding encoding, std::int32_t codepage);

/// \brief Validates Base64, then appends raw decoded bytes without BOM, transcoding or terminator.
/// \param text Base64 input. \param output Destination; malformed input leaves it unchanged.
/// \note Both native text-to-output signatures use this primitive: their encoding/page arguments
///       are unused in the original native body. Output storage failure may retain a written
///       prefix.
/// \throws Error for an unbound output or the not-yet-qualified above-threshold transform branch.
void NativeFromBase64(std::string_view text, OutStream &output);

}
