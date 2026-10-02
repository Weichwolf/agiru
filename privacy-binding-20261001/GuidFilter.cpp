#include "Filter.h"
#include "meta/TableDef.h"
#include "runtime/Record.h"
#include "type/Guid.h"

#include "Check.h"

#include <iostream>

int main() {
  return gate::Run("GuidFilterDiagnostic", [] {
    const agiru::Guid guid{"12345678-1234-1234-1234-123456789abc"};
    const agiru::FieldDef field{.type = agiru::FieldType::Guid};
    const auto filter = agiru::FilterText(guid);
    const auto row = guid.ToText();
    std::cout << "typed filter: " << filter << "\nrow text: " << row << '\n';
    CHECK_TRUE("typed Guid SetRange must match the same Guid row",
               agiru::detail::Matches(agiru::detail::ParseFilter(filter), row, field));
    CHECK_TRUE("braced spelling reaches the same row",
               agiru::detail::Matches(agiru::detail::ParseFilter(row), row, field));
  });
}
