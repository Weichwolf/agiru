#include "BodyWriter.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "Format.h"
#include "NativeSource.h"
#include "Parser.h"
#include "Refused.h"
#include "TableWriter.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr std::string_view kAlPath = "Projects/Resources/Pricing/ResourceCost.Table.al";

std::string Read(const std::filesystem::path &path) {
  const std::ifstream file(path);
  if (!file) { throw std::runtime_error("cannot read " + path.string()); }
  std::ostringstream text;
  text << file.rdbuf();
  return text.str();
}

std::vector<std::string> Lines(const std::string &text) {
  std::vector<std::string> lines;
  std::istringstream stream(text);
  std::string line;
  while (std::getline(stream, line)) { lines.push_back(line); }
  return lines;
}

/// THE PROOF OF THE WHOLE ROUTE: what the generator writes for table 202 and what was written by
/// hand as the specification are the SAME FILE. Everything before this was a claim about what the
/// output should look like; this is the claim being met.
void TheGeneratorReproducesTheTargetImage() {
  // The generator emits canonical text and the tree's own formatter decides the layout. That is
  // not a weakening of "the same file": clang-format is deterministic, it is already required by
  // `make lint`, and the alternative is an emitter that reimplements a line-wrapping algorithm and
  // drifts from it.
  const std::string generated = agiru::gen::Formatted(agiru::gen::FormatRequest{
      .source = agiru::gen::WriteHeader(
                    agiru::al::ParseTable(Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath)),
                    std::string(kAlPath),
                    agiru::gen::EnumIndex{},
                    {})
                    .text,
      .stylePath = std::string(AGIRU_SOURCE_DIR) + "/.clang-format",
      .assumedName = "ResourceCost.h"});
  const std::string target =
      Read(std::filesystem::path(AGIRU_SOURCE_DIR) / "test/transpiler/golden/ResourceCost.h");

  // The generated file is left on disk beside the failure, because a line number is not enough to
  // repair an emitter -- the whole output is.
  {
    std::ofstream dump("/tmp/agiru-generated-ResourceCost.h");
    dump << generated;
  }

  const std::vector<std::string> left = Lines(generated);
  const std::vector<std::string> right = Lines(target);

  // Report the FIRST difference rather than a bare "not equal": a diff of two whole files tells a
  // reader nothing they can act on.
  const std::size_t shared = left.size() < right.size() ? left.size() : right.size();
  for (std::size_t i = 0; i < shared; ++i) {
    if (left[i] != right[i]) {
      CHECK_TEXT("the generated header matches the target image, line " + std::to_string(i + 1),
                 left[i],
                 right[i]);
      return;
    }
  }
  CHECK_TRUE("an AL DateTime does not include the unrelated .NET DateTime",
             generated.find("dotnet/DateTime.h") == std::string::npos);
  CHECK_TRUE("the generated header has as many lines as the target image",
             left.size() == right.size());
  if (left.size() != right.size()) {
    CHECK_TEXT("the first line that only one of them has",
               left.size() > right.size() ? left[shared] : std::string("<end of file>"),
               right.size() > left.size() ? right[shared] : std::string("<end of file>"));
  }
}

/// The other half: the trigger bodies, which are AL statements rather than declarations.
void TheGeneratorReproducesTheTriggerBodies() {
  const std::string generated = agiru::gen::Formatted(agiru::gen::FormatRequest{
      .source = agiru::gen::WriteSource(
          agiru::al::ParseTable(Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath)),
          std::string(kAlPath),
          {}),
      .stylePath = std::string(AGIRU_SOURCE_DIR) + "/.clang-format",
      .assumedName = "ResourceCost.cpp"});
  const std::string target =
      Read(std::filesystem::path(AGIRU_SOURCE_DIR) / "test/transpiler/golden/ResourceCost.cpp");

  {
    std::ofstream dump("/tmp/agiru-generated-ResourceCost.cpp");
    dump << generated;
  }

  const std::vector<std::string> left = Lines(generated);
  const std::vector<std::string> right = Lines(target);
  const std::size_t shared = left.size() < right.size() ? left.size() : right.size();
  for (std::size_t i = 0; i < shared; ++i) {
    if (left[i] != right[i]) {
      CHECK_TEXT("the generated source matches the target image, line " + std::to_string(i + 1),
                 left[i],
                 right[i]);
      return;
    }
  }
  CHECK_TRUE("the generated source has as many lines as the target image",
             left.size() == right.size());
}

/// THE DEFINITION IS A UNIT OF ITS OWN. The field and key tables are READ as an object by every
/// page and codeunit that names the table, so they have to exist whenever the header does -- and a
/// body that does not compile yet must not take them down with it (board:0634).
void TheGeneratorReproducesTheDefinitionsUnit() {
  const std::string generated = agiru::gen::Formatted(agiru::gen::FormatRequest{
      .source = agiru::gen::WriteDefinitions(
          agiru::al::ParseTable(Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath)),
          std::string(kAlPath),
          {}),
      .stylePath = std::string(AGIRU_SOURCE_DIR) + "/.clang-format",
      .assumedName = "ResourceCost.def.cpp"});
  const std::string target =
      Read(std::filesystem::path(AGIRU_SOURCE_DIR) / "test/transpiler/golden/ResourceCost.def.cpp");

  {
    std::ofstream dump("/tmp/agiru-generated-ResourceCost.def.cpp");
    dump << generated;
  }

  const std::vector<std::string> left = Lines(generated);
  const std::vector<std::string> right = Lines(target);
  const std::size_t shared = left.size() < right.size() ? left.size() : right.size();
  for (std::size_t i = 0; i < shared; ++i) {
    if (left[i] != right[i]) {
      CHECK_TEXT("the generated definitions unit matches the target image, line " +
                     std::to_string(i + 1),
                 left[i],
                 right[i]);
      return;
    }
  }
  CHECK_TRUE("the generated definitions unit has as many lines as the target image",
             left.size() == right.size());
  CHECK_TRUE("and the bodies' unit no longer carries the field table",
             agiru::gen::WriteSource(
                 agiru::al::ParseTable(Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath)),
                 std::string(kAlPath),
                 {})
                     .find("kResourceCostFields") == std::string::npos);
}

/// THE NEGATIVE CONTROL. A comparison that only ever passes proves nothing: what makes the identity
/// above meaningful is that a changed `.al` produces a correspondingly changed header. The source
/// is altered in memory -- the repository under ~/Git/BCApps is never written to.
void AChangedSourceChangesTheOutput() {
  const std::string original = Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath);

  std::string widened = original;
  const std::size_t at = widened.find("Code[20]");
  CHECK_TRUE("the source declares Code[20] to widen", at != std::string::npos);
  widened.replace(at, std::string("Code[20]").size(), "Code[30]");

  const std::string generated =
      agiru::gen::WriteHeader(agiru::al::ParseTable(widened), std::string(kAlPath), {}, {}).text;
  CHECK_TRUE("a widened field widens its member",
             generated.find("::agiru::Code<30> Code{};") != std::string::npos);
  CHECK_TRUE("and the old width is gone",
             generated.find("::agiru::Code<20> Code{};") == std::string::npos);

  std::string renamed = original;
  const std::size_t nameAt = renamed.find("\"Work Type Code\"");
  CHECK_TRUE("the source declares the field to rename", nameAt != std::string::npos);
  renamed.replace(nameAt, std::string("\"Work Type Code\"").size(), "\"Work Kind Code\"");

  const std::string afterRename =
      agiru::gen::WriteHeader(agiru::al::ParseTable(renamed), std::string(kAlPath), {}, {}).text;
  CHECK_TRUE("a renamed field renames its member and its number",
             afterRename.find("WorkKindCode{};") != std::string::npos &&
                 afterRename.find("FieldNo WorkKindCode{3}") != std::string::npos);
  CHECK_TRUE("and the AL name follows it into the field table",
             agiru::gen::WriteDefinitions(agiru::al::ParseTable(renamed), std::string(kAlPath), {})
                     .find("\"Work Kind Code\"") != std::string::npos);
}

/// The negative control for the bodies. A statement translator that emitted a constant would pass
/// the identity above just as well, so a changed STATEMENT has to change the C++.
void AChangedStatementChangesTheBody() {
  const std::string original = Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath);

  std::string flipped = original;
  const std::size_t at = flipped.find("(Code <> '')");
  CHECK_TRUE("the trigger compares the code against the empty string", at != std::string::npos);
  flipped.replace(at, std::string("(Code <> '')").size(), "(Code = '')");

  const std::string generated =
      agiru::gen::WriteSource(agiru::al::ParseTable(flipped), std::string(kAlPath), {});
  CHECK_TRUE("a flipped comparison flips the operator",
             generated.find("Code == \"\"") != std::string::npos);
  CHECK_TRUE("and the old one is gone", generated.find("Code != \"\"") == std::string::npos);

  const std::string untouched =
      agiru::gen::WriteSource(agiru::al::ParseTable(original), std::string(kAlPath), {});
  CHECK_TRUE("AL conjunction owns both ordered Boolean values",
             untouched.find("::agiru::LogicalAnd({.left = static_cast<bool>(Code != \"\"), "
                            ".right = static_cast<bool>(Type ==") != std::string::npos);
  CHECK_TRUE("and no AL operator survives",
             untouched.find(" and ") == std::string::npos &&
                 untouched.find(" <> ") == std::string::npos);
  CHECK_TRUE("AL conjunction never uses C++ short circuit",
             untouched.find(" && ") == std::string::npos);
}

/// A FIELD NAME THAT COLLIDES WITH A RUNTIME TYPE. `Change Log Setup (Field)` really does declare a
/// field called `Field No.`, and its member is spelled exactly like `agiru::FieldNo` -- so from
/// that member onward the class's own name wins and every Field_No entry below it fails to
/// compile. The source is altered in memory; the repository under ~/Git/BCApps is never written to.
void AFieldThatShadowsARuntimeTypeStillCompiles() {
  const auto reserved = agiru::al::ParseTable(R"(table 90003 Reserved {
    fields { field(1; Field_No; Integer) { } field(2; State_Block; Integer) { } }
    keys { key(PK; Field_No) { } }
  })");
  const std::string reservedHeader =
      agiru::gen::WriteHeader(reserved, "Reserved.Table.al", {}, {}).text;
  CHECK_TRUE("a literal underscore cannot collide with generated field numbers",
             reservedHeader.find("Field_No_1{};") != std::string::npos);
  CHECK_TRUE("a field cannot replace the runtime state handle",
             reservedHeader.find("State_Block_2{};") != std::string::npos);

  const std::string original = Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath);

  std::string collided = original;
  const std::size_t at = collided.find("\"Work Type Code\"");
  CHECK_TRUE("the source declares the field to rename", at != std::string::npos);
  collided.replace(at, std::string("\"Work Type Code\"").size(), "\"Field No.\"");

  const std::string generated =
      agiru::gen::WriteHeader(agiru::al::ParseTable(collided), std::string(kAlPath), {}, {}).text;
  CHECK_TRUE("the field takes the name AL gave it",
             generated.find("FieldNo_3{};") != std::string::npos);
  CHECK_TRUE("and the field numbers reach past it to the runtime type",
             generated.find("static constexpr ::agiru::FieldNo Code{") != std::string::npos);
  // AND IT IS QUALIFIED EVEN WITHOUT A FIELD OF THAT NAME, because the BASE CLASS hides it:
  // `Table<Derived>` carries AL's own `Record.FieldNo(Field)`, and a member declared anywhere in a
  // class hides a namespace name for the whole class body. The negative control is therefore not
  // "unqualified elsewhere" but "the field itself still takes AL's spelling", which the check above
  // states.
  CHECK_TRUE("and a table with no such field qualifies it too, because the base class hides it",
             agiru::gen::WriteHeader(agiru::al::ParseTable(original), std::string(kAlPath), {}, {})
                     .text.find("static constexpr ::agiru::FieldNo Code{") != std::string::npos);
}

/// A RECORD VARIABLE CARRIES ITS FIELDS' `InitValue` BEFORE ANYTHING TOUCHES IT
/// (`devenv-initvalue-property.md`): `Item Jnl.-Post Line` rounds by a `Currency` global it never
/// loads, and the platform answers the field's `0.00001`. A table with any `InitValue` therefore
/// declares a constructor that applies them; one without declares none, which is the control.
void ATableWithAnInitValueConstructsWithIt() {
  const std::string original = Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath);
  CHECK_TRUE("the target table declares no InitValue, so no constructor",
             agiru::gen::WriteHeader(agiru::al::ParseTable(original), std::string(kAlPath), {}, {})
                     .text.find("ResourceCost_Table();") == std::string::npos);
  std::string valued = original;
  const std::size_t at = valued.find("field(5; \"Direct Unit Cost\"; Decimal)");
  CHECK_TRUE("the source declares the field to give a value", at != std::string::npos);
  const std::size_t brace = valued.find('{', at);
  valued.insert(brace + 1, "\n            InitValue = 0.5;");
  const std::string generated =
      agiru::gen::WriteHeader(agiru::al::ParseTable(valued), std::string(kAlPath), {}, {}).text;
  CHECK_TRUE("with one, the class declares its constructor",
             generated.find("  ResourceCost_Table();") != std::string::npos);
  CHECK_TRUE(
      "and defines it over the runtime's init values after the traits",
      generated.find("::ResourceCost_Table() {\n  ::agiru::detail::RuntimeInitValues(this, ") !=
          std::string::npos);
}

/// A TABLE DECLARED `TableType = Temporary` IS IN MEMORY IN EVERY VARIABLE.
/// `devenv-tabletype-property.md` calls it "an in-memory table used to store temporary data", and
/// 99 BaseApp tables declare it -- `Duplicate Price Line` among them, whose page `Set` does
/// `Rec.Copy(DuplicatePriceLine, true)` on two plain record variables and was refused as "neither
/// record is temporary" (8 cases of `Suggest Price Lines UT`, 2026-09-10). So the generated class
/// constructs its own store, the way `Temporary<T>` does, and the declaration decides rather than
/// the variable.
void ATableDeclaredTemporaryConstructsItsStore() {
  const std::string original = Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath);
  CHECK_TRUE("the target table is a database table, so no constructor",
             agiru::gen::WriteHeader(agiru::al::ParseTable(original), std::string(kAlPath), {}, {})
                     .text.find("ResourceCost_Table();") == std::string::npos);
  std::string declared = original;
  const std::size_t at = declared.find("Caption = 'Resource Cost';");
  CHECK_TRUE("the source carries the caption the property is placed beside",
             at != std::string::npos);
  declared.insert(at, "TableType = Temporary;\n    ");
  const std::string generated =
      agiru::gen::WriteHeader(agiru::al::ParseTable(declared), std::string(kAlPath), {}, {}).text;
  CHECK_TRUE("declared temporary, the class declares its constructor",
             generated.find("  ResourceCost_Table();") != std::string::npos);
  CHECK_TRUE(
      "and defines it over the runtime's temporary store after the traits",
      generated.find("::ResourceCost_Table() {\n  ::agiru::detail::RuntimeMakeTemporary(this, "
                     "&::agiru::kTempOps<") != std::string::npos);
  CHECK_TRUE("and the traits say so",
             agiru::gen::WriteDefinitions(agiru::al::ParseTable(declared), std::string(kAlPath), {})
                     .find(".tableType = ::agiru::TableType::Temporary,") != std::string::npos);
}

/// A FIELD'S `OnLookup` TRIGGER IS IN THE MAP BESIDE ITS TABLE, like `OnValidate`: the method was
/// always emitted and nothing could reach it, so a page control without its own `OnLookup` fell
/// straight through to "declares no such trigger" (26 cases of `Price List Line UT` and
/// `Price Worksheet Line UT`, 2026-09-10).
void AFieldWithAnOnLookupTriggerIsInTheLookupMap() {
  const std::string original = Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath);
  CHECK_TRUE("the target table declares no OnLookup, so no map",
             agiru::gen::WriteHeader(agiru::al::ParseTable(original), std::string(kAlPath), {}, {})
                     .text.find("kOnLookup") == std::string::npos);
  std::string looked = original;
  const std::size_t at = looked.find("field(5; \"Direct Unit Cost\"; Decimal)");
  CHECK_TRUE("the source declares the field to give a trigger", at != std::string::npos);
  const std::size_t brace = looked.find('{', at);
  looked.insert(brace + 1, "\n            trigger OnLookup()\n            begin\n            end;");
  const std::string generated =
      agiru::gen::WriteHeader(agiru::al::ParseTable(looked), std::string(kAlPath), {}, {}).text;
  CHECK_TRUE("with one, the traits carry the map",
             generated.find("> kOnLookup{{") != std::string::npos);
  CHECK_TRUE("naming the field and its method",
             generated.find("record.OnLookupDirectUnitCost(); }") != std::string::npos);
}

/// A KEY NAMED `Name` WOULD GIVE `kName`, which is already the table's own name constant. 19 of the
/// BaseApp's keys are called exactly that.
void AKeyNamedLikeAClassConstantStillCompiles() {
  const std::string original = Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath);

  std::string renamed = original;
  const std::size_t at = renamed.find("key(Key1;");
  CHECK_TRUE("the source declares a key to rename", at != std::string::npos);
  renamed.replace(at, std::string("key(Key1;").size(), "key(Name;");

  const std::string generated =
      agiru::gen::WriteHeader(agiru::al::ParseTable(renamed), std::string(kAlPath), {}, {}).text;
  CHECK_TRUE("the array is named by its position", generated.find("kKey1{") != std::string::npos);
  CHECK_TRUE("and never by the AL key name",
             generated.find("static constexpr std::array kName{") == std::string::npos);
  CHECK_TRUE("while the AL name still stands beside it in the KeyDef",
             agiru::gen::WriteDefinitions(agiru::al::ParseTable(renamed), std::string(kAlPath), {})
                     .find(".name = \"Name\", .fields = ResourceCost_Table::kKey1") !=
                 std::string::npos);
  CHECK_TRUE("and the table's own name constant is untouched",
             generated.find("std::string_view kName{\"Resource Cost\"}") != std::string::npos);
}

/// TWO DIFFERENT AL NAMES MAY COLLAPSE INTO ONE C++ IDENTIFIER, and the rule is one rule: the FIELD
/// keeps its spelling, because the field table addresses it by `offsetof` and AL code names it far
/// more often. What yields carries a seam no AL name can reach -- an interior underscore.
void ACollidingNameCarriesASeam() {
  const std::string source = R"(table 50000 "Colliding"
{
    fields
    {
        field(1; "No. Series"; Code[20]) { }
        field(2; "Use Concurrent Posting"; Boolean) { }
        field(3; "System Id"; Guid) { }
    }
    keys { key(PK; "No. Series") { Clustered = true; } }

    var
        NoSeries: Codeunit "No. Series";

    procedure UseConcurrentPosting(): Boolean
    begin
        exit(true);
    end;
})";
  const std::string generated =
      agiru::gen::WriteHeader(agiru::al::ParseTable(source), std::string(kAlPath), {}, {}).text;
  CHECK_TRUE("the field keeps its own name",
             generated.find("Code<20> NoSeries{};") != std::string::npos);
  CHECK_TRUE("and the variable of the same name yields",
             generated.find("NoSeries_Var;") != std::string::npos);
  CHECK_TRUE("a procedure named like a field yields too",
             generated.find("UseConcurrentPosting_Proc(") != std::string::npos);
  // THE PLATFORM'S FIVE ARE THE EXCEPTION AND IT IS NOT ARBITRARY: `WithSystemFields<T>` addresses
  // them by name and the door promises them, so an AL field of the same name is the one that moves.
  CHECK_TRUE("the platform's SystemId keeps its name",
             generated.find("Guid SystemId{};") != std::string::npos);
  CHECK_TRUE("and the AL field of that name carries its number",
             generated.find("SystemId_3{};") != std::string::npos);
}

void ReflectionDeclarationsRetainSourceAuthority() {
  const auto table = agiru::al::ParseTable(R"(namespace Microsoft.Fixture;
table 60003 "Reflection Source"
{
    Caption = 'Different caption';
    Scope = Cloud;
    ObsoleteState = Pending;
    ObsoleteReason = 'Use the successor table';
    DataClassification = AccountData;
    LinkedObject = true;
    fields { field(1; ID; Integer) { } }
})");
  agiru::gen::Objects objects;
  objects.module = "::agiru::app::Fixture::kModule";
  objects.moduleHeader = "FixtureModule.h";
  const std::string definitions = agiru::gen::WriteDefinitions(table, "fixture.al", objects);
  for (const std::string_view declaration : {".module = &::agiru::app::Fixture::kModule",
                                             ".nameSpace = \"Microsoft.Fixture\"",
                                             ".scope = \"Cloud\"",
                                             ".obsoleteReason = \"Use the successor table\"",
                                             ".dataClassification = \"AccountData\"",
                                             ".linkedObject = true",
                                             ".caption = \"Different caption\"",
                                             ".name = ReflectionSource_Table::kName"}) {
    CHECK_TRUE("reflection definitions retain " + std::string(declaration),
               definitions.contains(declaration));
  }
  CHECK_TRUE("the definition unit includes its source-owned module",
             definitions.contains("#include \"FixtureModule.h\""));
  const std::string header = agiru::gen::WriteHeader(table, "fixture.al", {}, objects).text;
  CHECK_TRUE("table headers do not depend on the owning module header",
             !header.contains("FixtureModule.h"));
  const auto refusals = agiru::gen::Refused(table);
  CHECK_TRUE("linked-object metadata does not activate unimplemented external SQL storage",
             refusals.size() == 1 && refusals.front().property == "LinkedObject");

  const auto unqualified =
      agiru::al::ParseTable("table 60004 Bare { fields { field(1; ID; Integer) {} } }");
  const std::string absent = agiru::gen::WriteDefinitions(unqualified, "bare.al", {});
  for (const std::string_view property :
       {".module =", ".nameSpace =", ".scope =", ".obsoleteReason =", ".dataClassification ="}) {
    CHECK_TRUE("absent source authority is not invented: " + std::string(property),
               !absent.contains(property));
  }
  auto changed = table;
  for (auto &property : changed.properties) {
    if (property.name == "LinkedObject") { property.text = "false"; }
  }
  const auto alternate = agiru::gen::WriteDefinitions(changed, "fixture.al", objects);
  CHECK_TRUE("changed source properties change their metadata",
             alternate.contains(".linkedObject = false") &&
                 !alternate.contains(".linkedObject = true"));
  for (auto &property : changed.properties) {
    if (property.name == "LinkedObject") { property.text = "unverified"; }
  }
  bool refused = false;
  try {
    static_cast<void>(agiru::gen::WriteDefinitions(changed, "fixture.al", objects));
  } catch (const std::invalid_argument &error) {
    refused = std::string_view(error.what()).contains("LinkedObject");
  }
  CHECK_TRUE("an invalid linked-object property refuses instead of becoming false", refused);
}

void AppIdentityComesFromTheRootManifest() {
  constexpr std::string_view valid = R"({
    "dependencies": [{"id":"85a884cd-20d8-4d18-91bd-e6c1baaa3a32","name":"Not the owner"}],
    "id":"834a40c9-7a26-46f2-9348-3f6cc8c71719",
    "name":"Nested \"Caf\u00e9\" Fixture",
    "publisher":"agiru tests\nwith tabs\tand slashes\\",
    "version":"2.3.4.5"
  })";
  const auto app = agiru::gen::ParseAppIdentity(valid);
  CHECK_TEXT(
      "a dependency ID does not become the owner", app.id, "834a40c9-7a26-46f2-9348-3f6cc8c71719");
  CHECK_TEXT("JSON strings are decoded without losing Unicode or quotes",
             app.name,
             "Nested \"Café\" Fixture");
  CHECK_TEXT("escaped control characters retain their original source value",
             app.publisher,
             "agiru tests\nwith tabs\tand slashes\\");
  CHECK_TEXT("the manifest version remains exact", app.version, "2.3.4.5");
  for (
      const std::string_view invalid :
      {"[]",
       "{}",
       "{",
       "{\"id\":42}",
       R"({"id":"834a40c9-7a26-46f2-9348-3f6cc8c71719","id":"834a40c9-7a26-46f2-9348-3f6cc8c71719","name":"x","publisher":"y","version":"1.0.0.0"})",
       R"({"id":"invalid-guid","name":"x","publisher":"y","version":"1.0.0.0"})",
       R"({"id":"834a40c9-7a26-46f2-9348-3f6cc8c71719","name":"","publisher":"y","version":"1.0.0.0"})",
       R"({"id":"834a40c9-7a26-46f2-9348-3f6cc8c71719","name":"x","publisher":"y"})"}) {
    bool refused = false;
    try {
      static_cast<void>(agiru::gen::ParseAppIdentity(invalid));
    } catch (const std::runtime_error &error) {
      refused = std::string_view(error.what()).contains("app.json");
    }
    CHECK_TRUE("invalid or ambiguous identity refuses instead of supplying defaults", refused);
  }
  CHECK_TRUE("a valid uppercase GUID is accepted",
             agiru::gen::IsAppGuid("834A40C9-7A26-46F2-9348-3F6CC8C71719"));
  CHECK_TRUE("the native and AL manifests share GUID validation",
             !agiru::gen::IsAppGuid("834a40c9-7a26-46f2-9348-3f6cc8c7171z"));
}

} // namespace

int main() {
  return gate::Run("GenTable", [] {
    TheGeneratorReproducesTheTargetImage();
    TheGeneratorReproducesTheTriggerBodies();
    TheGeneratorReproducesTheDefinitionsUnit();
    AChangedSourceChangesTheOutput();
    AChangedStatementChangesTheBody();
    AFieldThatShadowsARuntimeTypeStillCompiles();
    AKeyNamedLikeAClassConstantStillCompiles();
    ATableWithAnInitValueConstructsWithIt();
    ATableDeclaredTemporaryConstructsItsStore();
    AFieldWithAnOnLookupTriggerIsInTheLookupMap();
    ACollidingNameCarriesASeam();
    ReflectionDeclarationsRetainSourceAuthority();
    AppIdentityComesFromTheRootManifest();
  });
}
