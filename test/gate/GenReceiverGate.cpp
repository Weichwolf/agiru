#include "Ast.h"
#include "BodyWriter.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "Parser.h"
#include "RuntimeSurface.h"
#include "Scope.h"
#include "TableWriter.h"

#include <string>

namespace {

void DeclarationsTakePrecedenceOverGetterNames() {
  agiru::gen::Objects objects;
  objects.pages["fixture"].fields["description"] = "Description";
  objects.reports["fixture"].fields["description"] = "Description";
  objects.queries["fixture"].fields["count"] = "Count";
  const auto row = agiru::al::ParseTable(R"(table 50270 Row {
    fields { field(1; Description; Text[100]) {} }
  })");
  auto &binding = objects.tables["row"];
  binding.id = row.id;
  binding.name = row.name;
  binding.identifier = "::fixture::Row";
  binding.header = "fixture/Row.h";
  binding.fields["description"] = agiru::gen::FieldIdentifier(row, "Description");
  const auto table = agiru::al::ParseTable(R"(table 50271 Caller {
    fields { field(1; Stored; Text[100]) {} }
    var GlobalPage: TestPage Fixture;
    procedure Read(ParameterPage: TestPage Fixture)
    var
      LocalPage: TestPage Fixture;
      RequestPage: TestRequestPage Fixture;
      OrdinaryPage: Page Fixture;
      Rows: Query Fixture;
      Row: Record Row;
      Property: DotNet DesignerFieldProperty;
      Number: Integer;
    begin
      Stored := LocalPage.Description.Value();
      Stored := RequestPage.Description.Value();
      Stored := GlobalPage.Description.Value();
      Stored := ParameterPage.Description.Value();
      Stored := OrdinaryPage.Description;
      Number := Rows.Count;
      Stored := Row.Description;
      Number := Property.Description;
    end;
  })");
  const auto body = agiru::gen::WriteSource(table, "Caller.Table.al", objects);
  for (const auto *name : {"LocalPage", "RequestPage", "GlobalPage", "ParameterPage"}) {
    CHECK_TRUE("page controls remain fields despite a CLR getter with the same name",
               body.contains(std::string(name) + ".Description.Value()"));
  }
  CHECK_TRUE("ordinary page controls retain their receiver binding",
             body.contains("OrdinaryPage.Description;"));
  CHECK_TRUE("query columns are not primitive methods", body.contains("Rows.Count;"));
  CHECK_TRUE("record fields are not CLR getters", body.contains("Row.Description;"));
  CHECK_TRUE("the actual CLR property remains a getter", body.contains("Property.Description()"));
}

void HeadersAreNotInsertedTwice() {
  const std::string declared = "#include \"dotnet/Generic.h\"\nGenericDictionary2 Value;\n";
  CHECK_TRUE("declared native headers are not inserted again",
             !agiru::gen::RuntimeIncludes(declared, agiru::gen::ObjectKind::Codeunit)
                  .contains("#include \"dotnet/Generic.h\""));
  CHECK_TRUE(
      "a missing native header remains required",
      agiru::gen::RuntimeIncludes("GenericDictionary2 Value;\n", agiru::gen::ObjectKind::Codeunit)
          .contains("#include \"dotnet/Generic.h\""));
}

}

int main() {
  return gate::Run("GenReceiver", [] {
    DeclarationsTakePrecedenceOverGetterNames();
    HeadersAreNotInsertedTwice();
  });
}
