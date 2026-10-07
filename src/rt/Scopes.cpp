#include "runtime/Scopes.h"

#include "runtime/Error.h"
#include "runtime/ErrorValue.h"
#include "type/CommitBehavior.h"
#include "type/ErrorBehavior.h"

#include "SessionState.h"

#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru {

namespace {

std::vector<::agiru::CommitBehavior> &Commits() {
  return detail::SessionState::Current().commits;
}

std::vector<::agiru::ErrorBehavior> &Errors() {
  return detail::SessionState::Current().errors;
}

std::vector<std::string> &CollectedErrors() {
  return detail::SessionState::Current().collectedErrors;
}

}

CommitScope::CommitScope(::agiru::CommitBehavior behaviour) {
  auto &standing = Commits();
  if (!standing.empty() && standing.back() == CommitBehavior::Error) {
    behaviour = CommitBehavior::Error;
  }
  standing.push_back(behaviour);
}

CommitScope::~CommitScope() {
  if (!Commits().empty()) { Commits().pop_back(); }
}

std::optional<::agiru::CommitBehavior> CommitScope::Standing() {
  if (Commits().empty()) { return std::nullopt; }
  return Commits().back();
}

ErrorScope::ErrorScope(::agiru::ErrorBehavior behaviour) {
  Errors().push_back(behaviour);
}

ErrorScope::~ErrorScope() noexcept(false) {
  if (Errors().empty()) { return; }
  Errors().pop_back();
  if (!Errors().empty() || CollectedErrors().empty()) { return; }
  std::string aggregated;
  for (const std::string &one : CollectedErrors()) {
    if (!aggregated.empty()) { aggregated += "\n"; }
    aggregated += one;
  }
  CollectedErrors().clear();
  if (std::uncaught_exceptions() == 0) { throw Error(aggregated); }
}

bool ErrorScope::Collecting() {
  return !Errors().empty();
}

void ErrorScope::Collect(std::string message) {
  CollectedErrors().push_back(std::move(message));
}

const std::vector<std::string> &ErrorScope::Collected() {
  return CollectedErrors();
}

void ErrorScope::Clear() {
  CollectedErrors().clear();
}

void RaiseOrCollect(std::string_view message) {
  throw Error(message);
}

void CollectError(std::string_view message) {
  if (!ErrorScope::Collecting()) { throw Error(message); }
  ErrorScope::Collect(std::string(message));
}

}
