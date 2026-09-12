#pragma once

#include "type/Date.h"
#include "type/Refusal.h"

#include <compare>
#include <cstdint>
#include <expected>
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
/// \note THE ANGLE BRACKETS MEAN "DO NOT TRANSLATE". `system-calcdate-string-date-method.md`: "The
///       user can enter formulas in the currently selected language. The formula is stored in a
///       generic format ... When the formula must be displayed, the actual string that is displayed
///       is converted to the currently selected language", and "if a date formula is entered with
///       < > delimiters surrounding it, then the date formula is stored in a generic,
///       nonlanguage-dependent format". So `1W+1D` is `1S+1J` in French and `1S+1D` in Spanish,
///       `<1W+1D>` is the same formula in every language, and every formula the BaseApp writes
///       carries the brackets. The language is `Language::Current()`, the thread's.
///
/// \note THE LETTERS OF A LANGUAGE NEVER CLASH WITH THE INVARIANT ONES, which is what lets a
///       reader take both at once: German `L T W M Q J`, Danish `L D U M K Å`, Dutch `H D W M K J`,
///       French `C J S M T A`, Spanish `C D S M T A`, Swedish `L D V M K Å`, Norwegian
///       `L D U M K Å` -- where a letter is shared with `C D W M Q Y` it means the same thing. A
///       language without a table reads and shows the invariant letters, which is what BC does for
///       a language it has no translation for.
///
/// \note `Evaluate` is the only way to assign one in AL, and `Format` the only way to compare one
///       against text. The page says both outright.
class DateFormula {
public:
  /// \brief An empty formula, which moves a date nowhere.
  DateFormula() = default;

  /// \brief Reads a formula.
  /// \param text The formula: with its angle brackets in the invariant letters, without them in
  ///             the thread's language or the invariant letters, or in the stored form
  ///             (\see ToStorageText).
  /// \return The formula, or WHY the text is not one.
  ///
  /// \note A CHARACTER THE GRAMMAR DOES NOT KNOW IS A REFUSAL AND NOT A SKIP. It was skipped
  ///       until this returned a value: `<1Q+garbage>` parsed as `1Q` and moved the date
  ///       somewhere plausible, which is the silent-wrong-data board:0082 is filed for.
  [[nodiscard]] static std::expected<DateFormula, Refusal> FromText(std::string_view text);

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

  /// \brief AL `Format(DateFormula)` -- the text in the thread's language.
  /// \return The formula with its letters translated: `1Y` under English, `1J` under German.
  ///
  /// \note IT IS WHAT A PAGE SHOWS AND WHAT A MESSAGE SAYS. ERM General Journal UT reads a yearly
  ///       Recurring Frequency back through a page under `GlobalLanguage(1031)` and expects `1J`
  ///       (`RecurringFrequencyDisplaysLocalizedGermanDateFormula`); the predecessor's WI-1208 is
  ///       the same case.
  [[nodiscard]] std::string ToText() const;

  /// \brief The language-independent letters, `1Y` in every language.
  /// \return The formula as `<...>` would spell it, without the brackets.
  [[nodiscard]] std::string InvariantText() const { return text_; }

  /// \brief The form the COLUMN holds, which is BC's own.
  /// \return The digits and signs as ASCII and each unit as one control byte: `C` is 1, `D` 2,
  ///         `WD` 3, `W` 4, `M` 5, `Q` 6, `Y` 7 -- `10D` is `10\x02`, `CM-1M` is `\x01\x05-1\x05`.
  ///
  /// \warning THE DEMO DATABASE IS IN THIS FORM AND NOTHING ELSE READS IT. Every DateFormula column
  ///          of the CRONUS load carries it -- 60 columns of 55 tables hold a control byte
  ///          (measured over `agiru_seeded`, 2026-09-12): every payment term's due date
  ///          calculation, every lead time, every shipping time, every reminder grace period. Read
  ///          as text they were all refused and silently EMPTY, so a due date was the document
  ///          date. The predecessor decoded the same bytes from the same hex dump (openerp
  ///          WI-1077: `313002` is `10D`, `0105` is `CM`, `01072d31072b3102` is `CY-1Y+1D`), and
  ///          a filter on the column binds this form (\see FilterText).
  [[nodiscard]] std::string ToStorageText() const;

  /// \brief Compares two formulas.
  /// \param o The other formula.
  /// \return True when they are written the same.
  [[nodiscard]] bool operator==(const DateFormula &o) const { return text_ == o.text_; }

private:
  /// What one term of a formula does.
  enum class Kind : std::uint8_t {
    Period,     ///< `CM`, `CQ`, `CY`, `CW`, `CD` -- one end of the current period.
    Weekday,    ///< `WD3` -- the next or previous occurrence of an ISO weekday.
    Week,       ///< `W7` -- the Monday of ISO week 7 of the base year.
    DayOfMonth, ///< `D15` -- the 15th of the month the date is in.
    Amount,     ///< `30D`, `2W`, `1M`, `1Q`, `1Y` -- a quantity of a unit.
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

}
