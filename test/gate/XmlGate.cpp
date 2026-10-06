#include "dotnet/XmlDocument.h"
#include "dotnet/XmlNode.h"
#include "runtime/ErrorValue.h"
#include "type/Text.h"
#include "type/Variant.h"
#include "type/XmlAttribute.h"
#include "type/XmlAttributeCollection.h"
#include "type/XmlDeclaration.h"
#include "type/XmlDocument.h"
#include "type/XmlDocumentType.h"
#include "type/XmlElement.h"
#include "type/XmlNamespaceManager.h"
#include "type/XmlNode.h"
#include "type/XmlNodeList.h"
#include "type/XmlText.h"

#include "BuiltinsWritten.h"
#include "Check.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <type_traits>

using agiru::Error;
using agiru::Text;
using agiru::XmlAttribute;
using agiru::XmlAttributeCollection;
using agiru::XmlDocument;
using agiru::XmlElement;
using agiru::XmlNamespaceManager;
using agiru::XmlNode;
using agiru::XmlNodeList;

static_assert(std::is_const_v<decltype(agiru::dotnet::XmlDocument::XmlDocument)>);
static_assert(std::is_const_v<decltype(agiru::dotnet::XmlNamespaceManager::XmlNamespaceManager)>);

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

void EmptyNamespaceAliasesPreserveXPathTokens() {
  XmlDocument document;
  CHECK_TRUE(
      "empty-namespace fixture parses",
      XmlDocument::ReadFrom(
          R"(<root xmlns:p="urn:p"><item value="e:literal" code="one">plain</item><p:item code="two">qualified</p:item><élément>Unicode</élément></root>)",
          document));
  XmlNamespaceManager manager;
  manager.AddNamespace("e", "");
  manager.AddNamespace("p", "urn:p");
  manager.AddNamespace("child", "");
  manager.AddNamespace("leer_ä", "");
  XmlNode found;
  CHECK_TRUE("a prefix mapped to no namespace selects unqualified elements",
             document.SelectSingleNode("/root/e:item", manager, found));
  CHECK_TEXT("the empty alias does not select the same local name in another namespace",
             found.IsXmlElement() ? found.AsXmlElement().InnerText() : "",
             "plain");
  CHECK_TRUE("quoted prefix text is not rewritten",
             document.SelectSingleNode("/root/e:item[@value='e:literal']", manager, found));
  CHECK_TRUE("double-quoted prefix text is not rewritten",
             document.SelectSingleNode("/root/e:item[@value=\"e:literal\"]", manager, found));
  CHECK_TRUE("empty aliases apply to unqualified attribute names",
             document.SelectSingleNode("/root/e:item[@e:code='one']", manager, found));
  CHECK_TRUE("axis separators remain distinct from prefix separators",
             document.SelectSingleNode("/root/child::e:item", manager, found));
  CHECK_TRUE("prefix and local names keep their Unicode bytes",
             document.SelectSingleNode("/root/leer_ä:élément", manager, found));
  XmlNodeList all;
  CHECK_TRUE("empty namespace wildcard evaluates", document.SelectNodes("/root/e:*", manager, all));
  CHECK_TRUE("empty namespace wildcard excludes namespaced siblings", all.Count() == 2);
  CHECK_TRUE("mixed empty and ordinary aliases evaluate",
             document.SelectNodes("/root/e:item | /root/p:item", manager, all));
  CHECK_TRUE("mixed aliases retain both distinct namespaces", all.Count() == 2);
  CHECK_TRUE("an undeclared longer prefix is not partially rewritten",
             !document.SelectSingleNode("/root/ee:item", manager, found));
  manager.AddNamespace("e", "urn:p");
  CHECK_TRUE("rebinding the alias takes effect on the next operation",
             document.SelectSingleNode("/root/e:item", manager, found));
  CHECK_TEXT("rebinding preserves the qualified namespace",
             found.IsXmlElement() ? found.AsXmlElement().InnerText() : "",
             "qualified");
  manager.AddNamespace("e", "");
  CHECK_TRUE("rebinding back to no namespace takes effect",
             document.SelectSingleNode("/root/e:item", manager, found));
  CHECK_TEXT("the next lookup returns the unqualified node again",
             found.IsXmlElement() ? found.AsXmlElement().InnerText() : "",
             "plain");
}

void DotNetEmptyNamespaceUsesTheSharedXPathEngine() {
  agiru::dotnet::XmlDocument document;
  document = agiru::dotnet::XmlDocument::XmlDocument();
  document.LoadXml("<root/>");
  auto root = document.DocumentElement();
  auto child = document.CreateElement("empty", "item", "");
  child.InnerText("source-owned value");
  static_cast<void>(root.AppendChild(child));
  agiru::dotnet::XmlNamespaceManager manager;
  manager = agiru::dotnet::XmlNamespaceManager::XmlNamespaceManager(document.NameTable());
  manager.AddNamespace("empty", "");
  const auto found = root.SelectSingleNode("/root/empty:item", manager);
  CHECK_TRUE("the original XML DOM pattern finds its empty-namespace element",
             !agiru::IsNull(found));
  CHECK_TEXT(
      "the original XML DOM pattern retains its text", found.InnerText(), "source-owned value");
  const auto list = root.SelectNodes("/root/empty:*", manager);
  CHECK_TRUE("the .NET node-list path uses the same empty-alias semantics", list.Count() == 1);
  CHECK_TRUE("a missing empty-namespace element remains null",
             agiru::IsNull(root.SelectSingleNode("/root/empty:absent", manager)));
  CHECK_TRUE("an undeclared prefix remains unresolved",
             agiru::IsNull(root.SelectSingleNode("/root/unknown:item", manager)));
}

/// A DOCUMENT IS BUILT: `Create`, `Add` of an element, a text and a Variant carrying either, and a
/// node added to a tree is seen through every handle on it, because the types are references
/// (`xmlelement-data-type.md`).
void ElementsAreBuiltAndShared() {
  XmlDocument document = XmlDocument::Create();
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
  document = agiru::dotnet::XmlDocument::XmlDocument();
  CHECK_TRUE("a fresh document is not null", !agiru::IsNull(document));
  document.LoadXml(std::string(kSample));
  agiru::dotnet::XmlElement root = document.DocumentElement();
  CHECK_TEXT("the root is there", root.Name(), "root");
  const agiru::dotnet::XmlNode missing = root.SelectSingleNode("zzz");
  CHECK_TRUE("nothing found is null", agiru::IsNull(missing));
  const agiru::dotnet::XmlNode second = root.SelectSingleNode("a[@id='2']");
  CHECK_TEXT("found reads its text", second.InnerText(), "two");
  agiru::dotnet::XmlNamespaceManager manager;
  manager = agiru::dotnet::XmlNamespaceManager::XmlNamespaceManager(document.NameTable());
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
  for ([[maybe_unused]] const auto &node : root.ChildNodes()) { ++walked; }
  CHECK_TRUE("and foreach walks every child", walked == 4);
  CHECK_TEXT("an attribute is read by name",
             root.SelectSingleNode("a").Attributes().GetNamedItem("id").Value(),
             "1");
  // A NAMESPACE DECLARATION IS AN ATTRIBUTE IN .NET (`xmlns:p` sits in `Attributes` with the
  // uri as its value), and `XML DOM Management.AddNamespaces` reads the prefixes off the root
  // element's attributes that way; libxml2 keeps them apart, and without them here every
  // prefixed XPath of the PEPPOL import found nothing (Incoming Doc. To Data Exch.UT, 3 cases,
  // 2026-09-12).
  const agiru::dotnet::XmlAttributeCollection declared = root.Attributes();
  CHECK_TRUE("the root's namespace declaration counts as an attribute", declared.Count() == 1);
  CHECK_TEXT("named xmlns:prefix", declared.Item(0).Name(), "xmlns:p");
  CHECK_TEXT("with the uri as its value", declared.GetNamedItem("xmlns:p").Value(), "urn:p");
  int named = 0;
  for (const auto &attribute : declared) {
    if (std::string_view(attribute.Name()).starts_with("xmlns:")) { ++named; }
  }
  CHECK_TRUE("and foreach walks it", named == 1);
  const agiru::dotnet::XmlNode declaration = document.CreateXmlDeclaration("1.0", "UTF-8", "");
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

void DotNetLocationLoadAndDeclaration() {
  const auto path = std::filesystem::temp_directory_path() / "agiru-xml-gate-document.xml";
  {
    std::ofstream output(path);
    output << kSample;
  }
  agiru::dotnet::XmlDocument document;
  document = agiru::dotnet::XmlDocument::XmlDocument();
  document.Load(path.string());
  CHECK_TEXT("Load(filename) reads XML", document.DocumentElement().Name(), "root");
  const agiru::dotnet::XmlDeclaration declaration =
      document.CreateXmlDeclaration("1.0", "UTF-8", "yes");
  CHECK_TEXT("declaration version", declaration.Version(), "1.0");
  CHECK_TEXT("declaration encoding", declaration.Encoding(), "UTF-8");
  CHECK_TEXT("declaration standalone", declaration.Standalone(), "yes");
  bool wrongNode = false;
  try {
    const agiru::dotnet::XmlDeclaration element = document.DocumentElement();
    static_cast<void>(element.Version());
  } catch (const Error &) { wrongNode = true; }
  CHECK_TRUE("an element cannot masquerade as an XML declaration", wrongNode);
  bool threw = false;
  try {
    document.Load(path.string() + ".missing");
  } catch (const Error &) { threw = true; }
  CHECK_TRUE("missing XML location throws", threw);
  CHECK_TEXT(
      "failed load retains the previous document", document.DocumentElement().Name(), "root");
  std::filesystem::remove(path);
}

void DotNetParseFailuresRetainTheirKindAndPosition() {
  struct InvalidInput {
    std::string_view text;
    std::string_view diagnostic;
  };

  constexpr std::array cases{
      InvalidInput{.text = "ABC",
                   .diagnostic = "Data at the root level is invalid. Line 1, position 1."},
      InvalidInput{.text = " \nABC",
                   .diagnostic = "Data at the root level is invalid. Line 2, position 1."},
      InvalidInput{.text = "<?xml version=\"1.0\"?>ABC",
                   .diagnostic = "Data at the root level is invalid. Line 1, position 22."},
      InvalidInput{.text = "<root/>ABC",
                   .diagnostic = "Data at the root level is invalid. Line 1, position 8."},
      InvalidInput{.text = "雪",
                   .diagnostic = "Data at the root level is invalid. Line 1, position 1."},
      InvalidInput{.text = "<r>🙂</r>ABC",
                   .diagnostic = "Data at the root level is invalid. Line 1, position 10."},
      InvalidInput{.text = "<!--🙂-->ABC",
                   .diagnostic = "Data at the root level is invalid. Line 1, position 10."},
      InvalidInput{.text = {}, .diagnostic = "Root element is missing."},
      InvalidInput{.text = "", .diagnostic = "Root element is missing."},
      InvalidInput{.text = " \n\t", .diagnostic = "Root element is missing."},
      InvalidInput{.text = "<!--only-->", .diagnostic = "Root element is missing."},
      InvalidInput{.text = "<?xml version=\"1.0\"?>", .diagnostic = "Root element is missing."},
  };
  agiru::dotnet::XmlDocument document;
  document.LoadXml("<previous/>");
  for (const auto &[text, expected] : cases) {
    std::string diagnostic;
    try {
      document.LoadXml(text);
    } catch (const Error &error) { diagnostic = error.what(); }
    CHECK_TEXT(
        "XML failures keep their own qualified classification and location", diagnostic, expected);
  }
  std::string mismatch;
  try {
    document.LoadXml("<a><b></a>");
  } catch (const Error &error) { mismatch = error.what(); }
  CHECK_TRUE("a tag mismatch is not misreported as invalid root data",
             mismatch.find("mismatch") != std::string::npos &&
                 mismatch.find("root level") == std::string::npos);
  CHECK_TRUE("the first parse failure is not replaced by a cascading end-of-input error",
             mismatch.find("Premature end") == std::string::npos);
  XmlDocument typed;
  CHECK_TRUE("native AL XML still creates the initial typed tree",
             XmlDocument::ReadFrom("<typed/>", typed));
  CHECK_TRUE("consumed native AL XML failure returns false", !XmlDocument::ReadFrom("ABC", typed));
  XmlElement root;
  CHECK_TRUE("a failed Boolean parse retains its existing output", typed.GetRoot(root));
  CHECK_TEXT("the retained typed tree is not replaced with a default", root.Name(), "typed");
}

}

namespace {

void XmlFactoriesKeepTheOriginalArgumentRoles() {
  auto declaration = agiru::XmlDeclaration::Create("1.0", "UTF-8", "yes");
  CHECK_TEXT("AL declaration keeps its version argument", declaration.Version(), "1.0");
  CHECK_TEXT("AL declaration keeps its encoding argument", declaration.Encoding(), "UTF-8");
  CHECK_TEXT("AL declaration keeps its standalone argument", declaration.Standalone(), "yes");
  auto type = agiru::XmlDocumentType::Create("invoice", "public-id", "system-id", "");
  Text<0> text;
  CHECK_TRUE("AL document type exposes its name", type.GetName(text));
  CHECK_TEXT("AL document type keeps its name argument", text, "invoice");
  CHECK_TRUE("AL document type exposes its public identifier", type.GetPublicId(text));
  CHECK_TEXT("AL document type keeps its public identifier argument", text, "public-id");
  CHECK_TRUE("AL document type exposes its system identifier", type.GetSystemId(text));
  CHECK_TEXT("AL document type keeps its system identifier argument", text, "system-id");
  auto document = agiru::dotnet::XmlDocument::XmlDocument();
  const auto element = document.CreateElement("p", "invoice", "urn:invoice");
  CHECK_TEXT("CLR element keeps its prefix argument", element.Prefix(), "p");
  CHECK_TEXT("CLR element keeps its local name argument", element.LocalName(), "invoice");
  CHECK_TEXT("CLR element keeps its namespace argument", element.NamespaceURI(), "urn:invoice");
  const auto attribute = document.CreateAttribute("a", "number", "urn:attribute");
  CHECK_TEXT("CLR attribute keeps its prefix argument", attribute.Prefix(), "a");
  CHECK_TEXT("CLR attribute keeps its local name argument", attribute.LocalName(), "number");
  CHECK_TEXT(
      "CLR attribute keeps its namespace argument", attribute.NamespaceURI(), "urn:attribute");
  const agiru::dotnet::XmlDeclaration clrDeclaration =
      document.CreateXmlDeclaration("1.0", "UTF-8", "no");
  CHECK_TEXT("CLR declaration keeps its version argument", clrDeclaration.Version(), "1.0");
  CHECK_TEXT("CLR declaration keeps its encoding argument", clrDeclaration.Encoding(), "UTF-8");
  CHECK_TEXT("CLR declaration keeps its standalone argument", clrDeclaration.Standalone(), "no");
  const agiru::dotnet::XmlDocumentType clrType =
      document.CreateDocumentType("invoice", "public-id", "system-id", "");
  CHECK_TEXT("CLR document type keeps its name argument", clrType.LocalName(), "invoice");
  CHECK_TEXT(
      "CLR document type keeps its public identifier argument", clrType.PublicId(), "public-id");
  CHECK_TEXT(
      "CLR document type keeps its system identifier argument", clrType.SystemId(), "system-id");
}

}

int main() {
  return gate::Run("Xml", [] {
    ReadFromAndWriteToRoundTrip();
    ElementsAreWalkedAndRead();
    XPathSelectsWithNamespaces();
    EmptyNamespaceAliasesPreserveXPathTokens();
    DotNetEmptyNamespaceUsesTheSharedXPathEngine();
    ElementsAreBuiltAndShared();
    DotNetClassesWalkTheSameTree();
    DotNetLocationLoadAndDeclaration();
    DotNetParseFailuresRetainTheirKindAndPosition();
    XmlFactoriesKeepTheOriginalArgumentRoles();
  });
}
