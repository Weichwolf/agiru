#include "Filter.h"

#include "meta/TableDef.h"
#include "runtime/Error.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace agiru::detail {

namespace {

std::string_view Trim(std::string_view text) {
  while (!text.empty() && (std::isspace(static_cast<unsigned char>(text.front())) != 0)) {
    text.remove_prefix(1);
  }
  while (!text.empty() && (std::isspace(static_cast<unsigned char>(text.back())) != 0)) {
    text.remove_suffix(1);
  }
  return text;
}

bool HasWildcard(std::string_view text) {
  return text.find('*') != std::string_view::npos || text.find('?') != std::string_view::npos;
}

/// Splits on a separator that is not inside a quoted operand.
///
/// AL QUOTES AN OPERAND THAT CONTAINS AN OPERATOR: `'A&B'` is one value and not a conjunction. A
/// split that ignored the quotes would turn a company name into a filter, and a value arriving
/// through `%1` is data this runtime did not write.
std::vector<std::string_view> SplitOutsideQuotes(std::string_view text, char separator) {
  std::vector<std::string_view> parts;
  bool quoted = false;
  std::size_t start = 0;
  for (std::size_t i = 0; i < text.size(); ++i) {
    if (text[i] == '\'') { quoted = !quoted; }
    if (!quoted && text[i] == separator) {
      parts.push_back(text.substr(start, i - start));
      start = i + 1;
    }
  }
  parts.push_back(text.substr(start));
  return parts;
}

/// Removes the quotes AL puts around an operand, keeping the spaces inside them.
std::string Unquote(std::string_view text) {
  text = Trim(text);
  if (text.size() >= 2 && text.front() == '\'' && text.back() == '\'') {
    text.remove_prefix(1);
    text.remove_suffix(1);
    return std::string(text);
  }
  return std::string(text);
}

/// Reads the operator at the front of an atom, leaving the operand.
///
/// WHITESPACE IS INSIGNIFICANT and `=` IS OPTIONAL, and both cost the predecessor real defects.
/// `'< %1'`, `'> 5'` and `'<> 0'` are ordinary AL spellings, and matching against the raw string
/// missed every spaced form, so the whole expression fell into the equality branch and compared a
/// value against the literal text `"< 2028-01-28"`. And `SetFilter(F, '=1')` is the same filter as
/// `SetFilter(F, '1')` -- the `=` is optional syntax at 121 call sites in the corpus. Unrecognised,
/// the atom compared against the literal `"=1"` and matched NOTHING, silently, because an empty
/// result set is a legal outcome.
Compare TakeOperator(std::string_view &text) {
  text = Trim(text);
  for (const auto &[spelling, compare] : {std::pair{std::string_view("<>"), Compare::NotEqual},
                                          std::pair{std::string_view(">="), Compare::GreaterEqual},
                                          std::pair{std::string_view("<="), Compare::LessOrEqual},
                                          std::pair{std::string_view("<"), Compare::Less},
                                          std::pair{std::string_view(">"), Compare::Greater},
                                          std::pair{std::string_view("="), Compare::Equal}}) {
    if (text.starts_with(spelling)) {
      text.remove_prefix(spelling.size());
      text = Trim(text);
      return compare;
    }
  }
  return Compare::Equal;
}

Atom ReadAtom(std::string_view text) {
  text = Trim(text);

  // `@` IS A MODIFIER AND NOT A CHARACTER. It asks for a case-insensitive comparison, and every
  // comparison here is already case-insensitive the way AL compares text, so it is consumed and
  // the rest read as usual.
  if (text.starts_with("@")) { text.remove_prefix(1); }

  const Compare given = TakeOperator(text);

  // A RANGE IS NOT AN OPERATOR ON AN OPERAND, it is two operands. It is looked for only when no
  // operator was given, because `<..9` is not AL.
  if (given == Compare::Equal) {
    const std::size_t dots = text.find("..");
    if (dots != std::string_view::npos) {
      Atom range;
      range.compare = Compare::Between;
      range.value = Unquote(text.substr(0, dots));
      range.upper = Unquote(text.substr(dots + 2));
      range.openLower = range.value.empty();
      range.openUpper = range.upper.empty();
      return range;
    }
  }

  Atom atom;
  atom.value = Unquote(text);
  const bool wild = HasWildcard(atom.value) && Trim(text).front() != '\'';
  if (given == Compare::Equal) {
    atom.compare = wild ? Compare::Like : Compare::Equal;
  } else if (given == Compare::NotEqual) {
    atom.compare = wild ? Compare::NotLike : Compare::NotEqual;
  } else {
    atom.compare = given;
  }
  return atom;
}

char Lower(char c) {
  return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

/// AL compares text without regard to case, and so does the wildcard match.
bool SameText(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) { return false; }
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (Lower(a[i]) != Lower(b[i])) { return false; }
  }
  return true;
}

/// `*` stands for any run of characters and `?` for exactly one. NOT a regular expression, and not
/// translated into one: a value out of the database is full of characters a regex would read.
bool WildcardMatch(std::string_view pattern, std::string_view value) {
  std::size_t p = 0;
  std::size_t v = 0;
  std::size_t star = std::string_view::npos;
  std::size_t mark = 0;
  while (v < value.size()) {
    if (p < pattern.size() && (pattern[p] == '?' || Lower(pattern[p]) == Lower(value[v]))) {
      ++p;
      ++v;
    } else if (p < pattern.size() && pattern[p] == '*') {
      star = p++;
      mark = v;
    } else if (star != std::string_view::npos) {
      p = star + 1;
      v = ++mark;
    } else {
      return false;
    }
  }
  while (p < pattern.size() && pattern[p] == '*') { ++p; }
  return p == pattern.size();
}

/// Orders two rendered values the way the FIELD orders them.
///
/// A NUMBER IS NOT COMPARED AS TEXT: `"10" < "9"` lexically, and a filter `>=9` over an entry
/// number would then drop every row from ten upward.
std::strong_ordering Order(std::string_view value, std::string_view against, const FieldDef &def) {
  switch (def.type) {
    case FieldType::Integer:
    case FieldType::BigInteger:
    case FieldType::Option:
    case FieldType::Enum:
    case FieldType::Decimal: {
      try {
        const double left = std::stod(std::string(value));
        const double right = std::stod(std::string(against));
        return left < right   ? std::strong_ordering::less
               : right < left ? std::strong_ordering::greater
                              : std::strong_ordering::equal;
      } catch (const std::exception &) { break; }
    }
    default: break;
  }
  const int order = std::string(value).compare(std::string(against));
  return order < 0   ? std::strong_ordering::less
         : order > 0 ? std::strong_ordering::greater
                     : std::strong_ordering::equal;
}

bool Satisfies(const Atom &atom, std::string_view value, const FieldDef &def) {
  switch (atom.compare) {
    case Compare::Equal: return SameText(value, atom.value);
    case Compare::NotEqual: return !SameText(value, atom.value);
    case Compare::Like: return WildcardMatch(atom.value, value);
    case Compare::NotLike: return !WildcardMatch(atom.value, value);
    case Compare::Less: return Order(value, atom.value, def) == std::strong_ordering::less;
    case Compare::LessOrEqual:
      return Order(value, atom.value, def) != std::strong_ordering::greater;
    case Compare::Greater: return Order(value, atom.value, def) == std::strong_ordering::greater;
    case Compare::GreaterEqual: return Order(value, atom.value, def) != std::strong_ordering::less;
    case Compare::Between:
      return (atom.openLower || Order(value, atom.value, def) != std::strong_ordering::less) &&
             (atom.openUpper || Order(value, atom.upper, def) != std::strong_ordering::greater);
  }
  return false;
}

} // namespace

Expression ParseFilter(std::string_view text) {
  Expression expression;
  if (Trim(text).empty()) { return expression; }
  for (const std::string_view alternative : SplitOutsideQuotes(text, '|')) {
    All conjunction;
    for (const std::string_view part : SplitOutsideQuotes(alternative, '&')) {
      if (Trim(part).empty()) { throw Error("a filter has an empty term: " + std::string(text)); }
      conjunction.push_back(ReadAtom(part));
    }
    expression.push_back(std::move(conjunction));
  }
  return expression;
}

bool Matches(const Expression &expression, std::string_view value, const FieldDef &def) {
  if (expression.empty()) { return true; }
  for (const All &conjunction : expression) {
    bool all = true;
    for (const Atom &atom : conjunction) { all = all && Satisfies(atom, value, def); }
    if (all) { return true; }
  }
  return false;
}

namespace {

std::optional<std::int64_t> AsInteger(std::string_view text) {
  std::int64_t value = 0;
  const char *first = text.data();
  const char *last = first + text.size();
  if (!text.empty() && text.front() == '+') { ++first; }
  const std::from_chars_result read = std::from_chars(first, last, value);
  if (read.ec != std::errc{} || read.ptr != last) { return std::nullopt; }
  return value;
}

Intervals Normalised(Intervals set) {
  std::erase_if(set, [](const Interval &i) { return i.low > i.high; });
  std::ranges::sort(set, [](const Interval &a, const Interval &b) { return a.low < b.low; });
  Intervals merged;
  for (const Interval &next : set) {
    // ADJACENT INTERVALS ARE MERGED, NOT ONLY OVERLAPPING ONES. `1..5|6..9` is one run of integers,
    // and leaving it as two would emit two series where one describes the same rows.
    if (!merged.empty() && next.low <= merged.back().high + 1) {
      merged.back().high = std::max(merged.back().high, next.high);
      continue;
    }
    merged.push_back(next);
  }
  return merged;
}

Intervals Intersected(const Intervals &left, const Intervals &right) {
  Intervals both;
  std::size_t l = 0;
  std::size_t r = 0;
  while (l < left.size() && r < right.size()) {
    const std::int64_t low = std::max(left[l].low, right[r].low);
    const std::int64_t high = std::min(left[l].high, right[r].high);
    if (low <= high) { both.push_back(Interval{.low = low, .high = high}); }
    (left[l].high < right[r].high ? l : r)++;
  }
  return both;
}

std::optional<Intervals> Admitted(const Atom &atom, Interval domain) {
  const std::optional<std::int64_t> bound = AsInteger(atom.value);
  switch (atom.compare) {
    // A WILDCARD DESCRIBES NO INTERVAL. AL accepts `*1*` on an integer field, and a caller that
    // cannot scan has to refuse rather than guess at what it would have matched.
    case Compare::Like:
    case Compare::NotLike: return std::nullopt;
    case Compare::Equal:
      if (!bound) { return std::nullopt; }
      return Intervals{{.low = *bound, .high = *bound}};
    // `<>5` PUNCHES A HOLE and leaves two intervals, which is why a conjunction holds a SET.
    case Compare::NotEqual:
      if (!bound) { return std::nullopt; }
      return Normalised(
          {{.low = domain.low, .high = *bound - 1}, {.low = *bound + 1, .high = domain.high}});
    case Compare::Less:
      if (!bound) { return std::nullopt; }
      return Normalised({{.low = domain.low, .high = *bound - 1}});
    case Compare::LessOrEqual:
      if (!bound) { return std::nullopt; }
      return Normalised({{.low = domain.low, .high = *bound}});
    case Compare::Greater:
      if (!bound) { return std::nullopt; }
      return Normalised({{.low = *bound + 1, .high = domain.high}});
    case Compare::GreaterEqual:
      if (!bound) { return std::nullopt; }
      return Normalised({{.low = *bound, .high = domain.high}});
    case Compare::Between: break;
  }
  const std::optional<std::int64_t> upper = AsInteger(atom.upper);
  if ((!atom.openLower && !bound) || (!atom.openUpper && !upper)) { return std::nullopt; }
  return Normalised({{.low = atom.openLower ? domain.low : *bound,
                      .high = atom.openUpper ? domain.high : *upper}});
}

} // namespace

std::optional<Intervals> IntegerIntervals(const Expression &expression, Interval domain) {
  Intervals whole{{.low = domain.low, .high = domain.high}};
  if (expression.empty()) { return whole; }

  Intervals admitted;
  for (const All &conjunction : expression) {
    Intervals narrowed = whole;
    for (const Atom &atom : conjunction) {
      const std::optional<Intervals> one = Admitted(atom, domain);
      if (!one) { return std::nullopt; }
      narrowed = Intersected(narrowed, *one);
    }
    admitted.insert(admitted.end(), narrowed.begin(), narrowed.end());
  }
  return Normalised(std::move(admitted));
}

std::int64_t CountOf(const Intervals &intervals) {
  std::int64_t count = 0;
  for (const Interval &one : intervals) { count += one.high - one.low + 1; }
  return count;
}

} // namespace agiru::detail
