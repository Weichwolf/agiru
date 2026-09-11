#include "type/Variant.h"

#include "runtime/Error.h"

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <string>
#include <string_view>

namespace agiru {

std::string_view Variant::HeldName() const {
  return std::visit(
      [](const auto &held) -> std::string_view {
        using Held = std::remove_cvref_t<decltype(held)>;
        if constexpr (std::same_as<Held, std::monostate>) {
          return "nothing";
        } else if constexpr (std::same_as<Held, Boolean>) {
          return "Boolean";
        } else if constexpr (std::same_as<Held, Integer>) {
          return "Integer";
        } else if constexpr (std::same_as<Held, BigInteger>) {
          return "BigInteger";
        } else if constexpr (std::same_as<Held, Decimal>) {
          return "Decimal";
        } else if constexpr (std::same_as<Held, std::string>) {
          return "Text";
        } else if constexpr (std::same_as<Held, Date>) {
          return "Date";
        } else if constexpr (std::same_as<Held, Time>) {
          return "Time";
        } else if constexpr (std::same_as<Held, DateTime>) {
          return "DateTime";
        } else if constexpr (std::same_as<Held, Duration>) {
          return "Duration";
        } else if constexpr (std::same_as<Held, Guid>) {
          return "Guid";
        } else if constexpr (std::same_as<Held, RecordId>) {
          return "RecordId";
        } else if constexpr (std::same_as<Held, DateFormula>) {
          return "DateFormula";
        } else if constexpr (std::same_as<Held, Blob>) {
          return "Blob";
        } else if constexpr (std::same_as<Held, OrdinalInVariant>) {
          return "Option";
        } else if constexpr (std::same_as<Held, RecordInVariant>) {
          return "Record";
        } else if constexpr (std::same_as<Held, RecordRefInVariant>) {
          return "RecordRef";
        } else if constexpr (std::same_as<Held, CodeunitInVariant>) {
          return "Codeunit";
        } else {
          return "Xml";
        }
      },
      held_);
}

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

namespace agiru::detail {

namespace {

std::string_view Trimmed(std::string_view text) {
  while (!text.empty() && text.front() == ' ') { text.remove_prefix(1); }
  while (!text.empty() && text.back() == ' ') { text.remove_suffix(1); }
  return text;
}

bool WholeSpelled(std::string_view text, long long &into) {
  const std::string held(Trimmed(text));
  if (held.empty()) { return false; }
  char *end = nullptr;
  errno = 0;
  into = std::strtoll(held.c_str(), &end, 10);
  return errno == 0 && end != nullptr && *end == '\0';
}

}

bool TextSpells(std::string_view text, Decimal &into) {
  const std::string_view held = Trimmed(text);
  if (held.empty()) { return false; }
  try {
    into = Decimal::FromInvariantString(held);
  } catch (const DecimalError &) { return false; }
  return true;
}

bool TextSpells(std::string_view text, Integer &into) {
  long long whole = 0;
  if (!WholeSpelled(text, whole)) { return false; }
  if (whole < std::numeric_limits<std::int32_t>::min() ||
      whole > std::numeric_limits<std::int32_t>::max()) {
    return false;
  }
  into = static_cast<Integer>(whole);
  return true;
}

bool TextSpells(std::string_view text, BigInteger &into) {
  long long whole = 0;
  if (!WholeSpelled(text, whole)) { return false; }
  into = static_cast<BigInteger>(whole);
  return true;
}

bool TextSpells(std::string_view text, Boolean &into) {
  const std::string_view held = Trimmed(text);
  if (held == "1" || held == "true" || held == "Yes" || held == "yes" || held == "True") {
    into = true;
    return true;
  }
  if (held == "0" || held == "false" || held == "No" || held == "no" || held == "False") {
    into = false;
    return true;
  }
  return false;
}

std::int32_t OrdinalOf(const Variant &held) {
  if (held.Is<OrdinalInVariant>()) { return held.Get<OrdinalInVariant>().ordinal; }
  if (held.Is<Integer>()) { return held.Get<Integer>(); }
  throw Error("an Option takes a Variant holding an option or an integer, and this one holds " +
              std::string(held.HeldName()));
}

}
