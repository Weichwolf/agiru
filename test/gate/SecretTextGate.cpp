#include "type/SecretText.h"

#include "Check.h"

#include <string>
#include <type_traits>

using agiru::SecretText;

namespace {

/// THE POINT OF THE TYPE IS WHAT IT DOES NOT DO. A SecretText carries a password or a token, and
/// the platform keeps it out of the debugger, out of a message and out of telemetry. So the type
/// has no conversion to Text, no comparison against one, and nothing that renders it.
void ASecretDoesNotRenderItself() {
  const SecretText secret{"hunter2"};
  CHECK_TRUE("a secret with content is not empty", !secret.IsEmpty());
  CHECK_TRUE("an empty one says so", SecretText{}.IsEmpty());
  CHECK_TEXT("and Unwrap is the only way out", secret.Unwrap(), "hunter2");

  // The compile-time half of the claim: these must NOT exist, and the gate says so in prose because
  // C++ cannot assert the absence of a conversion without naming it.
  static_assert(!std::is_convertible_v<SecretText, std::string>,
                "a secret must not become a plain string by accident");
  static_assert(!std::is_constructible_v<std::string, SecretText>, "nor by construction");
}

/// `SecretStrSubstNo` EXISTS BECAUSE THE RESULT MUST STAY A SECRET. A token pasted through an
/// ordinary StrSubstNo would leave the type and become renderable.
void SubstitutionKeepsTheResultSecret() {
  const SecretText token{"abc123"};
  const SecretText built = SecretText::SecretStrSubstNo("Bearer %1", token);
  CHECK_TEXT("the placeholder is filled", built.Unwrap(), "Bearer abc123");
  static_assert(
      std::is_same_v<decltype(SecretText::SecretStrSubstNo("%1", SecretText{})), SecretText>,
      "the result of a secret substitution is a secret");

  const SecretText two =
      SecretText::SecretStrSubstNo("%1:%2", SecretText{"user"}, SecretText{"pass"});
  CHECK_TEXT("several placeholders are filled in order", two.Unwrap(), "user:pass");

  // THE NEGATIVE CONTROL: a placeholder with no value stays as it was written, rather than
  // vanishing and leaving a message that reads as if it had a value.
  const SecretText missing = SecretText::SecretStrSubstNo("%1 and %2", SecretText{"one"});
  CHECK_TEXT("an unfilled placeholder is left alone", missing.Unwrap(), "one and %2");
}

void TwoSecretsCompareByContent() {
  CHECK_TRUE("the same content is the same secret", SecretText{"a"} == SecretText{"a"});
  CHECK_TRUE("different content is not", !(SecretText{"a"} == SecretText{"b"}));
}

} // namespace

int main() {
  return gate::Run("SecretText", [] {
    ASecretDoesNotRenderItself();
    SubstitutionKeepsTheResultSecret();
    TwoSecretsCompareByContent();
  });
}
