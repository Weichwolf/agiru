#include "dotnet/XmlDocument.h"
#include "dotnet/XmlNode.h"
#include "dotnet/XmlReader.h"
#include "runtime/ErrorValue.h"
#include "type/Blob.h"
#include "type/Stream.h"
#include "type/StringValue.h"

#include "Check.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

using agiru::dotnet::StringReader;
using agiru::dotnet::XmlNodeType;
using agiru::dotnet::XmlReader;
using agiru::dotnet::XmlReaderSettings;

namespace {

constexpr std::uint8_t kUtf16LeBomFirst = 0xff;
constexpr std::uint8_t kUtf16LeBomSecond = 0xfe;

static_assert(std::is_const_v<decltype(StringReader::StringReader)>);
static_assert(std::is_const_v<decltype(XmlReaderSettings::XmlReaderSettings)>);
static_assert(std::is_const_v<decltype(agiru::dotnet::XmlTextReader::XmlTextReader)>);

XmlReader ReaderOver(std::string_view xml, const XmlReaderSettings &settings = {}) {
  StringReader source;
  source = StringReader::StringReader(xml);
  return XmlReader::Create(source, settings);
}

std::string ReadFailure(XmlReader reader, bool load) {
  try {
    if (load) {
      agiru::dotnet::XmlDocument document;
      document.Load(reader);
    } else {
      while (reader.Read()) {}
    }
  } catch (const agiru::Error &error) { return error.what(); }
  return {};
}

XmlReader ReaderOverEncoded(std::string_view xml, bool utf16, const XmlReaderSettings &settings) {
  agiru::Blob source;
  std::vector<std::uint8_t> bytes;
  if (utf16) { bytes = {kUtf16LeBomFirst, kUtf16LeBomSecond}; }
  for (const unsigned char byte : xml) {
    bytes.push_back(byte);
    if (utf16) { bytes.push_back(0); }
  }
  source.Set(std::move(bytes));
  const agiru::InStream stream = source.CreateInStream();
  return XmlReader::Create(stream, settings);
}

class EntityFixture {
public:
  EntityFixture() : path_("/tmp/agiru-xml-entity-" + std::to_string(::getpid())) {
    const int descriptor = ::open(path_.c_str(), O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (descriptor < 0) { throw agiru::Error("the XML entity fixture cannot be created"); }
    const auto written = ::write(descriptor, kPayload.data(), kPayload.size());
    const int closed = ::close(descriptor);
    if (written != static_cast<ssize_t>(kPayload.size()) || closed != 0) {
      static_cast<void>(::unlink(path_.c_str()));
      throw agiru::Error("the XML entity fixture cannot be written");
    }
  }

  EntityFixture(const EntityFixture &) = delete;
  EntityFixture &operator=(const EntityFixture &) = delete;

  ~EntityFixture() { static_cast<void>(::unlink(path_.c_str())); }

  [[nodiscard]] std::string Uri() const { return "file://" + path_; }

private:
  static constexpr std::string_view kPayload = "fixture-owned-entity";
  std::string path_;
};

void ProhibitRejectsInternalAndExternalDtdsThroughBothConsumers() {
  const EntityFixture external;
  const std::string outside =
      "<!DOCTYPE root [<!ENTITY word SYSTEM '" + external.Uri() + "'>]><root>&word;</root>";
  for (const std::string_view xml :
       {std::string_view("<!DOCTYPE root [<!ENTITY word 'hello'>]><root>&word;</root>"),
        std::string_view(outside)}) {
    for (const bool utf16 : {false, true}) {
      for (const bool load : {false, true}) {
        const std::string error = ReadFailure(ReaderOverEncoded(xml, utf16, {}), load);
        CHECK_TRUE("default Prohibit rejects a DTD before reader or DOM expansion",
                   error.find("DTD") != std::string::npos);
      }
    }
  }
  XmlReader reader = ReaderOver("<?before ready?><!DOCTYPE root><root/>");
  CHECK_TRUE("Prohibit preserves the preceding processing instruction", reader.Read());
  CHECK_TRUE("the preceding node is a processing instruction",
             reader.NodeType().Equals(XmlNodeType::ProcessingInstruction()));
  CHECK_TRUE("Prohibit fails when the DTD is reached, not at Create",
             ReadFailure(reader, false).find("DTD") != std::string::npos);
}

void IgnoreDiscardsDeclarationsBeforeEntityAndAttributeProcessing() {
  const EntityFixture external;
  const std::vector<std::string> declarations{
      "<!DOCTYPE root [<!ENTITY word 'hello'><!ATTLIST root status CDATA 'default'>]>",
      "<!DOCTYPE root [<!-- ]> is comment content --> <!ENTITY word 'quoted ]> value'>]>",
      "<!DOCTYPE root SYSTEM '" + external.Uri() + "'>",
      "<!DOCTYPE root [<!ENTITY % external SYSTEM '" + external.Uri() + "'> %external;]>"};
  XmlReaderSettings settings;
  settings.DtdProcessing(agiru::dotnet::DtdProcessing::Ignore());
  for (const auto &declaration : declarations) {
    for (const bool utf16 : {false, true}) {
      XmlReader reader = ReaderOverEncoded(declaration + "<root/>", utf16, settings);
      bool dtd = false;
      bool root = false;
      bool defaultAttribute = false;
      std::string error;
      try {
        while (reader.Read()) {
          dtd = dtd || reader.NodeType().Equals(XmlNodeType::DocumentType());
          if (reader.NodeType().Equals(XmlNodeType::Element())) {
            root = root || reader.Name().Value() == "root";
            defaultAttribute = defaultAttribute || reader.MoveToFirstAttribute();
          }
        }
      } catch (const agiru::Error &caught) { error = caught.what(); }
      CHECK_SILENT("Ignore never interprets the discarded DTD", error);
      CHECK_TRUE("Ignore never reports a discarded doctype node", !dtd);
      CHECK_TRUE("Ignore preserves the root without DTD default attributes",
                 root && !defaultAttribute);
      agiru::dotnet::XmlDocument document;
      try {
        document.Load(ReaderOverEncoded(declaration + "<root/>", utf16, settings));
      } catch (const agiru::Error &caught) { error = caught.what(); }
      CHECK_SILENT("DOM Load shares the reader's Ignore policy", error);
      CHECK_TRUE("DOM Load does not resurrect a discarded DTD",
                 document.OuterXml().Value().find("<!DOCTYPE") == std::string_view::npos);
    }
  }
  for (const bool utf16 : {false, true}) {
    for (const bool load : {false, true}) {
      const std::string error = ReadFailure(
          ReaderOverEncoded(
              "<!DOCTYPE root [<!ENTITY word 'hello'>]><root>&word;</root>", utf16, settings),
          load);
      CHECK_TRUE("Ignore leaves DTD-defined entity references undeclared", !error.empty());
    }
  }
}

void ReaderPolicyIsASnapshotAndMarkupLiteralsAreNotDtds() {
  XmlReaderSettings settings;
  const std::string_view xml = "<!DOCTYPE root [<!ENTITY word 'hello'>]><root>&word;</root>";
  const XmlReader prohibited = ReaderOver(xml, settings);
  settings.DtdProcessing(agiru::dotnet::DtdProcessing::Parse());
  CHECK_TRUE("later settings mutation cannot authorize an existing prohibited reader",
             ReadFailure(prohibited, false).find("DTD") != std::string::npos);
  const XmlReader parsed = ReaderOver(xml, settings);
  settings.DtdProcessing(agiru::dotnet::DtdProcessing::Prohibit());
  agiru::dotnet::XmlDocument document;
  document.Load(parsed);
  CHECK_TEXT("later settings mutation cannot change an existing Parse reader",
             document.DocumentElement().InnerText().Value(),
             "hello");
  for (const auto mode : {agiru::dotnet::DtdProcessing::Prohibit(),
                          agiru::dotnet::DtdProcessing::Ignore(),
                          agiru::dotnet::DtdProcessing::Parse()}) {
    settings.DtdProcessing(mode);
    const std::string error =
        ReadFailure(ReaderOver("<?work <!DOCTYPE fake?><root><!-- <!DOCTYPE fake> -->"
                               "<![CDATA[<!DOCTYPE fake>]]></root>",
                               settings),
                    true);
    CHECK_SILENT("DTD-looking PI, comment and CDATA content is ordinary XML", error);
  }
}

void TextReaderConstructionAndFactoryHaveDistinctDtdDefaults() {
  StringReader source;
  source =
      StringReader::StringReader("<!DOCTYPE root [<!ENTITY word 'hello'>]><root>&word;</root>");
  agiru::dotnet::XmlTextReader reader;
  reader = agiru::dotnet::XmlTextReader::XmlTextReader(source);
  agiru::dotnet::XmlDocument document;
  document.Load(reader);
  CHECK_TEXT("XmlTextReader construction retains its Parse default",
             document.DocumentType().Name().Value(),
             "root");
  CHECK_TEXT("constructed reader Load preserves internal entities",
             document.DocumentElement().InnerText().Value(),
             "hello");
  CHECK_TRUE("XmlTextReader Create retains the factory's Prohibit default",
             ReadFailure(agiru::dotnet::XmlTextReader::Create(source), false).find("DTD") !=
                 std::string::npos);
}

void IgnoreStillRequiresDoctypeHeaderAndClosingSyntax() {
  XmlReaderSettings settings;
  settings.DtdProcessing(agiru::dotnet::DtdProcessing::Ignore());
  for (const std::string_view declaration : {"<!DOCTYPEroot>",
                                             "<!DOCTYPE >",
                                             "<!DOCTYPE root SYSTEM>",
                                             "<!DOCTYPE root SYSTEM'outside'>",
                                             "<!DOCTYPE root SYSTEM outside>",
                                             "<!DOCTYPE root PUBLIC 'public'>",
                                             "<!DOCTYPE root PUBLIC'public' 'outside'>",
                                             "<!DOCTYPE root PUBLIC 'public''outside'>",
                                             "<!DOCTYPE root PUBLIC 'public' outside>",
                                             "<!DOCTYPE root UNKNOWN 'outside'>",
                                             "<!DOCTYPE root SYSTEM 'outside' extra>",
                                             "<!DOCTYPE root [ignored] extra>",
                                             "<!DOCTYPE root [ignored [nested]]>"}) {
    for (const bool utf16 : {false, true}) {
      for (const bool load : {false, true}) {
        CHECK_TRUE(
            "Ignore rejects malformed DOCTYPE header or closing syntax",
            !ReadFailure(ReaderOverEncoded(std::string(declaration) + "<root/>", utf16, settings),
                         load)
                 .empty());
      }
    }
  }
  for (const std::string_view declaration : {"<!DOCTYPE root>",
                                             "<!DOCTYPE root []>",
                                             "<!DOCTYPE root SYSTEM 'outside'>",
                                             "<!DOCTYPE root PUBLIC 'public' 'outside'>",
                                             "<!DOCTYPE root[ignored]>",
                                             "<!DOCTYPE root [<!-- ] --> <?work ]?> 'quoted ]'>] >",
                                             "<!DOCTYPE root [uninterpreted declaration text]>",
                                             "<!DOCTYPE root SYSTEM 'quoted > [ ]'>"}) {
    for (const bool utf16 : {false, true}) {
      for (const bool load : {false, true}) {
        CHECK_SILENT(
            "Ignore retains valid header boundaries without interpreting the subset",
            ReadFailure(ReaderOverEncoded(std::string(declaration) + "<root/>", utf16, settings),
                        load));
      }
    }
  }
  XmlReader reader = ReaderOver("<?before ready?><!DOCTYPEroot><root/>", settings);
  CHECK_TRUE("Ignore header validation preserves the preceding processing instruction",
             reader.Read() && reader.NodeType().Equals(XmlNodeType::ProcessingInstruction()));
  CHECK_TRUE("Ignore header validation throws at the declaration, not at Create",
             !ReadFailure(reader, false).empty());
}

void DocumentLoadConsumesTheSharedReader() {
  XmlReader reader = ReaderOver("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                                "<!--before--><?work one?><root xmlns:p=\"urn:one\" id=\"7\">"
                                "<p:child>hello<![CDATA[<world>]]></p:child></root><!--after-->");
  const XmlReader alias = reader;
  agiru::dotnet::XmlDocument document;
  document.Load(alias);
  CHECK_TEXT("Load consumes the root once", document.DocumentElement().Name().Value(), "root");
  CHECK_TEXT(
      "Load preserves root attributes", document.DocumentElement().GetAttribute("id").Value(), "7");
  CHECK_TEXT("Load preserves namespace identity",
             document.DocumentElement().FirstChild().NamespaceURI().Value(),
             "urn:one");
  CHECK_TEXT("Load preserves CDATA and text",
             document.DocumentElement().InnerText().Value(),
             "hello<world>");
  CHECK_TRUE("Load preserves comments and processing instructions",
             document.ChildNodes().Count() == 4);
  CHECK_TRUE("Load advances all reader aliases to EOF", reader.Eof());
  CHECK_TRUE("the consumed reader cannot replay the document", !reader.Read());
  const agiru::dotnet::XmlDeclaration declaration(document);
  CHECK_TEXT("Load preserves declared version", declaration.Version().Value(), "1.0");
  CHECK_TEXT("Load preserves declared encoding", declaration.Encoding().Value(), "UTF-8");
  CHECK_TEXT("Load preserves declared standalone", declaration.Standalone().Value(), "yes");
  reader.Close();
  CHECK_TEXT("the loaded tree owns its nodes after Close",
             document.DocumentElement().InnerText().Value(),
             "hello<world>");
}

void PositionedLoadDoesNotReloadConsumedSiblings() {
  XmlReader reader = ReaderOver(
      R"(<root xmlns:p="urn:one"><old/><p:last id="2"><nested>value</nested></p:last></root>)");
  while (reader.Read() && reader.Name().Value() != "p:last") {}
  const XmlReader alias = reader;
  agiru::dotnet::XmlDocument document;
  document.Load(alias);
  CHECK_TEXT("positioned Load starts at the current child",
             document.DocumentElement().Name().Value(),
             "p:last");
  CHECK_TEXT("positioned Load retains inherited namespace",
             document.DocumentElement().NamespaceURI().Value(),
             "urn:one");
  CHECK_TEXT("positioned Load retains attributes",
             document.DocumentElement().GetAttribute("id").Value(),
             "2");
  CHECK_TRUE("positioned Load never brings back consumed siblings",
             document.GetElementsByTagName("old").Count() == 0);
  CHECK_TEXT("positioned Load copies the child's full subtree",
             document.DocumentElement().InnerText().Value(),
             "value");
  CHECK_TRUE("Load stops at the parent's end element",
             reader.NodeType().Equals(XmlNodeType::EndElement()));
  CHECK_TEXT("the alias observes the consumed child boundary", reader.Name().Value(), "root");
  CHECK_TRUE("a parent end is not silently consumed as EOF", !reader.Eof());
  agiru::dotnet::XmlDocument empty;
  empty.Load(reader);
  CHECK_TRUE("loading from a parent end produces no document child", !empty.HasChildNodes());
  CHECK_TRUE("loading at a parent end leaves that boundary in place",
             reader.NodeType().Equals(XmlNodeType::EndElement()));
  CHECK_TRUE("the next explicit Read reaches EOF", !reader.Read() && alias.Eof());
}

void EndedReadersLoadAnEmptyDocument() {
  XmlReader reader = ReaderOver("<root/>");
  const XmlReader alias = reader;
  reader.Close();
  agiru::dotnet::XmlDocument document;
  document.LoadXml("<previous/>");
  document.Load(alias);
  CHECK_TRUE("closed reader Load replaces the old tree with an empty document",
             !document.HasChildNodes());
  CHECK_TRUE("closed reader Load still creates a non-null document", !document.IsNullObject());
  reader = ReaderOver("<root/>");
  while (reader.Read()) {}
  document.Load(reader);
  CHECK_TRUE("EOF reader Load does not resurrect the original root", !document.HasChildNodes());
  CHECK_TRUE("EOF reader Load leaves the reader at EOF", reader.Eof());
}

void LoadRefusesInvalidDocumentSequences() {
  for (const std::string_view xml :
       {"<root><first/><last/></root>", "<root id=\"1\"/>", "<root><broken></root>"}) {
    XmlReader reader = ReaderOver(xml);
    std::string said;
    try {
      if (xml.find("first") != std::string_view::npos) {
        while (reader.Read() && reader.Name().Value() != "first") {}
      } else if (xml.find("id=") != std::string_view::npos) {
        CHECK_TRUE("the attribute fixture reaches its root", reader.Read());
        CHECK_TRUE("the attribute fixture reaches its attribute", reader.MoveToFirstAttribute());
      }
      agiru::dotnet::XmlDocument document;
      document.Load(reader);
    } catch (const agiru::Error &error) { said = error.what(); }
    CHECK_TRUE("Load refuses multiple roots, an attribute root and malformed XML", !said.empty());
  }
}

void ReaderLoadPreservesDtdAndWhitespaceSemantics() {
  XmlReaderSettings settings;
  settings.DtdProcessing(agiru::dotnet::DtdProcessing::Parse());
  XmlReader reader =
      ReaderOver("<!DOCTYPE root [<!ENTITY word 'hello'>]><root>&word;</root>", settings);
  agiru::dotnet::XmlDocument document;
  document.Load(reader);
  CHECK_TEXT(
      "Load retains the reader's parsed DTD", document.DocumentType().Name().Value(), "root");
  CHECK_TEXT("Load retains the reader's expanded internal entity",
             document.DocumentElement().InnerText().Value(),
             "hello");
  CHECK_TRUE("a retained DTD is serialized",
             document.OuterXml().Value().find("<!DOCTYPE root") != std::string_view::npos);
  for (const bool preserve : {false, true}) {
    reader = ReaderOver("<root> <child/> </root>");
    document.PreserveWhitespace(preserve);
    document.Load(reader);
    CHECK_TRUE("Load honours the document whitespace setting",
               document.DocumentElement().ChildNodes().Count() == (preserve ? 3 : 1));
    reader = ReaderOver("<root xml:space=\"preserve\"> <child/> </root>");
    document.Load(reader);
    CHECK_TRUE("significant whitespace survives either document setting",
               document.DocumentElement().ChildNodes().Count() == 3);
  }
  reader =
      ReaderOver(R"(<root xml:space="preserve"> <child xml:space="default"> </child> </root>)");
  document.PreserveWhitespace(false);
  document.Load(reader);
  CHECK_TRUE("xml:space default resets inherited preservation",
             document.SelectSingleNode("/root/child").ChildNodes().Count() == 0);
  CHECK_TRUE("a descendant reset does not remove the parent's significant whitespace",
             document.DocumentElement().ChildNodes().Count() == 3);
}

void ReaderFileErrorsAreNotSuccessfulEmptyInput() {
  std::string said;
  try {
    const XmlReader reader = XmlReader::Create(".");
    static_cast<void>(reader);
  } catch (const agiru::Error &error) { said = error.what(); }
  CHECK_TRUE("file read errors refuse instead of yielding an empty reader",
             said.find("cannot be read") != std::string::npos);
}

void StylesheetInstructionsAreNotXmlDeclarations() {
  XmlReader reader = ReaderOver(R"(<?xml-stylesheet href="sheet.css"?><root/>)");
  CHECK_TRUE("the stylesheet instruction can be read", reader.Read());
  CHECK_TRUE("an xml-prefixed PI is not a synthetic declaration",
             reader.NodeType().Equals(XmlNodeType::ProcessingInstruction()));
  CHECK_TEXT("the instruction retains its exact name", reader.Name().Value(), "xml-stylesheet");
  agiru::dotnet::XmlDocument document;
  document.Load(reader);
  CHECK_TEXT("Load retains the current stylesheet instruction",
             document.FirstChild().Name().Value(),
             "xml-stylesheet");
  CHECK_TEXT("Load continues from the instruction to the root",
             document.DocumentElement().Name().Value(),
             "root");
}

void AliasesShareOneCursorAndCloseState() {
  StringReader source;
  source = StringReader::StringReader(R"(<?xml version="1.0"?><a x="one"><b/></a>)");
  XmlReader reader = XmlReader::Create(source);
  XmlReader alias = reader;
  CHECK_TRUE("one alias reads the declaration", alias.Read());
  CHECK_TRUE("the other alias sees the declaration",
             reader.NodeType().Equals(XmlNodeType::XmlDeclaration()));
  CHECK_TRUE("the next read advances the shared cursor", reader.Read());
  CHECK_TEXT("all aliases see the root, not an old declaration", alias.Name().Value(), "a");
  CHECK_TRUE("one alias moves the cursor to an attribute", alias.MoveToFirstAttribute());
  CHECK_TEXT("the other alias sees that attribute", reader.Value().Value(), "one");
  CHECK_TRUE("moving back to the element is shared", reader.MoveToElement());
  CHECK_TEXT("the alias sees the same element", alias.Name().Value(), "a");
  alias.Close();
  CHECK_TRUE("closing an alias ends the shared walk", !reader.Read());
  CHECK_TRUE("all aliases observe the existing closed/EOF contract", reader.Eof());
  CHECK_TRUE("a closed alias cannot access a stale node",
             reader.NodeType().Equals(XmlNodeType::None()));
  CHECK_TEXT("closing clears the shared displayed name", alias.Name().Value(), "");
  reader.Close();
  CHECK_TRUE("closing a second alias is harmless", !alias.Read());
  reader = XmlReader::Create(source);
  CHECK_TRUE("reassignment creates a fresh reader", reader.Read());
  CHECK_TRUE("reassignment does not reopen the old alias", !alias.Read());
  const XmlReader atEnd = reader;
  while (reader.Read()) {}
  CHECK_TRUE("EOF belongs to the shared cursor", atEnd.Eof());
}

/// A READER WALKS THE NODES FORWARD, AS .NET'S DOES, with the node types numbered the same:
/// `XML Buffer Writer` fills the XML Buffer from `Read`, `NodeType`, `Depth`, `Name`, `Value` and
/// the attribute moves, and the whitespace between elements is a node like in .NET (board:0676).
void AReaderWalksElementsAttributesAndText() {
  StringReader source;
  source = StringReader::StringReader(R"(<?xml version="1.0"?><a x="1"><b>hi</b><c/></a>)");
  XmlReaderSettings settings;
  settings = XmlReaderSettings::XmlReaderSettings();
  settings.DtdProcessing(agiru::dotnet::DtdProcessing::Ignore());
  XmlReader reader = XmlReader::Create(source, settings);
  std::vector<std::string> walk;
  while (reader.Read()) {
    walk.push_back(std::to_string(reader.NodeType().Number()) + ":" +
                   std::string(reader.Name().Value()) + "@" + std::to_string(reader.Depth()));
    if (reader.NodeType().Equals(XmlNodeType::Element()) && reader.MoveToFirstAttribute()) {
      do {
        walk.push_back("attr " + std::string(reader.Name().Value()) + "=" +
                       std::string(reader.Value().Value()));
      } while (reader.MoveToNextAttribute());
    }
  }
  CHECK_TRUE("the walk has the declaration, the elements, the text and the ends", walk.size() >= 7);
  CHECK_TEXT("the first node is the declaration", walk[0], "17:xml@0");
  CHECK_TEXT("then the root element", walk[1], "1:a@0");
  CHECK_TEXT("with its attribute", walk[2], "attr x=1");
  CHECK_TEXT("then b one deeper", walk[3], "1:b@1");
  CHECK_TEXT("its text two deeper", walk[4], "3:#text@2");
  CHECK_TEXT("its end", walk[5], "15:b@1");
  CHECK_TRUE("and the walk ends with the reader at its end", reader.Eof());
  std::string said;
  try {
    StringReader bad;
    bad = StringReader::StringReader("<a><b></a>");
    XmlReader broken = XmlReader::Create(bad, settings);
    while (broken.Read()) {}
  } catch (const agiru::Error &e) { said = e.what(); }
  CHECK_TRUE("malformed XML refuses when it is reached",
             said.find("well-formed") != std::string::npos);
}

} // namespace

int main() {
  return gate::Run("XmlReader", [] {
    AReaderWalksElementsAttributesAndText();
    AliasesShareOneCursorAndCloseState();
    DocumentLoadConsumesTheSharedReader();
    PositionedLoadDoesNotReloadConsumedSiblings();
    EndedReadersLoadAnEmptyDocument();
    LoadRefusesInvalidDocumentSequences();
    ReaderLoadPreservesDtdAndWhitespaceSemantics();
    ReaderFileErrorsAreNotSuccessfulEmptyInput();
    StylesheetInstructionsAreNotXmlDeclarations();
    ProhibitRejectsInternalAndExternalDtdsThroughBothConsumers();
    IgnoreDiscardsDeclarationsBeforeEntityAndAttributeProcessing();
    ReaderPolicyIsASnapshotAndMarkupLiteralsAreNotDtds();
    TextReaderConstructionAndFactoryHaveDistinctDtdDefaults();
    IgnoreStillRequiresDoctypeHeaderAndClosingSyntax();
  });
}
