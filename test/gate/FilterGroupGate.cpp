#include "runtime/ErrorValue.h"
#include "runtime/RecordRef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Integer.h"
#include "type/StringValue.h"
#include "type/Variant.h"

#include "Check.h"
#include "ResourceCost.h"

#include <array>
#include <string_view>

namespace {

using Cost = agiru::Projects::Resources::Pricing::ResourceCost_Table;

constexpr agiru::Integer kFirstIgnoredGroup = 256;

template <typename Record> void Groups(Record &record) {
  const Record &read = record;
  CHECK_TRUE("the initial group is zero", read.FilterGroup() == 0);
  constexpr std::array<agiru::Integer, 4> groups{-1, 0, 2, 255};
  for (const auto group : groups) {
    record.FilterGroup(group);
    CHECK_TRUE("a getter answers the selected group", read.FilterGroup() == group);
    CHECK_TRUE("a second getter does not reset it", read.FilterGroup() == group);
    record.FilterGroup(kFirstIgnoredGroup);
    CHECK_TRUE("group 256 is ignored", read.FilterGroup() == group);
    record.FilterGroup(agiru::IntegerRange::kMaximum);
    CHECK_TRUE("the greatest AL Integer group is ignored", read.FilterGroup() == group);
  }
  record.FilterGroup(0);
}

template <typename Read> void Refuses(std::string_view claim, Read read) {
  bool raised = false;
  try {
    static_cast<void>(read());
  } catch (const agiru::Error &) { raised = true; }
  CHECK_TRUE(claim, raised);
}

void LazyReads() {
  Cost record;
  const Cost &read = record;
  CHECK_TRUE("a fresh record has no allocated state", record.State_Block.Peek() == nullptr);
  CHECK_TRUE("a lazy group getter answers zero", read.FilterGroup() == 0);
  CHECK_TRUE("a lazy HasFilter answers false", !read.HasFilter());
  CHECK_TRUE("neither getter allocates state", record.State_Block.Peek() == nullptr);
  record.FilterGroup(kFirstIgnoredGroup);
  CHECK_TRUE("an ignored group does not allocate state", record.State_Block.Peek() == nullptr);
  record.FilterGroup(0);
  CHECK_TRUE("selecting the default group needs no state", record.State_Block.Peek() == nullptr);
}

template <typename Record> void TypedFilters(Record &record) {
  record.SetRange(record.Code, "A");
  record.FilterGroup(2);
  CHECK_TRUE("another group's filter does not fill this group", !record.HasFilter());
  CHECK_TRUE("a getter leaves the empty group selected", record.FilterGroup() == 2);
  record.SetRange(record.Code, "B");
  CHECK_TRUE("HasFilter finds the selected group's filter", record.HasFilter());
  CHECK_TEXT("GetFilter reads group two", record.GetFilter(record.Code).Value(), "B");
  record.SetRange(record.Code);
  CHECK_TRUE("clearing group two makes only that group empty", !record.HasFilter());
  record.FilterGroup(0);
  CHECK_TRUE("group zero remains filtered", record.HasFilter());
  CHECK_TEXT("clearing another group preserves the same field",
             record.GetFilter(record.Code).Value(),
             "A");
  record.Reset();
  CHECK_TRUE("Reset selects group zero", record.FilterGroup() == 0);
  CHECK_TRUE("Reset removes every group's filter", !record.HasFilter());
}

void RefFilters(agiru::RecordRef &record) {
  const auto field = record.Field(Cost::Field_No::Code.Value());
  field.SetFilter("A");
  record.FilterGroup(2);
  CHECK_TRUE("RecordRef HasFilter ignores other groups", !record.HasFilter());
  CHECK_TRUE("RecordRef getter leaves group two selected", record.FilterGroup() == 2);
  field.SetFilter("B");
  CHECK_TRUE("RecordRef HasFilter sees the selected group", record.HasFilter());
  CHECK_TEXT("FieldRef reads group two", field.GetFilter(), "B");
  CHECK_TEXT("RecordRef lists only group two", record.GetFilters(), "Code: B");
  field.SetRange();
  CHECK_TRUE("clearing group two does not report another group's filter", !record.HasFilter());
  record.FilterGroup(0);
  CHECK_TEXT("clearing group two retains group zero", field.GetFilter(), "A");
  record.Reset();
  CHECK_TRUE("RecordRef Reset selects group zero", record.FilterGroup() == 0);
  CHECK_TRUE("RecordRef Reset clears all filters", !record.HasFilter());
}

void Copies() {
  Cost original;
  original.FilterGroup(2);
  original.SetRange(original.Code, "B");
  Cost copied;
  copied.Copy(original);
  CHECK_TRUE("Record.Copy preserves the group", copied.FilterGroup() == 2);
  copied.FilterGroup(0);
  CHECK_TRUE("Record.Copy group state is independent", original.FilterGroup() == 2);
  copied.FilterGroup(2);
  copied.SetRange(copied.Code);
  CHECK_TRUE("Record.Copy filters are independent", original.HasFilter());
  agiru::RecordRef ref;
  ref.GetTable(original);
  CHECK_TRUE("GetTable copies the current group", ref.FilterGroup() == 2);
  agiru::RecordRef alias = ref;
  alias.FilterGroup(4);
  CHECK_TRUE("RecordRef assignment shares the group", ref.FilterGroup() == 4);
  CHECK_TRUE("GetTable does not change the source record", original.FilterGroup() == 2);
  agiru::RecordRef independent = ref.Duplicate();
  independent.FilterGroup(2);
  CHECK_TRUE("Duplicate preserves filters in the independent group", independent.HasFilter());
  CHECK_TRUE("Duplicate group selection is independent", ref.FilterGroup() == 4);
  independent.Field(Cost::Field_No::Code.Value()).SetRange();
  ref.FilterGroup(2);
  CHECK_TRUE("Duplicate clearing preserves the original filter", ref.HasFilter());
}

void Populate(agiru::Temporary<Cost> &record) {
  constexpr std::array<std::string_view, 4> codes{"A", "B", "C", "D"};
  for (const auto code : codes) {
    record.Code = code;
    record.WorkTypeCode = code == "A" || code == "C" ? "X" : "Y";
    record.Insert();
  }
}

void Intersections() {
  agiru::Temporary<Cost> record;
  Populate(record);
  record.SetFilter(record.Code, "A..C");
  record.FilterGroup(2);
  record.SetFilter(record.Code, "B..D");
  CHECK_TRUE("two groups on the same field intersect", record.Count() == 2);
  CHECK_TRUE("all groups apply while group two is selected", record.FindFirst());
  CHECK_TEXT("intersection starts at B", record.Code.Value(), "B");
  record.FilterGroup(0);
  CHECK_TRUE("changing the selected group does not deactivate filters", record.Count() == 2);
  record.FilterGroup(2);
  record.SetRange(record.Code);
  CHECK_TRUE("clearing one group restores only its excluded row", record.Count() == 3);
  record.Reset();
  record.FilterGroup(-1);
  record.SetRange(record.Code, "A");
  record.SetRange(record.WorkTypeCode, "Y");
  CHECK_TRUE("cross-column fields OR together", record.Count() == 3);
  record.FilterGroup(0);
  record.SetFilter(record.Code, "B..C");
  CHECK_TRUE("the cross-column union intersects ordinary groups", record.Count() == 1);
  agiru::RecordRef ref;
  ref.GetTable(record);
  CHECK_TRUE("RecordRef retains the same cross-group intersection", ref.Count() == 1);
  CHECK_TRUE("RecordRef finds the same intersection", ref.FindFirst());
  CHECK_TEXT(
      "RecordRef reaches B",
      std::string_view(ref.Field(Cost::Field_No::Code.Value()).Value().Get<agiru::Text<0>>()),
      "B");
  ref.FilterGroup(-1);
  CHECK_TRUE("RecordRef selection does not change the intersection", ref.Count() == 1);
  ref.Field(Cost::Field_No::Code.Value()).SetRange();
  ref.Field(Cost::Field_No::WorkTypeCode.Value()).SetRange();
  CHECK_TRUE("clearing both cross-column fields empties only that group", !ref.HasFilter());
  CHECK_TRUE("ordinary filters still apply", ref.Count() == 2);
  CHECK_TRUE("the source record retains its independent cross-column filters", record.Count() == 1);
  record.Reset();
  CHECK_TRUE("Reset retains temporary rows and removes all group constraints", record.Count() == 4);
}

void ClosedRef() {
  agiru::RecordRef record;
  const agiru::RecordRef &read = record;
  Refuses("a closed RecordRef group getter refuses", [&] { return read.FilterGroup(); });
  Refuses("a closed RecordRef group setter refuses", [&] { return record.FilterGroup(2); });
  Refuses("a closed RecordRef HasFilter refuses", [&] { return read.HasFilter(); });
}

}

int main() {
  return gate::Run("FilterGroup", [] {
    LazyReads();
    Cost regular;
    Groups(regular);
    TypedFilters(regular);
    agiru::Temporary<Cost> temporary;
    Groups(temporary);
    TypedFilters(temporary);
    agiru::RecordRef copied;
    copied.GetTable(regular);
    Groups(copied);
    RefFilters(copied);
    agiru::RecordRef owned;
    owned.Open(Cost::kId.Value(), true);
    Groups(owned);
    RefFilters(owned);
    Copies();
    Intersections();
    ClosedRef();
  });
}
