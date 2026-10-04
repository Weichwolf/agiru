#include "Ast.h"
#include "Check.h"
#include "CodeunitWriter.h"
#include "EnumWriter.h"
#include "Format.h"
#include "Parser.h"
#include "Refused.h"

#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
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
  agiru::al::VarDecl textParameter{.byReference = true,
                                   .temporary = false,
                                   .name = "Value",
                                   .type = "Text",
                                   .subtype = {},
                                   .length = 0,
                                   .members = {},
                                   .arguments = {},
                                   .dimensions = {},
                                   .attributes = {}};
  agiru::al::ProcedureDecl setText;
  setText.name = "SetText";
  setText.parameters.push_back(std::move(textParameter));
  objects.tables.insert_or_assign(
      "line number buffer",
      // THE KIND IS PART OF THE NAME, as it is in the transpiler's own index: 51 objects in the
      // read roots are a table AND a codeunit at once, and `enums::` already told them apart.
      agiru::gen::TableRef{.identifier = "::agiru::app::tables::LineNumberBuffer_Table",
                           .header = "LineNumberBuffer.h",
                           .fields = {},
                           .procedures = {},
                           .parts = {},
                           .name = {},
                           .dataItems = {},
                           .requestFields = {},
                           .columnSources = {},
                           .interfaceReturns = {},
                           .tryFunctions = {},
                           .procedureDeclarations = {}});
  objects.enums.insert_or_assign("sales line type",
                                 agiru::gen::EnumRef{.identifier = "SalesLineType",
                                                     .header = "SalesLineType.h",
                                                     .ordinals = {},
                                                     .members = {}});
  objects.codeunits.insert_or_assign(
      "json management",
      agiru::gen::TableRef{.identifier = "::agiru::app::codeunits::JsonManagement_Codeunit",
                           .header = "JsonManagement.h",
                           .fields = {},
                           .procedures = {{"settext", "SetText"}},
                           .parts = {},
                           .name = {},
                           .dataItems = {},
                           .requestFields = {},
                           .columnSources = {},
                           .interfaceReturns = {},
                           .tryFunctions = {},
                           .procedureDeclarations = {std::move(setText)}});
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
  const std::string target = Read(std::filesystem::path(AGIRU_SOURCE_DIR) /
                                  "test/transpiler/golden/TransferOldExtTextLines.h");

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
  // A MEMBER IS A HANDLE EITHER WAY (board:0037), so what `temporary` decides is the wrapper
  // INSIDE it and nothing else.
  CHECK_TRUE("and leaves the table itself",
             afterPermanent.find(
                 "Instance<::agiru::app::tables::LineNumberBuffer_Table> TempLineNumberBuffer") !=
                 std::string::npos);

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
  const std::string target = Read(std::filesystem::path(AGIRU_SOURCE_DIR) /
                                  "test/transpiler/golden/TransferOldExtTextLines.cpp");

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

  CHECK_TRUE("inline option declarations include the shared option vocabulary",
             generated.find("#include \"options/Types.h\"") != std::string::npos);
  CHECK_TRUE("the member uses the content-addressed option type",
             generated.find("Option<::agiru::options::OptionDraftPosted> Mode") !=
                 std::string::npos);
  CHECK_TRUE("the parameter's blank member participates in its type name",
             generated.find("Option<::agiru::options::OptionBlankItemResource> Kind") !=
                 std::string::npos);
  CHECK_TRUE("different member lists retain distinct shared types",
             generated.find("OptionDraftPosted") != generated.find("OptionBlankItemResource"));
  CHECK_TRUE("shared option definitions are not duplicated inside a codeunit",
             generated.find("enum class") == std::string::npos);
}

/// AL LETS A METHOD WITHOUT PARAMETERS BE CALLED WITHOUT PARENTHESES, and the BaseApp does so on a
/// static platform type: `CallStack := SessionInformation.Callstack;` (ErrorMessageManagement,
/// 21 UT cases blocked on 2026-09-09). The generator adds them where the door declares a static
/// function of that name on that type -- and nowhere else, since an enumerator is spelled the same.
void AStaticPlatformMemberWithoutParenthesesIsACall() {
  const std::string source = R"(codeunit 50001 "Some Caller"
{
    procedure Trace(): Text
    var
        Stack: Text;
        How: Verbosity;
    begin
        Stack := SessionInformation.Callstack;
        How := Verbosity::Normal;
        exit(Stack);
    end;
})";
  const std::string generated = agiru::gen::Formatted(agiru::gen::FormatRequest{
      .source = agiru::gen::WriteCodeunitSource(
          agiru::al::ParseCodeunit(source), std::string(kAlPath), Tables()),
      .stylePath = std::string(AGIRU_SOURCE_DIR) + "/.clang-format",
      .assumedName = "SomeCaller.cpp"});
  CHECK_TRUE("the static method gets its parentheses",
             generated.find("SessionInformation::Callstack()") != std::string::npos);
  // THE NEGATIVE CONTROL: an enumerator on a type name is not a call, and `Normal` IS a callable
  // name elsewhere in the door.
  CHECK_TRUE("an enumerator stays one", generated.find("Verbosity::Normal;") != std::string::npos);
}

void ARelationalLeftOperandRetainsItsAlGrouping() {
  const std::string source = R"(codeunit 50002 "Relational Grouping"
{
    procedure Check(SourceType: Integer; IsAssembleToOrder: Boolean): Boolean
    begin
        exit((SourceType = 900) = IsAssembleToOrder);
    end;
})";
  const std::string generated = agiru::gen::WriteCodeunitSource(
      agiru::al::ParseCodeunit(source), std::string(kAlPath), Tables());
  CHECK_TRUE("a nested equality is parenthesized before its next equality",
             generated.find("(SourceType == 900) == IsAssembleToOrder") != std::string::npos);
}

void ARemoteVarParameterLendsTheVariantsStoredType() {
  const std::string source = R"(codeunit 50003 "Remote Variant"
{
    var
        JSONManagement: Codeunit "JSON Management";

    procedure SetText(Held: Variant)
    begin
        JSONManagement.SetText(Held);
    end;
})";
  const std::string generated = agiru::gen::WriteCodeunitSource(
      agiru::al::ParseCodeunit(source), std::string(kAlPath), Tables());
  CHECK_TRUE("a remote var Text parameter lends Variant storage",
             generated.find("JSONManagement->SetText(Held.Lend<::agiru::Text<0>>())") !=
                 std::string::npos);
}

/// A FIELD NAMED ON AN ARRAY ELEMENT BELONGS TO THAT ELEMENT. `CurrencyExchRate2[CacheNo].SetRange(
/// "Currency Code", CurrencyCode)` in table 330 named its field once through `this` (the parameter
/// `CurrencyCode` shadowed the field) and once bare, and the runtime then measured the member's
/// offset from the wrong record: "declares no field at byte 18446744073709281496" in every case
/// that reached an exchange rate (measured 2026-09-09, 19 such sites in the tree).
void AFieldNamedOnAnArrayElementIsTheElementsField() {
  const std::string source = R"(codeunit 50002 "Array Caller"
{
    var
        Buffers: array[2] of Record "Line Number Buffer";

    procedure Narrow(OldLineNumber: Integer)
    begin
        Buffers[1].SetRange("Old Line Number", OldLineNumber);
        Buffers[2].SetRange("New Line Number", 0);
    end;
})";
  agiru::gen::Objects objects = Tables();
  objects.tables.at("line number buffer").fields = {{"old line number", "OldLineNumber"},
                                                    {"new line number", "NewLineNumber"}};
  const std::string generated = agiru::gen::Formatted(agiru::gen::FormatRequest{
      .source = agiru::gen::WriteCodeunitSource(
          agiru::al::ParseCodeunit(source), std::string(kAlPath), objects),
      .stylePath = std::string(AGIRU_SOURCE_DIR) + "/.clang-format",
      .assumedName = "ArrayCaller.cpp"});
  CHECK_TRUE(
      "the shadowed field is the element's",
      generated.find("At(Buffers, 1).SetRange(At(Buffers, 1).OldLineNumber, OldLineNumber)") !=
          std::string::npos);
  CHECK_TRUE("and so is the bare one",
             generated.find("At(Buffers, 2).SetRange(At(Buffers, 2).NewLineNumber, 0)") !=
                 std::string::npos);
}

/// A VARIABLE NAMED AFTER ITS ENUMERATION TYPE STILL SCOPES THROUGH THE TYPE. `TelemetryScope::All`
/// beside a parameter `TelemetryScope: TelemetryScope` is legal AL, and the platform lets a
/// variable of an enumeration type scope its members (`devenv-enum-type.md`); the generator refused
/// it as an enumeration this run does not have, in the telemetry module every posting reaches (17
/// UT cases, measured 2026-09-09).
void AVariableNamedAfterItsTypeScopesThroughTheType() {
  const std::string source = R"(codeunit 50003 "Scoped Caller"
{
    procedure Log(TelemetryScope: TelemetryScope): Boolean
    begin
        exit(TelemetryScope = TelemetryScope::All);
    end;
})";
  const std::string generated = agiru::gen::Formatted(agiru::gen::FormatRequest{
      .source = agiru::gen::WriteCodeunitSource(
          agiru::al::ParseCodeunit(source), std::string(kAlPath), Tables()),
      .stylePath = std::string(AGIRU_SOURCE_DIR) + "/.clang-format",
      .assumedName = "ScopedCaller.cpp"});
  CHECK_TRUE("the member is the type's",
             generated.find("::agiru::TelemetryScope::All") != std::string::npos);
  CHECK_TRUE("and nothing is refused", generated.find("RefusedOption") == std::string::npos);
}

/// `FieldRef.Type::Code` SCOPES THROUGH THE FIELD TYPE, on an array element as much as on a
/// variable: `Find Record Management` writes `SearchFieldRef[1].Type <>
/// SearchFieldRef[1].Type::Code`, and the generator wrote `RefusedOption(".::Code")` -- the
/// receiver's `.Type` was neither a field enumeration nor a method with an option (6 cases of
/// Record Set UT, 2026-09-10).
void AFieldRefsTypeScopesThroughFieldType() {
  const std::string source = R"(codeunit 50004 "Typed Caller"
{
    procedure IsCode(FieldRef: FieldRef; Refs: array[2] of FieldRef): Boolean
    begin
        exit((FieldRef.Type = FieldRef.Type::Code) and (Refs[1].Type = Refs[1].Type::Code));
    end;
})";
  const std::string generated = agiru::gen::Formatted(agiru::gen::FormatRequest{
      .source = agiru::gen::WriteCodeunitSource(
          agiru::al::ParseCodeunit(source), std::string(kAlPath), Tables()),
      .stylePath = std::string(AGIRU_SOURCE_DIR) + "/.clang-format",
      .assumedName = "TypedCaller.cpp"});
  CHECK_TRUE("the member is the field type's",
             generated.find("::agiru::FieldType::Code") != std::string::npos);
  CHECK_TRUE("and nothing is refused", generated.find("RefusedOption") == std::string::npos);
}

/// `foreach Item in Collection` OVER A .NET COLLECTION FILLS THE DECLARED VARIABLE: `Library -
/// Report Validation` declares `CellData: DotNet CellData` and reads `CellData.RowNumber` in the
/// loop, and a loop that bound a fresh `auto &` to the element left the declared variable's members
/// unreachable (the unit was out of the slice, 8 UT cases, 2026-09-10). A declared AL variable is
/// what the body names, so the element is assigned into it.
void AForeachOverADotNetCollectionFillsTheDeclaredVariable() {
  const std::string source = R"(codeunit 50005 "Cell Walker"
{
    procedure Rows(Reader: DotNet WorksheetReader): Integer
    var
        CellData: DotNet CellData;
        Last: Integer;
    begin
        foreach CellData in Reader do
            Last := CellData.RowNumber;
        exit(Last);
    end;
})";
  const std::string generated = agiru::gen::Formatted(agiru::gen::FormatRequest{
      .source = agiru::gen::WriteCodeunitSource(
          agiru::al::ParseCodeunit(source), std::string(kAlPath), Tables()),
      .stylePath = std::string(AGIRU_SOURCE_DIR) + "/.clang-format",
      .assumedName = "CellWalker.cpp"});
  CHECK_TRUE("the element goes into the declared variable",
             generated.find("CellData = Element_Block;") != std::string::npos);
  CHECK_TRUE("and the body reads that variable",
             generated.find("CellData.RowNumber()") != std::string::npos);
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
             generated.find("::agiru::app::tables::LineNumberBuffer_Table &LineNumberBuffer") !=
                 std::string::npos);

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

  // THE NEGATIVE CONTROL: an include list that carried only what the old walk saw would still
  // pass the second line, because a global table was always reached. The enumeration and the
  // local are the two it missed, and the local is why the count of includes matters rather than
  // their presence -- a set collapses the duplicate, so this asserts the file compiles as a
  // whole.
  CHECK_TRUE("the enumeration is included exactly once, however many places name it",
             generated.find("SalesLineType.h") == generated.rfind("SalesLineType.h"));
}

void ANativeFieldCannotCaptureARecordMethod() {
  const std::array tables{agiru::al::ParseTable(R"(table 2000000068 "Record Link" {
    fields { field(2; "Record ID"; RecordId) {} }
  })")};
  agiru::gen::Objects objects;
  objects.tables = agiru::gen::PlatformTables(tables);
  const auto unit = agiru::al::ParseCodeunit(R"(codeunit 50177 Caller {
    procedure Check()
    var Native: Record "Record Link"; MethodValue: RecordId; FieldValue: RecordId;
    begin
      MethodValue := Native.RecordId;
      MethodValue := Native.RecordID();
      FieldValue := Native."Record ID";
    end;
  })");
  const std::string body = agiru::gen::WriteCodeunitSource(unit, "Caller.Codeunit.al", objects);
  CHECK_TRUE(
      "property syntax selects the record method rather than a spaced AL field",
      body.contains(
          "MethodValue = Native.::agiru::Table<::agiru::platform::RecordLink>::RecordId();"));
  CHECK_TRUE("case-insensitive explicit calls select the same record method",
             body.find("RecordId();") != body.rfind("RecordId();"));
  CHECK_TRUE("quoted field access retains the native ABI field",
             body.contains("FieldValue = Native.RecordID;"));
  CHECK_TRUE("native field values are never called", !body.contains("Native.RecordID()"));
}

void NativeDeclarationsNeverBecomeSuccessfulEmptyMethods() {
  const auto unit = agiru::al::ParseCodeunit(R"(namespace System.Fixture;
    codeunit 50311 NativeFixture {
      [Native] procedure Empty() begin end;
      [nAtIvE] procedure Read(var Value: Integer) Result: Text
      var UnexpectedLocal: Integer;
      begin Value := 99; exit('success'); end;
      [Native] [IntegrationEvent(false, false)]
      procedure Publish(var Value: Integer) begin end;
      procedure Ordinary(): Integer begin exit(7); end;
    })");
  const auto refused = agiru::gen::Refused(unit);
  CHECK_TRUE("every Native overload remains counted", refused.size() == 3);
  CHECK_TEXT("refusal retains source namespace, ID, modes and named return",
             refused.at(1).where,
             "codeunit 50311 System.Fixture.NativeFixture.Read(var Value: Integer) Result: Text");
  CHECK_TEXT("Native capability is not a guessed property", refused.at(1).property, "Native");
  const auto source = agiru::gen::WriteCodeunitSource(unit, "NativeFixture.Codeunit.al", Tables());
  CHECK_TRUE("native void declarations refuse instead of falling off",
             source.contains("NativeFixture.Empty() has no native implementation"));
  CHECK_TRUE(
      "native publisher declarations refuse before raising events",
      source.contains("NativeFixture.Publish(var Value: Integer) has no native implementation"));
  CHECK_TRUE("Native ignores authored fallback bodies", !source.contains("Value = 99"));
  CHECK_TRUE("unbound Native never initializes AL locals", !source.contains("UnexpectedLocal{}"));
  CHECK_TRUE("ordinary AL procedures retain their implementation", source.contains("return 7;"));
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
    AFieldNamedOnAnArrayElementIsTheElementsField();
    AVariableNamedAfterItsTypeScopesThroughTheType();
    AFieldRefsTypeScopesThroughFieldType();
    AForeachOverADotNetCollectionFillsTheDeclaredVariable();
    AStaticPlatformMemberWithoutParenthesesIsACall();
    ARelationalLeftOperandRetainsItsAlGrouping();
    ARemoteVarParameterLendsTheVariantsStoredType();
    ACodeunitIncludesEveryObjectItNames();
    ANativeFieldCannotCaptureARecordMethod();
    NativeDeclarationsNeverBecomeSuccessfulEmptyMethods();
  });
}
