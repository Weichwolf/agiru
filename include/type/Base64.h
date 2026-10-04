#pragma once

#include <string>
#include <string_view>

namespace agiru {

class OutStream;

/// \brief Encodes arbitrary bytes with the RFC 4648 Base64 alphabet and padding.
/// \param bytes Binary input; no text conversion or BOM is added.
/// \param lineBreaks Insert CRLF before each group beyond column 76, never at the end.
/// \return Encoded ASCII, matching CLR Convert.ToBase64String's formatting profile.
[[nodiscard]] std::string EncodeBase64(std::string_view bytes, bool lineBreaks = false);

/// \brief Decodes the CLR Convert.FromBase64String profile into arbitrary bytes.
/// \param text Encoded input; only ASCII space, tab, CR and LF are ignored.
/// \return Bytes; unused padding bits need not be zero, matching CLR Convert.
/// \throws Error on invalid alphabet, padding or incomplete groups.
[[nodiscard]] std::string DecodeBase64(std::string_view text);

/// \brief Appends encoded ASCII in bounded blocks, without BOM or terminator.
/// \param bytes Binary input; a view borrowed from the output BLOB is preserved before writing.
/// \param output Destination; existing bytes are retained.
/// \param lineBreaks Use the same 76-column profile as the string-returning overload.
/// \note Nonalias input needs constant scratch space; alias input requires an owned copy.
void EncodeBase64(std::string_view bytes, OutStream &output, bool lineBreaks = false);

/// \brief Validates encoded input, then appends decoded bytes in bounded blocks.
/// \param text The same decoder profile as the string-returning overload.
/// \param output Destination; existing bytes are retained, without BOM or terminator.
/// \throws Error on invalid input, before any output bytes are written.
/// \note Nonalias input needs constant scratch space; alias input requires an owned copy.
///       Storage failures during valid output may leave an already-written prefix.
void DecodeBase64(std::string_view text, OutStream &output);

}
