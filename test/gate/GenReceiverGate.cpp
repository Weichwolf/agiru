#include "Ast.h"
#include "BodyWriter.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "ObjectKind.h"
#include "Parser.h"
#include "RuntimeSurface.h"
#include "TableWriter.h"

#include <initializer_list>
#include <string>

namespace {

void NativeAliasesRemainRebuiltTypes() {
  const auto &types = agiru::gen::RebuiltDotNet();
  agiru::al::VarDecl declaration;
  declaration.type = "DotNet";
  for (const auto *name : {"ArrayList",
                           "JArray",
                           "JObject",
                           "JProperty",
                           "JValue",
                           "BinaryReader",
                           "BinaryWriter",
                           "CultureInfo",
                           "String",
                           "Regex",
                           "TimeSpan",
                           "Encoding",
                           "UTF8Encoding",
                           "UnicodeEncoding",
                           "ASCIIEncoding",
                           "StreamReader",
                           "Uri",
                           "UriBuilder",
                           "DataColumn",
                           "DataTable",
                           "BusinessChartData",
                           "Queue",
                           "XmlDocument",
                           "XmlNamespaceManager",
                           "StringReader",
                           "XmlReaderSettings",
                           "XmlTextReader"}) {
    declaration.subtype = name;
    CHECK_TRUE("an AL type alias remains implemented", types.contains(name));
    CHECK_TRUE("the alias does not emit an absent CLR stub",
               agiru::gen::AbsentDotNetOf(declaration).empty());
  }
  declaration.subtype = "MissingClrFixture";
  CHECK_TRUE("an undeclared CLR type still refuses",
             agiru::gen::AbsentDotNetOf(declaration) == "MissingClrFixture");
}

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
    var
      GlobalPage: TestPage Fixture;
      GlobalReader: DotNet StreamReader;
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
      if GlobalReader.EndOfStream then
        Number := 1;
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
  CHECK_TRUE("global CLR properties remain getters", body.contains("GlobalReader.EndOfStream()"));
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

void ConversionResultNamesItsArrayDependency() {
  const auto complete =
      agiru::gen::RuntimeIncludes("dotnet::Convert Converter; Converter.FromBase64String(Text);",
                                  agiru::gen::ObjectKind::Codeunit);
  CHECK_TRUE("Convert return-value consumers name the complete Array dependency",
             complete.contains("#include \"dotnet/Regex.h\""));
  const auto numeric = agiru::gen::RuntimeIncludes(
      "dotnet::Convert Converter; Converter.ToInt32(1);", agiru::gen::ObjectKind::Codeunit);
  CHECK_TRUE("numeric refusals do not widen to the Array implementation",
             !numeric.contains("#include \"dotnet/Regex.h\""));
  const auto unrelated = agiru::gen::RuntimeIncludes("Local.FromBase64String(Text);",
                                                     agiru::gen::ObjectKind::Codeunit);
  CHECK_TRUE("an unrelated procedure name does not add a CLR Array dependency",
             !unrelated.contains("#include \"dotnet/Regex.h\""));
}

void QualifiedConversionNamesRemainHiddenByRuntimeMembers() {
  CHECK_TRUE("namespace-qualified Boolean conversion still hides the AL type name",
             agiru::gen::HiddenByABaseMember("Boolean"));
  CHECK_TRUE("an unrelated name does not gain qualification",
             !agiru::gen::HiddenByABaseMember("NoRuntimeMemberFixture"));
  CHECK_TRUE("a qualified function-pointer result is not a member name",
             !agiru::gen::HiddenByABaseMember("Action"));
  for (const auto *spelling : {"agiru::Boolean Flag;", "::agiru::Boolean Flag;"}) {
    CHECK_TRUE("runtime-qualified scalar names retain their direct provider",
               agiru::gen::RuntimeIncludes(spelling, agiru::gen::ObjectKind::Codeunit)
                   .contains("#include \"type/Boolean.h\""));
  }
  CHECK_TRUE("identifier substrings do not introduce scalar dependencies",
             !agiru::gen::RuntimeIncludes("BooleanCarrier Value;", agiru::gen::ObjectKind::Codeunit)
                  .contains("#include \"type/Boolean.h\""));
  CHECK_TRUE("a caller's unqualified scalar name does not introduce a runtime dependency",
             !agiru::gen::RuntimeIncludes("Boolean Flag;", agiru::gen::ObjectKind::Codeunit)
                  .contains("#include \"type/Boolean.h\""));
}

void RecordRefreshUsesItsRuntimeProvider() {
  const auto unit = agiru::al::ParseCodeunit(R"(codeunit 50272 "Refresh Caller" {
    procedure Refresh(TableNumber: Integer)
    begin
      SelectLatestVersion();
      SelectLatestVersion(TableNumber);
      Database.SelectLatestVersion();
      Database.SelectLatestVersion(TableNumber);
    end;
  })");
  const auto source = agiru::gen::WriteCodeunitSource(unit, "RefreshCaller.Codeunit.al", {});
  CHECK_TRUE("both refresh overloads name the narrow runtime provider",
             source.contains("#include \"runtime/RecordRefresh.h\""));
  CHECK_TRUE("AL refresh calls do not bind a SQL connection member",
             !source.contains("Database.SelectLatestVersion"));
  CHECK_TRUE("the AL table argument survives native refresh lowering",
             source.contains("SelectLatestVersion(TableNumber)"));
  CHECK_TRUE("the AL parameterless refresh survives native lowering",
             source.contains("SelectLatestVersion()"));
}

void RandomOverloadsRetainTheirArguments() {
  const auto unit = agiru::al::ParseCodeunit(R"(codeunit 50273 "Random Caller" {
    procedure Draw(): Integer
    begin
      Randomize();
      Randomize(0);
      System.Randomize();
      System.Randomize(11);
      exit(Random(-100));
    end;
  })");
  const auto source = agiru::gen::WriteCodeunitSource(unit, "RandomCaller.Codeunit.al", {});
  CHECK_TRUE("an omitted AL random seed stays omitted rather than becoming zero",
             source.contains("Randomize()"));
  CHECK_TRUE("an explicit AL zero random seed stays explicit", source.contains("Randomize(0)"));
  CHECK_TRUE("a qualified explicit AL random seed survives lowering",
             source.contains("Randomize(11)"));
  CHECK_TRUE("AL random bounds retain their sign", source.contains("Random(-100)"));
}

}

int main() {
  return gate::Run("GenReceiver", [] {
    NativeAliasesRemainRebuiltTypes();
    DeclarationsTakePrecedenceOverGetterNames();
    HeadersAreNotInsertedTwice();
    ConversionResultNamesItsArrayDependency();
    QualifiedConversionNamesRemainHiddenByRuntimeMembers();
    RecordRefreshUsesItsRuntimeProvider();
    RandomOverloadsRetainTheirArguments();
  });
}
