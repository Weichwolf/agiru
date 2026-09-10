#pragma once

#include "dotnet/Encoding.h"
#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/Text.h"

#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>

namespace agiru::dotnet {

/// \brief .NET `System.IO.StreamReader`, rebuilt over an `InStream`: the whole stream is read
///        at construction and decoded with the encoding given (UTF-8 by default), and
///        `ReadLine` hands the lines back one at a time, the way the test libraries read an
///        export back.
class StreamReader {
public:
  /// \brief The binder behind `R := R.StreamReader(InStream [, Encoding])`.
  struct Binder {
    /// \brief `new StreamReader(stream)`: UTF-8. \param stream The stream. \return The reader.
    [[nodiscard]] class StreamReader operator()(InStream &stream) const;

    /// \brief `new StreamReader(stream, encoding)`. \param stream The stream.
    /// \param encoding The bytes' encoding. \return The reader.
    [[nodiscard]] class StreamReader operator()(InStream &stream, const Encoding &encoding) const;

    /// \brief `new StreamReader(stream, detectEncodingFromByteOrderMarks)`.
    /// \param stream The stream. \param detect Whether a byte-order mark decides the encoding.
    /// \return The reader.
    [[nodiscard]] class StreamReader operator()(InStream &stream, Boolean detect) const;

    /// \brief `new StreamReader(stream, encoding, detectEncodingFromByteOrderMarks)`.
    /// \param stream The stream. \param encoding The encoding.
    /// \param detect Whether a byte-order mark overrides it. \return The reader.
    [[nodiscard]] class StreamReader
    operator()(InStream &stream, const Encoding &encoding, Boolean detect) const;

    /// \brief `new StreamReader(x [, ...])` over anything else AL hands it -- a .NET stream this
    ///        runtime does not rebuild, most of all.
    /// \tparam Arguments Whatever AL passed.
    /// \param arguments The arguments, read only to be discarded.
    /// \return Never.
    /// \throws Error always (board:0035).
    template <typename... Arguments>
      requires(sizeof...(Arguments) >= 1 &&
               !(std::convertible_to<Arguments &, InStream &> && ...) &&
               !(std::convertible_to<Arguments, std::string_view> && ...))
    [[nodiscard]] class StreamReader operator()(Arguments &&...arguments) const {
      (static_cast<void>(arguments), ...);
      throw ::agiru::Error(
          "StreamReader(...): the stream it was handed is a .NET type this runtime does not "
          "rebuild (board:0035)");
    }

    /// \brief `new StreamReader(path)`: the file read whole. \param path The file.
    /// \return The reader.
    [[nodiscard]] class StreamReader operator()(std::string_view path) const;
  };

  /// \brief The constructor AL calls as a member.
  Binder StreamReader;

  /// \brief `ReadLine()`, which AL writes as a statement too. \return The next line without its
  ///        line end, empty at the end.
  ::agiru::Text<0> ReadLine();

  /// \brief `ReadToEnd()`. \return Everything not yet read.
  [[nodiscard]] ::agiru::Text<0> ReadToEnd();

  /// \brief `Read()`. \return The next character's code point, -1 at the end.
  [[nodiscard]] Integer Read();

  /// \brief `Peek()`. \return The next character's code point without reading it, -1 at the end.
  [[nodiscard]] Integer Peek() const;

  /// \brief `EndOfStream`, which .NET spells as a property and AL reads without parentheses.
  ///
  /// \note IT HOLDS NOTHING AND FINDS ITS READER BY ITS OWN OFFSET, because `StreamReader`
  ///       carries a member named after the class -- the constructor AL calls -- and a class with
  ///       such a member may declare no constructor of its own, so a slot that stored a pointer
  ///       could not be repaired when the reader is copied (`R := R.StreamReader(...)` copies).
  class EndOfStreamSlot {
  public:
    /// \brief `if R.EndOfStream then`. \return Whether everything was read.
    operator Boolean() const { return Read_(); } // NOLINT(*-explicit-constructor)

    /// \brief `R.EndOfStream()`. \return Whether everything was read.
    Boolean operator()() const { return Read_(); }

  private:
    [[nodiscard]] Boolean Read_() const;
  };

  /// \brief `EndOfStream`.
  EndOfStreamSlot EndOfStream;

  /// \brief Where the slot sits inside the reader, which is how it finds it.
  static const std::size_t kEndOfStreamOffset;

  /// \brief Whether everything was read. \return The answer the slot hands on.
  [[nodiscard]] Boolean AtEnd_() const { return at_ >= text_.size(); }

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

}
