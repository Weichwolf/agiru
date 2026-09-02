#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

/// \file
/// \brief AL `SecretText` -- a string that does not show itself.

namespace agiru {

/// \brief AL `SecretText`.
///
/// From `secrettext-data-type.md`: "Denotes a secret text string, which is non-debuggable."
///
/// \note THE POINT OF THE TYPE IS WHAT IT DOES NOT DO. A SecretText carries a password, a key or a
///       token, and the platform keeps it out of the debugger, out of a rendered message and out of
///       telemetry. So this type has no conversion to Text, no comparison against one, and nothing
///       that renders it -- `Unwrap()` is the only way out and the page calls its use "discouraged
///       as it can lead to secret exposure". A convenience added here would be the exposure.
class SecretText {
public:
  /// \brief An empty secret.
  SecretText() = default;

  /// \brief Wraps a plain string.
  /// \param text The secret.
  explicit SecretText(std::string text) : text_(std::move(text)) {}

  /// \brief AL `SecretText.IsEmpty()`.
  /// \return True when the secret holds no content.
  [[nodiscard]] bool IsEmpty() const { return text_.empty(); }

  /// \brief AL `SecretText.Unwrap()` -- the plain text inside.
  ///
  /// \return The secret, in the clear.
  ///
  /// \warning DISCOURAGED, AND THE PLATFORM SAYS SO: it "exists for compatibility reasons and its
  ///          use is discouraged as it can lead to secret exposure". It is also on-premises only.
  ///          Every call is a place where a secret leaves the type that was protecting it.
  [[nodiscard]] std::string Unwrap() const { return text_; }

  /// \brief AL `SecretText.SecretStrSubstNo(Text, SecretText...)`.
  ///
  /// \param pattern The text with `%1`, `%2` ... in it.
  /// \param values  What they stand for.
  /// \return A secret carrying the substituted text.
  ///
  /// \note THE RESULT IS A SECRET AND NOT A TEXT, which is the whole reason the method exists: a
  ///       token pasted into an ordinary `StrSubstNo` would leave the type and become renderable.
  template <typename... Values>
  [[nodiscard]] static SecretText SecretStrSubstNo(std::string_view pattern,
                                                   const Values &...values) {
    std::string out(pattern);
    int placeholder = 0;
    (Substitute(out, ++placeholder, values), ...);
    return SecretText{std::move(out)};
  }

  /// \brief Compares two secrets.
  /// \param o The other secret.
  /// \return True when they hold the same content.
  /// \note Present because AL compares them; there is deliberately no comparison against a Text.
  [[nodiscard]] bool operator==(const SecretText &o) const = default;

private:
  static void Substitute(std::string &pattern, int placeholder, const SecretText &value) {
    Replace(pattern, "%" + std::to_string(placeholder), value.text_);
  }

  static void Substitute(std::string &pattern, int placeholder, std::string_view value) {
    Replace(pattern, "%" + std::to_string(placeholder), std::string(value));
  }

  static void Replace(std::string &pattern, const std::string &mark, const std::string &value) {
    for (std::size_t at = pattern.find(mark); at != std::string::npos;
         at = pattern.find(mark, at + value.size())) {
      pattern.replace(at, mark.size(), value);
    }
  }

  std::string text_;
};

} // namespace agiru
