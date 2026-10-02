#include "fixture/codeunit/SavedOptions.h"
#include "fixture/table/StoredFlag.h"
#include "platform/ObjectOptions.h"
#include "runtime/Table.h"

#include "Check.h"

int main() {
  return gate::Run("GeneratedObjectOptions", [] {
    agiru::Fixture::SavedOptions_Codeunit codeunit;
    CHECK_TRUE("nine original AL checks execute", codeunit.Exercise() == 9);
    agiru::Temporary<agiru::Fixture::StoredFlag_Table> row;
    row.Temporary_8 = true;
    CHECK_TRUE("ordinary wrapper and source field remain distinct", row.Temporary_8 && row.IsTemporary());
    agiru::platform::ObjectOptions options;
    options.Temporary_8 = true;
    CHECK_TRUE("persistent native flag is not session storage", options.Temporary_8 && !options.IsTemporary());
  });
}
