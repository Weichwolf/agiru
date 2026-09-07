#include "type/Media.h"

#include "runtime/Error.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/MediaSet.h"

#include <string>
#include <string_view>

namespace agiru {

namespace {

[[noreturn]] void RefuseWithoutStore(const std::string &what) {
  throw Error(what + " needs the tenant media table, which this runtime does not carry yet "
                     "(board:0031)");
}

std::string Importing(std::string_view filename, std::string_view description) {
  return "importing '" + std::string(filename) + "' as '" + std::string(description) + "'";
}

std::string Over(const Guid &current) {
  return current.IsNull() ? std::string{} : " over media " + current.ToText();
}

}

Guid Media::ImportFile(std::string_view filename,
                       std::string_view description,
                       std::string_view mimeType) {
  RefuseWithoutStore(Importing(filename, description) + " of type '" + std::string(mimeType) + "'" +
                     Over(id_));
}

Guid Media::ImportFile(std::string_view filename, std::string_view description) {
  RefuseWithoutStore(Importing(filename, description) + Over(id_));
}

std::string_view Media::ExportFile(std::string_view filename) const {
  RefuseWithoutStore("exporting media " + id_.ToText() + " to '" + std::string(filename) + "'");
}

Integer MediaSet::Count() const {
  if (id_.IsNull()) { return 0; }
  RefuseWithoutStore("counting media set " + id_.ToText());
}

Guid MediaSet::Item(Integer index) const {
  RefuseWithoutStore("reaching item " + std::to_string(index) + " of media set " + id_.ToText());
}

::agiru::Boolean MediaSet::Insert(const Guid &mediaId) {
  RefuseWithoutStore("adding media " + mediaId.ToText() + " to set " + id_.ToText());
}

Guid MediaSet::ImportFile(std::string_view filename, std::string_view description) {
  RefuseWithoutStore(Importing(filename, description) + " into set " + id_.ToText());
}

Boolean Media::ExportStream(class OutStream &Stream) {
  static_cast<void>(Stream);
  RefuseWithoutStore("exporting media " + id_.ToText() + " to a stream");
}

Guid Media::ImportStream(class InStream &Stream,
                         std::string_view Description,
                         std::string_view MimeType,
                         std::string_view FileName) {
  static_cast<void>(Stream);
  static_cast<void>(MimeType);
  RefuseWithoutStore(Importing(FileName, Description) + " from a stream");
}

List<Guid> Media::FindOrphans() {
  RefuseWithoutStore("sweeping the tenant media table for orphans");
}

Guid MediaSet::ImportStream(class InStream &Stream,
                            std::string_view Description,
                            std::string_view MimeType) {
  static_cast<void>(Stream);
  static_cast<void>(MimeType);
  RefuseWithoutStore(Importing("", Description) + " from a stream");
}

List<Guid> MediaSet::FindOrphans() {
  RefuseWithoutStore("sweeping the tenant media table for orphan sets");
}

}
