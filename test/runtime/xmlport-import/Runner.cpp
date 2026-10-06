#include "runtime/ErrorValue.h"
#include "runtime/Session.h"
#include "type/Blob.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/TextEncoding.h"

#include "Check.h"
#include "fixture/xmlport/ImportValidationConsumer.h"

#include <stdexcept>
#include <string>
#include <string_view>

namespace {
using Consumer = agiru::Fixture::ImportValidationConsumer_XmlPort;
constexpr agiru::Integer kRejectedFixtureValue = 99;

void Import(Consumer &consumer, std::string_view value) {
  agiru::Blob source;
  agiru::OutStream output;
  source.CreateOutStream(output, agiru::TextEncoding::UTF8);
  std::string xml = R"(<Root><Row ID="1" Other="20"><Value>)";
  xml.append(value);
  xml.append("</Value><Raw>30</Raw></Row></Root>");
  output.WriteText(xml);
  agiru::InStream input;
  source.CreateInStream(input, agiru::TextEncoding::UTF8);
  CHECK_TRUE("the generated XMLport imports the fixture", consumer.Import(input));
}

void ValuesAndOrder(bool temporary, bool defaults) {
  Consumer consumer{};
  consumer.SetImportProperties("fixture.xml", true);
  CHECK_TEXT("AL property syntax sets the owned filename",
             consumer.PropertyFilename().Value(),
             "fixture.xml");
  CHECK_TRUE("AL property syntax sets the owned import flag", consumer.PropertyImporting());
  Consumer copy = consumer;
  copy.SetImportProperties("copy.xml", false);
  CHECK_TEXT("copy property writes do not rebind the original owner",
             consumer.Filename().Value(),
             "fixture.xml");
  CHECK_TRUE("copy property writes retain the original import flag", consumer.ImportFile());
  CHECK_TEXT("method and property getter forms agree", copy.Filename().Value(), "copy.xml");
  CHECK_TRUE("method and property flag getter forms agree", !copy.ImportFile());
  Import(consumer, "10");
  CHECK_TRUE("OnAfterAssignField sees the imported element before validation",
             consumer.ValueObserved() == 10);
  CHECK_TRUE("OnAfterAssignField sees the imported attribute before validation",
             consumer.OtherObserved() == 20);
  CHECK_TRUE("FieldValidate no still runs the assignment trigger", consumer.RawObserved() == 30);
  CHECK_TRUE("element changes survive validation and reach the insert boundary",
             consumer.ValueFinal() == 11);
  CHECK_TRUE("attribute changes survive validation and reach the insert boundary",
             consumer.OtherFinal() == 22);
  CHECK_TRUE("unvalidated field changes reach the insert boundary", consumer.RawFinal() == 33);
  CHECK_TRUE("temporary XMLport sources bypass field validation, not assignment triggers",
             consumer.Validations() == (temporary ? 0 : (defaults ? 2 : 1)));
  CHECK_TRUE("explicit Yes validates the changed element on physical source nodes",
             consumer.ValueValidated() == (temporary ? 0 : 11));
  CHECK_TRUE("Undefined inherits the XMLport default on physical source nodes",
             consumer.OtherValidated() == (!temporary && defaults ? 22 : 0));
  CHECK_TRUE("AutoSave false retains the before-insert boundary", consumer.InsertBoundaries() == 1);
}

void ErrorsAndExplicitValidation(bool temporary) {
  Consumer consumer{};
  bool refused = false;
  try {
    Import(consumer, "98");
  } catch (const agiru::Error &error) {
    refused = std::string_view(error.what()) == "Rejected import value";
  }
  CHECK_TRUE("physical field validation errors propagate; temporary import alone bypasses them",
             refused != temporary);
  CHECK_TRUE("failed validation never reaches the before-insert boundary",
             consumer.InsertBoundaries() == (temporary ? 1 : 0));
  if (temporary) {
    bool explicitRefused = false;
    try {
      consumer.Row->Validate(consumer.Row->Value, kRejectedFixtureValue);
    } catch (const agiru::Error &error) {
      explicitRefused = std::string_view(error.what()) == "Rejected import value";
    }
    CHECK_TRUE("ordinary explicit Validate on a temporary record is not bypassed", explicitRefused);
  }
}
}

int main(int argc, char **argv) {
  return gate::Run("Generated XMLport Import", [argc, argv] {
    if (argc != 4) { throw std::runtime_error("expected gate DSN, temporary and default flags"); }
    const agiru::Session session(argv[1]);
    const bool temporary = std::string_view(argv[2]) == "true";
    const bool defaults = std::string_view(argv[3]) == "true";
    ValuesAndOrder(temporary, defaults);
    ErrorsAndExplicitValidation(temporary);
  });
}
