#pragma once

#include "dotnet/Refused.h"
#include "dotnet/TimeSpan.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Text.h"
#include "type/Variant.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

namespace agiru::dotnet {

/// \brief .NET `System.Array` as the BaseApp walks one: what `Regex.Split`, `GetGroupNames` and
///        `GetGroupNumbers` answer, read by a `foreach` and by `Length` / `GetValue`.
class Array {
public:
  /// \brief `Array.Length`. \return How many elements.
  [[nodiscard]] Integer Length() const { return static_cast<Integer>(items_.size()); }

  /// \brief `Array.GetValue(index)`, zero-based. \param index The position. \return The element.
  /// \throws Error when the index is outside the array.
  [[nodiscard]] const Variant &GetValue(Integer index) const;

  /// \brief `Array.GetLength(dimension)`: the length; the arrays here have one dimension.
  /// \param dimension The dimension, 0. \return The length.
  [[nodiscard]] Integer GetLength(Integer dimension) const {
    return dimension == 0 ? Length() : Integer{0};
  }

  /// \brief `Array.SetValue(value, index)`. \param value The value. \param index Zero-based.
  /// \throws Error when the index is outside the array.
  void SetValue(const Variant &value, Integer index);

  /// \brief `Array.CreateInstance(type, length)`: an array of `length` empty values.
  /// \tparam Type The .NET `Type`, which decides nothing here: every cell holds a `Variant`.
  /// \param length How many cells. \return The array.
  template <typename Type> [[nodiscard]] static Array CreateInstance(const Type &, Integer length) {
    Array out;
    out.items_.resize(static_cast<std::size_t>(length < 0 ? 0 : length));
    return out;
  }

  /// \brief Appends an element; how the runtime fills one.
  /// \param item The element.
  void Add(const Variant &item) { items_.push_back(item); }

  /// \brief AL `foreach Item in Array`: the first element. \return The iterator.
  [[nodiscard]] std::vector<Variant>::const_iterator begin() const { return items_.begin(); }

  /// \brief AL `foreach Item in Array`: past the last element. \return The iterator.
  [[nodiscard]] std::vector<Variant>::const_iterator end() const { return items_.end(); }

private:
  std::vector<Variant> items_;
};

/// \brief .NET `RegexOptions`, held as the .NET flag word: `IgnoreCase` 1, `Multiline` 2,
///        `ExplicitCapture` 4, `Compiled` 8, `Singleline` 16, `IgnorePatternWhitespace` 32,
///        `RightToLeft` 64, `ECMAScript` 256, `CultureInvariant` 512 -- the numbers AL's own
///        `Enum RegexOptions` carries, and what `Regex Options.GetRegexOptions()` sums.
class RegexOptions {
public:
  static constexpr std::int32_t kNone = 0;                     ///< No option.
  static constexpr std::int32_t kIgnoreCase = 1;               ///< `IgnoreCase`.
  static constexpr std::int32_t kMultiline = 2;                ///< `Multiline`.
  static constexpr std::int32_t kExplicitCapture = 4;          ///< `ExplicitCapture`.
  static constexpr std::int32_t kCompiled = 8;                 ///< `Compiled`.
  static constexpr std::int32_t kSingleline = 16;              ///< `Singleline`.
  static constexpr std::int32_t kIgnorePatternWhitespace = 32; ///< `IgnorePatternWhitespace`.
  static constexpr std::int32_t kRightToLeft = 64;             ///< `RightToLeft`.
  static constexpr std::int32_t kECMAScript = 256;             ///< `ECMAScript`.
  static constexpr std::int32_t kCultureInvariant = 512;       ///< `CultureInvariant`.

  /// \brief AL `Options := SomeInteger`, the flag word as `Regex Options` sums it.
  /// \param flags The word.
  /// \return These options.
  RegexOptions &operator=(Integer flags) {
    flags_ = flags;
    return *this;
  }

  /// \brief `RegexOptions.IgnoreCase` and its siblings, read as flags to combine.
  [[nodiscard]] static RegexOptions IgnoreCase() { return RegexOptions{kIgnoreCase}; }

  [[nodiscard]] static RegexOptions Multiline() {
    return RegexOptions{kMultiline};
  } ///< \see IgnoreCase

  [[nodiscard]] static RegexOptions Singleline() {
    return RegexOptions{kSingleline};
  } ///< \see IgnoreCase

  [[nodiscard]] static RegexOptions None() { return RegexOptions{kNone}; } ///< \see IgnoreCase

  /// \brief The word. \return The flags.
  [[nodiscard]] std::int32_t Flags() const { return flags_; }

  /// \brief Nothing set.
  RegexOptions() = default;

  /// \brief The given flags. \param flags The word.
  explicit RegexOptions(std::int32_t flags) : flags_(flags) {}

private:
  std::int32_t flags_ = 0;
};

class GroupCollection;
class CaptureCollection;

/// \brief .NET `Capture`: one stretch of the input a group captured.
class Capture {
public:
  /// \brief A capture at a position. \param index Where. \param value What.
  Capture(Integer index, std::string value) : index_(index), value_(std::move(value)) {}

  /// \brief A capture nothing was assigned to yet, as a declared `DotNet Capture` variable is.
  Capture() = default;

  /// \brief `Capture.Index`. \return Where in the input, zero-based.
  [[nodiscard]] Integer Index() const { return index_; }

  /// \brief `Capture.Length`. \return How many characters.
  [[nodiscard]] Integer Length() const { return static_cast<Integer>(value_.size()); }

  /// \brief `Capture.Value`. \return The text.
  [[nodiscard]] ::agiru::Text<0> Value() const { return ::agiru::Text<0>{value_}; }

private:
  Integer index_{};
  std::string value_;
};

/// \brief .NET `Group`: a capture group of one match, with its name and its captures.
class Group {
public:
  /// \brief A group. \param index Where. \param value What. \param success Whether it matched.
  ///        \param name Its name, or its number as text.
  Group(Integer index, std::string value, bool success, std::string name)
      : index_(index), value_(std::move(value)), success_(success), name_(std::move(name)) {}

  /// \brief A group nothing was assigned to yet, as a declared `DotNet Group` variable is.
  Group() = default;

  /// \brief `Group.Index`. \return Where in the input, zero-based.
  [[nodiscard]] Integer Index() const { return index_; }

  /// \brief `Group.Length`. \return How many characters.
  [[nodiscard]] Integer Length() const { return static_cast<Integer>(value_.size()); }

  /// \brief `Group.Value`. \return The text.
  [[nodiscard]] ::agiru::Text<0> Value() const { return ::agiru::Text<0>{value_}; }

  /// \brief `Group.Success`. \return Whether the group took part in the match.
  [[nodiscard]] Boolean Success() const { return success_; }

  /// \brief `Group.Name`. \return The name, or the number as text for an unnamed group.
  [[nodiscard]] ::agiru::Text<0> Name() const { return ::agiru::Text<0>{name_}; }

  /// \brief `Group.Captures`. \return The captures; one for a group that matched, none otherwise.
  [[nodiscard]] CaptureCollection Captures() const;

private:
  Integer index_{};
  std::string value_;
  bool success_ = false;
  std::string name_;
};

/// \brief .NET `Match`: one match of a regex over an input, with its groups.
class Match {
public:
  /// \brief A match that did not happen.
  Match() = default;

  /// \brief A match. \param index Where. \param value What. \param groups Its groups, group 0
  ///        first. \param replacementNames The group names, for `Result`.
  Match(Integer index, std::string value, std::vector<Group> groups)
      : index_(index), value_(std::move(value)), success_(true), groups_(std::move(groups)) {}

  /// \brief `Match.Index`. \return Where in the input, zero-based.
  [[nodiscard]] Integer Index() const { return index_; }

  /// \brief `Match.Length`. \return How many characters.
  [[nodiscard]] Integer Length() const { return static_cast<Integer>(value_.size()); }

  /// \brief `Match.Value`. \return The text.
  [[nodiscard]] ::agiru::Text<0> Value() const { return ::agiru::Text<0>{value_}; }

  /// \brief `Match.Success`. \return Whether there was a match.
  [[nodiscard]] Boolean Success() const { return success_; }

  /// \brief `Match.Groups`. \return The groups, the whole match as group 0.
  [[nodiscard]] GroupCollection Groups() const;

  /// \brief `Match.Result(replacement)`: the replacement pattern expanded for this match --
  ///        `$1`, `${name}`, `$0`, `$$`.
  /// \param replacement The pattern.
  /// \return The text.
  [[nodiscard]] ::agiru::Text<0> Result(std::string_view replacement) const;

private:
  Integer index_ = 0;
  std::string value_;
  bool success_ = false;
  std::vector<Group> groups_;
};

/// \brief .NET `CaptureCollection`. \see Group::Captures
class CaptureCollection {
public:
  /// \brief The captures. \param captures Them.
  explicit CaptureCollection(std::vector<Capture> captures) : captures_(std::move(captures)) {}

  /// \brief Nothing captured.
  CaptureCollection() = default;

  /// \brief `Captures.Count`. \return How many.
  [[nodiscard]] Integer Count() const { return static_cast<Integer>(captures_.size()); }

  /// \brief `Captures.Item(index)`, zero-based. \param index The position. \return The capture.
  /// \throws Error when the index is outside the collection.
  [[nodiscard]] const Capture &Item(Integer index) const;

  /// \brief AL `foreach`. \return The first.
  [[nodiscard]] std::vector<Capture>::const_iterator begin() const { return captures_.begin(); }

  /// \brief AL `foreach`. \return Past the last.
  [[nodiscard]] std::vector<Capture>::const_iterator end() const { return captures_.end(); }

private:
  std::vector<Capture> captures_;
};

/// \brief .NET `GroupCollection`. \see Match::Groups
class GroupCollection {
public:
  /// \brief The groups. \param groups Them.
  explicit GroupCollection(std::vector<Group> groups) : groups_(std::move(groups)) {}

  /// \brief No groups.
  GroupCollection() = default;

  /// \brief `Groups.Count`. \return How many.
  [[nodiscard]] Integer Count() const { return static_cast<Integer>(groups_.size()); }

  /// \brief `Groups.Item(index)`, zero-based. \param index The position. \return The group.
  /// \throws Error when the index is outside the collection.
  [[nodiscard]] const Group &Item(Integer index) const;

  /// \brief `Groups.Item(name)`. \param name The group's name. \return The group.
  /// \throws Error when no group has the name.
  [[nodiscard]] const Group &Item(std::string_view name) const;

  /// \brief AL `foreach`. \return The first.
  [[nodiscard]] std::vector<Group>::const_iterator begin() const { return groups_.begin(); }

  /// \brief AL `foreach`. \return Past the last.
  [[nodiscard]] std::vector<Group>::const_iterator end() const { return groups_.end(); }

private:
  std::vector<Group> groups_;
};

/// \brief .NET `MatchCollection`. \see Regex::Matches
class MatchCollection {
public:
  /// \brief The matches. \param matches Them.
  explicit MatchCollection(std::vector<Match> matches) : matches_(std::move(matches)) {}

  /// \brief No matches.
  MatchCollection() = default;

  /// \brief `Matches.Count`. \return How many.
  [[nodiscard]] Integer Count() const { return static_cast<Integer>(matches_.size()); }

  /// \brief `Matches.Item(index)`, zero-based. \param index The position. \return The match.
  /// \throws Error when the index is outside the collection.
  [[nodiscard]] const Match &Item(Integer index) const;

  /// \brief AL `foreach`. \return The first.
  [[nodiscard]] std::vector<Match>::const_iterator begin() const { return matches_.begin(); }

  /// \brief AL `foreach`. \return Past the last.
  [[nodiscard]] std::vector<Match>::const_iterator end() const { return matches_.end(); }

private:
  std::vector<Match> matches_;
};

/// \brief .NET `System.Text.RegularExpressions.Regex`, rebuilt over `std::regex` in its ECMAScript
///        grammar, which is what .NET's grammar is for the patterns the BaseApp writes.
///
/// \note THE MEMBERS ARE THE ONES `Regex Impl.` NAMES (measured 2026-09-10): the constructor with a
///       pattern, options and a timeout; `IsMatch`, `Matches`, `Replace`, `Split` with their start
///       positions and counts; `Escape` / `Unescape`; the group names and numbers; `CacheSize`.
///       Named groups `(?<name>...)` are rewritten to plain groups before `std::regex` sees them
///       and the names are kept beside the numbers, because ECMAScript in `std::regex` has none.
///       `RightToLeft`, `ECMAScript` and `CultureInvariant` change nothing here; `Compiled` and
///       `IgnorePatternWhitespace` are refused, since a pattern written for them would read
///       differently. The timeout is carried and not enforced.
class Regex {
public:
  /// \brief The binder behind `R := R.Regex(pattern[, options[, timeout]])`.
  struct Binder {
    /// \brief `new Regex(pattern)`. \param pattern The pattern. \return The regex.
    [[nodiscard]] class Regex operator()(std::string_view pattern) const;

    /// \brief `new Regex(pattern, options)`. \param pattern The pattern. \param options The
    ///        options. \return The regex.
    [[nodiscard]] class Regex operator()(std::string_view pattern,
                                         const RegexOptions &options) const;

    /// \brief `new Regex(pattern, options, timeout)`. \param pattern The pattern. \param options
    ///        The options. \param timeout The match timeout, carried. \return The regex.
    [[nodiscard]] class Regex operator()(std::string_view pattern,
                                         const RegexOptions &options,
                                         const TimeSpan &timeout) const;
  };

  /// \brief `R.Regex(...)`, the constructor as AL calls it.
  Binder Regex;

  /// \brief `Regex.IsMatch(input)`. \param input The text. \return Whether the pattern matches
  ///        somewhere in it.
  /// \brief Whether the variable was never given a pattern, which `IsNull(Regex)` answers.
  /// \return `true` before `Regex(pattern)` ran.
  [[nodiscard]] Boolean IsNull() const { return compiled_ == nullptr; }

  [[nodiscard]] Boolean IsMatch(std::string_view input) const;

  /// \brief `Regex.IsMatch(input)` over a member this build refuses. \param input The refusal.
  /// \return Never. \throws Error always, naming the refused member.
  [[nodiscard]] Boolean IsMatch(const Refused &input) const { return static_cast<Boolean>(input); }

  /// \brief `Regex.IsMatch(input, startAt)`. \param input The text. \param startAt Where to
  ///        begin, zero-based. \return Whether the pattern matches from there on.
  [[nodiscard]] Boolean IsMatch(std::string_view input, Integer startAt) const;

  /// \brief `Regex.Match(input)`. \param input The text. \return The first match, or an
  ///        unsuccessful one.
  [[nodiscard]] Match Match(std::string_view input) const;

  /// \brief `Regex.Matches(input)`. \param input The text. \return Every match, left to right.
  [[nodiscard]] MatchCollection Matches(std::string_view input) const;

  /// \brief `Regex.Matches(input, startAt)`. \param input The text. \param startAt Where to
  ///        begin. \return Every match from there on.
  [[nodiscard]] MatchCollection Matches(std::string_view input, Integer startAt) const;

  /// \brief `Regex.Replace(input, replacement)`. \param input The text. \param replacement The
  ///        replacement pattern (`$1`, `${name}`). \return The text with every match replaced.
  [[nodiscard]] ::agiru::Text<0> Replace(std::string_view input,
                                         std::string_view replacement) const;

  /// \brief `Regex.Replace(input, replacement, count)`. \param input The text. \param replacement
  ///        The pattern. \param count At most this many; -1 for all. \return The text.
  [[nodiscard]] ::agiru::Text<0>
  Replace(std::string_view input, std::string_view replacement, Integer count) const;

  /// \brief `Regex.Replace(input, replacement, count, startAt)`. \param input The text.
  ///        \param replacement The pattern. \param count At most this many. \param startAt
  ///        Where to begin. \return The text.
  [[nodiscard]] ::agiru::Text<0> Replace(std::string_view input,
                                         std::string_view replacement,
                                         Integer count,
                                         Integer startAt) const;

  /// \brief `Regex.Split(input)`. \param input The text. \return The pieces between matches.
  [[nodiscard]] Array Split(std::string_view input) const;

  /// \brief `Regex.Split(input, count)`. \param input The text. \param count At most this many
  ///        pieces; 0 for all. \return The pieces.
  [[nodiscard]] Array Split(std::string_view input, Integer count) const;

  /// \brief `Regex.Split(input, count, startAt)`. \param input The text. \param count At most this
  ///        many pieces. \param startAt Where to begin. \return The pieces.
  [[nodiscard]] Array Split(std::string_view input, Integer count, Integer startAt) const;

  /// \brief The static `Regex.Split(input, pattern)`, which AL calls on any `DotNet Regex`
  ///        variable. \param input The text. \param pattern The pattern. \return The pieces.
  [[nodiscard]] static Array Split(std::string_view input, std::string_view pattern);

  /// \brief The static `Regex.IsMatch(input, pattern)`. \param input The text.
  /// \param pattern The pattern. \return Whether it matches anywhere.
  [[nodiscard]] static Boolean IsMatch(std::string_view input, std::string_view pattern);

  /// \brief The static `Regex.Match(input, pattern)`. \param input The text.
  /// \param pattern The pattern. \return The first match.
  [[nodiscard]] static class Match Match(std::string_view input, std::string_view pattern);

  /// \brief The static `Regex.Replace(input, pattern, replacement)`. \param input The text.
  /// \param pattern The pattern. \param replacement What replaces each match. \return The text.
  [[nodiscard]] static ::agiru::Text<0>
  Replace(std::string_view input, std::string_view pattern, std::string_view replacement);

  /// \brief The static `Regex.Split(input, pattern, options)`. \param input The text.
  ///        \param pattern The pattern. \param options The options. \return The pieces.
  [[nodiscard]] static Array
  Split(std::string_view input, std::string_view pattern, const RegexOptions &options);

  /// \brief `Regex.GetGroupNames()`. \return The group names, `0` first, then the numbered and the
  ///        named groups as .NET orders them.
  [[nodiscard]] Array GetGroupNames() const;

  /// \brief `Regex.GetGroupNumbers()`. \return The group numbers.
  [[nodiscard]] Array GetGroupNumbers() const;

  /// \brief `Regex.GroupNameFromNumber(number)`. \param number The group's number. \return Its
  ///        name, or the number as text, or empty when there is no such group.
  [[nodiscard]] ::agiru::Text<0> GroupNameFromNumber(Integer number) const;

  /// \brief `Regex.GroupNumberFromName(name)`. \param name The group's name. \return Its number,
  ///        or -1 when there is no such group.
  [[nodiscard]] Integer GroupNumberFromName(std::string_view name) const;

  /// \brief `Regex.GetHashCode()`. \return A hash of the pattern.
  [[nodiscard]] Integer GetHashCode() const;

  /// \brief `Regex.Escape(text)`. \param text The text. \return The text with every
  ///        metacharacter escaped.
  [[nodiscard]] static ::agiru::Text<0> Escape(std::string_view text);

  /// \brief `Regex.Unescape(text)`. \param text The text. \return The text with the escapes undone.
  [[nodiscard]] static ::agiru::Text<0> Unescape(std::string_view text);

  /// \brief `Regex.CacheSize`, read. \return The size, a number this runtime carries and never
  /// uses.
  [[nodiscard]] static Integer CacheSize();

  /// \brief `Regex.CacheSize := n`, set. \param size The size.
  static void CacheSize(Integer size);

private:
  friend struct Binder;
  std::shared_ptr<const std::regex> compiled_;
  std::string pattern_;
  std::vector<std::string> groupNames_;
  RegexOptions options_;
  TimeSpan timeout_;
};

}
