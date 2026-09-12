#include "dotnet/Regex.h"

#include "dotnet/TimeSpan.h"
#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Decimal.h"
#include "type/Integer.h"
#include "type/Text.h"
#include "type/Variant.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::dotnet {

namespace {

constexpr std::int64_t kTicksPerMillisecond = 10000;
constexpr std::int64_t kMillisecondsPerSecond = 1000;

Integer &CacheSizeHeld() {
  static Integer size = 15;
  return size;
}

struct Translated {
  std::string pattern;
  std::vector<std::string> names;
};

Translated WithoutNamedGroups(std::string_view pattern) {
  Translated out;
  out.names.emplace_back("0");
  for (std::size_t i = 0; i < pattern.size(); ++i) {
    const char c = pattern[i];
    if (c == '\\' && i + 1 < pattern.size()) {
      out.pattern += c;
      out.pattern += pattern[i + 1];
      ++i;
      continue;
    }
    if (c == '(' && i + 1 < pattern.size() && pattern[i + 1] == '?') {
      if (i + 2 < pattern.size() && (pattern[i + 2] == '<' || pattern[i + 2] == '\'') &&
          i + 3 < pattern.size() && pattern[i + 3] != '=' && pattern[i + 3] != '!') {
        const char close = pattern[i + 2] == '<' ? '>' : '\'';
        const std::size_t end = pattern.find(close, i + 3);
        if (end != std::string_view::npos) {
          out.names.emplace_back(pattern.substr(i + 3, end - i - 3));
          out.pattern += '(';
          i = end;
          continue;
        }
      }
      out.pattern += c;
      continue;
    }
    if (c == '(') {
      out.names.emplace_back(std::to_string(out.names.size()));
    }
    out.pattern += c;
  }
  return out;
}

std::regex::flag_type FlagsOf(const RegexOptions &options) {
  if ((options.Flags() & RegexOptions::kIgnorePatternWhitespace) != 0) {
    throw Error("Regex: the IgnorePatternWhitespace option is not one this runtime reads a pattern with");
  }
  std::regex::flag_type flags = std::regex::ECMAScript;
  if ((options.Flags() & RegexOptions::kIgnoreCase) != 0) { flags |= std::regex::icase; }
  if ((options.Flags() & RegexOptions::kMultiline) != 0) { flags |= std::regex::multiline; }
  return flags;
}

Match MatchOf(const std::smatch &found, std::size_t offset, const std::vector<std::string> &names) {
  std::vector<Group> groups;
  for (std::size_t g = 0; g < found.size(); ++g) {
    const std::string name = g < names.size() ? names[g] : std::to_string(g);
    groups.emplace_back(static_cast<Integer>(found.position(g) + static_cast<std::ptrdiff_t>(offset)),
                        found[g].str(),
                        found[g].matched,
                        name);
  }
  return Match(static_cast<Integer>(found.position(0) + static_cast<std::ptrdiff_t>(offset)),
               found[0].str(),
               std::move(groups));
}

}

namespace {
constexpr std::int64_t kSecondsPerMinute = 60;
constexpr std::int64_t kMinutesPerHour = 60;
constexpr std::int64_t kHoursPerDay = 24;

std::int64_t TicksOf(std::int64_t days,
                     std::int64_t hours,
                     std::int64_t minutes,
                     std::int64_t seconds,
                     std::int64_t milliseconds) {
  const std::int64_t totalSeconds =
      ((days * kHoursPerDay + hours) * kMinutesPerHour + minutes) * kSecondsPerMinute + seconds;
  return (totalSeconds * kMillisecondsPerSecond + milliseconds) * kTicksPerMillisecond;
}
}

class TimeSpan TimeSpan::Binder::operator()(::agiru::Integer hours,
                                            ::agiru::Integer minutes,
                                            ::agiru::Integer seconds) const {
  return (*this)(TicksOf(0, hours, minutes, seconds, 0));
}

class TimeSpan TimeSpan::Binder::operator()(::agiru::Integer days,
                                            ::agiru::Integer hours,
                                            ::agiru::Integer minutes,
                                            ::agiru::Integer seconds) const {
  return (*this)(TicksOf(days, hours, minutes, seconds, 0));
}

class TimeSpan TimeSpan::Binder::operator()(::agiru::Integer days,
                                            ::agiru::Integer hours,
                                            ::agiru::Integer minutes,
                                            ::agiru::Integer seconds,
                                            ::agiru::Integer milliseconds) const {
  return (*this)(TicksOf(days, hours, minutes, seconds, milliseconds));
}

class TimeSpan TimeSpan::Binder::operator()(std::int64_t ticks) const {
  return TimeSpan::FromTicks(ticks);
}

class TimeSpan TimeSpan::FromTicks(std::int64_t ticks) {
  class TimeSpan span;
  span.ticks_ = ticks;
  return span;
}

class TimeSpan TimeSpan::FromSeconds(const Decimal &seconds) {
  return FromMilliseconds(seconds * Decimal{kMillisecondsPerSecond});
}

class TimeSpan TimeSpan::FromMilliseconds(const Decimal &milliseconds) {
  const Decimal whole = Round(milliseconds, Decimal{1}, RoundDirection::Nearest);
  return FromTicks(std::stoll(whole.ToInvariantString()) * kTicksPerMillisecond);
}

Decimal TimeSpan::TotalMilliseconds() const {
  return Decimal{ticks_} / Decimal{kTicksPerMillisecond};
}

Decimal TimeSpan::TotalSeconds() const {
  return Decimal{ticks_} / Decimal{kTicksPerMillisecond * kMillisecondsPerSecond};
}

const Variant &Array::GetValue(Integer index) const {
  if (index < 0 || static_cast<std::size_t>(index) >= items_.size()) {
    throw Error("the index " + std::to_string(index) + " is outside an array of " +
                std::to_string(items_.size()) + ", which .NET counts from zero");
  }
  return items_[static_cast<std::size_t>(index)];
}

CaptureCollection Group::Captures() const {
  if (!success_) { return CaptureCollection{}; }
  return CaptureCollection{std::vector<Capture>{Capture(index_, value_)}};
}

GroupCollection Match::Groups() const {
  return GroupCollection{groups_};
}

::agiru::Text<0> Match::Result(std::string_view replacement) const {
  std::string out;
  for (std::size_t i = 0; i < replacement.size(); ++i) {
    if (replacement[i] != '$' || i + 1 >= replacement.size()) {
      out += replacement[i];
      continue;
    }
    const char next = replacement[i + 1];
    if (next == '$') {
      out += '$';
      ++i;
      continue;
    }
    if (next == '{') {
      const std::size_t end = replacement.find('}', i + 2);
      if (end != std::string_view::npos) {
        const std::string_view name = replacement.substr(i + 2, end - i - 2);
        for (const Group &group : groups_) {
          if (group.Name().Value() == name) { out += std::string(group.Value().Value()); }
        }
        i = end;
        continue;
      }
    }
    if (next >= '0' && next <= '9') {
      std::size_t end = i + 1;
      while (end < replacement.size() && replacement[end] >= '0' && replacement[end] <= '9') { ++end; }
      const std::size_t number = std::stoul(std::string(replacement.substr(i + 1, end - i - 1)));
      if (number < groups_.size()) { out += std::string(groups_[number].Value().Value()); }
      i = end - 1;
      continue;
    }
    out += replacement[i];
  }
  return ::agiru::Text<0>{out};
}

const Capture &CaptureCollection::Item(Integer index) const {
  if (index < 0 || static_cast<std::size_t>(index) >= captures_.size()) {
    throw Error("the index " + std::to_string(index) + " is outside " +
                std::to_string(captures_.size()) + " capture(s), which .NET counts from zero");
  }
  return captures_[static_cast<std::size_t>(index)];
}

const Group &GroupCollection::Item(Integer index) const {
  if (index < 0 || static_cast<std::size_t>(index) >= groups_.size()) {
    throw Error("the index " + std::to_string(index) + " is outside " +
                std::to_string(groups_.size()) + " group(s), which .NET counts from zero");
  }
  return groups_[static_cast<std::size_t>(index)];
}

const Group &GroupCollection::Item(std::string_view name) const {
  for (const Group &group : groups_) {
    if (group.Name().Value() == name) { return group; }
  }
  throw Error("no group is named '" + std::string(name) + "'");
}

const Match &MatchCollection::Item(Integer index) const {
  if (index < 0 || static_cast<std::size_t>(index) >= matches_.size()) {
    throw Error("the index " + std::to_string(index) + " is outside " +
                std::to_string(matches_.size()) + " match(es), which .NET counts from zero");
  }
  return matches_[static_cast<std::size_t>(index)];
}

class Regex Regex::Binder::operator()(std::string_view pattern) const {
  return (*this)(pattern, RegexOptions{}, TimeSpan{});
}

class Regex Regex::Binder::operator()(std::string_view pattern, const RegexOptions &options) const {
  return (*this)(pattern, options, TimeSpan{});
}

class Regex Regex::Binder::operator()(std::string_view pattern,
                                      const RegexOptions &options,
                                      const TimeSpan &timeout) const {
  class Regex out;
  Translated translated = WithoutNamedGroups(pattern);
  try {
    out.compiled_ = std::make_shared<const std::regex>(translated.pattern, FlagsOf(options));
  } catch (const std::regex_error &e) {
    throw Error("Regex: the pattern '" + std::string(pattern) + "' is not one this runtime reads: " +
                e.what());
  }
  out.pattern_ = std::string(pattern);
  out.groupNames_ = std::move(translated.names);
  out.options_ = options;
  out.timeout_ = timeout;
  return out;
}

Boolean Regex::IsMatch(std::string_view input) const {
  return IsMatch(input, 0);
}

Boolean Regex::IsMatch(std::string_view input, Integer startAt) const {
  if (compiled_ == nullptr) { throw Error("Regex: no pattern was given yet"); }
  const std::size_t from = startAt < 0 ? 0 : std::min(static_cast<std::size_t>(startAt), input.size());
  const std::string rest(input.substr(from));
  return std::regex_search(rest, *compiled_);
}

dotnet::Match Regex::Match(std::string_view input) const {
  const MatchCollection all = Matches(input, 0);
  return all.Count() == 0 ? dotnet::Match{} : all.Item(0);
}

MatchCollection Regex::Matches(std::string_view input) const {
  return Matches(input, 0);
}

MatchCollection Regex::Matches(std::string_view input, Integer startAt) const {
  if (compiled_ == nullptr) { throw Error("Regex: no pattern was given yet"); }
  const std::size_t from = startAt < 0 ? 0 : std::min(static_cast<std::size_t>(startAt), input.size());
  const std::string rest(input.substr(from));
  std::vector<dotnet::Match> found;
  for (auto it = std::sregex_iterator(rest.begin(), rest.end(), *compiled_);
       it != std::sregex_iterator();
       ++it) {
    found.push_back(MatchOf(*it, from, groupNames_));
  }
  return MatchCollection{std::move(found)};
}

::agiru::Text<0> Regex::Replace(std::string_view input, std::string_view replacement) const {
  return Replace(input, replacement, -1, 0);
}

::agiru::Text<0>
Regex::Replace(std::string_view input, std::string_view replacement, Integer count) const {
  return Replace(input, replacement, count, 0);
}

::agiru::Text<0> Regex::Replace(std::string_view input,
                                std::string_view replacement,
                                Integer count,
                                Integer startAt) const {
  const MatchCollection all = Matches(input, startAt);
  std::string out;
  std::size_t at = 0;
  Integer done = 0;
  for (const dotnet::Match &match : all) {
    if (count >= 0 && done >= count) { break; }
    const auto index = static_cast<std::size_t>(match.Index());
    out += input.substr(at, index - at);
    out += std::string(match.Result(replacement).Value());
    at = index + static_cast<std::size_t>(match.Length());
    ++done;
  }
  out += input.substr(at);
  return ::agiru::Text<0>{out};
}

void Array::SetValue(const Variant &value, Integer index) {
  if (index < 0 || static_cast<std::size_t>(index) >= items_.size()) {
    throw Error("Array.SetValue: the index " + std::to_string(index) + " is outside an array of " +
                std::to_string(items_.size()));
  }
  items_[static_cast<std::size_t>(index)] = value;
}

Array Regex::Split(std::string_view input) const {
  return Split(input, 0, 0);
}

Boolean Regex::IsMatch(std::string_view input, std::string_view pattern) {
  return Regex::Binder{}(pattern).IsMatch(input);
}

class Match Regex::Match(std::string_view input, std::string_view pattern) {
  return Regex::Binder{}(pattern).Match(input);
}

::agiru::Text<0>
Regex::Replace(std::string_view input, std::string_view pattern, std::string_view replacement) {
  return Regex::Binder{}(pattern).Replace(input, replacement);
}

Array Regex::Split(std::string_view input, std::string_view pattern) {
  return Regex::Binder{}(pattern).Split(input);
}

MatchCollection Regex::Matches(std::string_view input, std::string_view pattern) {
  return Regex::Binder{}(pattern).Matches(input);
}

Array Regex::Split(std::string_view input, std::string_view pattern, const RegexOptions &options) {
  return Regex::Binder{}(pattern, options).Split(input);
}

Array Regex::Split(std::string_view input, Integer count) const {
  return Split(input, count, 0);
}

Array Regex::Split(std::string_view input, Integer count, Integer startAt) const {
  const MatchCollection all = Matches(input, startAt);
  Array out;
  std::size_t at = 0;
  for (const dotnet::Match &match : all) {
    if (count > 0 && out.Length() >= count - 1) { break; }
    const auto index = static_cast<std::size_t>(match.Index());
    out.Add(Variant(::agiru::Text<0>{std::string(input.substr(at, index - at))}));
    at = index + static_cast<std::size_t>(match.Length());
  }
  out.Add(Variant(::agiru::Text<0>{std::string(input.substr(at))}));
  return out;
}

Array Regex::GetGroupNames() const {
  Array out;
  for (const std::string &name : groupNames_) { out.Add(Variant(::agiru::Text<0>{name})); }
  return out;
}

Array Regex::GetGroupNumbers() const {
  Array out;
  for (std::size_t i = 0; i < groupNames_.size(); ++i) { out.Add(Variant(static_cast<Integer>(i))); }
  return out;
}

::agiru::Text<0> Regex::GroupNameFromNumber(Integer number) const {
  if (number < 0 || static_cast<std::size_t>(number) >= groupNames_.size()) { return ::agiru::Text<0>{}; }
  return ::agiru::Text<0>{groupNames_[static_cast<std::size_t>(number)]};
}

Integer Regex::GroupNumberFromName(std::string_view name) const {
  for (std::size_t i = 0; i < groupNames_.size(); ++i) {
    if (groupNames_[i] == name) { return static_cast<Integer>(i); }
  }
  return -1;
}

Integer Regex::GetHashCode() const {
  return static_cast<Integer>(std::hash<std::string>{}(pattern_) & 0x7fffffff);
}

::agiru::Text<0> Regex::Escape(std::string_view text) {
  std::string out;
  for (const char c : text) {
    if (std::string_view("\\*+?|{}[]()^$.# ").find(c) != std::string_view::npos) { out += '\\'; }
    out += c;
  }
  return ::agiru::Text<0>{out};
}

::agiru::Text<0> Regex::Unescape(std::string_view text) {
  std::string out;
  for (std::size_t i = 0; i < text.size(); ++i) {
    if (text[i] == '\\' && i + 1 < text.size()) {
      ++i;
    }
    out += text[i];
  }
  return ::agiru::Text<0>{out};
}

Integer Regex::CacheSize() {
  return CacheSizeHeld();
}

void Regex::CacheSize(Integer size) {
  CacheSizeHeld() = size;
}

}
