#include "dotnet/Regex.h"
#include "dotnet/String.h"
#include "runtime/Error.h"
#include "type/Char.h"
#include "type/Integer.h"
#include "type/Text.h"
#include "type/Variant.h"

#include "Check.h"

#include <string>

using agiru::dotnet::Array;
using agiru::dotnet::String;

namespace {

std::string Joined(const Array &pieces) {
  std::string out;
  for (const agiru::Variant &piece : pieces) {
    out += "[" + std::string(std::string_view(piece)) + "]";
  }
  return out;
}

/// `System.String` IS REBUILT AS THE BASEAPP USES IT: assigned from a text, split on the
/// characters another text or a char array names, trimmed, padded, searched and sliced with
/// zero-based indexes, and read back as text. `File Management.GetSafeFileName` splits a file
/// name on `Path.GetInvalidFileNameChars()` and joins the pieces (O365 Email as PDF UT,
/// 2026-09-12); `Office Line Generation` splits search terms on a separator's `ToCharArray()`.
void AStringSplitsTrimsPadsAndSlicesTheWayDotNetDoes() {
  String text;
  text = "a|b||c";
  CHECK_TRUE("Length counts the characters", text.Length() == 6);
  CHECK_TEXT("Split on one text keeps the empty piece", Joined(text.Split("|")), "[a][b][][c]");
  String separator;
  separator = "|";
  CHECK_TEXT("and Split on a char array does the same", Joined(text.Split(separator.ToCharArray())),
             "[a][b][][c]");
  CHECK_TEXT("Split on a text of several characters splits on any of them, an empty last piece included",
             Joined(text.Split("|c")), "[a][b][][][]");
  String padded;
  padded = "  x y  ";
  CHECK_TEXT("Trim takes white space off both ends", std::string(std::string_view(padded.Trim())), "x y");
  CHECK_TEXT("TrimStart the front", std::string(std::string_view(padded.TrimStart())), "x y  ");
  CHECK_TEXT("TrimEnd the back", std::string(std::string_view(padded.TrimEnd())), "  x y");
  String word;
  word = "abc";
  CHECK_TEXT("PadLeft fills to the width", std::string(std::string_view(word.PadLeft(5, agiru::Char{'*'}))), "**abc");
  CHECK_TEXT("PadRight too", std::string(std::string_view(word.PadRight(5))), "abc  ");
  CHECK_TEXT("and a width already reached pads nothing", std::string(std::string_view(word.PadLeft(2))), "abc");
  CHECK_TRUE("IndexOf is zero-based", word.IndexOf("c") == 2);
  CHECK_TRUE("and -1 when absent", word.IndexOf("z") == -1);
  CHECK_TRUE("IndexOf from a start", word.IndexOf("a", 1) == -1);
  String repeated;
  repeated = "ab-ab";
  CHECK_TRUE("LastIndexOf finds the last", repeated.LastIndexOf("ab") == 3);
  CHECK_TEXT("Replace replaces every occurrence", std::string(std::string_view(repeated.Replace("ab", "X"))), "X-X");
  CHECK_TEXT("Substring from an index", std::string(std::string_view(repeated.Substring(3))), "ab");
  CHECK_TEXT("Substring with a length", std::string(std::string_view(repeated.Substring(1, 3))), "b-a");
  CHECK_TRUE("StartsWith and EndsWith", repeated.StartsWith("ab") && repeated.EndsWith("-ab") && !repeated.EndsWith("x"));
  CHECK_TRUE("Contains", repeated.Contains("-") && !repeated.Contains("z"));
  CHECK_TRUE("Chars is zero-based", repeated.Chars(2) == agiru::Char{'-'});
  String made;
  made = made.String(word.ToCharArray());
  CHECK_TEXT("the constructor over a char array joins the characters", std::string(std::string_view(made)), "abc");
  CHECK_TEXT("ToCharArray keeps a multi-byte character whole", Joined(String{}.String("ä-b").ToCharArray()),
             "[ä][-][b]");
  agiru::Text<0> asText = agiru::Text<0>{std::string_view(word)};
  CHECK_TEXT("it reads back as Text", std::string(std::string_view(asText)), "abc");
  // THE NEGATIVE CONTROL: an index outside the string refuses, as .NET does, rather than
  // answering an empty slice.
  std::string said;
  try {
    static_cast<void>(word.Substring(4));
  } catch (const agiru::Error &e) { said = e.what(); }
  CHECK_TRUE("a Substring past the end refuses", !said.empty());
  said.clear();
  try {
    static_cast<void>(word.Replace("", "x"));
  } catch (const agiru::Error &e) { said = e.what(); }
  CHECK_TRUE("and replacing the empty text refuses", !said.empty());
}

} // namespace

int main() {
  return gate::Run("DotNetString", [] { AStringSplitsTrimsPadsAndSlicesTheWayDotNetDoes(); });
}
