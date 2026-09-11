#include "dotnet/XmlDocument.h"
#include "dotnet/XmlNode.h"
#include "runtime/Error.h"
#include "type/Blob.h"
#include "type/Stream.h"
#include "type/Text.h"
#include "type/Variant.h"
#include "type/XmlAttribute.h"
#include "type/XmlAttributeCollection.h"
#include "type/XmlDocument.h"
#include "type/XmlElement.h"
#include "type/XmlNamespaceManager.h"
#include "type/XmlNode.h"
#include "type/XmlNodeList.h"
#include "type/XmlText.h"

#include "BuiltinsWritten.h"
#include "Check.h"

#include <string>
#include <string_view>

using agiru::Error;
using agiru::Text;
using agiru::XmlAttribute;
using agiru::XmlAttributeCollection;
using agiru::XmlDocument;
using agiru::XmlElement;
using agiru::XmlNamespaceManager;
using agiru::XmlNode;
using agiru::XmlNodeList;

namespace {

constexpr std::string_view kSample =
    R"(<?xml version="1.0" encoding="UTF-8"?><root xmlns:p="urn:p"><a id="1">one</a><a id="2">two</a><p:b>three</p:b></root>)";

/// `XmlDocument.ReadFrom` PARSES AND `WriteTo` SERIALISES; what went in comes back out, and a text
/// that is not XML is refused with `false` rather than a document that is silently empty (openerp
/// WI-1185 paid for the other answer).
void ReadFromAndWriteToRoundTrip() {
  XmlDocument document;
  CHECK_TRUE("well-formed XML is read", XmlDocument::ReadFrom(kSample, document));
  Text<0> written;
  CHECK_TRUE("and written back", document.WriteTo(written));
  CHECK_TRUE("with the root in it",
             std::string(written.Value()).find("<root xmlns:p=\"urn:p\">") != std::string::npos);
  XmlDocument broken;
  CHECK_TRUE("and a text that is not XML is refused", !XmlDocument::ReadFrom("<a><b></a>", broken));
}

/// THE ROOT, ITS CHILDREN AND THEIR TEXT: `GetRoot`, `GetChildElements`, `InnerText`, `Name`,
/// `Attributes` and `XmlNodeList.Get` with its ONE-BASED index (`xmlnodelist-get-method.md`).
void ElementsAreWalkedAndRead() {
  XmlDocument document;
  CHECK_TRUE("read", XmlDocument::ReadFrom(kSample, document));
  XmlElement root;
  CHECK_TRUE("the root is found", document.GetRoot(root));
  CHECK_TEXT("and named", root.Name(), "root");
  XmlNodeList children = root.GetChildElements();
  CHECK_TRUE("three children", children.Count() == 3);
  XmlNode second;
  CHECK_TRUE("the index is one-based", children.Get(2, second));
  CHECK_TEXT("and reads its text", second.AsXmlElement().InnerText(), "two");
  XmlNode none;
  CHECK_TRUE("and past the end is false", !children.Get(4, none));
  XmlAttributeCollection attributes = second.AsXmlElement().Attributes();
  XmlAttribute id;
  CHECK_TRUE("an attribute is found by name", attributes.Get("id", id));
  CHECK_TEXT("with its value", id.Value(), "2");
  CHECK_TEXT("the prefixed child keeps its qualified name",
             root.GetChildElements("p:b").Count() == 1 ? "p:b" : "?",
             "p:b");
  CHECK_TEXT("and its local name and namespace apart",
             root.GetChildElements("b", "urn:p").Count() == 1 ? "b" : "?",
             "b");
}

/// XPATH WITH AND WITHOUT A NAMESPACE MANAGER: `SelectSingleNode` answers the first match, or
/// false, and a prefix is resolved through the manager
/// (`xmlnode-selectsinglenode-string-xmlnamespacemanager-xmlnode-method.md`).
void XPathSelectsWithNamespaces() {
  XmlDocument document;
  CHECK_TRUE("read", XmlDocument::ReadFrom(kSample, document));
  XmlNode found;
  CHECK_TRUE("a plain path selects", document.SelectSingleNode("/root/a[@id='2']", found));
  CHECK_TEXT("the right node", found.AsXmlElement().InnerText(), "two");
  XmlNamespaceManager manager;
  manager.AddNamespace("x", "urn:p");
  CHECK_TRUE("a prefixed path needs the manager", !document.SelectSingleNode("//x:b", found));
  CHECK_TRUE("and selects with it", document.SelectSingleNode("//x:b", manager, found));
  CHECK_TEXT("the namespaced node", found.AsXmlElement().InnerText(), "three");
  XmlNodeList all;
  CHECK_TRUE("SelectNodes answers a list", document.SelectNodes("//a", all));
  CHECK_TRUE("of both", all.Count() == 2);
  Text<0> uri;
  CHECK_TRUE("LookupNamespace answers the uri", manager.LookupNamespace("x", uri));
  CHECK_TEXT("that was added", std::string(uri.Value()), "urn:p");
}

/// A DOCUMENT IS BUILT: `Create`, `Add` of an element, a text and a Variant carrying either, and a
/// node added to a tree is seen through every handle on it, because the types are references
/// (`xmlelement-data-type.md`).
void ElementsAreBuiltAndShared() {
  XmlDocument document = XmlDocument{}.Create();
  XmlElement root = XmlElement::Create("order");
  CHECK_TRUE("the root goes in", document.Add(root));
  XmlElement line = XmlElement::Create("line", agiru::Variant(std::string("first")));
  line.SetAttribute("no", "10000");
  CHECK_TRUE("a line goes under the root", root.Add(line));
  CHECK_TRUE("and a text under the line", line.Add(agiru::Variant(std::string("!"))));
  Text<0> written;
  CHECK_TRUE("written", document.WriteTo(written));
  CHECK_TRUE("as one tree",
             std::string(written.Value()).find("<order><line no=\"10000\">first!</line></order>") !=
                 std::string::npos);
  XmlElement again;
  CHECK_TRUE("the root read back through the document", document.GetRoot(again));
  CHECK_TRUE("is the same element", again.GetChildElements().Count() == 1);
  XmlElement parent;
  CHECK_TRUE("the line knows its parent", line.GetParent(parent));
  CHECK_TEXT("which is the root", parent.Name(), "order");
  XmlNode node = line.AsXmlNode();
  CHECK_TRUE("a node converts to what it is", node.IsXmlElement() && !node.IsXmlText());
  bool threw = false;
  try {
    static_cast<void>(node.AsXmlText());
  } catch (const Error &) { threw = true; }
  CHECK_TRUE("and refuses to be what it is not", threw);
}

/// THE .NET SIDE OVER THE SAME TREE: `XmlDocument.LoadXml`, `SelectSingleNode` answering null,
/// `IsNull`, `InnerText`, `AppendChild` and `OuterXml`, the members `XMLDOMManagement` names.
void DotNetClassesWalkTheSameTree() {
  agiru::dotnet::XmlDocument document;
  document = document.XmlDocument();
  CHECK_TRUE("a fresh document is not null", !agiru::IsNull(document));
  document.LoadXml(std::string(kSample));
  agiru::dotnet::XmlElement root = document.DocumentElement();
  CHECK_TEXT("the root is there", root.Name(), "root");
  agiru::dotnet::XmlNode missing = root.SelectSingleNode("zzz");
  CHECK_TRUE("nothing found is null", agiru::IsNull(missing));
  agiru::dotnet::XmlNode second = root.SelectSingleNode("a[@id='2']");
  CHECK_TEXT("found reads its text", second.InnerText(), "two");
  agiru::dotnet::XmlNamespaceManager manager;
  manager = manager.XmlNamespaceManager(document.NameTable());
  manager.AddNamespace("x", "urn:p");
  CHECK_TEXT("a prefixed path resolves through the manager",
             root.SelectSingleNode("x:b", manager).InnerText(),
             "three");
  agiru::dotnet::XmlElement made = document.CreateElement("c");
  made.InnerText("four");
  static_cast<void>(root.AppendChild(made));
  CHECK_TRUE("an appended child is in the markup",
             std::string_view(root.OuterXml()).find("<c>four</c></root>") != std::string::npos);
  int walked = 0;
  for ([[maybe_unused]] auto &node : root.ChildNodes()) { ++walked; }
  CHECK_TRUE("and foreach walks every child", walked == 4);
  CHECK_TEXT("an attribute is read by name",
             root.SelectSingleNode("a").Attributes().GetNamedItem("id").Value(),
             "1");
  agiru::dotnet::XmlNode declaration = document.CreateXmlDeclaration("1.0", "UTF-8", "");
  static_cast<void>(document.InsertBefore(declaration, document.DocumentElement()));
  static_cast<void>(document.AppendChild(declaration));
  CHECK_TRUE(
      "a declaration inserted the .NET way is the document's own, not a child",
      std::string_view(document.OuterXml()).find("<?xml version=\"1.0\" encoding=\"UTF-8\"?>") ==
              0 &&
          document.ChildNodes().Count() == 1);
  bool threw = false;
  try {
    document.LoadXml("<a>");
  } catch (const Error &) { threw = true; }
  CHECK_TRUE("and bad XML throws, the way .NET throws", threw);
}

}

int main() {
  return gate::Run("Xml", [] {
    ReadFromAndWriteToRoundTrip();
    ElementsAreWalkedAndRead();
    XPathSelectsWithNamespaces();
    ElementsAreBuiltAndShared();
    DotNetClassesWalkTheSameTree();
  });
}
