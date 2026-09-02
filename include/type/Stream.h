#pragma once

#include "runtime/Error.h"
#include "type/Blob.h"
#include "type/Boolean.h"
#include "type/Integer.h"

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

  /// \brief AL `OutStream.Write(Value)` for a typed value.
  /// \tparam T The value's type.
  /// \param value The value.
  /// \throws Error always.
  /// \warning REFUSED. A typed Write puts the platform's own BINARY layout into the stream, and
  ///          inventing one would produce a BLOB that reads back wrong wherever BC reads it. Only
  ///          the text forms are here.
  template <typename T> void Write(const T &value) {
    static_cast<void>(value);
    RefuseTyped();
  }

private:
  [[noreturn]] static void RefuseTyped();

  Blob *blob_;
};

/// \brief AL `InStream` -- what a BLOB is read through.
class InStream {
public:
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
  void ResetPosition() { position_ = 0; }

  /// \brief AL `InStream.ReadText(var Text [, Length])`.
  ///
  /// \param text   Receives what was read.
  /// \param length How many characters at most; the whole rest when omitted.
  /// \return How many characters were read.
  Integer ReadText(std::string &text, Integer length);

  /// \brief AL `InStream.ReadText(var Text)` -- the whole rest of the stream.
  /// \param text Receives what was read.
  /// \return How many characters were read.
  Integer ReadText(std::string &text);

  /// \brief AL `InStream.Read(var Value)` for a typed value.
  /// \tparam T The value's type.
  /// \param value Receives the value.
  /// \throws Error always.
  /// \warning REFUSED, for the reason OutStream::Write gives.
  template <typename T> void Read(T &value) {
    static_cast<void>(value);
    RefuseTyped();
  }

private:
  [[noreturn]] static void RefuseTyped();

  const Blob *blob_;
  std::size_t position_ = 0;
};

} // namespace agiru
