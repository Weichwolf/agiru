#pragma once

#include "runtime/Error.h"
#include "type/Guid.h"
#include "type/Integer.h"

#include <string_view>

/// \file
/// \brief AL `MediaSet` -- a collection of media objects referenced by a record.

namespace agiru {

/// \brief AL `MediaSet`.
///
/// From `mediaset-data-type.md`: a complex type that encapsulates media in application database
/// tables, usable as a table field data type but not as a variable or parameter. Measured over
/// BCApps on 2026-09-02: 23 field declarations, no variables, no parameters.
///
/// \note THE FIELD HOLDS THE SET'S IDENTIFIER AND NOT ITS MEMBERS. `MediaSet.MediaId()` is the
///       GUID of the SET; `Item(Index)` reaches into the tenant media table for a member, which is
///       why it refuses here (board:0031).
class MediaSet {
public:
  /// \brief No media set.
  constexpr MediaSet() = default;

  /// \brief References a media set by its identifier.
  /// \param id The set's GUID.
  constexpr explicit MediaSet(const Guid &id) : id_(id) {}

  /// \brief AL `MediaSet.MediaId()`.
  /// \return The set's identifier, or the blank GUID when the field is empty.
  [[nodiscard]] constexpr const Guid &MediaId() const { return id_; }

  /// \brief AL `MediaSet.Count()`.
  ///
  /// \return How many media objects the set holds.
  /// \throws Error when the field references a set, because counting its members needs the tenant
  ///         media table (board:0031).
  ///
  /// \note An EMPTY field answers 0 without refusing, because that answer needs no table: a field
  ///       that references no set holds no media, and AL code guards on exactly that before
  ///       reaching for `Item`.
  [[nodiscard]] Integer Count() const;

  /// \brief AL `MediaSet.Item(Integer)`.
  /// \param index The one-based position in the set.
  /// \return The media object's identifier.
  /// \throws Error always.
  /// \warning REFUSED. Reaching a member needs the tenant media table (board:0031).
  [[nodiscard]] Guid Item(Integer index) const;

  /// \brief AL `MediaSet.Insert(Guid)`.
  /// \param mediaId The media object to add.
  /// \throws Error always.
  /// \warning REFUSED, for the reason Item gives.
  void Insert(const Guid &mediaId);

  /// \brief AL `MediaSet.ImportFile(Text, Text)`.
  /// \param filename    The full path and name of the file to add.
  /// \param description Text the client uses to describe the media.
  /// \return The identifier the media object was given.
  /// \throws Error always.
  /// \warning REFUSED, for the reason Item gives. The refusal names the file.
  Guid ImportFile(std::string_view filename, std::string_view description);

  /// \brief Orders two references by identifier.
  /// \param o The other.
  /// \return The ordering.
  [[nodiscard]] constexpr std::strong_ordering operator<=>(const MediaSet &o) const = default;

  /// \brief Compares two references.
  /// \param o The other.
  /// \return True when they name the same set.
  [[nodiscard]] constexpr bool operator==(const MediaSet &o) const = default;

private:
  Guid id_;
};

}
