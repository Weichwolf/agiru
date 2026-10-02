#include "TableKeys.h"

#include "Ast.h"
#include "EnumWriter.h"
#include "Names.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace agiru::gen {

void CompletePrimaryKey(al::TableObject &table) {
  if (!table.keys.empty()) { return; }
  if (table.fields.empty()) {
    throw std::runtime_error("implicit primary key has no declared field: " + table.name);
  }
  const auto first = std::ranges::min_element(table.fields, {}, &al::FieldDecl::number);
  const auto *fieldClass = al::Find(first->properties, "FieldClass");
  const std::string type = TypeName(first->type);
  if ((fieldClass != nullptr && LowerKey(fieldClass->text) != "normal") || type == "Blob" ||
      type == "Media" || type == "MediaSet") {
    throw std::runtime_error("implicit primary key field is not supported: " + table.name + "." +
                             first->name);
  }
  table.keys.push_back(al::KeyDecl{.name = first->name, .fields = {first->name}, .properties = {}});
}

}
