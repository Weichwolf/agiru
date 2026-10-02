#include "meta/TableDef.h"
#include <cstdio>

int main() {
  std::printf("FieldDef bytes=%zu align=%zu\n", sizeof(agiru::FieldDef), alignof(agiru::FieldDef));
}
