#pragma once

#include <compare>
#include <cstdint>

namespace agiru {

/// AL identifies objects and fields by number, and every one of those numbers is a 32-bit integer.
/// Left as `int` they are interchangeable at every call site, and AL swaps them silently -- which
/// is what CLAUDE.md's strong-type rule exists for, and what `bugprone-easily-swappable-parameters`
/// guards. `TableId{202}` and `FieldNo{202}` are different types and the compiler now knows it.
///
/// A tag template rather than a macro: a macro that generates types hides them from every tool that
/// reads this tree, starting with the one that reported the macro.
template <typename Tag> class Id {
public:
  constexpr Id() = default;

  constexpr explicit Id(std::int32_t value) : value_(value) {}

  [[nodiscard]] constexpr std::int32_t Value() const { return value_; }

  [[nodiscard]] constexpr auto operator<=>(const Id &) const = default;

private:
  std::int32_t value_{0};
};

struct TableIdTag;
struct FieldNoTag;
struct CodeunitIdTag;
struct PageIdTag;

using TableId = Id<TableIdTag>;
using FieldNo = Id<FieldNoTag>;
using CodeunitId = Id<CodeunitIdTag>;
using PageId = Id<PageIdTag>;

} // namespace agiru
