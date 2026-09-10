#pragma once

#include "dotnet/Encoding.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/Text.h"

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
  };

  /// \brief The constructor AL calls as a member.
  Binder StreamReader;

  /// \brief `ReadLine()`. \return The next line without its line end, empty at the end.
  [[nodiscard]] ::agiru::Text<0> ReadLine();

  /// \brief `ReadToEnd()`. \return Everything not yet read.
  [[nodiscard]] ::agiru::Text<0> ReadToEnd();

  /// \brief `Read()`. \return The next character's code point, -1 at the end.
  [[nodiscard]] Integer Read();

  /// \brief `Peek()`. \return The next character's code point without reading it, -1 at the end.
  [[nodiscard]] Integer Peek() const;

  /// \brief `EndOfStream`. \return Whether everything was read.
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

}
