#include "Ast.h"
#include "BodyWriter.h"
#include "Check.h"
#include "EnumWriter.h"
#include "Names.h"
#include "Parser.h"
#include "TableWriter.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

/// Table 37 declares `Type` as `Enum "Sales Line Type"`, and that enum is the sparse one: it
/// declares 0..5 and then 10. Two files, one reference between them -- the whole reason the
/// generator needs two passes.
constexpr std::string_view kEnumPath = "Sales/Document/SalesLineType.Enum.al";
constexpr std::string_view kTablePath = "Sales/Document/SalesLine.Table.al";

std::string Read(std::string_view relative) {
  const std::filesystem::path path = std::filesystem::path(AGIRU_AL_SOURCE) / relative;
  const std::ifstream file(path);
  if (!file) { throw std::runtime_error("cannot read " + path.string()); }
  std::ostringstream text;
  text << file.rdbuf();
  return text.str();
}

bool Has(const std::string &text, std::string_view needle) {
  return text.find(needle) != std::string::npos;
}

agiru::gen::EnumIndex IndexOf(const agiru::al::EnumObject &object) {
  agiru::gen::EnumIndex index;
  index.insert_or_assign(agiru::gen::LowerKey(object.name),
                         agiru::gen::EnumRef{.identifier = agiru::gen::Identifier(object.name),
                                             .header = agiru::gen::EnumHeaderPath(object)});
  return index;
}

void TheDeclaredOrdinalSurvivesTheTranslation() {
  const agiru::al::EnumObject object = agiru::al::ParseEnum(Read(kEnumPath));
  CHECK_TRUE("the enum object parses whole", object.values.size() == 7);
  CHECK_TRUE("and it is the sparse one", object.values.back().ordinal == 10);

  const std::string written = agiru::gen::WriteEnum(object, std::string(kEnumPath), {});
  CHECK_TRUE("the last enumerator carries 10 and not its position",
             Has(written, "AllocationAccount = 10,"));
  CHECK_TRUE("the value table carries the same ordinal",
             Has(written, ".ordinal = 10, .name = \"Allocation Account\""));
  CHECK_TRUE("a member that is no identifier is renamed", Has(written, "Blank = 0,"));
  CHECK_TRUE("and keeps its AL name, which is a single space",
             Has(written, ".ordinal = 0, .name = \" \""));
  CHECK_TRUE("the sortedness ValueOf() relies on is asserted beside the table",
             Has(written, "static_assert(agiru::ValuesAreSorted("));
}

void TheValueTableIsEmittedSortedEvenWhenAlIsNot() {
  // FlushingMethodFilter declares 0, 1, 2, 3, 4, 6, 50, 5 -- the one enum in the BaseApp that is
  // out of order. Emitting it in declaration order would fire the assert the generator itself
  // writes, so the generator sorts and the ENUMERATORS keep AL's order.
  const agiru::al::EnumObject object =
      agiru::al::ParseEnum(Read("Foundation/Enums/FlushingMethodFilter.Enum.al"));
  const std::string written = agiru::gen::WriteEnum(object, "FlushingMethodFilter.Enum.al", {});

  const std::size_t fifty = written.find(".ordinal = 50");
  const std::size_t five = written.find(".ordinal = 5,");
  CHECK_TRUE("both ordinals reach the value table",
             fifty != std::string::npos && five != std::string::npos);
  CHECK_TRUE("and 5 is emitted before 50 although AL declares it after", five < fifty);
  CHECK_TRUE("while the enumerators keep AL's declaration order",
             written.find("= 50,") < written.find("PickBackward = 5,"));
}

void ATableReachesTheEnumObjectByNameAndByHeader() {
  const agiru::al::EnumObject object = agiru::al::ParseEnum(Read(kEnumPath));
  const agiru::al::TableObject table = agiru::al::ParseTable(Read(kTablePath));
  const agiru::gen::TableHeader header =
      agiru::gen::WriteHeader(table, std::string(kTablePath), IndexOf(object), {});

  CHECK_TRUE("the field takes the enum type",
             Has(header.text, "Enum<enums::SalesLineType> Type{};"));
  CHECK_TRUE("and the header that declares it is included",
             Has(header.text, "#include \"sales/document/enum/SalesLineType.h\""));
  // The claim CHANGED with the design: the door was one master include and is now the headers the
  // file names. What it must never be again is `agiru.h`, which cost every generated translation
  // unit the whole door.
  CHECK_TRUE("the door is the headers this file names and not a master include",
             !Has(header.text, "#include \"agiru.h\"") &&
                 Has(header.text, "#include \"type/Enum.h\"") &&
                 Has(header.text, "#include \"runtime/Table.h\""));
}

/// THE NEGATIVE CONTROL. Without the index the reference cannot be resolved, and the generator has
/// to SAY so rather than write a type that is not there and let the tree find out later.
void AnEnumTheRunNeverSawIsReported() {
  const agiru::al::TableObject table = agiru::al::ParseTable(Read(kTablePath));
  const agiru::gen::TableHeader header =
      agiru::gen::WriteHeader(table, std::string(kTablePath), agiru::gen::EnumIndex{}, {});

  CHECK_TRUE("an unresolved enum is reported", !header.unresolvedEnums.empty());
  bool named = false;
  for (const std::string &missing : header.unresolvedEnums) {
    named = named || missing == "Sales Line Type";
  }
  CHECK_TRUE("under the AL name the field gave it", named);
  CHECK_TRUE("and no header is invented for it",
             !Has(header.text, "sales/document/enum/SalesLineType.h"));
}

/// AL type names are case-insensitive and the BaseApp writes `enum`, `GUID` and `BLOB` in several
/// spellings. A generated file needs ONE.
void TheTypeNameIsCanonicalWhateverAlWrote() {
  CHECK_TEXT("lower-case enum is still Enum", agiru::gen::TypeName("enum"), "Enum");
  CHECK_TEXT("GUID becomes Guid", agiru::gen::TypeName("GUID"), "Guid");
  CHECK_TEXT("BLOB becomes Blob", agiru::gen::TypeName("BLOB"), "Blob");
  CHECK_TEXT(
      "RecordID takes the documented spelling", agiru::gen::TypeName("RecordID"), "RecordId");
  CHECK_TEXT("Datetime too", agiru::gen::TypeName("Datetime"), "DateTime");
  CHECK_TEXT("and a type the generator does not know keeps what AL wrote",
             agiru::gen::TypeName("Nowhere"),
             "Nowhere");
}

/// AN ENUM FIELD SCOPES THROUGH ITS ENUMERATION AND NOT THROUGH ITSELF. AL writes the same words on
/// both sides of `"SEPA Partner Type" = "SEPA Partner Type"::Blank` -- the field on the left, the
/// enum object on the right -- and answering only for inline OPTIONS left the right-hand side
/// spelled as the field, which is not a scope.
void AnEnumFieldScopesThroughItsEnumeration() {
  const agiru::al::EnumObject object = agiru::al::ParseEnum(Read(kEnumPath));
  const agiru::al::TableObject table = agiru::al::ParseTable(Read(kTablePath));
  const std::string body = agiru::gen::WriteSource(table, std::string(kTablePath), {});

  CHECK_TRUE("the enum object parses", !object.values.empty());
  CHECK_TRUE("a comparison against a member names the ENUMERATION",
             body.find("enums::SalesLineType::") != std::string::npos);
  // THE NEGATIVE CONTROL. Spelling it as the field is what the defect looked like, and it is what
  // an emitter that answered only for inline options produces.
  CHECK_TRUE("and never the field it compares", body.find("Type == Type::") == std::string::npos);
}

} // namespace

int main() {
  return gate::Run("GenEnum", [] {
    TheDeclaredOrdinalSurvivesTheTranslation();
    TheValueTableIsEmittedSortedEvenWhenAlIsNot();
    ATableReachesTheEnumObjectByNameAndByHeader();
    AnEnumTheRunNeverSawIsReported();
    TheTypeNameIsCanonicalWhateverAlWrote();
    AnEnumFieldScopesThroughItsEnumeration();
  });
}
