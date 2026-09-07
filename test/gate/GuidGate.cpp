#include "type/Code.h"
#include "type/Guid.h"
#include "type/Text.h"

#include "Check.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <set>
#include <string>

using agiru::Code;
using agiru::Guid;
using agiru::Text;

namespace {

// `guid-data-type.md`: "The GUID is a 16-byte binary data type that can be logically grouped into
// the following subgroups: 4byte-2byte-2byte-2byte-6byte. The standard textual representation is
// {aaaaaaaa-0000-1111-2222-bbbbbbbbbbbb}."
constexpr std::array<std::uint8_t, Guid::kSize> kBytes{
    0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 0x11, 0x11, 0x22, 0x22, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB};
constexpr Guid kNamed{kBytes};

// AL's own lengths: `User."Full Name"` is Text[80] and `User."User Name"` Code[50],
// which is the pair a GUID is written into and read back from in the BaseApp.
constexpr std::size_t kFieldLength = 80;
constexpr std::size_t kCodeLength = 50;

static_assert(Guid{}.IsNull(), "the default GUID is the empty one AL calls null");
static_assert(!kNamed.IsNull());
static_assert(Guid{kBytes} == kNamed);

void TheTextIsTheDocumentedOneBracesIncluded() {
  CHECK_TEXT("the standard textual representation carries its braces",
             kNamed.ToText(),
             "{aaaaaaaa-0000-1111-2222-bbbbbbbbbbbb}");
  CHECK_TEXT("and a uuid column takes it without them",
             kNamed.ToStorageText(),
             "aaaaaaaa-0000-1111-2222-bbbbbbbbbbbb");
}

void TheTextGoesBothWays() {
  // guid-data-type.md: "You can assign and compare the Text data type and the GUID data type."
  CHECK_TRUE("a braced string reads back as the same GUID",
             Guid::FromText(kNamed.ToText()) == kNamed);
  CHECK_TRUE("and so does an unbraced one", Guid::FromText(kNamed.ToStorageText()) == kNamed);
  CHECK_TRUE("upper case reads the same",
             Guid::FromText("{AAAAAAAA-0000-1111-2222-BBBBBBBBBBBB}") == kNamed);
  CHECK_TRUE("text that is not a GUID gives the null one",
             Guid::FromText("not a guid at all").IsNull());
  CHECK_TRUE("and so does one that stops short", Guid::FromText("{aaaaaaaa-0000-1111}").IsNull());
}

// The layout the RFC gives a UUID and every platform's GUID follows: the version in the high
// nibble of byte 6, the variant in the top two bits of byte 8.
constexpr std::size_t kVersionByte = 6;
constexpr std::size_t kVariantByte = 8;
constexpr unsigned kNibbleBits = 4;
constexpr std::uint8_t kVariantMask = 0xC0;
constexpr std::uint8_t kVariantBits = 0x80;
constexpr std::size_t kTimestampBytes = 6;

bool IsWellFormed(const Guid &guid, std::uint8_t version) {
  return (guid.Bytes()[kVersionByte] >> kNibbleBits) == version &&
         (guid.Bytes()[kVariantByte] & kVariantMask) == kVariantBits;
}

void CreatedGuidsAreUniqueAndWellFormed() {
  constexpr int kHowMany = 1000;
  std::set<std::string> seen;
  bool wellFormed = true;
  for (int i = 0; i < kHowMany; ++i) {
    const Guid made = Guid::Create();
    seen.insert(made.ToText());
    // Version 4 in the high nibble of byte 6, and the RFC variant in the top bits of byte 8.
    wellFormed = wellFormed && !made.IsNull() && IsWellFormed(made, 4);
  }
  CHECK_TRUE("a thousand GUIDs are a thousand different GUIDs", seen.size() == kHowMany);
  CHECK_TRUE("and each is a well-formed version 4", wellFormed);
}

void SequentialGuidsRiseWithTime() {
  // guid-createsequentialguid-method.md: "Sequential GUIDs perform better when used as a field in a
  // key, because they are partially sequential instead of fully random." The leading bytes are what
  // has to rise; the rest may not.
  constexpr int kHowMany = 64;
  bool rising = true;
  Guid previous = Guid::CreateSequentialGuid();
  for (int i = 0; i < kHowMany; ++i) {
    const Guid next = Guid::CreateSequentialGuid();
    // The timestamp has millisecond resolution, so two in the same millisecond may tie on the
    // prefix; what must never happen is the prefix going backwards.
    for (std::size_t at = 0; at < kTimestampBytes; ++at) {
      if (next.Bytes()[at] != previous.Bytes()[at]) {
        rising = rising && next.Bytes()[at] > previous.Bytes()[at];
        break;
      }
    }
    previous = next;
  }
  CHECK_TRUE("the leading bytes never go backwards", rising);
  CHECK_TRUE("and it is still a well-formed version 7", IsWellFormed(previous, 7));
  CHECK_TRUE("a sequential GUID differs from a random one",
             Guid::CreateSequentialGuid() != Guid::Create());
}

/// AL ASSIGNS A TEXT TO A GUID WITHOUT A CAST, and the page says so outright:
/// `guid-data-type.md`, "You can assign and compare the Text data type and the GUID data type",
/// with `MyTableRec.MyGuid := MyTableRec.MyText` as its own example and the two accepted spellings
/// beside it. `ImportConsolidationFromAPI` writes `TempCompany.Id := JsonToken.AsValue().AsText()`,
/// which is the same assignment one builtin further out.
void ATextAssignsToAGuidTheWayALAssignsIt() {
  const std::string braced{"{aaaaaaaa-0000-1111-2222-bbbbbbbbbbbb}"};
  Guid held;

  held = braced;
  CHECK_TRUE("a std::string assigns, which is what a builtin returning Text hands back",
             held == kNamed);

  Text<kFieldLength> field;
  field = "aaaaaaaa-0000-1111-2222-bbbbbbbbbbbb";
  held = Guid{};
  held = field;
  CHECK_TRUE("and so does a Text[N] field, unbraced -- both spellings are documented",
             held == kNamed);

  Code<kCodeLength> code;
  code = braced;
  held = Guid{};
  held = code;
  CHECK_TRUE("and a Code[N] field, braced", held == kNamed);

  // AL COMPARES A TEXT AGAINST A GUID AS GUIDS, which is why the two spellings compare equal.
  CHECK_TRUE("a braced text equals the GUID", braced == kNamed);
  CHECK_TRUE("and so does the unbraced field", field == kNamed);
  CHECK_TRUE("text that is not a GUID does not", std::string{"Family No."} != kNamed);
}

} // namespace

int main() {
  return gate::Run("Guid", [] {
    TheTextIsTheDocumentedOneBracesIncluded();
    TheTextGoesBothWays();
    ATextAssignsToAGuidTheWayALAssignsIt();
    CreatedGuidsAreUniqueAndWellFormed();
    SequentialGuidsRiseWithTime();
  });
}
