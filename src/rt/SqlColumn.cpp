#include "SqlColumn.h"

#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"

#include <string>

namespace agiru::detail {

std::string SqlColumn(const FieldDef &field) {
  return Quoted(ColumnName(field));
}

bool HasRowVersion(const TableDef &table) {
  bool present = false;
  for (const FieldDef &field : table.fields) {
    if (!field.sqlTimestamp) { continue; }
    if (!Stored(field) || field.type != FieldType::BigInteger || field.autoIncrement) {
      throw Error("SqlTimestamp requires a stored BigInteger without AutoIncrement: " +
                  std::string(table.name) + "." + std::string(field.name));
    }
    present = true;
  }
  if (present) {
    for (const FieldDef &field : table.fields) {
      if (field.name == kWriteOwnerColumn) {
        throw Error("Storage: field name collides with private write ownership", "RecordVersion");
      }
      if (Stored(field) && !field.sqlTimestamp && field.name == kRowVersionColumn) {
        throw Error("SqlTimestamp collides with an ordinary timestamp column: " +
                    std::string(table.name));
      }
    }
  }
  return present;
}

}
