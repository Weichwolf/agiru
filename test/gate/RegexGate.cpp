#include "dotnet/Regex.h"
#include "dotnet/TimeSpan.h"
#include "runtime/ErrorValue.h"
#include "type/Text.h"

#include "Check.h"

#include <cstdint>
#include <string>
#include <utility>

using agiru::dotnet::Regex;
using agiru::dotnet::RegexOptions;

namespace {

constexpr std::int64_t kTimeoutTicks = 1000;

/// THE REGEX FAMILY IS REBUILT OVER `std::regex`, with the members `Regex Impl.` names: the
/// constructor with options and a timeout, `IsMatch`, `Matches` with groups, `Replace` with `$n`
/// and `${name}`, `Split`, `Escape`, and the group names beside the numbers (board:0675).
void ARegexMatchesReplacesAndSplitsAsDotNetDoes() {
  Regex regex;
  RegexOptions options;
  options = 1;
  regex = regex.Regex(
      "(?<word>[a-z]+)-(\\d+)", options, agiru::dotnet::TimeSpan::FromTicks(kTimeoutTicks));
  CHECK_TRUE("IsMatch finds the pattern, ignoring case", regex.IsMatch("Item-42 and other-7"));
  const agiru::dotnet::MatchCollection matches = regex.Matches("Item-42 and other-7");
  CHECK_TRUE("two matches", matches.Count() == 2);
  CHECK_TEXT(
      "the first is the whole match", std::string(matches.Item(0).Value().Value()), "Item-42");
  CHECK_TRUE("at its position", matches.Item(0).Index() == 0 && matches.Item(1).Index() == 12);
  CHECK_TRUE("with three groups, the whole first", matches.Item(0).Groups().Count() == 3);
  CHECK_TEXT("the named group by name",
             std::string(matches.Item(0).Groups().Item("word").Value().Value()),
             "Item");
  CHECK_TEXT("the numbered group by number",
             std::string(matches.Item(0).Groups().Item(2).Value().Value()),
             "42");
  CHECK_TEXT("Result expands the replacement",
             std::string(matches.Item(1).Result("${word}=$2").Value()),
             "other=7");
  CHECK_TEXT("Replace over all",
             std::string(regex.Replace("Item-42 and other-7", "$2").Value()),
             "42 and 7");
  CHECK_TEXT("Replace with a count",
             std::string(regex.Replace("Item-42 and other-7", "$2", 1).Value()),
             "42 and other-7");
  const agiru::dotnet::Array pieces = regex.Split("a Item-42 b other-7 c");
  CHECK_TRUE("Split gives the pieces between", pieces.Length() == 3);
  CHECK_TEXT("group names, the whole match as 0",
             std::string(regex.GroupNameFromNumber(1).Value()),
             "word");
  CHECK_TRUE("and back", regex.GroupNumberFromName("word") == 1);
  CHECK_TEXT("Escape escapes the metacharacters",
             std::string(Regex::Escape("a.b*c").Value()),
             "a\\.b\\*c");
  std::string said;
  try {
    Regex bad;
    bad = bad.Regex("(unclosed");
  } catch (const agiru::Error &e) { said = e.what(); }
  CHECK_TRUE("a bad pattern refuses, naming it", said.find("unclosed") != std::string::npos);
}

void CopiesAndMovesKeepTheCompiledPatternAlive() {
  Regex assigned;
  CHECK_TRUE("an unbound regex is null", assigned.IsNull());
  {
    Regex original;
    original = original.Regex("(?<word>[a-z]+)");
    Regex copied = original;
    const Regex moved = std::move(copied);
    copied = Regex::Binder{}("[0-9]+");
    CHECK_TRUE("a moved-from variable accepts a fresh binding",
               copied.IsMatch("42") && moved.IsMatch("abc"));
    CHECK_TRUE("a copy keeps the original usable", original.IsMatch("abc"));
    CHECK_TRUE("the moved copy matches", moved.IsMatch("abc"));
    assigned = moved;
    original = original.Regex("[0-9]+");
    CHECK_TRUE("rebinding the original does not mutate its copy", assigned.IsMatch("abc"));
    CHECK_TRUE("the rebound original uses the new pattern", !original.IsMatch("abc"));
  }
  CHECK_TRUE("the compiled pattern outlives its first owner", assigned.IsMatch("abc"));
  CHECK_TEXT("copied named groups remain available",
             std::string(assigned.Match("abc").Groups().Item("word").Value().Value()),
             "abc");
  Regex movedAssignment;
  movedAssignment = std::move(assigned);
  assigned = Regex{};
  CHECK_TRUE("move assignment permits resetting the source",
             assigned.IsNull() && !movedAssignment.IsNull());
  CHECK_TEXT("replacement still uses the copied group metadata",
             std::string(movedAssignment.Replace("abc", "${word}!").Value()),
             "abc!");
}

} // namespace

int main() {
  return gate::Run("Regex", [] {
    ARegexMatchesReplacesAndSplitsAsDotNetDoes();
    CopiesAndMovesKeepTheCompiledPatternAlive();
  });
}
