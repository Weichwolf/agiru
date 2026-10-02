#include "fixture/codeunit/ODataRoundtrip.h"

#include "Check.h"

int main() {
  return gate::Run("GeneratedOData", [] {
    agiru::Fixture::ODataRoundtrip_Codeunit codeunit;
    CHECK_TRUE("thirteen original AL guards execute", codeunit.Exercise() == 13);
  });
}
