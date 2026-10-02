#include "dotnet/Refused.h"
#include "runtime/Error.h"

#include "Check.h"

namespace {

void RefusedChainsNameTheMember() {
  bool getRefused = false;
  try {
    static_cast<void>(agiru::dotnet::Refused::Get(1));
  } catch (const agiru::Error &error) {
    getRefused = std::string_view(error.what()).find("<result>.Get") != std::string_view::npos;
  }
  CHECK_TRUE("a chained absent Get refuses at execution", getRefused);

  bool errorRefused = false;
  try {
    agiru::RaiseOrCollect(agiru::dotnet::Refused::Message);
  } catch (const agiru::Error &error) {
    errorRefused =
        std::string_view(error.what()).find("<result>.Message") != std::string_view::npos;
  }
  CHECK_TRUE("an absent error value preserves its refusal", errorRefused);
}

}

int main() {
  return gate::Run("Refused", [] { RefusedChainsNameTheMember(); });
}
