#include "Ast.h"
#include "Check.h"
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

}

int main() {
  return gate::Run("GenPage", [] { UserControlsArePageMembers(); });
}
