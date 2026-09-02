#include "Check.h"
#include "CodeunitWriter.h"
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
      agiru::gen::TableRef{.identifier = "LineNumberBuffer", .header = "LineNumberBuffer.h"});
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

} // namespace

int main() {
  return gate::Run("GenCodeunit", [] {
    TheGeneratorReproducesTheTargetImage();
    TheGeneratorReproducesTheProcedureBodies();
    AChangedSourceChangesTheOutput();
    ATableTheRunNeverSawIsReported();
  });
}
