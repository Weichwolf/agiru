#include "meta/TableDef.h"
#include "platform/AllObj.h"
#include "platform/AllObjWithCaption.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

namespace {

std::string_view Type(agiru::FieldType type) {
  switch (type) {
  case agiru::FieldType::Integer: return "Integer";
  case agiru::FieldType::Text: return "Text";
  case agiru::FieldType::Guid: return "Guid";
  case agiru::FieldType::Option: return "Option";
  default: return "Unsupported";
  }
}

void Table(const agiru::TableDef &table) {
  std::cout << "{\"id\":" << table.id.Value() << ",\"name\":" << std::quoted(std::string(table.name))
            << ",\"data_per_company\":" << (table.dataPerCompany ? "true" : "false")
            << ",\"inherent_permissions\":" << std::quoted(std::string(table.inherentPermissions))
            << ",\"fields\":[";
  bool comma = false;
  for (const auto &field : table.fields) {
    if (comma) { std::cout << ','; }
    comma = true;
    std::cout << "{\"number\":" << field.no.Value() << ",\"name\":" << std::quoted(std::string(field.name))
              << ",\"caption\":" << std::quoted(std::string(field.caption)) << ",\"type\":"
              << std::quoted(std::string(Type(field.type))) << ",\"length\":" << field.length
              << ",\"options\":[";
    bool optionComma = false;
    for (const auto &option : field.values) {
      if (optionComma) { std::cout << ','; }
      optionComma = true;
      std::cout << "{\"ordinal\":" << option.ordinal << ",\"name\":" << std::quoted(std::string(option.name))
                << ",\"caption\":" << std::quoted(std::string(option.caption)) << '}';
    }
    std::cout << "]}";
  }
  std::cout << "],\"keys\":[";
  comma = false;
  for (const auto &key : table.keys) {
    if (comma) { std::cout << ','; }
    comma = true;
    std::cout << "{\"name\":" << std::quoted(std::string(key.name)) << ",\"fields\":[";
    bool fieldComma = false;
    for (const auto field : key.fields) {
      if (fieldComma) { std::cout << ','; }
      fieldComma = true;
      std::cout << field.Value();
    }
    std::cout << "],\"clustered\":" << (key.clustered ? "true" : "false") << '}';
  }
  std::cout << "]}";
}

}

int main() {
  std::cout << '[';
  Table(agiru::platform::kAllObjTable);
  std::cout << ',';
  Table(agiru::platform::kAllObjWithCaptionTable);
  std::cout << "]\n";
}
