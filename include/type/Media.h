#pragma once

#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Guid.h"

#include <string_view>

/// \file
/// \brief AL `Media` -- one media object referenced by a record.

namespace agiru {

class OutStream;

/// \brief AL `Media`.
///
/// From `media-data-type.md`: "Encapsulates media files, such as image .jpg and .png files, in
/// application database tables. The Media data type can be used as a table field data type, but
/// cannot be used as a variable or parameter."
///
/// \note IT IS A FIELD TYPE AND NOTHING ELSE, and the BaseApp agrees with the page: measured over
///       BCApps on 2026-09-02, `Media` appears 90 times as a field declaration and NOT ONCE as a
///       variable or a parameter. So this type needs no assignment story, no arithmetic and no
///       formatting -- it needs to sit in a record and name a media object.
///
/// \note WHAT IT HOLDS IS THE IDENTIFIER. A media object's bytes live in the tenant's media table,
///       not in the row; the row carries the GUID that finds them. That is why the column is a
///       `uuid` and why everything that moves BYTES refuses here (board:0031).
class Media {
public:
  /// \brief No media object.
  constexpr Media() = default;

  /// \brief References a media object by its identifier.
  /// \param id The media object's GUID.
  constexpr explicit Media(const Guid &id) : id_(id) {}

  /// \brief AL `Media.MediaId()`.
  /// \return The media object's identifier, or the blank GUID when the field is empty.
  [[nodiscard]] constexpr const Guid &MediaId() const { return id_; }

  /// \brief AL `Media.HasValue()`.
  ///
  /// \return True when the field references a media object.
  ///
  /// \warning IT ANSWERS THE FIRST HALF OF THE DOCUMENTED QUESTION. The page asks whether the field
  ///          "has been initialized with a media object AND that the specified media object exists
  ///          in the database". The second half needs the tenant media table, which this runtime
  ///          does not have (board:0031); until it does, a set identifier answers true. The gap is
  ///          here rather than hidden, and it cannot be reached yet in any case -- nothing can put
  ///          an identifier into the field except a row read, and every import refuses.
  [[nodiscard]] constexpr Boolean HasValue() const { return !id_.IsNull(); }

  /// \brief AL `Media.ImportFile(Text, Text [, Text])`.
  ///
  /// \param filename    The full path and name of the file to add.
  /// \param description Text the client uses to describe the media.
  /// \param mimeType    The content type; deduced from the extension when omitted.
  /// \return The identifier the media object was given.
  /// \throws Error always.
  ///
  /// \warning REFUSED. Importing writes bytes into the tenant media table, which this runtime does
  ///          not carry (board:0031). The refusal names the file, so a caller learns WHICH import
  ///          it lost rather than that some import failed.
  ///
  /// \note An INSTANCE method, as `media-importfile-method.md` writes it: the syntax is
  ///       `[ID := ] Media.ImportFile(...)` over "an instance of the Media data type", and the
  ///       import sets the field it is called on. `FindOrphans()` is the type's only static method.
  Guid
  ImportFile(std::string_view filename, std::string_view description, std::string_view mimeType);

  /// \brief AL `Media.ImportFile(Text, Text)` -- with the MIME type deduced from the extension.
  /// \param filename    The full path and name of the file to add.
  /// \param description Text the client uses to describe the media.
  /// \return The identifier the media object was given.
  /// \throws Error always, for the reason the three-argument form gives.
  Guid ImportFile(std::string_view filename, std::string_view description);

  /// \brief AL `Media.ExportFile(Text)`.
  /// \param filename The file to write.
  /// \return The file that was written.
  /// \throws Error always.
  /// \warning REFUSED, for the reason ImportFile gives. The refusal names the media object.
  [[nodiscard]] std::string_view ExportFile(std::string_view filename) const;

  /// \brief Orders two references by identifier.
  /// \param o The other.
  /// \return The ordering.
  [[nodiscard]] constexpr std::strong_ordering operator<=>(const Media &o) const = default;

  /// \brief Compares two references.
  /// \param o The other.
  /// \return True when they name the same media object.
  [[nodiscard]] constexpr bool operator==(const Media &o) const = default;

private:
  Guid id_;
};

}
