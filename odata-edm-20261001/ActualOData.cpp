#include "system/integration/page/ODataEDMDefinitionCard.h"

#include "Check.h"
#include "platform/ODataEdmType.h"
#include "runtime/Table.h"
#include "type/Text.h"

int main() {
  return gate::Run("ActualOData", [] {
    agiru::System::Integration::ODataEDMDefinitionCard_Page page;
    agiru::detail::RuntimeMakeTemporary(&page.Rec, &agiru::kTempOps<agiru::platform::ODataEdmType>);
    CHECK_TRUE("fixture explicitly uses temporary storage", page.Rec.IsTemporary());
    CHECK_TEXT("actual getter accepts an empty Blob", agiru::AsText(page.GetEDMXML()), "");
    page.Rec.Key = "FIXTURE";
    page.Rec.Description = "Fixture definition";
    page.Rec.Insert();
    CHECK_TRUE("fixture starts with one temporary record", page.Rec.Count() == 1);
    const agiru::Text<0> first{"<Schema Name=\"Ägirū\"/>"};
    page.SetEDMXML(first);
    CHECK_TEXT("actual setter/getter UTF8 roundtrip", agiru::AsText(page.GetEDMXML()), agiru::AsText(first));
    CHECK_TRUE("actual setter modifies the temporary row", page.Rec.FindFirst());
    CHECK_TEXT("actual getter reads the stored first XML", agiru::AsText(page.GetEDMXML()), agiru::AsText(first));
    page.OnAfterGetRecord();
    CHECK_TEXT("actual record trigger reads the stored XML", agiru::AsText(page.ODataEDMXMLTxt), agiru::AsText(first));
    const agiru::Text<0> replacement{"<B/>"};
    page.SetEDMXML(replacement);
    CHECK_TEXT("actual setter replaces a longer Blob", agiru::AsText(page.GetEDMXML()), agiru::AsText(replacement));
    CHECK_TRUE("actual replacement remains stored", page.Rec.FindFirst());
    CHECK_TEXT("stored replacement has no stale tail", agiru::AsText(page.GetEDMXML()), agiru::AsText(replacement));
    CHECK_TEXT("actual setter preserves the original key", page.Rec.FieldFormat(agiru::FieldNo{1}), "FIXTURE");
    CHECK_TRUE("actual setter does not insert extra records", page.Rec.Count() == 1);
  });
}
