#pragma once

#include <cstddef>
#include <cstdint>
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
/// \note THE STREAMS ARE NOT HERE YET. `CreateInStream`, `CreateOutStream`, `Import` and `Export`
///       each take or return an AL type the runtime does not have -- InStream, OutStream and a file
///       -- so writing them would mean inventing signatures the platform does not document. The AL
///       surface baseline counts them absent. What a table needs today is a field that holds bytes,
///       says whether it has any, and reaches storage; that is what this is.
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

  /// \brief Compares two BLOBs.
  /// \param o The other BLOB.
  /// \return True when they hold the same bytes.
  [[nodiscard]] bool operator==(const Blob &o) const = default;

private:
  std::vector<std::uint8_t> bytes_;
};

} // namespace agiru
