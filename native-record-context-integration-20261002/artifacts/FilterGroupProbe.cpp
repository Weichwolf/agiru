#include "platform/Field.h"
#include "runtime/RecordRef.h"
#include "runtime/Table.h"
#include <cstdio>
#include <exception>

int main() {
  try {
    agiru::Temporary<agiru::platform::Field> row;
    row.FilterGroup(2);
    agiru::RecordRef ref;
    ref.GetTable(row);
    const auto first = ref.FilterGroup();
    const auto second = ref.FilterGroup();
    row.FilterGroup(256);
    const auto outOfRange = row.FilterGroup();
    std::printf("RecordRef getter: %d then %d; typed group after 256: %d\n", first, second, outOfRange);
    return first == 2 && second == 0 && outOfRange == 256 ? 0 : 1;
  } catch (const std::exception &error) {
    std::fputs(error.what(), stderr);
    return 1;
  }
}
