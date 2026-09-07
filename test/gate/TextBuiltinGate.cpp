#include "BuiltinsWritten.h"
#include "Check.h"

#include <string>
#include <string_view>

using agiru::ConvertStr;
using agiru::CopyStr;
using agiru::DelChr;
using agiru::DelStr;
using agiru::IncStr;
using agiru::InsStr;
using agiru::LowerCase;
using agiru::PadStr;
using agiru::SelectStr;
using agiru::StrCheckSum;
using agiru::StrPos;
using agiru::UpperCase;

namespace {

void DelChrTakesAWhereAndAWhich() {
  // Every case here is an example from `text-delchr-method.md` -- and the pairing is the trap the
  // predecessor paid for: the SECOND argument is Where, not the character set.
  CHECK_TEXT("Where '<>' with a Which trims only the ends",
             DelChr("Windy West Solutions", "<>", "Ws"),
             "indy West Solution");
  CHECK_TEXT(
      "Where '=' deletes everywhere", DelChr("This is an example", "=", "sx"), "Thi i an eample");
  CHECK_TEXT("Where '>' deletes at the end only",
             DelChr("This is an example", ">", "Tely"),
             "This is an examp");
  CHECK_TEXT("Where '<' deletes at the front only",
             DelChr("This is an example", "<", "This "),
             "an example");
  CHECK_TEXT(
      "an omitted Which means a space", DelChr(" This is an example", "<"), "This is an example");
  CHECK_TEXT(
      "omitting both means every space", DelChr(" Windy West Solutions "), "WindyWestSolutions");
  CHECK_TEXT("'<>' collapses a blank to nothing", DelChr(" ", "<>"), "");
  CHECK_TEXT("'=' beats '<' and '>' when all three are given",
             DelChr("CH93 0076 2011 6238 5295 7", "=<>"),
             "CH9300762011623852957");
}

void APositionIsOneBased() {
  CHECK_TEXT("CopyStr without a length copies the rest", CopyStr("Hello world", 7), "world");
  CHECK_TEXT("CopyStr with a length copies that many", CopyStr("Hello world", 1, 5), "Hello");
  // "If Position is greater than the length of the string, then COPYSTR returns an empty string."
  CHECK_TEXT("a position past the end copies nothing", CopyStr("abc", 9), "");
  CHECK_TEXT("a length past the end copies what is there", CopyStr("abc", 2, 99), "bc");
  CHECK_TEXT("DelStr without a length cuts the tail", DelStr("Hello world", 6), "Hello");
  CHECK_TEXT("DelStr with a length cuts the middle", DelStr("Hello world", 6, 1), "Helloworld");
  // "If Position exceeds the length of String, DELSTR returns the original string, unchanged."
  CHECK_TEXT("a position past the end changes nothing", DelStr("abc", 9), "abc");
  CHECK_TEXT("InsStr inserts before the position",
             InsStr("Hello world", "small ", 7),
             "Hello small world");
  CHECK_TEXT("a position past the end appends", InsStr("ab", "c", 9), "abc");
  CHECK_TRUE("StrPos counts from one", StrPos("Hello world", "world") == 7);
  CHECK_TRUE("StrPos says 0 for what is absent", StrPos("Hello", "z") == 0);
  CHECK_TRUE("StrPos says 0 for nothing at all", StrPos("Hello", "") == 0);
  CHECK_TEXT("SelectStr picks the numbered piece", SelectStr(2, "a,b,c"), "b");
  CHECK_TEXT("and an empty piece is a piece", SelectStr(2, "a,,c"), "");
}

void TheRestOfTheStringBuiltins() {
  CHECK_TEXT("ConvertStr swaps character for character", ConvertStr("abc", "ac", "xz"), "xbz");
  // "Each element in source is only converted ONCE a double-replacement cannot happen."
  CHECK_TEXT("and converts each character once", ConvertStr("ab", "ab", "ba"), "ba");
  CHECK_TEXT("PadStr pads to the length", PadStr("ab", 5), "ab   ");
  CHECK_TEXT("PadStr takes the fill character it is given", PadStr("ab", 5, "."), "ab...");
  CHECK_TEXT("PadStr truncates what is too long", PadStr("abcdef", 3), "abc");
  CHECK_TEXT("IncStr increments the last number", IncStr("DOC-001"), "DOC-002");
  CHECK_TEXT("and keeps the width", IncStr("a12b99c"), "a12b100c");
  CHECK_TEXT("and finds a number that is not at the end",
             IncStr("Account no. 99 does not balance."),
             "Account no. 100 does not balance.");
  CHECK_TEXT("a string with no number increments to nothing", IncStr("ABC"), "");
  CHECK_TEXT("IncStr takes an increment", IncStr("DOC-001", 9), "DOC-010");
  CHECK_TEXT("LowerCase lowers", LowerCase("AbC"), "abc");
  CHECK_TEXT("UpperCase raises", UpperCase("AbC"), "ABC");
  // The EAN example from `text-strchecksum-method.md`.
  CHECK_TRUE("StrCheckSum computes the EAN check digit",
             StrCheckSum("577622135746", "131313131313") == 3);
  CHECK_TRUE("an empty string checksums to 0", StrCheckSum("") == 0);
  CHECK_TRUE("a missing weight counts as 1", StrCheckSum("12") == StrCheckSum("12", "11"));
}

} // namespace

namespace agiru::caption_gate {

/// A CAPTION JOINED TO A CAPTION IS THE TWO CAPTIONS, and it took a board item to make that
/// true. `Record.TableCaption()` hands back a `std::string` and `Record.FieldCaption()` a
/// `std::string_view`; the standard library joins neither pair, so the join found
/// `operator+(Text<0>, Guid)` -- one user-defined conversion on each side -- and wrote
/// `Family {00000000-0000-0000-0000-000000000000}` where AL writes `Family No.` (board:0608).
///
/// IT IS WRITTEN INSIDE `namespace agiru` ON PURPOSE. Both operands are `std::` types, so argument-
/// dependent lookup searches `std` and nothing else; the door's operator is found by ORDINARY
/// lookup from an enclosing namespace, which is where every generated body sits and where the
/// defect lived.
static void ACaptionJoinedToACaptionIsTheTwoCaptions() {
  const std::string table{"Family"};
  const std::string_view field{"No."};

  CHECK_TEXT("a std::string joins a std::string_view", table + " " + field, "Family No.");
  CHECK_TEXT("and the other way round", field + std::string{" of "} + table, "No. of Family");
  CHECK_TEXT("and two views join", std::string_view{"No"} + std::string_view{"."}, "No.");
}

}

int main() {
  return gate::Run("TextBuiltin", [] {
    DelChrTakesAWhereAndAWhich();
    APositionIsOneBased();
    TheRestOfTheStringBuiltins();
    agiru::caption_gate::ACaptionJoinedToACaptionIsTheTwoCaptions();
  });
}
