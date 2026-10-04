#include "meta/Ids.h"
#include "meta/ModuleDef.h"
#include "meta/TableDef.h"
#include "platform/TableMetadata.h"
#include "runtime/Catalogue.h"
#include "runtime/RecordRef.h"
#include "runtime/TableDefinition.h"
#include "type/Integer.h"

#include "Check.h"

#include <charconv>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>

namespace {

void OriginalDeclarations(const char *inventory, char **identity) {
  std::ifstream input(inventory);
  CHECK_TRUE("independent source inventory is readable", input.good());
  std::string line;
  int count = 0;
  while (std::getline(input, line)) {
    const auto first = line.find('\t');
    const auto second = line.find('\t', first + 1);
    CHECK_TRUE("original inventory retains namespace and name",
               first != std::string::npos && second != std::string::npos);
    if (first == std::string::npos || second == std::string::npos) { continue; }
    int number = 0;
    const auto parsed = std::from_chars(line.data(), line.data() + first, number);
    CHECK_TRUE("original numeric identity parses exactly",
               parsed.ec == std::errc{} && parsed.ptr == line.data() + first);
    const auto *entry = agiru::FindTable(agiru::TableId{number});
    CHECK_TRUE("original native declaration is installed",
               entry != nullptr && entry->sourceBinding != nullptr);
    if (entry == nullptr || entry->sourceBinding == nullptr) { continue; }
    ++count;
    CHECK_TEXT("original native namespace survives",
               entry->table->nameSpace,
               std::string_view(line).substr(first + 1, second - first - 1));
    CHECK_TEXT("original native AL name survives",
               entry->table->name,
               std::string_view(line).substr(second + 1));
    const auto *module = entry->table->module;
    CHECK_TRUE("original module is installed", module != nullptr);
    if (module != nullptr) {
      CHECK_TEXT("original module GUID survives", module->id, identity[0]);
      CHECK_TEXT("original module name survives", module->name, identity[1]);
      CHECK_TEXT("original module publisher survives", module->publisher, identity[2]);
      CHECK_TEXT("original module version survives", module->version, identity[3]);
    }
    agiru::RecordRef record;
    record.Open(agiru::Integer{number});
    CHECK_TRUE("type-erased factories use the same source declaration",
               record.TableDefinition() == entry->table);
    CHECK_TEXT(
        "type-erased caption consumes source metadata", record.Caption(), entry->table->caption);
  }
  CHECK_TRUE("original native declarations execute", count > 0);
  using Native = agiru::platform::TableMetadata;
  const auto *entry = agiru::FindTable(Native::kId);
  CHECK_TRUE("typed native record uses the installed source declaration",
             entry != nullptr && &agiru::TableDefinition<Native>() == entry->table);
  CHECK_TRUE("compile-time ABI is not relabelled as original-source ownership",
             agiru::TableTraits<Native>::kTable.module == nullptr);
}

}

int main(int argc, char **argv) {
  constexpr int kArguments = 6;
  return gate::Run("Original Native Sources", [&] {
    CHECK_TRUE("source inventory and original module identity are explicit", argc == kArguments);
    if (argc == kArguments) { OriginalDeclarations(argv[1], argv + 2); }
  });
}
