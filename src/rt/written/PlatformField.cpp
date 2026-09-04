#include "meta/EnumDef.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/Field.h"
#include "runtime/Catalogue.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Option.h"

#include <algorithm>
#include <string>

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
  return std::ranges::any_of(table.keys[0].fields,
                             [no](const FieldNo held) { return held.Value() == no.Value(); });
}

}

Boolean Field::Get(::agiru::Integer TableNo, ::agiru::Integer No) {
  const TableEntry *entry = FindTable(TableId{TableNo});
  if (entry == nullptr) { return false; }
  const auto wanted = std::ranges::find_if(
      entry->table->fields, [No](const FieldDef &def) { return def.no.Value() == No; });
  if (wanted == entry->table->fields.end()) { return false; }
  const FieldDef &def = *wanted;
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

}
