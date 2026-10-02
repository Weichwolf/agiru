#include "fixture/codeunit/ChartRoundtrip.h"
#include "Check.h"

int main() {
  return gate::Run("GeneratedChart", [] {
    agiru::Fixture::ChartRoundtrip_Codeunit unit;
    CHECK_TRUE("original Chart AL retains all twenty-one guards", unit.Exercise() == 21);
  });
}
