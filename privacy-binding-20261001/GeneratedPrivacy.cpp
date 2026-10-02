#include "fixture/codeunit/NativePrivacy.h"

#include "Check.h"

int main() {
  return gate::Run("GeneratedNativePrivacy", [] {
    agiru::Fixture::NativePrivacy_Codeunit codeunit;
    CHECK_TRUE("eighteen checks execute through original AL native bindings", codeunit.Exercise() == 18);
  });
}
