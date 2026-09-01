#include "agiru/Text.h"

#include "Check.h"

#include <string>

using agiru::Code;
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

} // namespace

int main() {
  CodeNormalisesPerTheDocumentation();
  LengthIsCheckedAfterTrimming();
  TheMessageIsTheBcMessage();
  TextKeepsWhatCodeChanges();
  LengthCountsTheWayDotNetDoes();
  CodeOrdersNumericallyWhereBothSidesAreDigits();
  return gate::Done("Text");
}
