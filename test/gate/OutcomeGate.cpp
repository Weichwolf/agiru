#include "type/Guid.h"
#include "type/Outcome.h"
#include "type/Refusal.h"

#include "Check.h"

#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace {

static_assert(std::is_same_v<agiru::Outcome<agiru::Guid, agiru::Refusal>,
                             decltype(agiru::Guid::FromText(""))>);
static_assert(noexcept(agiru::Failed(agiru::Refusal{})));

void OwnedAlternativesPreserveValuesAndErrors() {
  constexpr int kValue = 23;
  agiru::Outcome<std::unique_ptr<int>, std::string> value = std::make_unique<int>(kValue);
  CHECK_TRUE("the success alternative owns a move-only value", **value == kValue);
  const int *const owned = value->get();
  const auto moved = std::move(value);
  CHECK_TRUE("moving the outcome transfers the same ownership",
             **moved == kValue && moved->get() == owned);

  std::string reason = "refused";
  const agiru::Outcome<int, std::string> copied = agiru::Failed(reason);
  reason.clear();
  CHECK_TEXT("an lvalue error is copied into the outcome", copied.error(), "refused");
  const agiru::Outcome<int, std::string> literal = agiru::Failed("literal refusal");
  CHECK_TEXT("a literal converts to the declared owned error", literal.error(), "literal refusal");

  auto failure = std::make_unique<int>(kValue);
  const agiru::Outcome<int, std::unique_ptr<int>> transferred = agiru::Failed(std::move(failure));
  CHECK_TRUE("a move-only error is transferred intact",
             !transferred.has_value() && *transferred.error() == kValue && failure == nullptr);

  constexpr std::size_t kPosition = 17;
  const agiru::Outcome<int, agiru::Refusal> refusal =
      agiru::Failed(agiru::Refusal{.what = "invalid input", .at = kPosition});
  CHECK_TRUE("a refusal preserves its exact diagnostic and position",
             !refusal.has_value() && refusal.error().what == "invalid input" &&
                 refusal.error().at == kPosition);
}

}

int main() {
  return gate::Run("Outcome", OwnedAlternativesPreserveValuesAndErrors);
}
