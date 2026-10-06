#pragma once

#include "dotnet/Encoding.h"
#include "runtime/ErrorValue.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/StringValue.h"

#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>

namespace agiru::dotnet {

/// \brief .NET `System.IO.StreamReader`, rebuilt over an `InStream`: the whole stream is read
///        at construction and decoded with the encoding given (UTF-8 by default), and
///        `ReadLine` hands the lines back one at a time, the way the test libraries read an
///        export back.
class DecodedStreamReader {
public:
  /// \brief The binder behind `R := R.StreamReader(InStream [, Encoding])`.
  struct Binder {
    /// \brief `new StreamReader(stream)`: UTF-8. \param stream The stream. \return The reader.
    [[nodiscard]] DecodedStreamReader operator()(InStream &stream) const;

    /// \brief `new StreamReader(stream, encoding)`. \param stream The stream.
    /// \param encoding The bytes' encoding. \return The reader.
    [[nodiscard]] DecodedStreamReader operator()(InStream &stream, const Encoding &encoding) const;

    /// \brief `new StreamReader(stream, detectEncodingFromByteOrderMarks)`.
    /// \param stream The stream. \param detect Whether a byte-order mark decides the encoding.
    /// \return The reader.
    [[nodiscard]] DecodedStreamReader operator()(InStream &stream, Boolean detect) const;

    /// \brief `new StreamReader(stream, encoding, detectEncodingFromByteOrderMarks)`.
    /// \param stream The stream. \param encoding The encoding.
    /// \param detect Whether a byte-order mark overrides it. \return The reader.
    [[nodiscard]] DecodedStreamReader
    operator()(InStream &stream, const Encoding &encoding, Boolean detect) const;

    /// \brief `new StreamReader(x [, ...])` over anything else AL hands it -- a .NET stream this
    ///        runtime does not rebuild, most of all.
    /// \tparam First The stream. \tparam Rest Whatever follows it.
    /// \param first The stream, read only to be discarded. \param rest The rest, likewise.
    /// \return Never.
    /// \throws Error always (board:0035).
    /// \note IT IS THE FIRST ARGUMENT THAT DECIDES. A constraint folded over ALL the arguments
    ///       let `StreamReader(InStream, Encoding.GetEncoding(0))` fall through to this refusal,
    ///       because the Encoding is neither a stream nor a path and an exact-match template
    ///       beats the `const Encoding &` overload (Payment Export XMLPort UT, 3 cases,
    ///       2026-09-12).
    template <typename First, typename... Rest>
      requires(!std::convertible_to<First &, InStream &> &&
               !std::convertible_to<First, std::string_view>)
    [[nodiscard]] DecodedStreamReader operator()(First &&first, Rest &&...rest) const {
      static_cast<void>(first);
      (static_cast<void>(rest), ...);
      throw ::agiru::Error(
          "StreamReader(...): the stream it was handed is a .NET type this runtime does not "
          "rebuild (board:0035)");
    }

    /// \brief `new StreamReader(path)`: the file read whole. \param path The file.
    /// \return The reader.
    [[nodiscard]] DecodedStreamReader operator()(std::string_view path) const;
  };

  /// \brief The constructor AL calls as a member.
  static constexpr Binder StreamReader{};

  /// \brief `ReadLine()`, which AL writes as a statement too. \return The next line without its
  ///        line end, empty at the end.
  ::agiru::Text<0> ReadLine();

  /// \brief `ReadToEnd()`. \return Everything not yet read.
  [[nodiscard]] ::agiru::Text<0> ReadToEnd();

  /// \brief `Read()`. \return The next character's code point, -1 at the end.
  [[nodiscard]] Integer Read();

  /// \brief `Peek()`. \return The next character's code point without reading it, -1 at the end.
  [[nodiscard]] Integer Peek() const;

  /// \brief The AL property getter reads this instance's current position.
  /// \return Whether everything was read.
  [[nodiscard]] Boolean EndOfStream() const { return at_ >= text_.size(); }

  /// \brief `CurrentEncoding`. \return The encoding the reader decoded with.
  [[nodiscard]] Encoding CurrentEncoding() const { return encoding_; }

  /// \brief `Close()`: forgets the text.
  void Close();

  /// \brief `Dispose()`: the same as `Close`.
  void Dispose() { Close(); }

private:
  std::string text_;
  std::size_t at_ = 0;
  Encoding encoding_;
};

/// \brief Preserves the AL name while its immutable factory has the same spelling.
using StreamReader = DecodedStreamReader;

}
