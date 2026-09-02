#include "Check.h"
#include "CodeunitWriter.h"
#include "EnumWriter.h"
#include "Format.h"
#include "Parser.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr std::string_view kAlPath = "Foundation/ExtendedText/TransferOldExtTextLines.Codeunit.al";

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

/// The index the RUN supplies, standing in for one app's tables. The header path is the one this
/// gate compiles against; a real run gives the app-relative path instead, and the emitter does not
/// care which -- that is what makes an enum or a table reachable across apps without qualification.
agiru::gen::Objects Tables() {
  agiru::gen::Objects objects;
  objects.tables.insert_or_assign(
      "line number buffer",
      // THE KIND IS PART OF THE NAME, as it is in the transpiler's own index: 51 objects in the
      // read roots are a table AND a codeunit at once, and `enums::` already told them apart.
      agiru::gen::TableRef{.identifier = "tables::LineNumberBuffer",
                           .header = "LineNumberBuffer.h"});
  objects.enums.insert_or_assign(
      "sales line type",
      agiru::gen::EnumRef{.identifier = "SalesLineType", .header = "SalesLineType.h"});
  return objects;
}

std::string Generated(const std::string &source) {
  return agiru::gen::Formatted(agiru::gen::FormatRequest{
      .source = agiru::gen::WriteCodeunit(
                    agiru::al::ParseCodeunit(source), std::string(kAlPath), Tables())
                    .text,
      .stylePath = std::string(AGIRU_SOURCE_DIR) + "/.clang-format",
      .assumedName = "TransferOldExtTextLines.h"});
}

/// THE SAME PROOF THE TABLES ALREADY HAVE, for the object type that carries the code: what the
/// generator writes for codeunit 379 and what was written by hand as its specification are the
/// SAME FILE.
void TheGeneratorReproducesTheTargetImage() {
  const std::string generated = Generated(Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath));
  const std::string target =
      Read(std::filesystem::path(AGIRU_SOURCE_DIR) / "test/target/TransferOldExtTextLines.h");

  {
    std::ofstream dump("/tmp/agiru-generated-TransferOldExtTextLines.h");
    dump << generated;
  }

  const std::vector<std::string> left = Lines(generated);
  const std::vector<std::string> right = Lines(target);
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

/// THE NEGATIVE CONTROL. A comparison that only ever passes proves nothing.
void AChangedSourceChangesTheOutput() {
  const std::string original = Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath);

  // `local` is exactly C++'s private, so removing it moves the procedure across the line.
  std::string opened = original;
  const std::size_t at = opened.find("local procedure InsertLineNumbers");
  CHECK_TRUE("the source declares a local procedure", at != std::string::npos);
  opened.replace(at, std::string("local ").size(), "");
  const std::string afterOpening = Generated(opened);
  CHECK_TRUE("a procedure that stops being local moves above the private line",
             afterOpening.find("InsertLineNumbers") < afterOpening.find("private:"));
  CHECK_TRUE("while in the original it is below it",
             Generated(original).find("InsertLineNumbers") > Generated(original).find("private:"));

  // `temporary` is what decides whether a record reaches the database at all.
  std::string permanent = original;
  const std::size_t temp = permanent.find("Record \"Line Number Buffer\" temporary;");
  CHECK_TRUE("the source declares a temporary record", temp != std::string::npos);
  permanent.replace(temp,
                    std::string("Record \"Line Number Buffer\" temporary;").size(),
                    "Record \"Line Number Buffer\";");
  const std::string afterPermanent = Generated(permanent);
  CHECK_TRUE("dropping `temporary` drops the wrapper",
             afterPermanent.find("Temporary<LineNumberBuffer> TempLineNumberBuffer") ==
                 std::string::npos);
  CHECK_TRUE("and leaves the table itself",
             afterPermanent.find("LineNumberBuffer TempLineNumberBuffer") != std::string::npos);

  // A named return value is a return TYPE and nothing else in C++.
  std::string voided = original;
  const std::size_t ret = voided.find("AttachedLineNo: Integer) Result: Integer");
  CHECK_TRUE("the source declares a named return value", ret != std::string::npos);
  voided.replace(ret,
                 std::string("AttachedLineNo: Integer) Result: Integer").size(),
                 "AttachedLineNo: Integer)");
  CHECK_TRUE("removing it makes the procedure return void",
             Generated(voided).find("void TransferExtendedText") != std::string::npos);
}

/// The other half: the procedure bodies, which are AL statements rather than declarations.
void TheGeneratorReproducesTheProcedureBodies() {
  const std::string generated = agiru::gen::Formatted(agiru::gen::FormatRequest{
      .source = agiru::gen::WriteCodeunitSource(
          agiru::al::ParseCodeunit(Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath)),
          std::string(kAlPath),
          Tables()),
      .stylePath = std::string(AGIRU_SOURCE_DIR) + "/.clang-format",
      .assumedName = "TransferOldExtTextLines.cpp"});
  const std::string target =
      Read(std::filesystem::path(AGIRU_SOURCE_DIR) / "test/target/TransferOldExtTextLines.cpp");

  {
    std::ofstream dump("/tmp/agiru-generated-TransferOldExtTextLines.cpp");
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
  if (left.size() != right.size()) {
    CHECK_TEXT("the first line that only one of them has",
               left.size() > right.size() ? left[shared] : std::string("<end of file>"),
               right.size() > left.size() ? right[shared] : std::string("<end of file>"));
  }
}

/// A codeunit that names a table the run never saw is REPORTED, not emitted against a type that is
/// not there.
void ATableTheRunNeverSawIsReported() {
  const agiru::gen::CodeunitHeader header = agiru::gen::WriteCodeunit(
      agiru::al::ParseCodeunit(Read(std::filesystem::path(AGIRU_AL_SOURCE) / kAlPath)),
      std::string(kAlPath),
      agiru::gen::Objects{});
  CHECK_TRUE("the unresolved table is reported", header.unresolvedTables.size() == 1);
  CHECK_TEXT("under the AL name the declaration gave it",
             header.unresolvedTables.front(),
             "Line Number Buffer");
  CHECK_TRUE("and no header is invented for it",
             header.text.find("LineNumberBuffer.h") == std::string::npos);
}

/// AN INLINE `Option A,B,C` HAS NO NAME AND DECLARES ITS OWN MEMBERS. A table's version becomes an
/// enumeration beside the table; a procedure's needs one too, or `Type::All` in the body has no
/// vocabulary and the declaration is an Integer that lost it.
void AnInlineOptionGetsAnEnumerationOfItsOwn() {
  const std::string source = R"(codeunit 50000 "Some Thing"
{
    var
        Mode: Option Draft,Posted;

    procedure Check(Kind: Option " ",Item,Resource)
    var
        Step: Option First,Second;
    begin
    end;
})";
  const std::string generated = Generated(source);

  CHECK_TRUE("a codeunit variable gets an enumeration named after the codeunit and itself",
             generated.find("enum class SomeThingMode") != std::string::npos);
  CHECK_TRUE("a parameter gets one named after its procedure too",
             generated.find("enum class SomeThingCheckKind") != std::string::npos);
  CHECK_TRUE("and so does a local",
             generated.find("enum class SomeThingCheckStep") != std::string::npos);
  CHECK_TRUE("the member names are kept as AL wrote them",
             generated.find("\"Draft\"") != std::string::npos &&
                 generated.find("\"Posted\"") != std::string::npos);
  CHECK_TRUE("a member that is no identifier is renamed and its ordinal kept",
             generated.find("Blank = 0,") != std::string::npos);
  CHECK_TRUE("the variable's type names its own enumeration",
             generated.find("Option<SomeThingMode> Mode") != std::string::npos);
  CHECK_TRUE("and so does the parameter",
             generated.find("Option<SomeThingCheckKind> Kind") != std::string::npos);

  // THE NEGATIVE CONTROL. A rule that named every option the same would pass every check above.
  CHECK_TRUE("two options in one codeunit do not share an enumeration",
             generated.find("SomeThingCheckKind") != generated.find("SomeThingCheckStep"));
}

/// A PARAMETER MAY BE NAMED AFTER ITS TYPE, and AL writes it constantly. C++ then has the name hide
/// the type, so the declaration has to qualify it -- and WHICH namespace it qualifies with is
/// decided by what the type IS. An AL object becomes a class in `agiru::app`; every other AL type
/// is a door type in `agiru`.
///
/// This rule had no case, and that is why one wrong qualification could stand: `LibraryAssert`
/// writes `RecordRef: RecordRef`, the generator wrote `agiru::app::RecordRef`, and that single line
/// was the FIRST diagnostic of 1 375 of 3 123 failing headers -- 44 % of the tree stopped on a file
/// nothing in the gate looked at.
void AParameterNamedAfterItsTypeIsQualifiedWhereTheTypeLives() {
  const std::string source = R"(codeunit 50000 "Some Thing"
{
    procedure Check(var RecordRef: RecordRef; var LineNumberBuffer: Record "Line Number Buffer"; Date: Date)
    begin
    end;
})";
  const std::string generated = Generated(source);

  CHECK_TRUE("a door type is qualified into agiru",
             generated.find("agiru::RecordRef &RecordRef") != std::string::npos);
  CHECK_TRUE("and so is a value type", generated.find("agiru::Date Date") != std::string::npos);
  // AN AL OBJECT NEEDS NO QUALIFICATION ANY MORE, because its KIND carries it: a parameter named
  // `LineNumberBuffer` cannot shadow `tables::LineNumberBuffer`, which is a qualified name. The
  // kind namespace solved the collision the qualification was invented for, and it solved the
  // table-against-codeunit collision with it.
  CHECK_TRUE("an AL object is reached through its kind",
             generated.find("tables::LineNumberBuffer &LineNumberBuffer") != std::string::npos);

  // THE NEGATIVE CONTROL, and it is the whole point: a rule that sent everything to one namespace
  // would pass one of the three lines above and fail the tree. Neither wrong form may appear.
  CHECK_TRUE("a door type never lands in agiru::app",
             generated.find("agiru::app::RecordRef") == std::string::npos);
  CHECK_TRUE("and an AL object never lands beside the door",
             generated.find("agiru::tables::LineNumberBuffer") == std::string::npos);
}

/// A CODEUNIT INCLUDES WHAT IT NAMES, AND IT NAMED THREE KINDS WITHOUT ASKING FOR TWO OF THEM.
/// `LibraryNoSeries` declares `Enum<enums::NoSeriesImplementation>` and included the table beside
/// it but not the enumeration; that one missing line was the FIRST diagnostic of 1 159 failing
/// headers in the locked run. A procedure's own LOCAL variables were not walked at all.
void ACodeunitIncludesEveryObjectItNames() {
  const std::string source = R"(codeunit 50000 "Some Thing"
{
    procedure Check(Kind: Option " ",Item,Resource)
    var
        Sorting: Enum "Sales Line Type";
    begin
    end;
})";
  const std::string generated = Generated(source);

  CHECK_TRUE("the enumeration a LOCAL variable names is included",
             generated.find("#include \"SalesLineType.h\"") != std::string::npos);

  // THE NEGATIVE CONTROL: an include list that carried only what the old walk saw would still pass
  // the second line, because a global table was always reached. The enumeration and the local are
  // the two it missed, and the local is why the count of includes matters rather than their
  // presence -- a set collapses the duplicate, so this asserts the file compiles as a whole.
  CHECK_TRUE("the enumeration is included exactly once, however many places name it",
             generated.find("SalesLineType.h") == generated.rfind("SalesLineType.h"));
}

} // namespace

int main() {
  return gate::Run("GenCodeunit", [] {
    TheGeneratorReproducesTheTargetImage();
    TheGeneratorReproducesTheProcedureBodies();
    AChangedSourceChangesTheOutput();
    ATableTheRunNeverSawIsReported();
    AnInlineOptionGetsAnEnumerationOfItsOwn();
    AParameterNamedAfterItsTypeIsQualifiedWhereTheTypeLives();
    ACodeunitIncludesEveryObjectItNames();
  });
}
