#include "platform/Field.h"

#include "meta/TableDef.h"
#include "runtime/Catalogue.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace agiru::platform {

namespace {

Option<agiru::FieldType> AsType(FieldType type) {
  return Option<agiru::FieldType>{type};
}

Option<FieldClass> AsClass(const FieldDef &def) {
  static_cast<void>(def);
  return Option<FieldClass>{FieldClass::Normal};
}

std::string OptionStringOf(const FieldDef &def) {
  std::string out;
  for (const EnumValueDef &value : def.values) {
    if (!out.empty()) { out += ','; }
    out += value.name;
  }
  return out;
}

bool InPrimaryKey(const TableDef &table, FieldNo no) {
  if (table.keys.empty()) { return false; }
  for (const FieldNo held : table.keys[0].fields) {
    if (held.Value() == no.Value()) { return true; }
  }
  return false;
}

}

Boolean Field::Get(::agiru::Integer TableNo, ::agiru::Integer No) {
  const TableEntry *entry = FindTable(TableId{TableNo});
  if (entry == nullptr) { return false; }
  for (const FieldDef &def : entry->table->fields) {
    if (def.no.Value() != No) { continue; }
    this->TableNo = TableNo;
    this->No = No;
    TableName = entry->table->name;
    FieldName = def.name;
    Type = AsType(def.type);
    Len = static_cast<::agiru::Integer>(def.length);
    Class = AsClass(def);
    OptionString = OptionStringOf(def);
    FieldCaption = def.caption;
    Enabled = true;
    IsPartOfPrimaryKey = InPrimaryKey(*entry->table, def.no);
    return true;
  }
  return false;
}

}
