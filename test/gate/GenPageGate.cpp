#include "Ast.h"
#include "BodyWriter.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "PageWriter.h"
#include "Parser.h"

#include <string>

namespace {

void UserControlsArePageMembers() {
  const agiru::al::PageObject page = agiru::al::ParsePage(R"(page 50100 "Control Host"
{
    PageType = Card;
    layout
    {
        area(content)
        {
            usercontrol(Chart; BusinessChart)
            {
            }
        }
    }
})");
  const agiru::gen::PageHeader header =
      agiru::gen::WritePage(page, "Test/ControlHost.Page.al", agiru::gen::Objects{});
  CHECK_TRUE("a user control includes its explicit absent contract",
             header.text.find("#include \"absent/Types.h\"") != std::string::npos);
  CHECK_TRUE("CurrPage's user control is a typed page member",
             header.text.find("absent::BusinessChart Chart;") != std::string::npos);
  CHECK_TRUE("a user control contributes its type to the absent contract",
             header.absent.contains("BusinessChart"));
}

void PartNamesDoNotHidePageProcedures() {
  const agiru::al::PageObject page = agiru::al::ParsePage(R"(page 50101 "Collision Host"
{
    layout
    {
        area(content)
        {
            part("Dispatch Work"; Child) { }
            part("Dispatch-Work"; Child) { }
        }
    }
    procedure DispatchWork() begin end;
    procedure DispatchWork_2() begin end;
    procedure Invoke()
    begin
        DispatchWork();
        CurrPage."Dispatch Work".Page.Touch();
        CurrPage."Dispatch-Work".Page.Touch();
    end;
})");
  const auto names = agiru::gen::ControlIdentifiers(page);
  CHECK_TEXT("a part avoids both a procedure and an occupied suffix",
             agiru::gen::ControlIdentifier(names, "Dispatch Work"),
             "DispatchWork_3");
  CHECK_TEXT("a second normalized part name remains distinct",
             agiru::gen::ControlIdentifier(names, "Dispatch-Work"),
             "DispatchWork_4");
  agiru::gen::Objects objects;
  objects.pages["collision host"].fields = names;
  objects.pages["collision host"].parts = agiru::gen::PartPages(page);
  objects.pages["child"].identifier = "::agiru::Child_Page";
  objects.pages["child"].header = "Child.h";
  const auto header = agiru::gen::WritePage(page, "CollisionHost.Page.al", objects);
  CHECK_TRUE("the first part declaration uses the allocated name",
             header.text.contains("> DispatchWork_3;"));
  CHECK_TRUE("the second part declaration uses the allocated name",
             header.text.contains("> DispatchWork_4;"));
  CHECK_TRUE("part lookup preserves the original AL name",
             header.text.contains("name == \"Dispatch Work\"") &&
                 header.text.contains("&DispatchWork_3.Held()"));
  CHECK_TRUE("the procedure retains its own declaration",
             header.text.contains("void DispatchWork();"));
  const auto source = agiru::gen::WriteSource(page, "CollisionHost.Page.al", objects, nullptr);
  CHECK_TRUE("unqualified procedure calls retain procedure spelling",
             source.contains("DispatchWork();"));
  CHECK_TRUE("CurrPage selects the allocated first part",
             source.contains("DispatchWork_3.Page().Touch()"));
  CHECK_TRUE("CurrPage selects the allocated second part",
             source.contains("DispatchWork_4.Page().Touch()"));
}

}

int main() {
  return gate::Run("GenPage", [] {
    UserControlsArePageMembers();
    PartNamesDoNotHidePageProcedures();
  });
}
