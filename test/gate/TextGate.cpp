#include "type/Code.h"
#include "type/Guid.h"
#include "type/SecretText.h"
#include "type/StringValue.h"
#include "type/Text.h"

#include "Check.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

using agiru::Code;
using agiru::Guid;
using agiru::MaxStrLen;
using agiru::StringError;
using agiru::StrLen;
using agiru::Text;

namespace {

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

void ConcatenationPreservesTextTypeAndAssignmentLimits() {
  const Text<2> left{"ab"};
  const Text<2> right{"cd"};
  const Code<2> code{"xy"};
  const auto joined = left + right;
  CHECK_TRUE("Text plus Text retains Text", (std::is_same_v<decltype(left + right), Text<0>>));
  CHECK_TRUE("Text plus Code retains Text", (std::is_same_v<decltype(left + code), Text<0>>));
  CHECK_TRUE("Code plus Text retains Text", (std::is_same_v<decltype(code + right), Text<0>>));
  CHECK_TRUE("Text plus literal retains Text", (std::is_same_v<decltype(left + " "), Text<0>>));
  CHECK_TRUE("literal plus Text retains Text", (std::is_same_v<decltype(" " + left), Text<0>>));
  CHECK_TRUE("Text plus string retains Text",
             (std::is_same_v<decltype(left + std::string{}), Text<0>>));
  CHECK_TRUE("string plus Text retains Text",
             (std::is_same_v<decltype(std::string{} + left), Text<0>>));
  CHECK_TRUE("nested joins retain Text", (std::is_same_v<decltype(left + code + right), Text<0>>));
  CHECK_TRUE(
      "caption storage joins retain Text",
      (std::is_same_v<decltype(agiru::operator+(std::string_view{}, std::string{})), Text<0>>));
  CHECK_TRUE("native string joins remain standard strings",
             (std::is_same_v<decltype(std::string{} + std::string{}), std::string>));
  CHECK_TEXT("joining sized inputs imposes no input limit on the result", joined, "abcd");
  CHECK_TEXT("Code changes only its own input", left + code, "abXY");
  CHECK_TEXT("case and spaces are retained", Text<0>{" lower "} + "Mixed ", " lower Mixed ");
  CHECK_TEXT("caption storage retains case and spaces",
             agiru::operator+(std::string_view{" lower "}, std::string{"Mixed "}),
             " lower Mixed ");
  CHECK_TEXT("Unicode is retained",
             Text<0>{"\xc3\xa4"} + Text<0>{"\xf0\x9f\x92\xa1"},
             "\xc3\xa4\xf0\x9f\x92\xa1");
  bool overflow = false;
  try {
    const Text<3> limited{joined};
  } catch (const StringError &) { overflow = true; }
  CHECK_TRUE("Text destination still rejects overflow", overflow);
  overflow = false;
  try {
    const Code<3> limited{joined};
  } catch (const StringError &) { overflow = true; }
  CHECK_TRUE("Code destination still rejects overflow", overflow);
  CHECK_TEXT("a Code destination still normalizes", Code<4>{Text<0>{" ab "} + "c"}, "AB C");
}

void AnOwnedUnboundedResultTransfersItsBuffer() {
  constexpr std::size_t kHeapFixtureLength = 4096;
  std::string storage(kHeapFixtureLength, 'x');
  const char *const allocation = storage.data();
  const Text<0> owned{std::move(storage)};
  CHECK_TRUE("an owned result is not copied", owned.Value().data() == allocation);
  CHECK_TRUE("all owned bytes remain present", owned.Value().size() == kHeapFixtureLength);
}

constexpr int Selected(const Text<0> &) {
  return 1;
}

constexpr int Selected(const std::string &) {
  return 2;
}

constexpr int Selected(const Guid &) {
  return 3;
}

constexpr int Selected(const agiru::SecretText &) {
  return 4;
}

template <typename Left, typename Right>
concept Joinable = requires(const Left &left, const Right &right) { left + right; };

void GuidConcatenationPreservesTextAndNativeStorage() {
  const Guid identity{"{aaaaaaaa-0000-1111-2222-bbbbbbbbbbbb}"};
  const Text<2> bounded{" x"};
  const Text<0> unbounded{"y "};
  const Code<2> code{"ab"};
  const std::string canonical = identity.ToText();
  CHECK_TRUE("bounded Text plus Guid retains Text",
             (std::is_same_v<decltype(bounded + identity), Text<0>>));
  CHECK_TRUE("Guid plus bounded Text retains Text",
             (std::is_same_v<decltype(identity + bounded), Text<0>>));
  CHECK_TRUE("unbounded Text plus Guid retains Text",
             (std::is_same_v<decltype(unbounded + identity), Text<0>>));
  CHECK_TRUE("Guid plus unbounded Text retains Text",
             (std::is_same_v<decltype(identity + unbounded), Text<0>>));
  CHECK_TEXT("bounded Text imposes no result limit", bounded + identity, " x" + canonical);
  CHECK_TEXT("Guid before bounded Text retains spaces", identity + bounded, canonical + " x");
  CHECK_TEXT(
      "unbounded Text retains case and trailing spaces", unbounded + identity, "y " + canonical);
  CHECK_TEXT("Guid before unbounded Text retains case", identity + unbounded, canonical + "y ");
  const auto chained = bounded + identity + unbounded + code;
  CHECK_TRUE("Text Guid Code chain retains Text",
             (std::is_same_v<decltype(chained), const Text<0>>));
  CHECK_TEXT("Code normalization stays local to its operand", chained, " x" + canonical + "y AB");
  CHECK_TEXT("Guid between temporary Text values owns its result",
             Text<0>{"a"} + identity + Text<0>{"b"},
             "a" + canonical + "b");
  CHECK_TEXT("a null Guid uses the same formatter",
             Text<0>{""} + Guid{},
             "{00000000-0000-0000-0000-000000000000}");
  const auto unicode = Text<3>{"\xc3\xa4\xf0\x9f\x92\xa1"} + identity;
  CHECK_TEXT(
      "Unicode survives Guid concatenation", unicode, "\xc3\xa4\xf0\x9f\x92\xa1" + canonical);
  CHECK_TRUE("Unicode and Guid length counts UTF-16 units", StrLen(unicode) == 41);
  CHECK_TRUE("a Text join selects the Text overload", Selected(bounded + identity) == 1);
  CHECK_TRUE("the reverse Text join selects the Text overload", Selected(identity + bounded) == 1);
  CHECK_TRUE("typed Guid still selects Guid", Selected(identity) == 3);
  CHECK_TRUE("SecretText still selects SecretText", Selected(agiru::SecretText{"secret"}) == 4);
  CHECK_TRUE("a secret cannot concatenate with Guid", (!Joinable<agiru::SecretText, Guid>));
  CHECK_TRUE("Guid cannot concatenate with a secret", (!Joinable<Guid, agiru::SecretText>));
  CHECK_TRUE("native string plus Guid stays native",
             (std::is_same_v<decltype(std::string{} + identity), std::string>));
  CHECK_TRUE("Guid plus native string stays native",
             (std::is_same_v<decltype(identity + std::string{}), std::string>));
  CHECK_TRUE("native Guid join selects the native overload",
             Selected(std::string{"a"} + identity) == 2);
  CHECK_TEXT(
      "native prefix joins with the canonical Guid", std::string{"a"} + identity, "a" + canonical);
  CHECK_TEXT(
      "native suffix joins with the canonical Guid", identity + std::string{"b"}, canonical + "b");
  CHECK_TEXT("native literal prefix stays supported", "a" + identity, "a" + canonical);
  CHECK_TEXT("native literal suffix stays supported", identity + "b", canonical + "b");
  CHECK_TEXT(
      "native view prefix stays supported", std::string_view{"a"} + identity, "a" + canonical);
  CHECK_TEXT(
      "native view suffix stays supported", identity + std::string_view{"b"}, canonical + "b");
  Text<3> destination{"old"};
  std::string diagnostic;
  try {
    destination = bounded + identity;
  } catch (const StringError &error) { diagnostic = error.what(); }
  CHECK_TEXT("destination overflow keeps the BC diagnostic",
             diagnostic.substr(0, diagnostic.find(". Value")),
             "The length of the string is 40, but it must be less than or equal to 3 characters");
  CHECK_TEXT("failed assignment preserves the destination", destination, "old");
  bool overflow = false;
  try {
    const Code<3> limited{identity + bounded};
  } catch (const StringError &) { overflow = true; }
  CHECK_TRUE("Code destination also enforces its limit", overflow);
}

} // namespace

int main() {
  return gate::Run("Text", [] {
    CodeNormalisesPerTheDocumentation();
    LengthIsCheckedAfterTrimming();
    TheMessageIsTheBcMessage();
    TextKeepsWhatCodeChanges();
    LengthCountsTheWayDotNetDoes();
    CodeOrdersNumericallyWhereBothSidesAreDigits();
    ConcatenationPreservesTextTypeAndAssignmentLimits();
    AnOwnedUnboundedResultTransfersItsBuffer();
    GuidConcatenationPreservesTextAndNativeStorage();
  });
}
