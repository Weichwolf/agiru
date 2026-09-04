#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/Error.h"
#include "runtime/Storage.h"
#include "type/Guid.h"
#include "type/Media.h"
#include "type/MediaSet.h"

#include "Check.h"

#include <span>
#include <string>

using agiru::Error;
using agiru::FieldDef;
using agiru::FieldType;
using agiru::Guid;
using agiru::Media;
using agiru::MediaSet;

namespace {

const Guid kSomeMedia = Guid::FromText("{B6666666-F5A2-E911-8180-001DD8B7338E}");

FieldDef Declared(FieldType type) {
  return FieldDef{.no = agiru::FieldNo{1},
                  .name = "Picture",
                  .caption = "Picture",
                  .type = type,
                  .length = 0,
                  .offset = 0,
                  .values = std::span<const agiru::EnumValueDef>{},
                  .initValue = {}};
}

/// A Media FIELD HOLDS AN IDENTIFIER, NOT BYTES. The media object lives in the tenant media table
/// and the row carries the GUID that finds it -- which is why the column is a `uuid` and not a
/// `bytea` the way a BLOB's is.
void ItReferencesRatherThanCarries() {
  const Media empty;
  CHECK_TRUE("a fresh Media references nothing", !empty.HasValue());
  CHECK_TRUE("and its identifier is blank", empty.MediaId().IsNull());

  const Media set{kSomeMedia};
  CHECK_TRUE("one that names an object has a value", set.HasValue());
  CHECK_TRUE("and hands the identifier back", set.MediaId() == kSomeMedia);

  CHECK_TEXT("the column is a uuid", agiru::ColumnType(Declared(FieldType::Media)), "uuid");
  CHECK_TEXT("and so is a MediaSet's", agiru::ColumnType(Declared(FieldType::MediaSet)), "uuid");
  // THE NEGATIVE CONTROL for the sentence above: a BLOB carries bytes and is a different column.
  CHECK_TEXT("while a Blob carries bytes", agiru::ColumnType(Declared(FieldType::Blob)), "bytea");
}

std::string Refusal(const auto &call) {
  try {
    call();
  } catch (const Error &e) { return e.what(); }
  return {};
}

/// EVERYTHING THAT MOVES BYTES REFUSES, and says which table it is missing. A runtime that answered
/// these would be inventing a media store, and an import that quietly did nothing is the exact
/// shape of defect that leaves a picture missing three layers away from the code that lost it.
void MovingBytesRefusesAndSaysWhy() {
  Media blank;
  const std::string importing = Refusal([&blank] { return blank.ImportFile("/tmp/a.png", "a"); });
  CHECK_TRUE("importing refuses", !importing.empty());
  CHECK_TRUE("and names the table it wants",
             importing.find("tenant media table") != std::string::npos);
  // THE REFUSAL NAMES WHAT IT LOST. "some import failed" sends a reader to the wrong file, and the
  // filename is the only thing in that sentence that tells them which one.
  CHECK_TRUE("and names the file it would not import",
             importing.find("/tmp/a.png") != std::string::npos);

  const Media one{kSomeMedia};
  const std::string exporting = Refusal([&one] { return one.ExportFile("/tmp/a.png"); });
  CHECK_TRUE("exporting refuses", !exporting.empty());
  CHECK_TRUE("and names the media object it would not export",
             exporting.find(kSomeMedia.ToText()) != std::string::npos);

  const std::string reaching = Refusal([] { return MediaSet{kSomeMedia}.Item(1); });
  CHECK_TRUE("reaching into a set refuses", !reaching.empty());
  CHECK_TRUE("and names the set", reaching.find(kSomeMedia.ToText()) != std::string::npos);

  CHECK_TRUE("adding to a set refuses", !Refusal([] {
                                           MediaSet set{kSomeMedia};
                                           set.Insert(kSomeMedia);
                                         }).empty());
}

/// AN EMPTY SET COUNTS ZERO WITHOUT ASKING ANYONE, because that answer needs no table: a field that
/// references no set holds no media, and AL code guards on exactly that before reaching for `Item`.
/// A NON-empty one must NOT answer zero -- that would be the silent wrong number this whole tree is
/// arranged against, and it is the control that makes the line above mean something.
void AnEmptySetCountsZeroAndAFullOneRefuses() {
  CHECK_TRUE("an empty MediaSet counts zero", MediaSet{}.Count() == 0);
  const std::string said = Refusal([] { return MediaSet{kSomeMedia}.Count(); });
  CHECK_TRUE("a set that names something refuses rather than answering zero", !said.empty());
  CHECK_TRUE("and names the table it wants", said.find("tenant media table") != std::string::npos);
}

} // namespace

int main() {
  return gate::Run("Media", [] {
    ItReferencesRatherThanCarries();
    MovingBytesRefusesAndSaysWhy();
    AnEmptySetCountsZeroAndAFullOneRefuses();
  });
}
