#include "fixture/codeunit/NativeField.h"

#include "Check.h"

int main() {
  return gate::Run("GeneratedNativeField", [] {
    agiru::Fixture::NativeField_Codeunit codeunit;
    CHECK_TRUE("thirteen checks execute through original AL and the generated native binding",
               codeunit.Exercise() == 13);
  });
}
