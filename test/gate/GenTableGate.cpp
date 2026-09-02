#include "BodyWriter.h"
#include "Check.h"
#include "Format.h"
#include "Parser.h"
#include "TableWriter.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
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
      Read(std::filesystem::path(AGIRU_SOURCE_DIR) / "test/target/ResourceCost.h");

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
      Read(std::filesystem::path(AGIRU_SOURCE_DIR) / "test/target/ResourceCost.cpp");

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
             afterRename.find("\"Work Kind Code\"") != std::string::npos);
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

  // AL's `and` is not C++'s, and neither is its `=`. Both mappings are asserted, because an
  // emitter that passed one through untouched would still compile and mean something else.
  const std::string untouched =
      agiru::gen::WriteSource(agiru::al::ParseTable(original), std::string(kAlPath), {});
  CHECK_TRUE("`and` becomes `&&`", untouched.find(" && ") != std::string::npos);
  CHECK_TRUE("and no AL operator survives",
             untouched.find(" and ") == std::string::npos &&
                 untouched.find(" <> ") == std::string::npos);
  // The parentheses AL needed are gone, because C++ binds the comparison tighter than the
  // conjunction. An emitter that parenthesised everything would also be correct and would not
  // match the target image.
  CHECK_TRUE("redundant parentheses are not emitted",
             untouched.find("(Code != \"\") &&") == std::string::npos);
}

/// A FIELD NAME THAT COLLIDES WITH A RUNTIME TYPE. `Change Log Setup (Field)` really does declare a
/// field called `Field No.`, and its member is spelled exactly like `agiru::FieldNo` -- so from
/// that member onward the class's own name wins and every Field_No entry below it fails to
/// compile. The source is altered in memory; the repository under ~/Git/BCApps is never written to.
void AFieldThatShadowsARuntimeTypeStillCompiles() {
  const std::string original = Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath);

  std::string collided = original;
  const std::size_t at = collided.find("\"Work Type Code\"");
  CHECK_TRUE("the source declares the field to rename", at != std::string::npos);
  collided.replace(at, std::string("\"Work Type Code\"").size(), "\"Field No.\"");

  const std::string generated =
      agiru::gen::WriteHeader(agiru::al::ParseTable(collided), std::string(kAlPath), {}, {}).text;
  CHECK_TRUE("the field takes the name AL gave it",
             generated.find("FieldNo{};") != std::string::npos);
  CHECK_TRUE("and the field numbers reach past it to the runtime type",
             generated.find("static constexpr ::agiru::FieldNo Code{") != std::string::npos);
  CHECK_TRUE("while a table with no such field says it plainly",
             agiru::gen::WriteHeader(agiru::al::ParseTable(original), std::string(kAlPath), {}, {})
                     .text.find("static constexpr FieldNo Code{") != std::string::npos);
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
             generated.find(".name = \"Name\", .fields = ResourceCost::kKey1") !=
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

} // namespace

int main() {
  return gate::Run("GenTable", [] {
    TheGeneratorReproducesTheTargetImage();
    TheGeneratorReproducesTheTriggerBodies();
    AChangedSourceChangesTheOutput();
    AChangedStatementChangesTheBody();
    AFieldThatShadowsARuntimeTypeStillCompiles();
    AKeyNamedLikeAClassConstantStillCompiles();
    ACollidingNameCarriesASeam();
  });
}
