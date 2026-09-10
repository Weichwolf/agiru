#include "type/Variant.h"

#include "runtime/Error.h"

#include <string>

namespace agiru {

std::string_view Variant::Rendered() const {
  rendered_ = std::visit(
      [](const auto &held) -> std::string {
        using Held = std::remove_cvref_t<decltype(held)>;
        if constexpr (std::same_as<Held, std::monostate>) {
          return {};
        } else if constexpr (std::same_as<Held, Boolean>) {
          return held ? "Yes" : "No";
        } else if constexpr (std::same_as<Held, std::string>) {
          return held;
        } else if constexpr (std::same_as<Held, Integer> || std::same_as<Held, BigInteger>) {
          return std::to_string(held);
        } else if constexpr (requires { held.ToInvariantString(); }) {
          return held.ToInvariantString();
        } else if constexpr (requires { held.ToText(); }) {
          return std::string(std::string_view(held.ToText()));
        } else {
          return {};
        }
      },
      held_);
  if (rendered_.empty() && !HoldsSomethingTextual()) { Refuse("Text"); }
  return rendered_;
}

bool Variant::HoldsSomethingTextual() const {
  return std::visit(
      [](const auto &held) {
        using Held = std::remove_cvref_t<decltype(held)>;
        return std::same_as<Held, std::monostate> || std::same_as<Held, Boolean> ||
               std::same_as<Held, std::string> || std::same_as<Held, Integer> ||
               std::same_as<Held, BigInteger> || requires { held.ToInvariantString(); } ||
               requires { held.ToText(); };
      },
      held_);
}

void Variant::Refuse(const char *wanted) const {
  throw Error(std::string("the Variant does not hold ") + wanted + " (it holds alternative " +
              std::to_string(held_.index()) + " of Variant::Held)");
}

}

namespace agiru {

Variant::Variant(const Variant &o) = default;
Variant::Variant(Variant &&o) noexcept = default;
Variant &Variant::operator=(const Variant &o) = default;
Variant &Variant::operator=(Variant &&o) noexcept = default;
Variant::~Variant() = default;

bool Variant::operator==(const Variant &o) const {
  return held_ == o.held_;
}

}
