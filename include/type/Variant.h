#pragma once

#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Date.h"
#include "type/DateFormula.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/RecordId.h"
#include "type/Time.h"

#include <string>
#include <type_traits>
#include <variant>

/// \file
/// \brief AL `Variant` -- one value, of whichever AL type it was given.

namespace agiru {

/// \brief AL `Variant`.
///
/// From `variant-data-type.md`: "Represents an AL variable object. The AL variant data type can
/// contain many AL data types."
///
/// \note IT ANSWERS WHAT IT HOLDS AND NEVER CONVERTS. The page gives some sixty `IsX()` predicates
///       and no conversions, because a Variant is how AL passes a value whose type the callee
///       decides on. A `Get<T>()` that coerced would turn "this is not a Date" into a plausible
///       wrong Date, which is the class of defect this whole tree is built to move to compile time.
///
/// \note `IsDuration()` IS MISSING BECAUSE OF AN ALIAS, and this is the first thing that alias has
///       cost. `Duration` and `BigInteger` are both `std::int64_t` here -- deliberately, since
///       generated AL code does arithmetic on both constantly and a wrapper would forward every
///       operator -- so C++ cannot tell them apart and a variant cannot hold both. AL can: it asks
///       `IsDuration()` and `IsBigInteger()` as two questions. Answering one of them wrongly is
///       worse than answering neither, so the type is left out and the reason is written down
///       rather than hidden (board:0024).
///
/// \note THE OBJECT TYPES ARE NOT IN IT YET -- Record, RecordRef, InStream, DotNet and the rest.
///       They are not values, they are handles, and each needs its own type in the runtime first.
///       A Variant handed one refuses rather than holding a scalar that looks like it.
class Variant {
public:
  /// \brief What a Variant can hold today. An empty Variant holds the first alternative.
  using Held = std::variant<std::monostate,
                            Boolean,
                            Integer,
                            BigInteger,
                            Decimal,
                            std::string,
                            Date,
                            Time,
                            DateTime,
                            Guid,
                            RecordId,
                            DateFormula>;

  /// \brief An empty Variant, which is what an unassigned one holds.
  Variant() = default;

  /// \brief Holds a value.
  /// \tparam T The AL type.
  /// \param value The value.
  template <typename T>
    requires std::is_constructible_v<Held, T>
  explicit Variant(T value) : held_(std::move(value)) {}

  /// \brief AL `Variant.IsEmpty()` -- whether nothing was ever assigned.
  /// \return True when the Variant holds no value.
  [[nodiscard]] bool IsEmpty() const { return std::holds_alternative<std::monostate>(held_); }

  /// \brief Whether the Variant holds a given AL type.
  /// \tparam T The AL type.
  /// \return True when it does.
  ///
  /// The sixty `IsX()` predicates the page lists are this one question with the type spelled into
  /// the name. They are written out below for the types that exist, because AL code calls them by
  /// those names and a reader looking for `IsDate` must find `IsDate`.
  template <typename T> [[nodiscard]] bool Is() const { return std::holds_alternative<T>(held_); }

  /// \brief AL `Variant.IsBoolean()`. \return True when it holds one.
  [[nodiscard]] bool IsBoolean() const { return Is<Boolean>(); }

  /// \brief AL `Variant.IsInteger()`. \return True when it holds one.
  [[nodiscard]] bool IsInteger() const { return Is<Integer>(); }

  /// \brief AL `Variant.IsBigInteger()`. \return True when it holds one.
  [[nodiscard]] bool IsBigInteger() const { return Is<BigInteger>(); }

  /// \brief AL `Variant.IsDecimal()`. \return True when it holds one.
  [[nodiscard]] bool IsDecimal() const { return Is<Decimal>(); }

  /// \brief AL `Variant.IsText()`. \return True when it holds one.
  [[nodiscard]] bool IsText() const { return Is<std::string>(); }

  /// \brief AL `Variant.IsCode()`. \return True when it holds one.
  /// \note A Code and a Text are one alternative here, because AL's Code IS a Text with a
  ///       normalisation rule, and a Variant carries the VALUE rather than the rule.
  [[nodiscard]] bool IsCode() const { return Is<std::string>(); }

  /// \brief AL `Variant.IsDate()`. \return True when it holds one.
  [[nodiscard]] bool IsDate() const { return Is<Date>(); }

  /// \brief AL `Variant.IsTime()`. \return True when it holds one.
  [[nodiscard]] bool IsTime() const { return Is<Time>(); }

  /// \brief AL `Variant.IsDateTime()`. \return True when it holds one.
  [[nodiscard]] bool IsDateTime() const { return Is<DateTime>(); }

  /// \brief AL `Variant.IsGuid()`. \return True when it holds one.
  [[nodiscard]] bool IsGuid() const { return Is<Guid>(); }

  /// \brief AL `Variant.IsRecordId()`. \return True when it holds one.
  [[nodiscard]] bool IsRecordId() const { return Is<RecordId>(); }

  /// \brief AL `Variant.IsDateFormula()`. \return True when it holds one.
  [[nodiscard]] bool IsDateFormula() const { return Is<DateFormula>(); }

  /// \brief The value, if the Variant holds that type.
  ///
  /// \tparam T The AL type.
  /// \return The value.
  /// \throws Error when the Variant holds something else, naming both types.
  ///
  /// \warning IT DOES NOT CONVERT. AL assigns a Variant to a typed variable and the platform
  ///          refuses a mismatch; a `Get<Date>()` that read an Integer as a day number would turn a
  ///          type error into a wrong date, silently.
  template <typename T> [[nodiscard]] const T &Get() const {
    const T *value = std::get_if<T>(&held_);
    if (value == nullptr) { Refuse(); }
    return *value;
  }

  /// \brief Compares two Variants.
  /// \param o The other.
  /// \return True when they hold the same type and the same value.
  [[nodiscard]] bool operator==(const Variant &o) const = default;

private:
  [[noreturn]] static void Refuse();

  Held held_;
};

} // namespace agiru
