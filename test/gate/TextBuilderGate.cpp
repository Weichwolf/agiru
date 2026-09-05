#include "type/TextBuilder.h"

#include "Check.h"

using agiru::TextBuilder;

namespace {

// textbuilder-remove-method.md and the predecessor's live call site
// (`RecordTextBuilder.Remove(RecordTextBuilder.Length, 1)` strips the trailing separator):
// the index is ONE-BASED, as everywhere else a text is indexed in AL.
void TheIndexIsOneBased() {
  TextBuilder builder;
  builder.Append("A, B, C, ");
  builder.Remove(builder.Length(), 1);
  CHECK_TEXT("Remove(Length, 1) drops the last character", builder.ToText(), "A, B, C,");

  TextBuilder middle;
  middle.Append("abcdef");
  middle.Remove(2, 3);
  CHECK_TEXT("Remove(2, 3) takes bcd out", middle.ToText(), "aef");

  TextBuilder inserted;
  inserted.Append("aef");
  inserted.Insert(2, "bcd");
  CHECK_TEXT("Insert(2, ...) writes before the second character", inserted.ToText(), "abcdef");

  TextBuilder part;
  part.Append("abcdef");
  CHECK_TEXT("ToText(2, 3) reads bcd", part.ToText(2, 3), "bcd");
}

// textbuilder-append-method.md / textbuilder-appendline-method.md
void AppendingGrowsTheText() {
  TextBuilder builder;
  builder.Append("one");
  builder.AppendLine(" two");
  builder.Append("three");
  CHECK_TEXT("AppendLine ends the line it appends", builder.ToText(), "one two\nthree");
  CHECK_TRUE("and the length counts what is there", builder.Length() == 13);
  builder.Clear();
  CHECK_TEXT("Clear empties it", builder.ToText(), "");
}

// textbuilder-replace-text-text-method.md, and the range form beside it.
void ReplaceWorksWholeAndInRange() {
  TextBuilder all;
  all.Append("a,b,c");
  all.Replace(",", ";");
  CHECK_TEXT("the two-argument form replaces every occurrence", all.ToText(), "a;b;c");

  TextBuilder ranged;
  ranged.Append("a,b,c");
  ranged.Replace(",", ";", 1, 3);
  CHECK_TEXT("the range form leaves what is outside it alone", ranged.ToText(), "a;b,c");
}

// textbuilder-length-method.md: `[OldLength := ] TextBuilder.Length([NewLength])` -- the setter
// returns what the length WAS.
void LengthReadsAndTruncates() {
  TextBuilder builder;
  builder.Append("abcdef");
  CHECK_TRUE("the setter returns the old length", builder.Length(3) == 6);
  CHECK_TEXT("and cuts the text to the new one", builder.ToText(), "abc");
}

} // namespace

int main() {
  return gate::Run("TextBuilder", [] {
    TheIndexIsOneBased();
    AppendingGrowsTheText();
    ReplaceWorksWholeAndInRange();
    LengthReadsAndTruncates();
  });
}
