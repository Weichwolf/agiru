#include "Ast.h"
#include "BodyWriter.h"
#include "CodeunitWriter.h"
#include "Parser.h"
#include <cstdio>
int main() {
  agiru::gen::Objects objects;
  objects.tables = agiru::gen::PlatformTables();
  objects.tables["system.reflection.field"] = objects.tables.at("field");
  for (const char *alias : {"Field", "2000000041", "System.Reflection.Field"}) {
    auto page = agiru::al::ParsePage("page 50183 Fixture { SourceTable = " + std::string(alias) + R"(;
      var Saved: Integer; Temporary: Boolean; Caption: Text;
      trigger OnOpenPage() begin
        Saved := Rec.FilterGroup; Rec.FilterGroup := 2; Rec.FilterGroup := Saved;
        Saved := FilterGroup; FilterGroup := 3; Saved := xRec.FilterGroup;
        Saved := Rec.Count; Temporary := Rec.IsTemporary; Caption := Rec.TableCaption;
        Caption := Rec.TableName; Caption := TableName;
      end;
    })");
    for (const auto &property : page.properties) {
      std::printf("PROPERTY %s [%s]\n", property.name.c_str(), property.text.c_str());
    }
    std::printf("%s\n%s\n", alias, agiru::gen::WriteSource(page, "Fixture.Page.al", objects, nullptr).c_str());
  }
}
