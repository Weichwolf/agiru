#include "fixture/codeunit/NativeMetadata.h"

#include "Check.h"

int main() {
  return gate::Run("GeneratedNativeMetadata", [] {
    agiru::Fixture::NativeMetadata_Codeunit codeunit;
    CHECK_TRUE("every source-backed metadata check executes", codeunit.Exercise() == 39);
  });
}
