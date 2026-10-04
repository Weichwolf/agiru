#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "platform/AllProfile.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordRef.h"
#include "runtime/RecordState.h"
#include "runtime/Session.h"
#include "runtime/Storage.h"
#include "runtime/Table.h"
#include "runtime/TableDefinition.h"
#include "type/Decimal.h"
#include "type/FieldClass.h"

#include "Check.h"
#include "ResourceCost.h"

#include <array>
#include <cstddef>
#include <span>
#include <string_view>

namespace {

using agiru::FieldNo;
using agiru::detail::RecordState;
constexpr FieldNo kA{1};
constexpr FieldNo kB{2};
constexpr FieldNo kC{3};
constexpr FieldNo kBlob{4};
constexpr FieldNo kFilter{5};
constexpr FieldNo kFlow{6};
constexpr std::array fields{
    agiru::FieldDef{.name = "A", .no = kA, .type = agiru::FieldType::Integer},
    agiru::FieldDef{.name = "B", .no = kB, .type = agiru::FieldType::Integer},
    agiru::FieldDef{.name = "C", .no = kC, .type = agiru::FieldType::Integer},
    agiru::FieldDef{.name = "Payload", .no = kBlob, .type = agiru::FieldType::Blob},
    agiru::FieldDef{.name = "Flow Filter",
                    .no = kFilter,
                    .fieldClass = agiru::FieldClass::FlowFilter,
                    .type = agiru::FieldType::Integer},
    agiru::FieldDef{.name = "Flow",
                    .no = kFlow,
                    .fieldClass = agiru::FieldClass::FlowField,
                    .type = agiru::FieldType::Integer}};
constexpr std::array primary{kA};
constexpr std::array disabled{kB, kC};
constexpr std::array first{kB, kA};
constexpr std::array second{kB, kC};
constexpr std::array included{kC, kA};
constexpr std::array keys{
    agiru::KeyDef{.name = "PK", .fields = primary, .clustered = true},
    agiru::KeyDef{.name = "Disabled", .fields = disabled, .enabled = false},
    agiru::KeyDef{.name = "First", .fields = first},
    agiru::KeyDef{.name = "Second", .fields = second},
    agiru::KeyDef{.name = "Included", .fields = included, .includedFields = "B"}};
constexpr agiru::TableDef declaration{
    .name = "Current Key Fixture", .fields = fields, .keys = keys};

template <std::size_t N>
bool Selected(const RecordState &state, const std::array<FieldNo, N> &expected) {
  if (state.key.size() != expected.size()) { return false; }
  for (std::size_t i = 0; i < expected.size(); ++i) {
    if (state.key[i].field != expected[i] || !state.key[i].ascending) { return false; }
  }
  return true;
}

void DeclarationSelection() {
  RecordState state;
  constexpr std::array request{kB};
  CHECK_TRUE("a valid prefix succeeds", agiru::detail::SetCurrentKey(state, declaration, request));
  CHECK_TRUE("the first active prefix contributes its entire key", Selected(state, first));
  constexpr std::array unindexed{kA, kC};
  CHECK_TRUE("sortable unindexed fields succeed",
             agiru::detail::SetCurrentKey(state, declaration, unindexed));
  CHECK_TRUE("unindexed sorting preserves the requested order", Selected(state, unindexed));
  auto onlyIncluded = declaration;
  onlyIncluded.keys = std::span(keys).subspan(keys.size() - 1);
  CHECK_TRUE("IncludedFields do not change success",
             agiru::detail::SetCurrentKey(state, onlyIncluded, request));
  CHECK_TRUE("IncludedFields are not matched as key columns", Selected(state, request));
  for (const FieldNo no : {kBlob, kFilter}) {
    const std::array bad{no};
    CHECK_TRUE("known unsortable fields return false",
               !agiru::detail::SetCurrentKey(state, declaration, bad));
    CHECK_TRUE("failed selections preserve the prior key", Selected(state, request));
  }
  bool refused = false;
  try {
    constexpr std::array flow{kFlow};
    static_cast<void>(agiru::detail::SetCurrentKey(state, declaration, flow));
  } catch (const agiru::Error &error) {
    refused = std::string_view(error.what()).find("FlowField sorting is not implemented") !=
              std::string_view::npos;
  }
  CHECK_TRUE("unknown FlowField sort semantics refuse explicitly", refused);
  CHECK_TRUE("unsupported selections preserve the prior key", Selected(state, request));
  for (const auto invalid : {std::span<const FieldNo>{}, std::span<const FieldNo>{request}}) {
    auto broken = declaration;
    broken.fields = {};
    bool raised = false;
    try {
      static_cast<void>(agiru::detail::SetCurrentKey(state, broken, invalid));
    } catch (const agiru::Error &) { raised = true; }
    CHECK_TRUE("empty requests and absent fields refuse", raised);
    CHECK_TRUE("invalid requests preserve the prior key", Selected(state, request));
  }
}

void OriginalProfile() {
  constexpr int kHighRole = 20;
  agiru::Temporary<agiru::platform::AllProfile> rows;
  rows.ProfileID = "HIGH";
  rows.RoleCenterID = kHighRole;
  rows.Insert();
  rows.Init();
  rows.ProfileID = "LOW";
  rows.RoleCenterID = 10;
  rows.Insert();
  CHECK_TRUE("the original native declaration accepts unindexed sorting",
             rows.SetCurrentKey(rows.RoleCenterID));
  CHECK_TRUE("unindexed native sorting finds a row", rows.FindFirst());
  CHECK_TEXT("native rows follow the requested order", rows.ProfileID.Value(), "LOW");
}

using Cost = agiru::Projects::Resources::Pricing::ResourceCost_Table;

template <typename Row> void Wrappers(Row &rows) {
  CHECK_TRUE("typed unindexed sorting succeeds", rows.SetCurrentKey(rows.DirectUnitCost));
  CHECK_TEXT(
      "typed unindexed sorting keeps its source field", rows.CurrentKey(), "Direct Unit Cost");
  CHECK_TRUE("typed unindexed sorting finds its first row", rows.FindFirst());
  CHECK_TEXT("unindexed rows follow the requested value rather than the primary key",
             rows.WorkTypeCode.Value(),
             "Z");
  if (!rows.IsTemporary()) {
    CHECK_TRUE("SQL FindFirst opens a cursor", rows.State_Block.Peek()->open.Held() != nullptr);
  }
  CHECK_TRUE("typed prefix selection succeeds", rows.SetCurrentKey(rows.Type));
  CHECK_TRUE("successful key selection closes the previous SQL cursor",
             rows.State_Block.Peek()->open.Held() == nullptr);
  CHECK_TEXT("typed prefixes select the complete declared key",
             rows.CurrentKey(),
             "Type,Code,Work Type Code");
  CHECK_TRUE("typed prefix selection finds its first row", rows.FindFirst());
  CHECK_TEXT("the full declared prefix orders the first tied row", rows.WorkTypeCode.Value(), "A");
  CHECK_TRUE("typed prefix ordering can advance", rows.Next() == 1);
  CHECK_TEXT("the full declared prefix orders the next tied row", rows.WorkTypeCode.Value(), "Z");
  agiru::RecordRef reference;
  reference.GetTable(rows);
  reference.SetView("SORTING(Type)");
  CHECK_TRUE("reflected prefixes select the same key", reference.CurrentKeyIndex() == 1);
  CHECK_TRUE("reflected sorting finds the seeded row", reference.FindFirst());
  reference.SetTable(rows);
  CHECK_TEXT("reflected prefixes order the tied rows identically", rows.WorkTypeCode.Value(), "A");
  reference.SetView("SORTING(Direct Unit Cost)");
  CHECK_TRUE("unindexed reflected sorting has no declared key", reference.CurrentKeyIndex() == -1);
  CHECK_TRUE("unindexed reflected sorting finds the seeded row", reference.FindFirst());
  reference.SetTable(rows);
  CHECK_TEXT(
      "unindexed reflected ordering agrees with typed ordering", rows.WorkTypeCode.Value(), "Z");
}

template <typename Row> void Fill(Row &row) {
  row.Code = "ONE";
  row.WorkTypeCode = "Z";
  row.DirectUnitCost = agiru::Decimal::FromInvariantString("10.00");
  row.Insert();
  row.Init();
  row.Code = "ONE";
  row.WorkTypeCode = "A";
  row.DirectUnitCost = agiru::Decimal::FromInvariantString("20.00");
  row.Insert();
  row.Init();
  row.Code = "TWO";
  row.WorkTypeCode = "B";
  row.DirectUnitCost = agiru::Decimal::FromInvariantString("30.00");
  row.Insert();
}

}

int main() {
  return gate::Run("CurrentKey", [] {
    DeclarationSelection();
    OriginalProfile();
    agiru::Temporary<Cost> temporary;
    Fill(temporary);
    Wrappers(temporary);
    const agiru::Session session(AGIRU_TEST_DSN);
    const auto &table = agiru::TableDefinition<Cost>();
    agiru::DropTable(agiru::Session::Current().Database(), table);
    agiru::CreateTable(agiru::Session::Current().Database(), table);
    Cost stored;
    Fill(stored);
    Wrappers(stored);
  });
}
