#include "Check.h"
#include "Names.h"
#include "Scope.h"

#include <string>

using agiru::gen::DeclarationOf;
using agiru::gen::ObjectDeclaration;
using agiru::gen::ObjectKind;

namespace {

/// THE DECLARATION MAY BE THE FIRST LINE OF THE FILE, and 5 387 of BCApps' 14 282 codeunit files
/// begin with it -- 38 %, measured 2026-09-02. A search for "\ncodeunit " alone found none of them,
/// and an object that is not indexed is not RESOLVED: `Codeunit "Library - Lower Permissions"` then
/// emitted a bare AL name, which is where 129 of one run's unresolved declarations came from.
void ADeclarationOnTheFirstLineIsFound() {
  const ObjectDeclaration first = DeclarationOf(
      "codeunit 132217 \"Library - Lower Permissions\"\n{\n}\n", ObjectKind::Codeunit);
  CHECK_TRUE("a file that opens with its declaration declares something", first.found);
  CHECK_TEXT("and the AL name is kept whole", first.name, "Library - Lower Permissions");
  CHECK_TEXT("with no namespace, because none can stand above line one", first.nameSpace, "");

  // THE NEGATIVE CONTROL: the usual shape must keep working, or the fix above would be a swap
  // rather than a widening. Most BC files open with a copyright block.
  const ObjectDeclaration after = DeclarationOf(
      "// Copyright\n\nnamespace Microsoft.Sales.Document;\n\ncodeunit 80 \"Sales-Post\"\n{\n}\n",
      ObjectKind::Codeunit);
  CHECK_TRUE("a declaration below a comment is still found", after.found);
  CHECK_TEXT("with its name", after.name, "Sales-Post");
  CHECK_TEXT("and the namespace above it", after.nameSpace, "Microsoft.Sales.Document");
}

void AnUnquotedNameAndAMissingOneAreBothAnswered() {
  const ObjectDeclaration bare =
      DeclarationOf("codeunit 50000 SomeThing\n{\n}\n", ObjectKind::Codeunit);
  CHECK_TRUE("an unquoted name is read", bare.found);
  CHECK_TEXT("as itself", bare.name, "SomeThing");

  CHECK_TRUE("a file declaring nothing of that kind answers false",
             !DeclarationOf("table 18 Customer\n{\n}\n", ObjectKind::Codeunit).found);
  CHECK_TRUE("and so does an empty one", !DeclarationOf("", ObjectKind::Codeunit).found);

  // A NAMESPACE BELOW THE OBJECT BELONGS TO NOTHING THIS FILE DECLARES. Taking it would put the
  // generated header in a directory the object does not live in, which is a wrong include path in
  // every file that names it.
  const ObjectDeclaration below =
      DeclarationOf("codeunit 50000 SomeThing\n{\n}\nnamespace Later;\n", ObjectKind::Codeunit);
  CHECK_TEXT("a namespace after the declaration is not taken", below.nameSpace, "");
}

/// THE KEYWORD IS AN ARGUMENT because AL declares five kinds of object the same way, and an index
/// that could only find codeunits would have to be written again for each of the others.
void ItReadsWhicheverKindItIsAsked() {
  const std::string source = "namespace Microsoft.Foundation;\n\ntable 27 Item\n{\n}\n";
  CHECK_TEXT("a table is read the same way", DeclarationOf(source, ObjectKind::Table).name, "Item");
  CHECK_TRUE("and asking for another kind finds nothing",
             !DeclarationOf(source, ObjectKind::Codeunit).found);
}

} // namespace

int main() {
  return gate::Run("GenNamesDeclaration", [] {
    ADeclarationOnTheFirstLineIsFound();
    AnUnquotedNameAndAMissingOneAreBothAnswered();
    ItReadsWhicheverKindItIsAsked();
  });
}
