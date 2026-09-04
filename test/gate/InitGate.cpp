#include "runtime/Error.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "type/Decimal.h"

#include "Check.h"
#include "EnumWriter.h"
#include "Parser.h"
#include "ResourceCost.h"
#include "TableWriter.h"

#include <string>

using agiru::CreateTable;
using agiru::Decimal;
using agiru::DropTable;
using agiru::Error;
using agiru::Session;
using agiru::app::tables::ResourceCost;
using agiru::app::tables::ResourceCostCostType;
using agiru::app::tables::ResourceCostType;

namespace {

using CodeValue = decltype(ResourceCost::Code);

std::string Emitted(const std::string &al, const agiru::gen::EnumIndex &enums) {
  return agiru::gen::WriteHeader(agiru::al::ParseTable(al), "Gate.Table.al", enums, {}).text;
}

// `devenv-initvalue-property.md`. The property is on the FIELD, and the value AL writes is read in
// the field's own vocabulary: a Boolean literal, a quoted Code, or the NAME of an enumeration
// value. The column takes `true`, ` ` and an ORDINAL, and the translation happens where the
// enumeration is in scope -- which is the transpiler and never the runtime.
void AnInitValueIsTranslatedIntoTheColumnsOwnSpelling() {
  const std::string text = Emitted(R"(table 90000 "Gate"
{
    fields
    {
        field(1; "Code"; Code[20]) { }
        field(2; "Direct Posting"; Boolean) { InitValue = true; }
        field(3; "Filler"; Code[10]) { InitValue = ' '; }
        field(4; "Kind"; Option)
        {
            OptionMembers = First,Second,Third;
            InitValue = Third;
        }
    }
    keys { key(Key1; "Code") { } }
})",
                                   {});
  CHECK_TRUE("a Boolean carries its literal",
             text.find("offsetof(Gate, DirectPosting), \"true\")") != std::string::npos);
  CHECK_TRUE("a Code carries the quoted text, spaces and all",
             text.find("offsetof(Gate, Filler), \" \")") != std::string::npos);
  CHECK_TRUE("AN OPTION CARRIES ITS ORDINAL AND NOT ITS MEMBER NAME",
             text.find("offsetof(Gate, Kind), \"2\")") != std::string::npos);
  CHECK_TRUE("a field that declares none carries nothing",
             text.find("offsetof(Gate, Code)),") != std::string::npos);
}

// An ENUM field's value lives in another object, so the ordinal is DECLARED rather than counted --
// and the generator has to look it up rather than take the position.
void AnEnumInitValueResolvesThroughTheEnumIndex() {
  agiru::gen::EnumIndex enums;
  enums.insert_or_assign("gate answer",
                         agiru::gen::EnumRef{.identifier = "GateAnswer",
                                             .header = "GateAnswer.h",
                                             .ordinals = {{"yes", 0}, {"no", 10}}});
  const std::string text = Emitted(R"(table 90001 "Gate"
{
    fields
    {
        field(1; "Code"; Code[20]) { }
        field(2; "Answer"; Enum "Gate Answer") { InitValue = No; }
    }
    keys { key(Key1; "Code") { } }
})",
                                   enums);
  CHECK_TRUE("the DECLARED ordinal is emitted, not the position",
             text.find("offsetof(Gate, Answer), \"10\")") != std::string::npos);
}

// AND AN ENUM THIS RUN DOES NOT CARRY YIELDS NOTHING RATHER THAN A GUESS. The table already reports
// that enum as unresolved, which is the count for it; emitting the member name would put a word
// where the column wants a number and would fail at run time, one row at a time.
void AnUnknownEnumYieldsNoInitValueAtAll() {
  const std::string text = Emitted(R"(table 90002 "Gate"
{
    fields
    {
        field(1; "Code"; Code[20]) { }
        field(2; "Answer"; Enum "Gate Answer") { InitValue = No; }
    }
    keys { key(Key1; "Code") { } }
})",
                                   {});
  CHECK_TRUE("no init value is emitted",
             text.find("offsetof(Gate, Answer), \"") == std::string::npos);
  CHECK_TRUE("and the field is still declared",
             text.find("offsetof(Gate, Answer))") != std::string::npos);
}

// `record-init-method.md`: "This method assigns default values to each field in the record" and
// "Primary key and timestamp fields aren't initialized."
void InitReturnsEveryFieldButTheKeyToItsDefault() {
  DropTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
  CreateTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
  ResourceCost rec;
  rec.Type = ResourceCostType::GroupResource;
  rec.Code = CodeValue("R01");
  rec.WorkTypeCode = "hours";
  rec.CostType = ResourceCostCostType::LCYExtra;
  rec.DirectUnitCost = Decimal::FromInvariantString("7.50");
  rec.UnitCost = Decimal::FromInvariantString("9.00");

  rec.Init();

  CHECK_TRUE("a primary key field keeps its value", rec.Type == ResourceCostType::GroupResource);
  CHECK_TEXT("and so does the second one", std::string(rec.Code.Value()), "R01");
  // UPPERCASED, BECAUSE A Code UPPERCASES ON ASSIGNMENT (board:0010) -- the value survived `Init`
  // as the field stored it, which is the claim.
  CHECK_TEXT("and the third", std::string(rec.WorkTypeCode.Value()), "HOURS");
  CHECK_TRUE("a field outside the key goes back to its default",
             rec.CostType == ResourceCostCostType::Fixed);
  CHECK_TRUE("and so does a Decimal", rec.DirectUnitCost.IsZero());
  CHECK_TRUE("and the other one", rec.UnitCost.IsZero());
  DropTable(Session::Current().Database(), agiru::TableTraits<ResourceCost>::kTable);
}

} // namespace

int main() {
  return gate::Run("Init", [] {
    AnInitValueIsTranslatedIntoTheColumnsOwnSpelling();
    AnEnumInitValueResolvesThroughTheEnumIndex();
    AnUnknownEnumYieldsNoInitValueAtAll();
    try {
      const Session session(AGIRU_TEST_DSN);
      InitReturnsEveryFieldButTheKeyToItsDefault();
    } catch (const Error &e) { CHECK_TEXT("the gate needs a database", e.what(), "a database"); }
  });
}
