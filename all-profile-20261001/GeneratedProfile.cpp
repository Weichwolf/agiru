#include "fixture/codeunit/NativeProfile.h"

#include "Check.h"

int main() {
  return gate::Run("GeneratedNativeProfile", [] {
    agiru::Fixture::NativeProfile_Codeunit codeunit;
    CHECK_TRUE("twenty-four checks execute through the source-backed native profile binding",
               codeunit.Exercise() == 24);
  });
}
