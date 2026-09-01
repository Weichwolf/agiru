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
          std::string(kAlPath)),
      .stylePath = std::string(AGIRU_SOURCE_DIR) + "/.clang-format"});
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
      agiru::gen::WriteHeader(agiru::al::ParseTable(widened), std::string(kAlPath));
  CHECK_TRUE("a widened field widens its member",
             generated.find("::agiru::Code<30> Code;") != std::string::npos);
  CHECK_TRUE("and the old width is gone",
             generated.find("::agiru::Code<20> Code;") == std::string::npos);

  std::string renamed = original;
  const std::size_t nameAt = renamed.find("\"Work Type Code\"");
  CHECK_TRUE("the source declares the field to rename", nameAt != std::string::npos);
  renamed.replace(nameAt, std::string("\"Work Type Code\"").size(), "\"Work Kind Code\"");

  const std::string afterRename =
      agiru::gen::WriteHeader(agiru::al::ParseTable(renamed), std::string(kAlPath));
  CHECK_TRUE("a renamed field renames its member and its number",
             afterRename.find("WorkKindCode;") != std::string::npos &&
                 afterRename.find("FieldNo WorkKindCode{3}") != std::string::npos);
  CHECK_TRUE("and the AL name follows it into the field table",
             afterRename.find("\"Work Kind Code\"") != std::string::npos);
}

} // namespace

int main() {
  return gate::Run("GenTable", [] {
    TheGeneratorReproducesTheTargetImage();
    AChangedSourceChangesTheOutput();
  });
}
