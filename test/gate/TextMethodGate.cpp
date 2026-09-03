#include "Check.h"
#include "type/Char.h"
#include "type/List.h"
#include "type/Text.h"

#include <string>

using agiru::Char;
using agiru::List;
using agiru::Text;

namespace {

const Text<50> kAbc{"Hello world"};

void PositionsAreOneBased() {
  // `text-indexof-method.md`: "the one-based index of the first occurrence ... If the index
  // returned is 0, the value is not present in the string."
  CHECK_TRUE("IndexOf counts from one", kAbc.IndexOf("world") == 7);
  CHECK_TRUE("IndexOf says 0 for what is absent", kAbc.IndexOf("World") == 0);
  CHECK_TRUE("IndexOf starts where it is told", kAbc.IndexOf("o", 6) == 8);
  CHECK_TRUE("LastIndexOf is the last one", kAbc.LastIndexOf("o") == 8);
  CHECK_TRUE("LastIndexOf searches backwards from its start", kAbc.LastIndexOf("o", 6) == 5);
  CHECK_TRUE("IndexOfAny takes the first of any", kAbc.IndexOfAny("dw") == 7);
  CHECK_TRUE("IndexOfAny says 0 for none of them", kAbc.IndexOfAny("xyz") == 0);
}

void ASubstringIsCutOneBased() {
  CHECK_TEXT("Substring without a count is the rest", kAbc.Substring(7), "world");
  CHECK_TEXT("Substring with a count is that many", kAbc.Substring(1, 5), "Hello");
  // `text-substring-method.md`, Remarks: since application version 27.1 a Count past the end is
  // no longer an error.
  CHECK_TEXT("a count past the end yields the rest", kAbc.Substring(7, 99), "world");
  CHECK_TEXT("Remove without a count cuts the tail", kAbc.Remove(6), "Hello");
  CHECK_TEXT("Remove with a count cuts the middle", kAbc.Remove(6, 1), "Helloworld");
}

void TheEdgesAreTrimmedAndPadded() {
  CHECK_TEXT("Trim takes white space off both ends", Text<50>("  x  ").Trim(), "x");
  CHECK_TEXT("TrimStart takes it off the front only", Text<50>("  x  ").TrimStart(), "x  ");
  CHECK_TEXT("TrimEnd takes it off the back only", Text<50>("  x  ").TrimEnd(), "  x");
  CHECK_TEXT("TrimEnd takes the characters it is given", Text<50>("x..").TrimEnd("."), "x");
  // `text-padleft-method.md`: Count is the length of the RESULT, not the padding added.
  CHECK_TEXT("PadLeft pads to a total length", Text<50>("7").PadLeft(3), "  7");
  CHECK_TEXT("PadLeft takes the character it is given", Text<50>("7").PadLeft(3, Char{'0'}), "007");
  CHECK_TEXT("PadRight pads on the other side", Text<50>("7").PadRight(3, Char{'0'}), "700");
  CHECK_TEXT("padding a long enough text changes nothing", Text<50>("1234").PadLeft(3), "1234");
}

void CaseAndReplacementAreWholesale() {
  CHECK_TEXT("ToUpper raises every letter", kAbc.ToUpper(), "HELLO WORLD");
  CHECK_TEXT("ToLower lowers every letter", kAbc.ToLower(), "hello world");
  CHECK_TEXT("Replace replaces every occurrence", Text<50>("a-b-c").Replace("-", "+"), "a+b+c");
  CHECK_TEXT("replacing with nothing removes", Text<50>("a-b-c").Replace("-", ""), "abc");
  CHECK_TRUE("Contains sees a substring", kAbc.Contains("lo w"));
  CHECK_TRUE("StartsWith sees the front", kAbc.StartsWith("Hell"));
  CHECK_TRUE("EndsWith sees the back", kAbc.EndsWith("rld"));
}

void SplitKeepsWhatItFinds() {
  const List<std::string> parts = Text<50>("a,b,,c").Split(",");
  CHECK_TRUE("Split yields every piece", parts.Count() == 4);
  CHECK_TEXT("including the first", parts.Get(1), "a");
  // .NET drops empty pieces only when asked to, and AL's Split has nothing to ask with.
  CHECK_TEXT("and the empty one between two separators", parts.Get(3), "");
  CHECK_TEXT("and the last", parts.Get(4), "c");
  // `text-split-text-method.md`, Remarks: "If no separators are specified, the text is split at
  // white-space characters."
  const List<std::string> words = Text<50>("one two").Split();
  CHECK_TRUE("Split with no separator splits at white space", words.Count() == 2);
  CHECK_TEXT("into the words", words.Get(2), "two");
  List<Char> commas;
  commas.Add(Char{','});
  CHECK_TRUE("a List of [Char] separates too", Text<50>("a,b").Split(commas).Count() == 2);
}

void AnIndexCountsUtf16Units() {
  // The same rule StrLen follows: a Latin-1 letter is one unit and four UTF-8 bytes are two.
  const Text<50> umlaut{"\xc3\xa4x"};
  CHECK_TRUE("a position after a two-byte character is still one-based in units",
             umlaut.IndexOf("x") == 2);
  CHECK_TEXT("and a substring cuts on the character, not the byte", umlaut.Substring(2), "x");
  CHECK_TEXT("and one before it keeps the whole character", umlaut.Substring(1, 1), "\xc3\xa4");
}

} // namespace

int main() {
  return gate::Run("TextMethod", [] {
    PositionsAreOneBased();
    ASubstringIsCutOneBased();
    TheEdgesAreTrimmedAndPadded();
    CaseAndReplacementAreWholesale();
    SplitKeepsWhatItFinds();
    AnIndexCountsUtf16Units();
  });
}
