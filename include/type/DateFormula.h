#pragma once

#include "type/Date.h"

#include <compare>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

/// \file
/// \brief AL `DateFormula` -- a date calculation written down, and what it does to a date.

namespace agiru {

/// \brief AL `DateFormula`.
///
/// From `dateformula-data-type.md`: "Represents a date formula that has the same capabilities as an
/// ordinary input string for the CALCDATE Method (Date)", written as
/// `<Prefix><Unit><Sign><Number><Unit>...` -- `30D`, `2W`, `CM+10D`, `CQ+1M+20D`, `D15`.
///
/// \note THE ANGLE BRACKETS MEAN "DO NOT TRANSLATE". Without them the formula is read in the user's
///       language, where `1W+1D` is `1S+1J` in French and `1S+1D` in Spanish. Every formula the
///       BaseApp writes carries them, and this type reads the invariant form only -- a session
///       here has no language, and inventing one would be worse than refusing.
///
/// \note `Evaluate` is the only way to assign one in AL, and `Format` the only way to compare one
///       against text. The page says both outright.
class DateFormula {
public:
  /// \brief An empty formula, which moves a date nowhere.
  DateFormula() = default;

  /// \brief Reads a formula.
  /// \param text The formula, with or without its angle brackets.
  /// \return The formula; an empty one when the text is not a formula.
  [[nodiscard]] static DateFormula FromText(std::string_view text);

  /// \return True when the formula has no terms, which is what an unset field holds.
  [[nodiscard]] bool IsEmpty() const { return terms_.empty(); }

  /// \brief AL `CalcDate(Formula, Date)`.
  ///
  /// \param from The reference date.
  /// \return The date the formula reaches from it, or the undefined date when `from` is undefined.
  ///
  /// The terms apply left to right, each to the result of the last, which is what
  /// `system-calcdate-string-date-method.md` shows: `<CQ+1M-10D>` from 1996-05-21 is the end of the
  /// quarter, then a month on, then ten days back -- 1996-07-20.
  [[nodiscard]] Date CalcDate(const Date &from) const;

  /// \brief AL `Format(DateFormula)` -- the invariant text.
  /// \return The formula as it was read, in the language-independent form.
  [[nodiscard]] std::string ToText() const { return text_; }

  /// \brief Compares two formulas.
  /// \param o The other formula.
  /// \return True when they are written the same.
  [[nodiscard]] bool operator==(const DateFormula &o) const { return text_ == o.text_; }

private:
  /// What one term of a formula does.
  enum class Kind : std::uint8_t {
    Period,  ///< `CM`, `CQ`, `CY`, `CW`, `CD` -- one end of the current period.
    Weekday, ///< `WD3` -- the next or previous occurrence of an ISO weekday.
    Week,    ///< `W7` -- the Monday of ISO week 7 of the base year.
    Amount,  ///< `30D`, `2W`, `1M`, `1Q`, `1Y` -- a quantity of a unit.
  };

  struct Term {
    Kind kind = Kind::Amount;
    char unit = 'D';
    int count = 0;
    bool negative = false;
  };

  std::vector<Term> terms_;
  std::string text_;
};

} // namespace agiru
