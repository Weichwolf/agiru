#pragma once

#include "agiru/Error.h"

#include <compare>
#include <cstdint>
#include <span>
#include <string_view>

namespace agiru {

class OptionError : public Error {
public:
  using Error::Error;
};

/// The declared members of one AL option, as static const data.
///
/// The generator specialises this per option. Everything in it lives in `.rodata`: it is paged in
/// on first touch, shared between processes and costs nothing at startup -- which is the whole
/// reason object metadata is emitted rather than built (see CLAUDE.md, the 512 MB target).
///
/// The AL spelling is kept even where the C++ identifier cannot be: `OptionMembers` may contain
/// `"Group(Resource)"` or `"% Extra"`, which no identifier may spell. The enumerator is named by
/// the generator, the MEMBER NAME here stays what AL wrote, because that is what an error message
/// and a filter string have to say.
template <typename E> struct OptionTraits;

/// AL `Option` -- `methods-auto/option/option-data-type.md`: "The Option type is a zero-based
/// enumerator type, which means that the option values are assigned to sequential numbers, starting
/// with 0. You can convert option data types to integers."
///
/// An option is therefore an INTEGER that carries a name table, not a closed set. AL lets an option
/// hold an ordinal outside its declared members -- assigning an integer is legal and the platform
/// does not refuse it -- so this type does not refuse it either; `Name()` says so instead.
template <typename E> class Option {
public:
  using Enum = E;
  using Traits = OptionTraits<E>;

  constexpr Option() = default;

  constexpr explicit Option(E value) : ordinal_(static_cast<std::int32_t>(value)) {}

  constexpr explicit Option(std::int32_t ordinal) : ordinal_(ordinal) {}

  constexpr Option &operator=(E value) {
    ordinal_ = static_cast<std::int32_t>(value);
    return *this;
  }

  /// AL converts an option to an integer without ceremony; so does this.
  [[nodiscard]] constexpr std::int32_t AsInteger() const { return ordinal_; }

  [[nodiscard]] constexpr E Value() const { return static_cast<E>(ordinal_); }

  [[nodiscard]] constexpr bool IsDeclared() const {
    return ordinal_ >= 0 && static_cast<std::size_t>(ordinal_) < Traits::kMembers.size();
  }

  /// The member name as AL spelled it, or empty when the ordinal is outside the declared list.
  [[nodiscard]] constexpr std::string_view Name() const {
    return IsDeclared() ? Traits::kMembers[static_cast<std::size_t>(ordinal_)] : std::string_view{};
  }

  /// The display caption. `OptionCaption` may differ from `OptionMembers` -- table 202 declares the
  /// same text for both, table 4 does not -- so they are two lists and never one.
  [[nodiscard]] constexpr std::string_view Caption() const {
    return IsDeclared() ? Traits::kCaptions[static_cast<std::size_t>(ordinal_)]
                        : std::string_view{};
  }

  [[nodiscard]] constexpr std::strong_ordering operator<=>(const Option &o) const = default;
  [[nodiscard]] constexpr bool operator==(const Option &o) const = default;

private:
  std::int32_t ordinal_{0};
};

} // namespace agiru
