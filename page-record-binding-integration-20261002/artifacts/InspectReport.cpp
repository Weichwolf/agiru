#include "BodyWriter.h"
#include "CodeunitWriter.h"
#include "Parser.h"
#include "PageWriter.h"

#include <cstdio>
#include <exception>

int main() {
  try {
    agiru::gen::Objects objects;
    objects.tables = agiru::gen::PlatformTables();
    objects.fieldEnums = agiru::gen::PlatformFieldEnums();
    auto report = agiru::al::ParseReport(R"(report 50181 Fixture {
      dataset { dataitem(Fields; Field) {
        trigger OnAfterGetRecord() begin
          Rec.Class := Rec.Class::FlowFilter;
          Rec.Class := Fields.Class::Normal;
        end;
      } }
    })");
    agiru::gen::PrepareReport(report);
    const auto body = agiru::gen::WriteSource(report, "Fixture.Report.al", objects, nullptr);
    std::fputs(body.c_str(), stdout);
    return body.contains("RefusedOption") ? 1 : 0;
  } catch (const std::exception &error) {
    std::fputs(error.what(), stderr);
    return 2;
  }
}
