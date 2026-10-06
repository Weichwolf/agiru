#pragma once

#include "meta/Ids.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace agiru {

struct TableDef;

namespace detail {

struct CatalogueReader {
  std::size_t size;
  void *candidate;
  void *selected;
  void *anchor;
  void (*key)(void *row, std::size_t index);
  void (*project)(void *row, std::size_t index);
  void (*require)(FieldNo no) = nullptr;
};

bool FindCatalogue(void *record,
                   const TableDef &table,
                   std::string_view which,
                   const CatalogueReader &reader);

std::int32_t NextCatalogue(void *record,
                           const TableDef &table,
                           std::int32_t steps,
                           const CatalogueReader &reader);

std::int32_t CountCatalogue(const void *record,
                            const TableDef &table,
                            bool firstOnly,
                            const CatalogueReader &reader);

}

}
