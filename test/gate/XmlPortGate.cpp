#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/XmlPort.h"
#include "type/Blob.h"
#include "type/Stream.h"
#include "type/TextEncoding.h"

#include "Check.h"

#include <array>
#include <string>
#include <string_view>

namespace {

using agiru::XmlPortDef;
using agiru::XmlPortFormat;
using agiru::XmlPortInput;
using agiru::XmlPortOutput;

/// A SEPARATOR PROPERTY IS SPELLED WITH PLACEHOLDERS -- `<TAB>`, `<NewLine>`, `<None>`, `<,>` --
/// and `FieldSeparator('')` at run time overrides it (board:0065).
void SeparatorTextReadsThePlaceholders() {
  CHECK_TEXT("a tab", agiru::detail::SeparatorText("<TAB>"), "\t");
  CHECK_TEXT("a new line is CR LF, what the platform writes",
             agiru::detail::SeparatorText("<NewLine>"),
             "\r\n");
  CHECK_TEXT("two new lines", agiru::detail::SeparatorText("<NewLine><NewLine>"), "\r\n\r\n");
  CHECK_TEXT("none is nothing", agiru::detail::SeparatorText("<None>"), "");
  CHECK_TEXT("a character in brackets is itself", agiru::detail::SeparatorText("<,>"), ",");
  CHECK_TEXT("plain text is itself", agiru::detail::SeparatorText(";"), ";");
}

/// VARIABLE TEXT: a table element's record is a line, its values the fields between the
/// separator, each wrapped in the delimiter; FIXED TEXT: each value padded to its `Width`;
/// XML: the element tree with attributes and escaping (`devenv-format-property.md`).
void TheOutputWritesTheThreeFormats() {
  XmlPortDef variable{
      .id = agiru::XmlPortId{1}, .name = "V", .format = XmlPortFormat::VariableText};
  XmlPortOutput out;
  out.Start(variable, ";", "\r\n", "\"", "\r\n\r\n");
  out.BeginGroup("Root");
  out.BeginRecord("Line");
  out.Value("A", "one", false, 0);
  out.Value("B", "two;three", false, 0);
  out.EndRecord("Line");
  out.BeginRecord("Line");
  out.Value("A", "four", false, 0);
  out.EndRecord("Line");
  out.EndGroup("Root");
  CHECK_TEXT("variable text lines", out.Finish(), "\"one\";\"two;three\"\r\n\"four\"\r\n");

  XmlPortDef fixed{.id = agiru::XmlPortId{2}, .name = "F", .format = XmlPortFormat::FixedText};
  XmlPortOutput fixedOut;
  fixedOut.Start(fixed, "", "\r\n", "", "");
  fixedOut.BeginRecord("Line");
  fixedOut.Value("A", "ab", false, 4);
  fixedOut.Value("B", "toolong", false, 3);
  fixedOut.EndRecord("Line");
  CHECK_TEXT("fixed text pads and cuts to the width", fixedOut.Finish(), "ab  too\r\n");

  XmlPortDef xml{.id = agiru::XmlPortId{3}, .name = "X", .format = XmlPortFormat::Xml};
  XmlPortOutput xmlOut;
  xmlOut.Start(xml, "\t", "\r\n", "", "");
  xmlOut.BeginGroup("Root");
  xmlOut.BeginRecord("Line");
  xmlOut.Value("id", "7", true, 0);
  xmlOut.Value("Name", "A & B <C>", false, 0);
  xmlOut.EndRecord("Line");
  xmlOut.EndGroup("Root");
  CHECK_TEXT("xml nests, attributes on the open element, text escaped",
             xmlOut.Finish(),
             "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>"
             "<Root><Line id=\"7\"><Name>A &amp; B &lt;C&gt;</Name></Line></Root>");
}

/// THE INPUT IS ONE CURSOR FOR ALL FORMATS: `Enter` the root, `Enter` a record, `Enter` its
/// fields in order -- a text format serves lines and fields under the same names.
void TheInputWalksTextAndXmlAlike() {
  XmlPortDef variable{
      .id = agiru::XmlPortId{1}, .name = "V", .format = XmlPortFormat::VariableText};
  XmlPortInput in;
  in.Load("\"one\";\"two;three\"\r\nfour;five\r\n", variable, ";", "\r\n", "\"");
  CHECK_TRUE("the root opens", in.Enter("Root"));
  CHECK_TRUE("the first line", in.Enter("Line"));
  CHECK_TRUE("its first field", in.Enter("A"));
  CHECK_TEXT("without the delimiter", in.Text(), "one");
  in.Leave();
  CHECK_TRUE("its second field", in.Enter("B"));
  CHECK_TEXT("a delimited field keeps the separator inside it", in.Text(), "two;three");
  in.Leave();
  CHECK_TRUE("no third field", !in.Enter("C"));
  in.Leave();
  CHECK_TRUE("the second line", in.Enter("Line"));
  CHECK_TRUE("its first field", in.Enter("A"));
  CHECK_TEXT("plain", in.Text(), "four");
  in.Leave();
  in.Leave();
  CHECK_TRUE("no third line", !in.Enter("Line"));
  in.Leave();

  XmlPortDef xml{.id = agiru::XmlPortId{3}, .name = "X", .format = XmlPortFormat::Xml};
  XmlPortInput xmlIn;
  xmlIn.Load("<Root><Line id=\"7\"><Name>A &amp; B</Name></Line><Line><Name>C</Name></Line></Root>",
             xml,
             "",
             "",
             "");
  CHECK_TRUE("the xml root", xmlIn.Enter("root"));
  CHECK_TRUE("the first record", xmlIn.Enter("Line"));
  CHECK_TEXT("an attribute", xmlIn.Attribute("id"), "7");
  CHECK_TRUE("its field", xmlIn.Enter("Name"));
  CHECK_TEXT("unescaped", xmlIn.Text(), "A & B");
  xmlIn.Leave();
  xmlIn.Leave();
  CHECK_TRUE("the second record", xmlIn.Enter("Line"));
  CHECK_TEXT("no attribute is empty", xmlIn.Attribute("id"), "");
  xmlIn.Leave();
  CHECK_TRUE("no third", !xmlIn.Enter("Line"));
}

/// THE NAMESPACES ARE DECLARED ON THE ROOT AND NOWHERE ELSE. `devenv-namespaces-property.md`: "the
/// namespaces declarations are only supported in the root element ... `<Root xmlns:mybcprefix=
/// "mybcnamespace" xmlns="urn:bc:schema:all">`"; an element's `NamespacePrefix` is part of its
/// name, and on the way back in a prefixed name is matched by its local part, because the parsed
/// tree carries local names. `Sales Invoice - PEPPOL 3.0` exports `<Invoice xmlns="urn:oasis:...
/// Invoice-2" ...>`, and `Data Exch. Line Def.ValidateNamespace` reads that namespace back from
/// the root (Incoming Doc. To Data Exch. UT, 13 cases, 2026-09-12).
void TheNamespacesAreDeclaredOnTheRoot() {
  static constexpr std::array<agiru::XmlNamespaceDef, 2> kSpaces{{
      agiru::XmlNamespaceDef{.prefix = "", .uri = "urn:bc:schema:all"},
      agiru::XmlNamespaceDef{.prefix = "cbc", .uri = "urn:bc:basic"},
  }};
  XmlPortDef xml{.id = agiru::XmlPortId{4},
                 .name = "N",
                 .format = XmlPortFormat::Xml,
                 .rootName = "Root",
                 .namespaces = kSpaces};
  XmlPortOutput out;
  out.Start(xml, "\t", "\r\n", "", "");
  out.BeginRecord("Root");
  out.Value("cbc:ID", "7", false, 0);
  out.BeginGroup("Line");
  out.Value("cbc:Name", "A", false, 0);
  out.EndGroup("Line");
  out.EndRecord("Root");
  const std::string written = out.Finish();
  CHECK_TEXT("the root carries every declaration and the prefixed names stay prefixed",
             written,
             "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>"
             "<Root xmlns=\"urn:bc:schema:all\" xmlns:cbc=\"urn:bc:basic\"><cbc:ID>7</cbc:ID>"
             "<Line><cbc:Name>A</cbc:Name></Line></Root>");

  XmlPortDef plain{.id = agiru::XmlPortId{5},
                   .name = "D",
                   .format = XmlPortFormat::Xml,
                   .rootName = "Root",
                   .defaultNamespace = "urn:bc:default",
                   .useDefaultNamespace = true};
  XmlPortOutput defaulted;
  defaulted.Start(plain, "\t", "\r\n", "", "");
  defaulted.BeginRecord("Root");
  defaulted.EndRecord("Root");
  CHECK_TEXT("UseDefaultNamespace writes DefaultNamespace as xmlns",
             defaulted.Finish(),
             "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>"
             "<Root xmlns=\"urn:bc:default\" />");

  XmlPortInput in;
  in.Load(written, xml, "", "", "");
  CHECK_TRUE("the root reads back", in.Enter("Root"));
  CHECK_TRUE("and a prefixed name finds the element by its local part", in.Enter("cbc:ID"));
  CHECK_TEXT("with its text", in.Text(), "7");
  in.Leave();
  CHECK_TRUE("the group", in.Enter("Line"));
  CHECK_TRUE("and its prefixed child", in.Enter("cbc:Name"));
  CHECK_TEXT("holds A", in.Text(), "A");
}

/// `Xmlport.Export(Number, ...)` resolves through the catalogue; a number this build carries no
/// xmlport for refuses with the number (board:0034).
void AnUnknownNumberRefusesByName() {
  CHECK_TRUE("no xmlport 999999", agiru::FindXmlPort(agiru::XmlPortId{999999}) == nullptr);
  agiru::Blob blob;
  agiru::OutStream out = blob.CreateOutStream();
  std::string message;
  try {
    static_cast<void>(agiru::XmlPort<>::Export(999999, out));
  } catch (const agiru::Error &e) { message = e.what(); }
  CHECK_TRUE("the refusal names the number",
             message.find("Xmlport.Export(999999)") != std::string::npos);
  CHECK_TEXT("an encoding round trip keeps the text",
             agiru::detail::DecodeForXmlPort(
                 agiru::detail::EncodeForXmlPort("Ärger", agiru::TextEncoding::Windows),
                 agiru::TextEncoding::Windows),
             "Ärger");
}

}

int main() {
  return gate::Run("XmlPort", [] {
    SeparatorTextReadsThePlaceholders();
    TheOutputWritesTheThreeFormats();
    TheInputWalksTextAndXmlAlike();
    TheNamespacesAreDeclaredOnTheRoot();
    AnUnknownNumberRefusesByName();
  });
}
