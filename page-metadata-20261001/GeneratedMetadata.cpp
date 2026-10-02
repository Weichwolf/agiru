#include "fixture/codeunit/MetadataRoundtrip.h"

#include "Check.h"

int main() {
  return gate::Run("GeneratedPageMetadata", [] {
    agiru::Fixture::MetadataRoundtrip_Codeunit codeunit;
    CHECK_TRUE("twenty-seven original AL checks execute", codeunit.Exercise() == 27);
  });
}
