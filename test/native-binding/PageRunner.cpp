#include "meta/PageDef.h"
#include "meta/TableDef.h"
#include "platform/PageTableField.h"
#include "runtime/Record.h"

#include "Check.h"
#include "system/tooling/page/PageFieldsSelectionList.h"

#include <stdexcept>
#include <string>

namespace {

using Page = agiru::System::Tooling::PageFieldsSelectionList_Page;

std::string ReadCaption(const Page &page) {
  const auto *control =
      agiru::Control(agiru::System::Tooling::kPageFieldsSelectionListPage.layout, "Caption");
  if (control == nullptr) { throw std::runtime_error("original Caption control is missing"); }
  const auto *field =
      agiru::Field(agiru::TableTraits<agiru::platform::PageTableField>::kTable, control->field);
  if (field == nullptr) { throw std::runtime_error("original Caption source field is missing"); }
  return agiru::FieldText(&page.Rec, *field);
}

void OriginalBareField() {
  Page first;
  Page second;
  first.Rec.Caption = "First record caption";
  second.Rec.Caption = "Second record caption";
  CHECK_TEXT("original bare Caption reads Rec, not Page.Caption",
             ReadCaption(first),
             "First record caption");
  CHECK_TEXT("page records remain instance-owned", ReadCaption(second), "Second record caption");
  first.Rec.Caption = "Changed";
  CHECK_TEXT("source binding reads the current record", ReadCaption(first), "Changed");
  const auto &metadata = agiru::System::Tooling::kPageFieldsSelectionListPage;
  CHECK_TRUE("original native source table ID survives",
             metadata.source == agiru::platform::PageTableField::kId);
  const auto *caption = agiru::Control(metadata.layout, "Caption");
  CHECK_TRUE("original field survives",
             caption != nullptr && caption->kind == agiru::ControlKind::Field);
  CHECK_TRUE("original Caption field number survives",
             caption != nullptr &&
                 caption->field == agiru::platform::PageTableField::Field_No::Caption);
}

}

int main() {
  return gate::Run("Original native page binding", OriginalBareField);
}
