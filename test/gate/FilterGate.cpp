#include "meta/TableDef.h"

#include "Check.h"
#include "Filter.h"

#include <cstdint>
#include <optional>
#include <string_view>

using agiru::FieldDef;
using agiru::FieldType;
using agiru::detail::Intervals;
using agiru::detail::Matches;
using agiru::detail::ParseFilter;

namespace {

const FieldDef &TextField() {
  static const FieldDef def{.no = agiru::FieldNo{1},
                            .name = "Name",
                            .caption = "Name",
                            .type = FieldType::Text,
                            .length = 50,
                            .offset = 0,
                            .values = {},
                            .initValue = {}};
  return def;
}

const FieldDef &NumberField() {
  static const FieldDef def{.no = agiru::FieldNo{2},
                            .name = "Entry No.",
                            .caption = "Entry No.",
                            .type = FieldType::Integer,
                            .length = 0,
                            .offset = 0,
                            .values = {},
                            .initValue = {}};
  return def;
}

bool Passes(std::string_view filter, std::string_view value, const FieldDef &def) {
  return Matches(ParseFilter(filter), value, def);
}

/// `&` BINDS TIGHTER THAN `|`, which is the whole reason the parse is a list of lists.
void ConjunctionBindsTighterThanDisjunction() {
  CHECK_TRUE("a range written as two comparisons holds",
             Passes(">=1000&<=2000", "1500", NumberField()));
  CHECK_TRUE("and excludes what falls outside it", !Passes(">=1000&<=2000", "2500", NumberField()));
  // `1|>=1000&<=2000` is `1` OR `(>=1000 AND <=2000)`, never `(1 OR >=1000) AND <=2000`.
  CHECK_TRUE("an alternative on its own passes", Passes("1|>=1000&<=2000", "1", NumberField()));
  CHECK_TRUE("and a value in the other branch passes",
             Passes("1|>=1000&<=2000", "1500", NumberField()));
  CHECK_TRUE("while one in neither does not", !Passes("1|>=1000&<=2000", "500", NumberField()));
}

/// WHITESPACE IS INSIGNIFICANT, and missing that made the predecessor compare a date against the
/// literal text `"< 2028-01-28"`. `'< %1'` and `'> 5'` are ordinary AL spellings.
void SpacesAroundAnOperatorAreNotPartOfTheValue() {
  CHECK_TRUE("a space after the operator is not part of the operand",
             Passes("> 5", "9", NumberField()));
  CHECK_TRUE("and neither is one before it", Passes("  <> 0 ", "3", NumberField()));
  CHECK_TRUE("the same filter without spaces means the same", Passes(">5", "9", NumberField()));
  // THE NEGATIVE CONTROL: the trimming must not reach INSIDE a quoted operand.
  CHECK_TRUE("a quoted operand keeps its spaces", Passes("'A B'", "A B", TextField()));
  CHECK_TRUE("and does not match the trimmed form", !Passes("'A B'", "AB", TextField()));
}

/// `=` IS OPTIONAL SYNTAX, at 121 call sites in the BaseApp. Unrecognised, the atom compares
/// against the literal text `"=1"` and matches NOTHING -- silently, because an empty result set is
/// a legal outcome.
void TheEqualsSignIsOptional() {
  CHECK_TRUE("an explicit equals means equality", Passes("=1", "1", NumberField()));
  CHECK_TRUE("and is the same filter as the bare value", Passes("1", "1", NumberField()));
  CHECK_TRUE("it does not become part of the value", !Passes("=1", "=1", NumberField()));
  CHECK_TRUE("an equals in front of a wildcard is still a wildcard",
             Passes("=*Ltd*", "Acme Ltd Co", TextField()));
}

/// `*` IS NOT A REGULAR EXPRESSION, and a value out of the database is full of characters one would
/// read: a customer name with a `.` or a `+` in it must not become a pattern.
void WildcardsAreWildcardsAndNotRegularExpressions() {
  CHECK_TRUE("a star stands for any run", Passes("*Ltd*", "Acme Ltd Co", TextField()));
  CHECK_TRUE("and matches at the end", Passes("Acme*", "Acme Ltd", TextField()));
  CHECK_TRUE("a question mark stands for exactly one", Passes("A?me", "Acme", TextField()));
  CHECK_TRUE("and not for none", !Passes("A?me", "Ame", TextField()));
  CHECK_TRUE("a dot in the pattern is a dot", Passes("A.B", "A.B", TextField()));
  CHECK_TRUE("and matches nothing else", !Passes("A.B", "AxB", TextField()));
  CHECK_TRUE("<> in front of a wildcard negates it", !Passes("<>*Ltd*", "Acme Ltd", TextField()));
  CHECK_TRUE("and passes what does not match", Passes("<>*Ltd*", "Acme Inc", TextField()));
}

/// `@` ASKS FOR A CASE-INSENSITIVE COMPARISON. It is a modifier and not a character to look for.
void TheAtSignIsAModifier() {
  CHECK_TRUE("@ does not become part of the pattern", Passes("@*ltd*", "Acme LTD Co", TextField()));
  CHECK_TRUE("and a filter with no @ compares text without case either, as AL does",
             Passes("*ltd*", "Acme LTD Co", TextField()));
  CHECK_TRUE("an @ on its own value still compares", Passes("@acme", "ACME", TextField()));
}

/// A RANGE IS TWO OPERANDS AND EITHER END MAY BE OPEN.
void RangesIncludeBothEndsAndMayBeOpen() {
  CHECK_TRUE("the lower end is included", Passes("1000..2000", "1000", NumberField()));
  CHECK_TRUE("the upper end is included", Passes("1000..2000", "2000", NumberField()));
  CHECK_TRUE("and what lies outside is not", !Passes("1000..2000", "2001", NumberField()));
  CHECK_TRUE("an open lower end takes everything below", Passes("..2000", "1", NumberField()));
  CHECK_TRUE("an open upper end takes everything above", Passes("1000..", "999999", NumberField()));
}

/// A NUMBER IS NOT COMPARED AS TEXT. `"10" < "9"` lexically, so a filter `>=9` over an entry number
/// would drop every row from ten upward -- and look like a correct empty result.
void NumbersCompareAsNumbers() {
  CHECK_TRUE("ten is not less than nine", Passes(">=9", "10", NumberField()));
  CHECK_TRUE("and a hundred is above two", Passes(">2", "100", NumberField()));
  CHECK_TRUE("while the same values as TEXT compare the other way",
             !Passes(">=9", "10", TextField()));
}

/// AL QUOTES AN OPERAND THAT CONTAINS AN OPERATOR, and a value arriving through `%1` is data this
/// runtime did not write. Splitting without regard to the quotes turns a company name into a
/// filter.
void AQuotedOperandIsNotSplitOnItsOperators() {
  CHECK_TRUE("an ampersand inside quotes is part of the value",
             Passes("'A&B'", "A&B", TextField()));
  CHECK_TRUE("and a pipe too", Passes("'A|B'", "A|B", TextField()));
  CHECK_TRUE("a star inside quotes is a star and not a wildcard",
             Passes("'A*B'", "A*B", TextField()));
  CHECK_TRUE("so it does not match what the wildcard would", !Passes("'A*B'", "AxxB", TextField()));
  // THE NEGATIVE CONTROL: unquoted, the same characters ARE operators.
  CHECK_TRUE("unquoted, an ampersand is a conjunction", !Passes("A&B", "A&B", TextField()));
  CHECK_TRUE("and an unquoted star is a wildcard", Passes("A*B", "AxxB", TextField()));
}

void AnEmptyFilterPassesEverything() {
  CHECK_TRUE("no filter is not a filter that matches nothing", Passes("", "anything", TextField()));
  CHECK_TRUE("and the blank value is a filter of its own", Passes("''", "", TextField()));
  CHECK_TRUE("which excludes what is not blank", !Passes("''", "x", TextField()));
}

/// A DISJUNCTION OF CONJUNCTIONS IS A UNION OF INTERSECTIONS. The `Integer` virtual table has no
/// rows to scan -- `devenv-integer-virtual-table.md`: "by applying a filter to the Integer virtual
/// table, you can easily get a subset or range of numbers" -- so the filter IS the input, and this
/// is the function that turns it into one.
constexpr agiru::detail::Interval kSmallDomain{.low = 0, .high = 100};

/// The set a filter admits, or an empty one when it does not reduce -- which `Reduces` asks
/// separately, so that no assertion below reads an optional it has not checked.
Intervals Admitted(std::string_view text) {
  const std::optional<Intervals> set =
      agiru::detail::IntegerIntervals(ParseFilter(text), kSmallDomain);
  return set.has_value() ? *set : Intervals{};
}

bool Reduces(std::string_view text) {
  return agiru::detail::IntegerIntervals(ParseFilter(text), kSmallDomain).has_value();
}

void AFilterOverIntegersIsASetOfIntervals() {
  CHECK_TRUE("an empty filter admits the whole domain",
             (Admitted("") == Intervals{{.low = 0, .high = 100}}));
  CHECK_TRUE("a range is one interval", (Admitted("10..20") == Intervals{{.low = 10, .high = 20}}));
  CHECK_TRUE("an open lower end takes the domain's",
             (Admitted("..20") == Intervals{{.low = 0, .high = 20}}));
  CHECK_TRUE("and an open upper end likewise",
             (Admitted("10..") == Intervals{{.low = 10, .high = 100}}));
  CHECK_TRUE("a single value is a single-member interval",
             (Admitted("7") == Intervals{{.low = 7, .high = 7}}));
  CHECK_TRUE("a conjunction intersects",
             (Admitted(">=10&<=20") == Intervals{{.low = 10, .high = 20}}));

  // `<>5` PUNCHES A HOLE, so a conjunction holds a SET and not merely a range. A version that
  // returned one interval per conjunction would answer 0..100 here and admit the 5.
  CHECK_TRUE("a negation leaves two intervals",
             (Admitted("<>5") == Intervals{{.low = 0, .high = 4}, {.low = 6, .high = 100}}));
  CHECK_TRUE("and it still narrows inside a conjunction",
             (Admitted("1..9&<>5") == Intervals{{.low = 1, .high = 4}, {.low = 6, .high = 9}}));

  // ADJACENT INTERVALS MERGE. Two series over 1..5 and 6..9 emit the same rows as one over 1..9,
  // and emitting both is a duplicate row rather than a tidiness question.
  CHECK_TRUE(
      "a disjunction unions",
      (Admitted("1..5|20..30") == Intervals{{.low = 1, .high = 5}, {.low = 20, .high = 30}}));
  CHECK_TRUE("touching parts become one",
             (Admitted("1..5|6..9") == Intervals{{.low = 1, .high = 9}}));
  CHECK_TRUE("and overlapping ones too",
             (Admitted("1..8|4..12") == Intervals{{.low = 1, .high = 12}}));

  CHECK_TRUE("a filter matching nothing admits nothing", Admitted("50..40").empty());
  CHECK_TRUE("and one outside the domain likewise", Admitted("200..300").empty());

  // THE NEGATIVE CONTROL, and it is what keeps the whole thing honest: a wildcard describes no
  // interval, and answering the domain for it would silently admit every row the filter excludes.
  CHECK_TRUE("a wildcard reduces to nothing at all", !Reduces("*1*"));
  CHECK_TRUE("and so does a value that is not an integer", !Reduces("abc"));
}

/// A SEQUENCE TABLE ANSWERS Count() BY ARITHMETIC and never asks the database: the count is exact
/// and costs the number of intervals rather than the number of rows.
void ASetCountsItselfWithoutCountingRows() {
  CHECK_TRUE("one interval counts its own members",
             agiru::detail::CountOf({{.low = 1, .high = 10}}) == 10);
  CHECK_TRUE("a single member counts one", agiru::detail::CountOf({{.low = 7, .high = 7}}) == 1);
  CHECK_TRUE("two intervals add up",
             (agiru::detail::CountOf({{.low = 1, .high = 5}, {.low = 20, .high = 24}}) == 10));
  CHECK_TRUE("an empty set counts nothing", agiru::detail::CountOf({}) == 0);

  // THE DOMAIN THE PAGE GIVES, counted without producing a single row of it.
  constexpr std::int64_t kBillion = 1000000000;
  CHECK_TRUE("and the documented domain counts two billion and one",
             agiru::detail::CountOf({{.low = -kBillion, .high = kBillion}}) == 2000000001);
}

} // namespace

int main() {
  return gate::Run("Filter", [] {
    AFilterOverIntegersIsASetOfIntervals();
    ASetCountsItselfWithoutCountingRows();
    ConjunctionBindsTighterThanDisjunction();
    SpacesAroundAnOperatorAreNotPartOfTheValue();
    TheEqualsSignIsOptional();
    WildcardsAreWildcardsAndNotRegularExpressions();
    TheAtSignIsAModifier();
    RangesIncludeBothEndsAndMayBeOpen();
    NumbersCompareAsNumbers();
    AQuotedOperandIsNotSplitOnItsOperators();
    AnEmptyFilterPassesEverything();
  });
}
