#pragma once

#include "dotnet/Regex.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Text.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace agiru::dotnet {

/// \brief .NET `System.Text.Encoding`, rebuilt for the four encodings the BaseApp names: UTF-8
///        (code page 65001), UTF-16 little-endian (`Unicode`, 1200), ASCII (20127) and the
///        single-byte pages, which are read and written as Latin-1 -- one byte per code point
///        below 256 and `?` above it.
///
/// \note A BYTE ARRAY IS A `dotnet::Array` OF INTEGERS 0..255, the way AL reads a `byte[]` back
///       (`Array.GetValue(i)` is an Integer there too). `GetEncoding(0)` is the process default,
///       which is UTF-8 on .NET Core and here.
class Encoding {
public:
  /// \brief The binder behind `E := E.Encoding()`, which AL never calls with arguments.
  struct Binder {
    /// \brief `new Encoding()`: the default, UTF-8. \return The encoding.
    [[nodiscard]] class Encoding operator()() const;
  };

  /// \brief The constructor AL calls as a member.
  Binder Encoding;

  /// \brief `Encoding.UTF8`. \return UTF-8, with a preamble.
  [[nodiscard]] static class Encoding UTF8();

  /// \brief `Encoding.Unicode`. \return UTF-16 little-endian, with a preamble.
  [[nodiscard]] static class Encoding Unicode();

  /// \brief `Encoding.ASCII`. \return ASCII.
  [[nodiscard]] static class Encoding ASCII();

  /// \brief `Encoding.Default`. \return UTF-8.
  [[nodiscard]] static class Encoding Default();

  /// \brief `Encoding.UTF32`. \return UTF-32 little-endian.
  [[nodiscard]] static class Encoding UTF32();

  /// \brief `Encoding.GetEncoding(codePage)`. \param codePage 65001, 1200, 20127, 0, or a
  ///        single-byte page. \return The encoding.
  [[nodiscard]] static class Encoding GetEncoding(Integer codePage);

  /// \brief `Encoding.GetEncoding(name)`. \param name `utf-8`, `utf-16`, `us-ascii`,
  ///        `windows-1252` and the like. \return The encoding. \throws Error for a name unknown.
  [[nodiscard]] static class Encoding GetEncoding(std::string_view name);

  /// \brief `Encoding.Convert(from, to, bytes)`. \param from The bytes' encoding. \param to The
  ///        wanted one. \param bytes The bytes. \return The bytes in the wanted encoding.
  [[nodiscard]] static Array
  Convert(const class Encoding &from, const class Encoding &to, const Array &bytes);

  /// \brief `Encoding.GetBytes(text)`. \param text The text. \return Its bytes.
  [[nodiscard]] Array GetBytes(std::string_view text) const;

  /// \brief `Encoding.GetByteCount(text)`. \param text The text. \return How many bytes.
  [[nodiscard]] Integer GetByteCount(std::string_view text) const;

  /// \brief `Encoding.GetString(bytes)`. \param bytes The bytes. \return The text.
  [[nodiscard]] ::agiru::Text<0> GetString(const Array &bytes) const;

  /// \brief `Encoding.GetString(bytes, index, count)`. \param bytes The bytes. \param index From.
  /// \param count How many. \return The text.
  [[nodiscard]] ::agiru::Text<0> GetString(const Array &bytes, Integer index, Integer count) const;

  /// \brief `Encoding.GetChars(bytes)`. \param bytes The bytes. \return The code points, one
  ///        Integer each, as `Array.GetValue` reads a `char[]`.
  [[nodiscard]] Array GetChars(const Array &bytes) const;

  /// \brief `Encoding.GetChars(bytes, index, count)`. \param bytes The bytes. \param index From.
  /// \param count How many bytes. \return The code points.
  [[nodiscard]] Array GetChars(const Array &bytes, Integer index, Integer count) const;

  /// \brief `Encoding.GetBytes(chars, index, count)`: the bytes of `count` code points from
  ///        `index`. \param chars The code points. \param index From. \param count How many.
  /// \return The bytes.
  [[nodiscard]] Array GetBytes(const Array &chars, Integer index, Integer count) const;

  /// \brief `Encoding.GetBytes(chars, index, count, bytes, byteIndex)`: the bytes of `count`
  ///        code points from `index`, written into `bytes` from `byteIndex`. \param chars The
  ///        code points. \param index From. \param count How many. \param bytes The array
  ///        written into. \param byteIndex Where in it. \return How many bytes were written.
  Integer
  GetBytes(const Array &chars, Integer index, Integer count, Array &bytes, Integer byteIndex) const;

  /// \brief `Encoding.GetPreamble()`. \return The byte order mark, empty when the encoding
  ///        writes none.
  [[nodiscard]] Array GetPreamble() const;

  /// \brief `Encoding.CodePage`. \return The code page.
  [[nodiscard]] Integer CodePage() const { return codePage_; }

  /// \brief `Encoding.WebName`. \return `utf-8`, `utf-16`, `us-ascii` or `windows-<page>`.
  [[nodiscard]] ::agiru::Text<0> WebName() const;

  /// \brief `Encoding.EncodingName`. \return The same name as `WebName`.
  [[nodiscard]] ::agiru::Text<0> EncodingName() const { return WebName(); }

  /// \brief Whether the variable was never given an encoding, which `IsNull` answers.
  /// \return `true` before any factory ran.
  [[nodiscard]] Boolean IsNull() const { return codePage_ == kUnset; }

  /// \brief The bytes of a text, as a string of bytes rather than an `Array`.
  /// \param text The text. \return The bytes.
  [[nodiscard]] std::string Encode(std::string_view text) const;

  /// \brief The text of some bytes. \param bytes The bytes. \return The text, UTF-8.
  [[nodiscard]] std::string Decode(std::string_view bytes) const;

  static constexpr std::int32_t kUtf8 = 65001;    ///< The UTF-8 code page.
  static constexpr std::int32_t kUtf16 = 1200;    ///< The UTF-16 little-endian code page.
  static constexpr std::int32_t kAscii = 20127;   ///< The ASCII code page.
  static constexpr std::int32_t kDefault = 0;     ///< `GetEncoding(0)`, the default.
  static constexpr std::int32_t kWindows1252 = 1252; ///< The Western European single-byte page.
  static constexpr std::int32_t kUtf32 = 12000;   ///< The UTF-32 little-endian code page.

  /// \brief An encoding by code page, the factory every named one goes through.
  /// \param codePage The code page. \param preamble Whether `GetPreamble` answers a mark.
  /// \return The encoding.
  static class Encoding Made(std::int32_t codePage, bool preamble) {
    class Encoding out;
    out.codePage_ = codePage;
    out.preamble_ = preamble;
    return out;
  }

private:
  static constexpr std::int32_t kUnset = -1;

  std::int32_t codePage_ = kUnset;
  bool preamble_ = false;
};

/// \brief .NET `UTF8Encoding`: UTF-8 with or without a byte order mark.
class UTF8Encoding : public Encoding {
public:
  /// \brief The binder behind `E := E.UTF8Encoding([emitBom])`.
  struct Binder {
    /// \brief `new UTF8Encoding()` and `new UTF8Encoding(emitBom)`. \param emitBom Whether
    ///        `GetPreamble` answers the byte order mark. \return The encoding.
    [[nodiscard]] class UTF8Encoding operator()(Boolean emitBom = false) const;
  };

  /// \brief The constructor AL calls as a member.
  Binder UTF8Encoding;

private:
  friend struct Binder;
};

/// \brief .NET `UnicodeEncoding`: UTF-16 little-endian.
class UnicodeEncoding : public Encoding {
public:
  /// \brief The binder behind `E := E.UnicodeEncoding()`.
  struct Binder {
    /// \brief `new UnicodeEncoding()`. \return The encoding.
    [[nodiscard]] class UnicodeEncoding operator()() const;
  };

  /// \brief The constructor AL calls as a member.
  Binder UnicodeEncoding;
};

/// \brief .NET `ASCIIEncoding`.
class ASCIIEncoding : public Encoding {
public:
  /// \brief The binder behind `E := E.ASCIIEncoding()`.
  struct Binder {
    /// \brief `new ASCIIEncoding()`. \return The encoding.
    [[nodiscard]] class ASCIIEncoding operator()() const;
  };

  /// \brief The constructor AL calls as a member.
  Binder ASCIIEncoding;
};

}
