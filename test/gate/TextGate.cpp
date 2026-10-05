#include "runtime/ErrorValue.h"
#include "type/AlArray.h"
#include "type/Char.h"
#include "type/Code.h"
#include "type/Integer.h"
#include "type/StringValue.h"
#include "type/Text.h"

#include "Check.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <string_view>

using agiru::Code;
using agiru::MaxStrLen;
using agiru::StringError;
using agiru::StrLen;
using agiru::Text;

namespace {

constexpr std::size_t kPositionCapacity = 20;
constexpr agiru::Integer kEuroPosition = 5;

void Utf8FramingPreservesCharacterBoundaries() {
  struct Character {
    std::int32_t code;
    std::string_view encoded;
  };

  constexpr std::array cases{Character{.code = 0x7F, .encoded = "\x7F"},
                             Character{.code = 0x80, .encoded = "\xC2\x80"},
                             Character{.code = 0x7FF, .encoded = "\xDF\xBF"},
                             Character{.code = 0x800, .encoded = "\xE0\xA0\x80"},
                             Character{.code = 0xFFFF, .encoded = "\xEF\xBF\xBF"}};
  for (const auto &one : cases) {
    CHECK_TRUE("Char decodes each UTF-8 width boundary", agiru::Char{one.encoded} == one.code);
    CHECK_TEXT("Char encodes each UTF-8 width boundary",
               agiru::Encoded(agiru::Char{one.code}),
               one.encoded);
    const Text<1> text{one.encoded};
    auto at = text.begin();
    CHECK_TRUE("foreach decodes the same boundary character", *at == one.code);
    ++at;
    CHECK_TRUE("foreach consumes exactly the boundary sequence", !(at != text.end()));
  }
  const Text<2> supplementary{"💡"};
  auto at = supplementary.begin();
  CHECK_TRUE("foreach still decodes four-byte framing", *at == agiru::Char{"💡"});
  ++at;
  CHECK_TRUE("foreach consumes the complete four-byte sequence", !(at != supplementary.end()));
  for (const std::string_view invalid : {"\x80", "\xC2", "\xC2\x41", "\xE0\xA0", "AB"}) {
    bool refused = false;
    try {
      static_cast<void>(agiru::Char{invalid});
    } catch (const agiru::Error &) { refused = true; }
    CHECK_TRUE("invalid sequence framing still refuses", refused);
  }
}

std::string V(const auto &s) {
  return std::string(s.Value());
}

void CodeNormalisesPerTheDocumentation() {
  // code-data-type.md: "converted to uppercase and removes any trailing or leading spaces".
  CHECK_TEXT("a code is uppercased", V(Code<20>("abc")), "ABC");
  CHECK_TEXT("leading and trailing spaces go", V(Code<20>("  abc  ")), "ABC");
  // The documentation's own example, with a Code of maximum length 4.
  CHECK_TEXT("the documentation's example: ' 2 '", V(Code<4>(" 2 ")), "2");
  CHECK_TRUE("an empty code stays empty", Code<4>("").IsEmpty());
  CHECK_TRUE("only spaces make an empty code", Code<4>("   ").IsEmpty());
  CHECK_TRUE("its length is one, not three", Code<4>(" 2 ").Length() == 1);
  // Inner spaces are not touched -- the documentation says leading and trailing only.
  CHECK_TEXT("an inner space stays", V(Code<20>(" a b ")), "A B");
}

void LengthIsCheckedAfterTrimming() {
  // "The length of a Code variable equals the number of characters in the text without leading or
  // trailing spaces" -- so this fits a Code[3] even though the literal is five characters.
  CHECK_TEXT("the trim happens before the length check", V(Code<3>("  ab ")), "AB");

  bool threw = false;
  try {
    const Code<3> c("abcd");
  } catch (const StringError &) { threw = true; }
  CHECK_TRUE("an over-length code throws", threw);

  threw = false;
  try {
    const Text<3> t("abcd");
  } catch (const StringError &) { threw = true; }
  CHECK_TRUE("an over-length text throws", threw);
}

void TheMessageIsTheBcMessage() {
  // THE WORDING IS LOAD-BEARING: BC test code matches substrings of this through
  // Assert.ExpectedError, so a paraphrase would turn a green case red.
  std::string message;
  try {
    const Text<3> t("abcd");
  } catch (const StringError &e) { message = e.what(); }
  CHECK_TEXT("the platform's own wording",
             message.substr(0, message.find(". Value")),
             "The length of the string is 4, but it must be less than or equal to 3 characters");
}

void TextKeepsWhatCodeChanges() {
  CHECK_TEXT("a text is not uppercased", V(Text<20>("abc")), "abc");
  CHECK_TEXT("a text keeps its spaces", V(Text<20>("  abc  ")), "  abc  ");
}

void LengthCountsTheWayDotNetDoes() {
  CHECK_TRUE("ascii counts one per character", StrLen(Text<20>("abc")) == 3);
  // Two bytes in UTF-8, one UTF-16 unit.
  CHECK_TRUE("a latin-1 letter counts one", StrLen(Text<20>("\xc3\xa4")) == 1);
  // Four bytes in UTF-8, a surrogate pair in UTF-16, so .NET counts two.
  CHECK_TRUE("a character beyond the BMP counts two", StrLen(Text<20>("\xf0\x9f\x92\xa1")) == 2);
  CHECK_TRUE("MaxStrLen is the declared length", MaxStrLen(Code<20>("x")) == 20);
}

void CodeOrdersNumericallyWhereBothSidesAreDigits() {
  // NOT IN THE DOCUMENTATION -- a predecessor finding, carried because
  // NoSeriesStatelessImpl.Codeunit.al:109 compares number-series codes with < and >.
  CHECK_TRUE("all-digit codes order as numbers", Code<20>("109003") < Code<20>("1010999"));
  CHECK_TRUE("and not as strings", !(Code<20>("109003") > Code<20>("1010999")));
  CHECK_TRUE("leading zeros do not change the order", Code<20>("0009") < Code<20>("10"));
  CHECK_TRUE("a non-numeric side falls back to string order", Code<20>("A9") < Code<20>("AB"));
  // Equality stays exact, so "01" and "1" remain different primary keys.
  CHECK_TRUE("equality is exact string, not numeric", !(Code<20>("01") == Code<20>("1")));
  CHECK_TRUE("but they still order", Code<20>("01") < Code<20>("2"));
}

void GeneratedPositionsReadCharactersNotBytes() {
  Text<kPositionCapacity> text("AÆØÅ€Z");
  const Text<kPositionCapacity> frozen(text);
  CHECK_TRUE("mutable generated positions decode a two-byte character",
             static_cast<agiru::Char>(agiru::At(text, 2)) == 198);
  CHECK_TRUE("const generated positions decode a two-byte character", agiru::At(frozen, 2) == 198);
  CHECK_TRUE("mutable generated positions decode a three-byte character",
             static_cast<agiru::Char>(agiru::At(text, 5)) == 8364);
  CHECK_TRUE("const generated positions agree with direct string indexing",
             agiru::At(frozen, 6) == frozen[6]);
  CHECK_TRUE("generated positions compare Unicode character literals", agiru::At(text, 3) == "Ø");
  bool refused = false;
  try {
    (void)static_cast<agiru::Char>(agiru::At(text, text.Length() + 1));
  } catch (const agiru::Error &) { refused = true; }
  CHECK_TRUE("UTF-8 byte length does not extend the readable character range", refused);
}

void GeneratedPositionsReplaceWholeCharacters() {
  Text<kPositionCapacity> text("AÆØÅ€Z");
  agiru::At(text, 2) = agiru::Char{"A"};
  CHECK_TEXT("ASCII replacement consumes the complete old UTF-8 character", V(text), "AAØÅ€Z");
  agiru::At(text, 3) = agiru::Char{"€"};
  CHECK_TEXT("a wider replacement preserves the following characters", V(text), "AA€Å€Z");
  agiru::At(text, 4) = agiru::Char{"Æ"}.AsInteger();
  CHECK_TEXT(
      "integer assignment encodes its character rather than its low byte", V(text), "AA€Æ€Z");
  agiru::At(text, kEuroPosition) = "Ø";
  CHECK_TEXT("one-character literals may occupy multiple UTF-8 bytes", V(text), "AA€ÆØZ");
  CHECK_TRUE("replacements preserve the UTF-16 length", text.Length() == 6);
}

void GeneratedPositionAssignmentsCopyValues() {
  Text<kPositionCapacity> source("ÆØ");
  Text<kPositionCapacity> target("AB");
  agiru::At(target, 1) = agiru::At(source, 2);
  CHECK_TEXT("same-type position assignment copies the character, not the proxy", V(target), "ØB");
  Text<10> narrow("Å");
  agiru::At(target, 2) = agiru::At(narrow, 1);
  CHECK_TEXT("different-type position assignment also copies a decoded character", V(target), "ØÅ");
  agiru::At(target, 1) = agiru::At(target, 2);
  CHECK_TEXT("same-string position assignment reads before replacing", V(target), "ÅÅ");
  agiru::At(target, 1) = agiru::At(target, 1);
  CHECK_TEXT("same-position assignment leaves the character intact", V(target), "ÅÅ");
  auto position = agiru::At(target, 1);
  const auto &samePosition = position;
  position = samePosition;
  CHECK_TEXT("proxy self-assignment preserves its character", V(target), "ÅÅ");
  auto invalid = agiru::At(target, target.Length() + 1);
  const auto &sameInvalid = invalid;
  bool refused = false;
  try {
    invalid = sameInvalid;
  } catch (const StringError &) { refused = true; }
  CHECK_TRUE("proxy self-assignment still validates the readable source position", refused);
}

void GeneratedPositionsAppendWithinDeclaredBounds() {
  Text<3> text("ÆØ");
  agiru::At(text, 3) = agiru::Char{"€"};
  CHECK_TEXT(
      "the UTF-16 length plus one appends despite a longer byte representation", V(text), "ÆØ€");
  bool refused = false;
  try {
    agiru::At(text, 4) = "Å";
  } catch (const StringError &) { refused = true; }
  CHECK_TRUE("appending respects the declared UTF-16 capacity", refused);
  CHECK_TEXT("an over-capacity append preserves the previous value", V(text), "ÆØ€");
  refused = false;
  try {
    agiru::At(text, text.Length() + 2) = "A";
  } catch (const agiru::Error &) { refused = true; }
  CHECK_TRUE("writes beyond the null-terminator position refuse", refused);
  refused = false;
  try {
    agiru::At(text, 0) = "A";
  } catch (const agiru::Error &) { refused = true; }
  CHECK_TRUE("zero is not a writable AL character index", refused);
  Text<2> empty;
  agiru::At(empty, 1) = agiru::Char{"Æ"}.AsInteger();
  agiru::At(empty, 2) = "Ø";
  CHECK_TEXT("integer and literal assignments build Unicode text from empty", V(empty), "ÆØ");
}

void GeneratedPositionsDoNotSplitSurrogatePairs() {
  Text<10> text("A💡Z");
  CHECK_TRUE("a generated read after a surrogate pair uses its UTF-16 position",
             static_cast<agiru::Char>(agiru::At(text, 4)) == 90);
  agiru::At(text, 4) = "Æ";
  CHECK_TEXT("a generated write after a surrogate pair preserves the pair", V(text), "A💡Æ");
  agiru::At(text, text.Length() + 1) = "Ø";
  CHECK_TEXT("append counts the pair as two UTF-16 units", V(text), "A💡ÆØ");
  for (const agiru::Integer index : {2, 3}) {
    bool refused = false;
    try {
      agiru::At(text, index) = "X";
    } catch (const StringError &) { refused = true; }
    CHECK_TRUE("unsupported half-surrogate replacement refuses explicitly", refused);
    CHECK_TEXT("a refused half-surrogate replacement preserves the text", V(text), "A💡ÆØ");
  }
}

void GeneratedPositionWritesRejectInvalidCharacters() {
  Text<10> text("ÆØ");
  for (const agiru::Integer code : {-1, 65536, 55296, 57343}) {
    bool refused = false;
    try {
      agiru::At(text, 1) = code;
    } catch (const agiru::Error &) { refused = true; }
    CHECK_TRUE("a position cannot encode an out-of-range or isolated surrogate value", refused);
    CHECK_TEXT("refused character writes preserve the text", V(text), "ÆØ");
  }
  for (const std::string_view value : {"", "AB", "💡"}) {
    bool refused = false;
    try {
      agiru::At(text, 1) = value;
    } catch (const agiru::Error &) { refused = true; }
    CHECK_TRUE("a position requires exactly one representable UTF-16 unit", refused);
  }
}

} // namespace

int main() {
  return gate::Run("Text", [] {
    Utf8FramingPreservesCharacterBoundaries();
    CodeNormalisesPerTheDocumentation();
    LengthIsCheckedAfterTrimming();
    TheMessageIsTheBcMessage();
    TextKeepsWhatCodeChanges();
    LengthCountsTheWayDotNetDoes();
    CodeOrdersNumericallyWhereBothSidesAreDigits();
    GeneratedPositionsReadCharactersNotBytes();
    GeneratedPositionsReplaceWholeCharacters();
    GeneratedPositionAssignmentsCopyValues();
    GeneratedPositionsAppendWithinDeclaredBounds();
    GeneratedPositionsDoNotSplitSurrogatePairs();
    GeneratedPositionWritesRejectInvalidCharacters();
  });
}
