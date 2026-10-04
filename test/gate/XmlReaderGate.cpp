#include "dotnet/XmlDocument.h"
#include "dotnet/XmlNode.h"
#include "dotnet/XmlReader.h"
#include "runtime/ErrorValue.h"

#include "Check.h"

#include <string>
#include <string_view>
#include <vector>

using agiru::dotnet::StringReader;
using agiru::dotnet::XmlNodeType;
using agiru::dotnet::XmlReader;
using agiru::dotnet::XmlReaderSettings;

namespace {

XmlReader ReaderOver(std::string_view xml, const XmlReaderSettings &settings = {}) {
  StringReader source;
  source = source.StringReader(xml);
  return XmlReader::Create(source, settings);
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
  source = source.StringReader(R"(<?xml version="1.0"?><a x="one"><b/></a>)");
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
  source = source.StringReader(R"(<?xml version="1.0"?><a x="1"><b>hi</b><c/></a>)");
  XmlReaderSettings settings;
  settings = settings.XmlReaderSettings();
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
    bad = bad.StringReader("<a><b></a>");
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
  });
}
