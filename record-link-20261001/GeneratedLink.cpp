#include "fixture/codeunit/NativeLink.h"

#include "Check.h"

int main() {
  return gate::Run("GeneratedNativeLink", [] {
    agiru::Fixture::NativeLink_Codeunit codeunit;
    CHECK_TRUE("all source-backed link and collision checks execute", codeunit.Exercise() == 28);
  });
}
