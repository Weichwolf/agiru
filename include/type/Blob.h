#pragma once

#include "type/TextEncoding.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/// \file
/// \brief AL `Blob` -- bytes of no declared length.

namespace agiru {

/// \brief AL `Blob`.
///
/// From `blob-data-type.md`: "Variables of this data type differ from normal numeric and string
/// variables in that BLOBs have a variable length. The maximum size of a BLOB (binary large object)
/// is 2 GB."
///
/// \note Streams borrow this BLOB; writes append to its owned byte storage. Client file
///       import/export still refuse. Stream text-encoding selection remains unimplemented.
class Blob {
public:
  /// \brief An empty BLOB, which is what a field holds until something writes to it.
  Blob() = default;

  /// \brief The largest BLOB AL accepts, from `blob-data-type.md`.
  static constexpr std::size_t kMaximumSize = 2UL * 1024 * 1024 * 1024;

  /// \brief AL `Blob.HasValue()`.
  /// \return True when the BLOB holds at least one byte.
  [[nodiscard]] bool HasValue() const { return !bytes_.empty(); }

  /// \brief AL `Blob.Length()`.
  /// \return The number of bytes.
  [[nodiscard]] std::size_t Length() const { return bytes_.size(); }

  /// \return The bytes.
  [[nodiscard]] const std::vector<std::uint8_t> &Bytes() const { return bytes_; }

  /// \brief Replaces the bytes.
  /// \param bytes The new content.
  void Set(std::vector<std::uint8_t> bytes) { bytes_ = std::move(bytes); }

  /// \brief AL `Blob.CreateOutStream(OutStream)` -- points a stream at this BLOB to write into.
  /// \return The stream.
  /// \note The stream writes into THIS BLOB and does not own a copy of it, which is what makes
  ///       `Rec.Blob.CreateOutStream(Out); Out.WriteText(x)` leave the value in the record.
  [[nodiscard]] class OutStream CreateOutStream();

  /// \brief AL `Blob.CreateInStream(InStream)` -- points a stream at this BLOB to read from.
  /// \return The stream.
  [[nodiscard]] class InStream CreateInStream() const;

  /// \brief AL `Blob.CreateOutStream(var OutStream [, TextEncoding])` -- the `var` form.
  /// \param into     The stream to bind to this BLOB.
  /// \param Encoding The encoding, currently ignored; text conversion remains unimplemented.
  void CreateOutStream(class OutStream &into, const TextEncoding &Encoding = {});

  /// \brief AL `Blob.CreateInStream(var InStream [, TextEncoding])` -- the `var` form.
  /// \param from     The stream to bind to this BLOB.
  /// \param Encoding The encoding, currently ignored; text conversion remains unimplemented.
  void CreateInStream(class InStream &from, const TextEncoding &Encoding = {}) const;

  /// \brief AL `Blob.Export(Text)`.
  /// \param Name The full path and name of the file the bytes are written to.
  /// \return The name of the file that was written.
  /// \throws Error always.
  /// \warning REFUSED. Writing a client file needs a client (board:0030); the refusal names the
  ///          file, so a caller learns WHICH export it lost.
  std::string Export(std::string_view Name);

  /// \brief AL `Blob.Import(Text)`.
  /// \param Name The full path and name of the file the bytes are read from.
  /// \return The name of the file that was read.
  /// \throws Error always.
  /// \warning REFUSED, for the reason Export gives.
  std::string Import(std::string_view Name);

  /// \brief Compares two BLOBs.
  /// \param o The other BLOB.
  /// \return True when they hold the same bytes.
  [[nodiscard]] bool operator==(const Blob &o) const = default;

private:
  friend class OutStream;
  std::vector<std::uint8_t> bytes_;
};

}
