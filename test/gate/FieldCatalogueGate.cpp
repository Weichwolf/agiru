#include "meta/SystemFields.h"
#include "platform/Company.h"
#include "platform/Field.h"
#include "runtime/ErrorValue.h"
#include "runtime/RecordRef.h"
#include "runtime/RecordState.h"
#include "runtime/Table.h"
#include "type/Guid.h"

#include "Check.h"

#include <cstdint>
#include <limits>
#include <string>

namespace {

using agiru::platform::Company;
using agiru::platform::Field;

constexpr std::int32_t kCompanyDeclaredFields = 5;
constexpr std::int32_t kPositiveRuntime18ImplicitFields = 9;
constexpr auto kLastCompanyField = Company::Field_No::BusinessProfileId.Value();
constexpr auto kCompanyIdentityField = Company::Field_No::Id.Value();

std::string Failure(auto &&operation) {
  try {
    operation();
  } catch (const agiru::Error &error) { return error.what(); }
  return {};
}

void CountsAndNavigationUseOriginalPositiveKeys() {
  Field rows;
  rows.SetRange(rows.TableNo, Company::kId.Value());
  CHECK_TRUE("native Field is not temporary", !rows.IsTemporary());
  CHECK_TRUE("native Field counts declared plus positive Runtime-18 fields",
             rows.Count() == kCompanyDeclaredFields + kPositiveRuntime18ImplicitFields);
  CHECK_TRUE("native Field IsEmpty uses the same population", !rows.IsEmpty());
  rows.SetFilter(rows.No, "<%1", agiru::SystemFieldNumbers::SystemId.Value());
  CHECK_TRUE("ordinary-field census excludes timestamp zero",
             rows.Count() == kCompanyDeclaredFields);
  CHECK_TRUE("native FindSet needs no SQL snapshot or database session", rows.FindSet());
  CHECK_TRUE("first native field is the original positive key", rows.No == 1);
  std::int32_t seen = 1;
  while (rows.Next() != 0) {
    ++seen;
    CHECK_TRUE("every native field key is positive", rows.No > 0);
  }
  CHECK_TRUE("native navigation retains the complete filtered population",
             seen == kCompanyDeclaredFields);
  const auto *state = reinterpret_cast<const agiru::detail::StateHandle *>(&rows)->Peek();
  CHECK_TRUE("native cursor stores no copied catalogue rows",
             state != nullptr && state->view.empty());
  CHECK_TRUE("native cursor opens no SQL portal",
             state != nullptr && state->open.Held() == nullptr);
  CHECK_TRUE("Count does not move the current buffer",
             rows.Count() == seen && rows.No == kLastCompanyField);
  CHECK_TRUE("native Next zero preserves the current row",
             rows.Next(0) == 0 && rows.No == kLastCompanyField);
  CHECK_TRUE("native negative Next returns actual signed movement",
             rows.Next(-2) == -2 && rows.No == 3);
  CHECK_TRUE("oversized reverse steps stop at the real beginning",
             rows.Next(std::numeric_limits<std::int32_t>::min()) == -2 && rows.No == 1);
  CHECK_TRUE("native FindLast respects the filter",
             rows.FindLast() && rows.No == kLastCompanyField);
  rows.Ascending(false);
  CHECK_TRUE("descending native FindFirst selects the highest key",
             rows.FindFirst() && rows.No == kLastCompanyField);
  CHECK_TRUE("descending native Next reverses primary order",
             rows.Next() == 1 && rows.No == kCompanyIdentityField);
  CHECK_TRUE("descending native reverse Next restores the prior row",
             rows.Next(-1) == -1 && rows.No == kLastCompanyField);
  rows.SetRange(rows.No, 0);
  CHECK_TRUE("timestamp zero is absent from native Count and IsEmpty",
             rows.Count() == 0 && rows.IsEmpty());
  CHECK_TRUE("timestamp zero is absent from native FindFirst", !rows.FindFirst());
  CHECK_TRUE("failed native Find unpositions the cursor", rows.Next() == 0);
}

void FiltersRelativeReadsMarksAndBuffersRemainIndependent() {
  Field rows;
  rows.SetRange(rows.TableNo, Company::kId.Value());
  rows.SetRange(rows.No, 1, kLastCompanyField);
  CHECK_TRUE("native Get ignores the current filter", rows.Get(Company::kId.Value(), 1));
  rows.SetRange(rows.No, 3, kLastCompanyField);
  CHECK_TRUE("native Find equality preserves filter semantics", !rows.Find("="));
  CHECK_TRUE("relative native Find chooses the next admitted row", rows.Find(">") && rows.No == 3);
  rows.Mark(true);
  CHECK_TRUE("native marks retain variable-local keys", rows.Mark());
  rows.MarkedOnly(true);
  CHECK_TRUE("native Count applies marks", rows.Count() == 1);
  CHECK_TRUE("native Find applies the same marks", rows.FindFirst() && rows.No == 3);
  CHECK_TRUE("native Next applies marks", rows.Next() == 0);
  rows.MarkedOnly(false);
  CHECK_TRUE("native FindSet restores the filtered population", rows.FindSet());
  rows.No = kCompanyIdentityField;
  CHECK_TRUE("buffer key edits do not replace an unchanged native cursor bookmark",
             rows.Next() == 1 && rows.No == kCompanyIdentityField);
  rows.SetRange(rows.No, kLastCompanyField);
  CHECK_TRUE("changed filters re-anchor native navigation",
             rows.Next() == 1 && rows.No == kLastCompanyField);

  Field other;
  other.SetRange(other.TableNo, Company::kId.Value());
  other.SetRange(other.No, 1, 2);
  CHECK_TRUE("another native handle retains its own filters and cursor",
             other.FindSet() && other.No == 1);
  CHECK_TRUE("other native handles do not change the first buffer", rows.No == kLastCompanyField);
  other.FilterGroup(-1);
  other.SetRange(other.No, 1);
  other.SetRange(other.FieldName, "Evaluation Company");
  other.FilterGroup(0);
  CHECK_TRUE("native cross-column filters OR without escaping normal groups", other.Count() == 2);
  other.Reset();
  other.SetRange(other.TableNo, Company::kId.Value());
  other.SetRange(other.No, 1, kLastCompanyField);
  other.SetView(
      "SORTING(No.) ORDER(Descending) WHERE(TableNo=FILTER(2000000006),No.=FILTER(1..8005))");
  CHECK_TRUE("nonindexed native order preserves real record ordering",
             other.FindFirst() && other.No == kLastCompanyField);
  CHECK_TRUE("nonindexed native order preserves signed navigation",
             other.Next(2) == 2 && other.No == 3);
  CHECK_TRUE("invalid native Find forms refuse instead of changing search semantics",
             !Failure([&] { other.Find("-="); }).empty());
}

void TypedAndReflectedNamesUseTheLiveCatalogue() {
  Company source;
  source.SystemId = agiru::Guid("11111111-2222-3333-4444-555555555555");
  Field rows;
  rows.SetRange(rows.TableNo, Company::kId.Value());
  rows.SetRange(rows.FieldName, source.FieldName(source.SystemId));
  CHECK_TRUE("the actual FieldName caller finds native system identity", rows.FindFirst());
  CHECK_TRUE("native name lookup preserves the reserved identity number",
             rows.No == agiru::SystemFieldNumbers::SystemId.Value());
  agiru::RecordRef sourceRef;
  sourceRef.GetTable(source);
  const auto value = sourceRef.Field(rows.No).Value();
  CHECK_TRUE("the native name-derived FieldRef reads the actual typed source value",
             value.IsGuid() && value.Get<agiru::Guid>() == source.SystemId);

  rows.Reset();
  rows.SetRange(rows.TableNo, Company::kId.Value());
  rows.SetRange(rows.No, 1, Company::Field_No::DisplayName.Value());
  agiru::RecordRef reflected;
  reflected.GetTable(rows);
  CHECK_TRUE("RecordRef native Count shares typed cardinality", reflected.Count() == rows.Count());
  CHECK_TRUE("RecordRef native FindSet shares the immutable provider", reflected.FindSet());
  reflected.SetTable(rows);
  CHECK_TRUE("RecordRef native FindSet preserves exact key values", rows.No == 1);
  CHECK_TRUE("RecordRef native Next shares signed movement", reflected.Next(2) == 2);
  reflected.SetTable(rows);
  CHECK_TRUE("RecordRef native navigation lands on the exact row", rows.No == 3);
  CHECK_TRUE("native catalogue version is the original frozen value", rows.SystemRowVersion == 1);
  CHECK_TRUE("native catalogue identity follows the original key encoding",
             rows.SystemId == agiru::Guid("77359429-9406-7735-0300-000000000000"));

  CHECK_TRUE("native catalogue insertion refuses", !Failure([&] { rows.Insert(); }).empty());
  CHECK_TRUE("native catalogue modification refuses", !Failure([&] { rows.Modify(); }).empty());
  CHECK_TRUE("native catalogue deletion refuses", !Failure([&] { rows.Delete(); }).empty());
  CHECK_TRUE("native catalogue DeleteAll refuses", !Failure([&] { rows.DeleteAll(); }).empty());
  rows.SetRange(rows.No, 0);
  CHECK_TRUE("native ModifyAll refuses even when no row matches",
             !Failure([&] { rows.ModifyAll(rows.Enabled, false); }).empty());
  CHECK_TRUE("native triggered DeleteAll refuses even when no row matches",
             !Failure([&] { rows.DeleteAll(true); }).empty());
  rows.SetFilter(rows.SQLDataType, "0");
  CHECK_TRUE("unqualified attributes refuse rather than silently filtering default values",
             !Failure([&] { static_cast<void>(rows.Count()); }).empty());

  agiru::Temporary<Field> temporary;
  temporary.TableNo = Company::kId.Value();
  temporary.No = 0;
  temporary.Insert();
  CHECK_TRUE("temporary catalogue zero keys remain independently writable and navigable",
             temporary.Count() == 1 && temporary.FindSet() && temporary.No == 0);
  temporary.DeleteAll();
  CHECK_TRUE("temporary catalogue deletion does not affect the live source", temporary.IsEmpty());
  CHECK_TRUE("empty temporary bulk writes remain valid",
             Failure([&] { temporary.ModifyAll(temporary.Enabled, false); }).empty());
  CHECK_TRUE("empty temporary triggered deletion remains valid",
             Failure([&] { temporary.DeleteAll(true); }).empty());
}

}

int main() {
  return gate::Run("FieldCatalogue", [] {
    CountsAndNavigationUseOriginalPositiveKeys();
    FiltersRelativeReadsMarksAndBuffersRemainIndependent();
    TypedAndReflectedNamesUseTheLiveCatalogue();
  });
}
