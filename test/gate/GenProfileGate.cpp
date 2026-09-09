#include "Ast.h"
#include "CodeunitWriter.h"
#include "Parser.h"

#include "Check.h"

#include <string>

namespace {

/// A PROFILE IS ONE ROW OF `All Profile`, and the parser reads the whole object kind: a name, its
/// properties, and a `Customizations` block it steps over.
void TheParserReadsAProfile() {
  const std::string source = R"(profile "ORDER PROCESSOR"
{
    Caption = 'Sales Order Processor';
    ProfileDescription = 'Full functionality for order processors.';
    RoleCenter = 9006;
    Enabled = false;
    Promoted = true;
    Customizations = SomeCustomization;
})";
  const agiru::al::ProfileObject profile = agiru::al::ParseProfile(source);
  CHECK_TEXT("the name is the Profile ID", profile.name, "ORDER PROCESSOR");
  CHECK_TRUE("every property is read", profile.properties.size() == 6);
  CHECK_TEXT("the role centre is its number",
             agiru::al::Find(profile.properties, "RoleCenter")->text,
             "9006");
  // THE NEGATIVE CONTROL: a profile that names no role centre is still a profile; the transpiler
  // counts the unresolved one rather than refusing the object.
  const agiru::al::ProfileObject bare = agiru::al::ParseProfile(R"(profile "BARE" { })");
  CHECK_TRUE("a bare profile parses", bare.name == "BARE" && bare.properties.empty());
}

} // namespace

int main() {
  return gate::Run("GenProfile", [] { TheParserReadsAProfile(); });
}
