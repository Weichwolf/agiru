#include "meta/SystemFields.h"
#include "meta/TableType.h"

#include "Check.h"
#include "CodeunitWriter.h"
#include "EnumWriter.h"
#include "Parser.h"
#include "TableWriter.h"

#include <array>
#include <exception>
#include <string>
#include <string_view>
#include <utility>

namespace {

void SelectedDeclarations() {
  const std::array kinds{
      std::pair{std::string_view{"Normal"}, agiru::TableType::Normal},
      std::pair{std::string_view{"CRM"}, agiru::TableType::CRM},
      std::pair{std::string_view{"CDS"}, agiru::TableType::CDS},
      std::pair{std::string_view{"ExternalSQL"}, agiru::TableType::ExternalSQL},
      std::pair{std::string_view{"Exchange"}, agiru::TableType::Exchange},
      std::pair{std::string_view{"MicrosoftGraph"}, agiru::TableType::MicrosoftGraph},
      std::pair{std::string_view{"Temporary"}, agiru::TableType::Temporary}};
  for (const auto profile :
       {agiru::SystemFieldProfile::Runtime17, agiru::SystemFieldProfile::Runtime18}) {
    for (const auto &[name, kind] : kinds) {
      for (const bool linked : {false, true}) {
        const auto table =
            agiru::al::ParseTable("table 50198 Probe { TableType = " + std::string(name) +
                                  "; LinkedObject = " + (linked ? "true" : "false") +
                                  "; fields { field(1; ID; Integer) {} } }");
        agiru::gen::Objects objects;
        objects.hostProfile = profile;
        const auto header = agiru::gen::WriteHeader(table, "Probe.Table.al", {}, objects).text;
        const auto definitions = agiru::gen::TableDefinitions(table, objects);
        const auto binding = agiru::gen::BindTable(table, "::fixture::Probe", "Probe.h", profile);
        const auto assertions = agiru::gen::NativeTableAssertions(table, binding);
        CHECK_TRUE("selected records carry a physical timestamp member",
                   header.contains("BigInteger SystemRowVersion{};"));
        CHECK_TRUE("selected definitions use the complete materializer",
                   definitions.contains("WithImplicitFields<"));
        for (const auto &field : agiru::kImplicitSystemFields) {
          const bool present = agiru::IncludesSystemField(field, profile, kind, linked);
          CHECK_TRUE("the source binder selects the same implicit population",
                     binding.fields.contains(agiru::gen::LowerKey(std::string(field.name))) ==
                         present);
          CHECK_TRUE("the record storage selects the same implicit population",
                     header.contains(" " + std::string(field.name) + "{};") == present);
          CHECK_TRUE("native offset contracts select the same implicit population",
                     assertions.contains("offsetof(::fixture::Probe, " + std::string(field.name) +
                                         ")") == present);
        }
        CHECK_TRUE(
            "native effective counts include only the selected implicit population",
            assertions.contains(".fields.size() == " +
                                std::to_string(table.fields.size() +
                                               agiru::ImplicitFieldCount(profile, kind, linked))));
        CHECK_TRUE("native contracts preserve the readonly timestamp role",
                   assertions.contains("field->sqlTimestamp == true && field->editable == false"));
      }
    }
  }
}

void BindingMismatch() {
  const auto table =
      agiru::al::ParseTable("table 50198 Probe { fields { field(1; ID; Integer) {} } }");
  const auto binding = agiru::gen::BindTable(
      table, "::fixture::Probe", "Probe.h", agiru::SystemFieldProfile::Runtime17);
  agiru::gen::Objects objects;
  objects.module = "::fixture::kModule";
  objects.moduleHeader = "Module.h";
  objects.hostProfile = agiru::SystemFieldProfile::Runtime18;
  bool refused = false;
  try {
    static_cast<void>(agiru::gen::NativeTableDefinition(table, binding, objects));
  } catch (const std::exception &error) {
    refused = std::string_view(error.what()).contains("host profiles differ");
  }
  CHECK_TRUE("a native runtime-17 binding cannot silently emit runtime-18 declarations", refused);
  objects.hostProfile.reset();
  refused = false;
  try {
    static_cast<void>(agiru::gen::NativeTableDefinition(table, binding, objects));
  } catch (const std::exception &error) {
    refused = std::string_view(error.what()).contains("host profiles differ");
  }
  CHECK_TRUE("omitting the output profile cannot silently downgrade an explicit binding", refused);
}

}

int main() {
  return gate::Run("GenSystemProfile", [] {
    SelectedDeclarations();
    BindingMismatch();
  });
}
