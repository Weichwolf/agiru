#include "system/visualization/page/CopyGenericChart.h"

#include "Check.h"
#include "platform/Chart.h"

int main() {
  return gate::Run("ActualChart", [] {
    agiru::System::Visualization::CopyGenericChart_Page page;
    agiru::platform::Chart first;
    first.ID = "  source  ";
    first.Name = "  Mixed title  ";
    first.BLOB.Set({1, 2, 3});
    CHECK_TEXT("actual page starts with its declared caption", page.Caption(), "Copy Generic Chart");
    page.SetSourceChart(first);
    CHECK_TEXT("actual setter appends the original Code field", page.Caption(), "Copy Generic Chart SOURCE");
    CHECK_TEXT("actual setter owns source ID", page.SourceChart->ID.Value(), "SOURCE");
    CHECK_TEXT("actual setter preserves Text case and spaces", page.SourceChart->Name.Value(), "  Mixed title  ");
    CHECK_TRUE("actual setter owns copied Blob content", page.SourceChart->BLOB == first.BLOB);
    first.ID = "changed";
    first.Name = "Changed title";
    first.BLOB.Set({});
    CHECK_TEXT("later caller changes cannot alter the page ID", page.SourceChart->ID.Value(), "SOURCE");
    CHECK_TEXT("later caller changes cannot alter page Text", page.SourceChart->Name.Value(), "  Mixed title  ");
    CHECK_TRUE("later caller changes cannot clear the page Blob", page.SourceChart->BLOB.HasValue());
    agiru::platform::Chart second;
    second.ID = "next";
    second.Name = "Second";
    page.SetSourceChart(second);
    CHECK_TEXT("actual repeated call uses the current caption", page.Caption(), "Copy Generic Chart SOURCE NEXT");
    CHECK_TEXT("actual repeated call replaces record values", page.SourceChart->Name.Value(), "Second");
    agiru::System::Visualization::CopyGenericChart_Page other;
    CHECK_TEXT("another page has independent caption state", other.Caption(), "Copy Generic Chart");
    other.SetSourceChart(second);
    CHECK_TEXT("another page owns its own caption", other.Caption(), "Copy Generic Chart NEXT");
  });
}
