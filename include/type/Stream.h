#pragma once

#include "runtime/Error.h"
#include "type/Blob.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/StringValue.h"

#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>

/// \file
/// \brief AL `InStream` and `OutStream` -- bytes read out of and written into a BLOB.

namespace agiru {

/// \brief AL `OutStream` -- what a BLOB is written through.
///
/// \note IT DOES NOT OWN THE BLOB. `Blob.CreateOutStream(Out)` points a stream at a BLOB that
///       already exists, and everything written goes into that BLOB. A stream that owned a copy
///       would leave the caller's BLOB empty and every test of it green for the wrong reason.
class OutStream {
public:
  /// \brief A stream bound to nothing yet.
  ///
  /// \note AL DECLARES THE VARIABLE BEFORE IT HAS A SOURCE. `ProfileConfigurationOutStream:
  ///       OutStream;` is a `var` line and the source arrives later, through
  ///       `File.CreateOutStream(...)` or `Blob.CreateOutStream()`. So the type has to be default
  ///       constructible; what it writes to until then is nothing, and every operation on it
  ///       refuses.
  OutStream() = default;

  /// \brief A stream that writes into a BLOB.
  /// \param into The BLOB.
  explicit OutStream(Blob &into) : blob_(&into) {}

  /// \brief AL `OutStream.WriteText(Text)`.
  /// \param text The text to write.
  /// \return How many characters were written.
  Integer WriteText(std::string_view text);

  /// \brief AL `OutStream.WriteText()` -- with no text at all.
  ///
  /// \return How many characters were written, which is two.
  ///
  /// \warning IT WRITES A LINE BREAK, and the page says so outright: "if you do not specify this, a
  ///          carriage return and a line feed are written". An empty write would leave every
  ///          generated file on one line, and nothing would raise.
  Integer WriteText();

  /// \brief Raw bytes, no terminator and no encoding: what a rebuilt .NET writer puts into the
  ///        stream in the layout .NET defines (`BinaryWriter`).
  /// \param bytes The bytes.
  /// \return How many were written.
  Integer WriteBytes(std::string_view bytes);

  /// \brief AL `OutStream.Write(Value)` for a TEXT value.
  ///
  /// \tparam T The value's type, which must read as a `std::string_view`.
  /// \param value The value.
  /// \return How many bytes were written, the terminator counted.
  ///
  /// \note IT WRITES A ZERO BYTE AFTER THE TEXT AND `WriteText` DOES NOT.
  ///       `devenv-write-read-methods-line-break-behavior.md` states that difference as the whole
  ///       point of the pair, and works it through an example: what `Write` puts in, `Read` takes
  ///       out again, terminator and all, while `ReadText` stops at the first line break as well.
  ///       So this is the platform's own layout for a string rather than an invention -- which is
  ///       what the numeric forms below still lack.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  Integer Write(const T &value) {
    return WriteTerminated(std::string_view(value), -1);
  }

  /// \brief AL `OutStream.Write(Value, Length)` for a TEXT value.
  ///
  /// \tparam T The value's type, which must read as a `std::string_view`.
  /// \param value The value.
  /// \param length How many bytes of it to write.
  /// \return How many bytes were written, the terminator counted.
  ///
  /// \note THE LENGTH CUTS AND DOES NOT PAD. The page says a length that differs from the size of
  ///       the variable is an ERROR "in the case of data types other than string, code, and
  ///       binary" -- so for those three it is a length and nothing else.
  template <typename T>
    requires std::convertible_to<const T &, std::string_view>
  Integer Write(const T &value, const Integer &length) {
    return WriteTerminated(std::string_view(value), length);
  }

  /// \brief AL `OutStream.Write(Value [, Length])` for a value that is not text.
  /// \tparam T The value's type.
  /// \param value The value.
  /// \throws Error always.
  /// \warning REFUSED. A typed Write puts the platform's own BINARY layout into the stream, and
  ///          inventing one would produce a BLOB that reads back wrong wherever BC reads it. Only
  ///          the text forms are here.
  template <typename T>
    requires(!std::convertible_to<const T &, std::string_view>)
  Integer Write(const T &value) {
    static_cast<void>(value);
    RefuseTyped();
  }

  /// \brief AL `OutStream.Write(Value, Length)` for a value that is not text.
  /// \tparam T The value's type.
  /// \param value The value.
  /// \param length The AL `Integer` the page names as the second parameter.
  /// \return The AL `Written`, which the page brackets.
  /// \throws Error always.
  /// \warning REFUSED for the same reason as the one-argument form.
  template <typename T>
    requires(!std::convertible_to<const T &, std::string_view>)
  Integer Write(const T &value, const Integer &length) {
    static_cast<void>(value);
    static_cast<void>(length);
    RefuseTyped();
  }

private:
  Integer WriteTerminated(std::string_view text, Integer length);

  [[nodiscard]] Blob &Bound() const;

  [[noreturn]] static void RefuseTyped();

  Blob *blob_ = nullptr;
};

/// \brief AL `InStream` -- what a BLOB is read through.
class InStream {
public:
  /// \brief A stream bound to nothing yet.
  ///
  /// \note AL DECLARES THE VARIABLE BEFORE IT HAS A SOURCE. `ProfileConfigurationOutStream:
  ///       OutStream;` is a `var` line and the source arrives later, through
  ///       `File.CreateInStream(...)` or `Blob.CreateInStream()`. So the type has to be default
  ///       constructible; what it reads to until then is nothing, and every operation on it
  ///       refuses.
  InStream() = default;

  /// \brief A stream that reads from a BLOB.
  /// \param from The BLOB.
  explicit InStream(const Blob &from) : blob_(&from) {}

  /// \brief AL `InStream.EOS()`.
  /// \return True when nothing is left to read.
  [[nodiscard]] Boolean EOS() const;

  /// \brief AL `InStream.Length()`.
  /// \return How many bytes the stream holds altogether.
  [[nodiscard]] Integer Length() const;

  /// \brief AL `InStream.Position()`.
  /// \return How far into the stream the next read starts, counting from one as AL counts.
  [[nodiscard]] Integer Position() const { return static_cast<Integer>(position_) + 1; }

  /// \brief AL `InStream.ResetPosition()` -- starts again from the beginning.
  ::agiru::Boolean ResetPosition() {
    position_ = 0;
    return true;
  }

  /// \brief AL `InStream.ReadText(var Text [, Length])`.
  ///
  /// \param text   Receives what was read.
  /// \param length How many characters at most; the whole rest when omitted.
  /// \return How many characters were read.
  Integer ReadText(::agiru::Text<0> &text, Integer length);

  /// \brief AL `InStream.ReadText(var Text)` -- the whole rest of the stream.
  /// \param text Receives what was read.
  /// \return How many characters were read.
  Integer ReadText(::agiru::Text<0> &text);

  /// \brief Raw bytes from the position on, at most `count`: the reverse of `WriteBytes`.
  /// \param count How many to take.
  /// \return The bytes taken, fewer at the end of the stream.
  std::string ReadBytes(Integer count);

  /// \brief AL `InStream.Read(var Value [, Length])` for a TEXT value.
  ///
  /// \tparam T The value's type, which must assign from a `std::string_view`.
  /// \param value Receives what was read.
  /// \param length How many bytes at most; the rest of the stream when omitted.
  /// \return How many bytes were read, the terminator counted.
  ///
  /// \note IT READS UNTIL A ZERO BYTE AND NOT UNTIL A LINE BREAK, which is the whole difference
  ///       from `ReadText`: `devenv-write-read-methods-line-break-behavior.md` reads
  ///       `A<CR><LF>B` back as one value here and as two there.
  template <typename T>
    requires std::assignable_from<T &, std::string_view>
  Integer Read(T &value, Integer length) {
    std::string got;
    const Integer read = ReadTerminated(got, length);
    value = std::string_view(got);
    return read;
  }

  /// \brief AL `InStream.Read(var Value)` for a TEXT value.
  /// \tparam T The value's type, which must assign from a `std::string_view`.
  /// \param value Receives what was read.
  /// \return How many bytes were read, the terminator counted.
  template <typename T>
    requires std::assignable_from<T &, std::string_view>
  Integer Read(T &value) {
    return Read(value, -1);
  }

  /// \brief AL `InStream.Read(var Value)` for a value that is not text.
  /// \tparam T The value's type.
  /// \param value Receives the value.
  /// \throws Error always.
  /// \warning REFUSED, for the reason OutStream::Write gives.
  template <typename T>
    requires(!std::assignable_from<T &, std::string_view>)
  void Read(T &value) {
    static_cast<void>(value);
    RefuseTyped();
  }

private:
  Integer ReadTerminated(std::string &into, Integer length);

  [[nodiscard]] const Blob &Bound() const;

  [[noreturn]] static void RefuseTyped();

  const Blob *blob_ = nullptr;
  std::size_t position_ = 0;
};

}
