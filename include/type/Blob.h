#pragma once

#include "type/TextEncoding.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
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
/// \note Streams retain this value's byte provider after the wrapper ends. BLOB copies
///       have independent bytes. Client import/export and stream encoding remain gaps.
class Blob {
public:
  /// \brief An empty BLOB, which is what a field holds until something writes to it.
  Blob() = default;

  /// \brief Copies the byte value into an independent provider. \param other The source value.
  Blob(const Blob &other);

  /// \brief Copies bytes without aliasing the source provider. \param other The source value.
  /// \return This value; self-assignment retains its provider.
  Blob &operator=(const Blob &other);

  /// \brief Transfers the provider; existing streams keep it alive. \param other The source.
  Blob(Blob &&other) noexcept = default;

  /// \brief Transfers the provider. \param other The source. \return This value.
  Blob &operator=(Blob &&other) noexcept = default;

  /// \brief The largest BLOB AL accepts, from `blob-data-type.md`.
  static constexpr std::size_t kMaximumSize = 2UL * 1024 * 1024 * 1024;

  /// \brief AL `Blob.HasValue()`.
  /// \return True when the BLOB holds at least one byte.
  [[nodiscard]] bool HasValue() const { return Length() != 0; }

  /// \brief AL `Blob.Length()`.
  /// \return The number of bytes.
  [[nodiscard]] std::size_t Length() const;

  /// \return The bytes.
  [[nodiscard]] const std::vector<std::uint8_t> &Bytes() const;

  /// \brief Replaces the bytes.
  /// \param bytes The new content.
  void Set(std::vector<std::uint8_t> bytes);

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
  [[nodiscard]] bool operator==(const Blob &o) const;

private:
  friend class OutStream;
  friend class InStream;
  using Storage = std::vector<std::uint8_t>;
  [[nodiscard]] std::shared_ptr<Storage> Pin() const;
  mutable std::shared_ptr<Storage> bytes_;
};

}
