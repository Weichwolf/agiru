#include "meta/Declare.h"
#include "platform/PageTableField.h"

#include <iomanip>
#include <iostream>

int main() {
  using Row = agiru::platform::PageTableField;
  const auto &table = agiru::TableTraits<Row>::kTable;
  std::cout << table.id.Value() << '\t' << std::quoted(table.name) << '\t'
            << table.dataPerCompany << '\t' << std::quoted(Row::kScope) << '\n';
  for (const auto &field : table.fields) {
    if (field.no.Value() >= agiru::kSystemFields.front().no.Value()) { continue; }
    std::cout << "field\t" << field.no.Value() << '\t' << std::quoted(field.name)
              << '\t' << std::quoted(field.caption) << '\t' << static_cast<int>(field.type)
              << '\t' << field.length << '\t' << std::quoted(field.optionOrdinalValues)
              << '\t' << std::quoted(field.obsoleteState)
              << '\t' << std::quoted(field.obsoleteReason) << '\n';
    for (const auto &value : field.values) {
      std::cout << "option\t" << field.no.Value() << '\t' << value.ordinal << '\t'
                << std::quoted(value.name) << '\t' << std::quoted(value.caption) << '\n';
    }
  }
  for (const auto &key : table.keys) {
    std::cout << "key\t" << std::quoted(key.name) << '\t' << key.clustered;
    for (const auto no : key.fields) { std::cout << '\t' << no.Value(); }
    std::cout << '\n';
  }
  std::cout << "Brick";
  for (const auto no : Row::kBrick) { std::cout << '\t' << no.Value(); }
  std::cout << '\n';
}
