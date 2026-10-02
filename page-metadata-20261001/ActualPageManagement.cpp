#include "utilities/codeunit/PageManagement.h"

#include "Check.h"

int main() {
  return gate::Run("ActualPageManagement", [] {
    agiru::Utilities::PageManagement_Codeunit codeunit;
    CHECK_TRUE("actual zero-table card lookup remains zero", codeunit.GetDefaultCardPageID(0) == 0);
    CHECK_TRUE("actual zero-table lookup remains zero", codeunit.GetDefaultLookupPageID(0) == 0);
  });
}
