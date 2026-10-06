#include "XmlEngine.h"

#include "type/Outcome.h"
#include "type/XmlHandle.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <libxml/globals.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlerror.h>
#include <libxml/xmlstring.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

namespace agiru::detail {

namespace {

const xmlChar *Bytes(std::string_view text) {
  return reinterpret_cast<const xmlChar *>(text.data());
}

std::string Owned(xmlChar *text) {
  const std::string out = Text(text);
  xmlFree(text);
  return out;
}

}

XmlTree *NewTree(xmlDocPtr doc) {
  auto *tree = new XmlTree{};
  tree->doc = doc;
  tree->count = 0;
  return tree;
}

void XmlRetain(XmlTree *tree) noexcept {
  if (tree != nullptr) { ++tree->count; }
}

void XmlRelease(XmlTree *tree) noexcept {
  if (tree == nullptr) { return; }
  if (--tree->count > 0) { return; }
  for (XmlTree *kept : tree->kept) { XmlRelease(kept); }
  if (tree->doc != nullptr) { xmlFreeDoc(tree->doc); }
  delete tree;
}

XmlHandle EmptyDocument() {
  xmlDocPtr doc = xmlNewDoc(Bytes("1.0"));
  return {NewTree(doc), doc};
}

XmlHandle HandleOf(XmlTree *tree, void *node) {
  return {tree, node};
}

XmlHandle Detached(xmlNodePtr node) {
  xmlDocPtr doc = xmlNewDoc(Bytes("1.0"));
  xmlSetTreeDoc(node, doc);
  return {NewTree(doc), node};
}

xmlNodePtr NodeOf(const XmlHandle &handle) {
  return static_cast<xmlNodePtr>(handle.node);
}

xmlDocPtr DocOf(const XmlHandle &handle) {
  return handle.tree == nullptr ? nullptr : handle.tree->doc;
}

void KeepAlive(XmlTree *keeper, XmlTree *kept) {
  if (keeper == nullptr || kept == nullptr || keeper == kept) { return; }
  if (std::ranges::find(keeper->kept, kept) != keeper->kept.end()) { return; }
  XmlRetain(kept);
  keeper->kept.push_back(kept);
}

std::string Text(const xmlChar *text) {
  return text == nullptr ? std::string{} : std::string(reinterpret_cast<const char *>(text));
}

std::string Utf8(std::string_view text) {
  return std::string(text);
}

std::string Dump(const XmlHandle &handle) {
  xmlNodePtr node = NodeOf(handle);
  if (node == nullptr) { return {}; }
  if (node->type == XML_DOCUMENT_NODE) {
    xmlChar *out = nullptr;
    int size = 0;
    xmlDocDumpMemoryEnc(reinterpret_cast<xmlDocPtr>(node), &out, &size, "UTF-8");
    return Owned(out);
  }
  if (node->type == XML_ATTRIBUTE_NODE) {
    return QualifiedName(node) + "=\"" + Owned(xmlNodeGetContent(node)) + "\"";
  }
  xmlBufferPtr buffer = xmlBufferCreate();
  xmlNodeDump(buffer, node->doc, node, 0, 0);
  const std::string out = Text(xmlBufferContent(buffer));
  xmlBufferFree(buffer);
  return out;
}

std::string DumpChildren(const XmlHandle &handle) {
  xmlNodePtr node = NodeOf(handle);
  if (node == nullptr) { return {}; }
  std::string out;
  for (xmlNodePtr child = node->children; child != nullptr; child = child->next) {
    xmlBufferPtr buffer = xmlBufferCreate();
    xmlNodeDump(buffer, child->doc, child, 0, 0);
    out += Text(xmlBufferContent(buffer));
    xmlBufferFree(buffer);
  }
  return out;
}

namespace {

void Quiet([[maybe_unused]] void *context, [[maybe_unused]] xmlErrorPtr error) {}

int Utf16Column(const xmlParserInput &input, int scalarColumn) noexcept {
  constexpr unsigned char kFourBytePrefixMask = 0xf8;
  constexpr unsigned char kFourBytePrefix = 0xf0;
  int extra = 0;
  for (const xmlChar *at = input.base; at != nullptr && at < input.cur; ++at) {
    if (*at == '\r' || *at == '\n') {
      extra = 0;
    } else if ((*at & kFourBytePrefixMask) == kFourBytePrefix) {
      ++extra;
    }
  }
  return scalarColumn + extra;
}

class XmlParseFailure {
public:
  XmlParseFailure() = default;
  XmlParseFailure(const XmlParseFailure &) = delete;
  XmlParseFailure &operator=(const XmlParseFailure &) = delete;

  ~XmlParseFailure() { xmlResetError(&error_); }

  static void Capture(void *user, xmlErrorPtr error) noexcept {
    auto *context = static_cast<xmlParserCtxtPtr>(user);
    if (context == nullptr) { return; }
    auto *failure = static_cast<XmlParseFailure *>(context->_private);
    if (failure == nullptr || error == nullptr || error->level < XML_ERR_WARNING ||
        (failure->error_.code != XML_ERR_OK &&
         (failure->error_.level >= XML_ERR_ERROR || error->level < XML_ERR_ERROR))) {
      return;
    }
    static_cast<void>(xmlCopyError(error, &failure->error_));
    if (context->input != nullptr) {
      failure->error_.int2 = Utf16Column(*context->input, failure->error_.int2);
    }
    failure->atEnd_ = context->input == nullptr || context->input->cur == context->input->end;
    if (!failure->atEnd_) { failure->byte_ = *context->input->cur; }
  }

  [[nodiscard]] std::string Text() const {
    if (error_.code == XML_ERR_DOCUMENT_EMPTY && atEnd_) { return "Root element is missing."; }
    const bool documentBoundary =
        error_.code == XML_ERR_DOCUMENT_EMPTY || error_.code == XML_ERR_DOCUMENT_END;
    constexpr unsigned char kFirstXmlTextByte = 0x20;
    const bool rootText = documentBoundary && !atEnd_ && byte_ >= kFirstXmlTextByte && byte_ != '<';
    std::string text = rootText                    ? "Data at the root level is invalid."
                       : error_.message != nullptr ? error_.message
                                                   : "XML parsing failed.";
    const auto end = text.find_last_not_of("\r\n");
    if (end != std::string::npos) { text.resize(end + 1); }
    if (error_.line > 0 && error_.int2 > 0) {
      if (!text.empty() && text.back() != '.') { text += '.'; }
      text += " Line " + std::to_string(error_.line) + ", position " + std::to_string(error_.int2) +
              '.';
    }
    return text;
  }

private:
  xmlError error_{};
  bool atEnd_ = true;
  unsigned char byte_ = 0;
};

template <typename Read>
Outcome<XmlHandle, std::string>
ReadDocument(bool preserveWhitespace, int options, const Read &read) {
  XmlParseFailure failure;
  const std::unique_ptr<xmlParserCtxt, decltype(&xmlFreeParserCtxt)> context(xmlNewParserCtxt(),
                                                                             &xmlFreeParserCtxt);
  if (context == nullptr) { return Failed("XML parser context allocation failed."); }
  context->_private = &failure;
  context->sax->serror = &XmlParseFailure::Capture;
  options |= XML_PARSE_NOERROR | XML_PARSE_NOWARNING;
  if (!preserveWhitespace) { options |= XML_PARSE_NOBLANKS; }
  std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> doc(read(context.get(), options), &xmlFreeDoc);
  if (doc == nullptr) { return Failed(failure.Text()); }
  XmlTree *const tree = NewTree(doc.get());
  return XmlHandle(tree, doc.release());
}

}

Outcome<XmlHandle, std::string> ReadXml(std::string_view text, bool preserveWhitespace) {
  if (text.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
    return Failed("XML input exceeds the parser's size limit.");
  }
  return ReadDocument(
      preserveWhitespace, XML_PARSE_NONET, [&](xmlParserCtxtPtr context, int options) {
        return xmlCtxtReadMemory(context,
                                 text.empty() ? "" : text.data(),
                                 static_cast<int>(text.size()),
                                 "agiru.xml",
                                 nullptr,
                                 options);
      });
}

Outcome<XmlHandle, std::string> ReadXmlLocation(std::string_view location,
                                                bool preserveWhitespace) {
  const std::string held(location);
  return ReadDocument(preserveWhitespace, 0, [&](xmlParserCtxtPtr context, int options) {
    return xmlCtxtReadFile(context, held.c_str(), nullptr, options);
  });
}

bool Parse(std::string_view text, bool preserveWhitespace, XmlHandle &into) {
  auto result = ReadXml(text, preserveWhitespace);
  if (!result.has_value()) { return false; }
  into = std::move(*result);
  return true;
}

bool ParseLocation(std::string_view location, bool preserveWhitespace, XmlHandle &into) {
  auto result = ReadXmlLocation(location, preserveWhitespace);
  if (!result.has_value()) { return false; }
  into = std::move(*result);
  return true;
}

namespace {

bool XPathNameByte(char value) {
  constexpr unsigned char kNonAscii = 0x80;
  return (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
         (value >= '0' && value <= '9') || value == '_' || value == '-' || value == '.' ||
         static_cast<unsigned char>(value) >= kNonAscii;
}

bool EmptyNamespacePrefix(std::string_view name,
                          const std::vector<std::pair<std::string, std::string>> &namespaces) {
  const auto found = std::ranges::find_if(namespaces, [&](const auto &binding) {
    return binding.first == name && binding.second.empty();
  });
  return found != namespaces.end() && xmlValidateNCName(Bytes(found->first), 0) == 0;
}

void AppendXPathNameTest(std::string_view expression,
                         std::size_t &at,
                         const std::vector<std::pair<std::string, std::string>> &namespaces,
                         std::string &out) {
  const std::size_t start = at;
  while (at < expression.size() && XPathNameByte(expression[at])) { ++at; }
  const std::size_t colon = at;
  if (at + 1 >= expression.size() || expression[at] != ':' || expression[at + 1] == ':') {
    out.append(expression.substr(start, at - start));
    return;
  }
  ++at;
  const std::size_t local = at;
  if (expression[at] == '*') {
    ++at;
  } else {
    while (at < expression.size() && XPathNameByte(expression[at])) { ++at; }
  }
  const std::size_t before = expression.find_last_not_of(" \t\r\n", start == 0 ? 0 : start - 1);
  const std::size_t after = expression.find_first_not_of(" \t\r\n", at);
  const bool variable = start != 0 && before != std::string_view::npos && expression[before] == '$';
  const bool function = after != std::string_view::npos && expression[after] == '(';
  const bool malformed = local == at || (at < expression.size() && expression[at] == ':');
  if (variable || function || malformed ||
      !EmptyNamespacePrefix(expression.substr(start, colon - start), namespaces)) {
    out.append(expression.substr(start, at - start));
  } else if (expression[local] == '*') {
    out += "*[namespace-uri()='']";
  } else {
    out.append(expression.substr(local, at - local));
  }
}

std::string
ResolveEmptyNamespaces(std::string_view expression,
                       const std::vector<std::pair<std::string, std::string>> &namespaces) {
  std::string out;
  out.reserve(expression.size());
  for (std::size_t at = 0; at < expression.size();) {
    if (expression[at] == '\'' || expression[at] == '"') {
      const std::size_t end = expression.find(expression[at], at + 1);
      const std::size_t size =
          end == std::string_view::npos ? expression.size() - at : end - at + 1;
      out.append(expression.substr(at, size));
      at += size;
      continue;
    }
    if (!XPathNameByte(expression[at])) {
      out += expression[at++];
      continue;
    }
    AppendXPathNameTest(expression, at, namespaces, out);
  }
  return out;
}

}

std::vector<XmlHandle> XPath(const XmlHandle &from,
                             std::string_view expression,
                             const std::vector<std::pair<std::string, std::string>> &namespaces) {
  std::vector<XmlHandle> found;
  xmlNodePtr node = NodeOf(from);
  xmlDocPtr doc = DocOf(from);
  if (node == nullptr || doc == nullptr) { return found; }
  xmlXPathContextPtr context = xmlXPathNewContext(doc);
  if (context == nullptr) { return found; }
  context->error = &Quiet;
  context->node = node->type == XML_DOCUMENT_NODE ? reinterpret_cast<xmlNodePtr>(doc) : node;
  for (const auto &[prefix, uri] : namespaces) {
    if (!prefix.empty() && !uri.empty()) { xmlXPathRegisterNs(context, Bytes(prefix), Bytes(uri)); }
  }
  const std::string held = ResolveEmptyNamespaces(expression, namespaces);
  xmlXPathObjectPtr result = xmlXPathEvalExpression(Bytes(held), context);
  if (result != nullptr) {
    if (result->type == XPATH_NODESET && result->nodesetval != nullptr) {
      for (int i = 0; i < result->nodesetval->nodeNr; ++i) {
        found.emplace_back(from.tree, result->nodesetval->nodeTab[i]);
      }
    }
    xmlXPathFreeObject(result);
  }
  xmlXPathFreeContext(context);
  return found;
}

std::vector<XmlHandle> Children(const XmlHandle &of, bool elementsOnly) {
  std::vector<XmlHandle> out;
  xmlNodePtr node = NodeOf(of);
  if (node == nullptr) { return out; }
  for (xmlNodePtr child = node->children; child != nullptr; child = child->next) {
    if (elementsOnly && child->type != XML_ELEMENT_NODE) { continue; }
    out.emplace_back(of.tree, child);
  }
  return out;
}

namespace {

void Walk(XmlTree *tree, xmlNodePtr node, bool elementsOnly, std::vector<XmlHandle> &out) {
  for (xmlNodePtr child = node->children; child != nullptr; child = child->next) {
    if (!elementsOnly || child->type == XML_ELEMENT_NODE) { out.emplace_back(tree, child); }
    Walk(tree, child, elementsOnly, out);
  }
}

}

std::vector<XmlHandle> Descendants(const XmlHandle &of, bool elementsOnly) {
  std::vector<XmlHandle> out;
  if (NodeOf(of) != nullptr) { Walk(of.tree, NodeOf(of), elementsOnly, out); }
  return out;
}

std::vector<XmlHandle> Attributes(const XmlHandle &of) {
  std::vector<XmlHandle> out;
  xmlNodePtr node = NodeOf(of);
  if (node == nullptr || node->type != XML_ELEMENT_NODE) { return out; }
  for (xmlAttrPtr attribute = node->properties; attribute != nullptr; attribute = attribute->next) {
    out.emplace_back(of.tree, attribute);
  }
  for (xmlNsPtr ns = node->nsDef; ns != nullptr; ns = ns->next) {
    const std::string name =
        ns->prefix == nullptr ? std::string("xmlns") : "xmlns:" + Text(ns->prefix);
    xmlDocPtr doc = xmlNewDoc(Bytes("1.0"));
    xmlNodePtr holder = xmlNewDocNode(doc, nullptr, Bytes("xmlns-holder"), nullptr);
    xmlDocSetRootElement(doc, holder);
    xmlAttrPtr made = xmlNewProp(holder, Bytes(name), ns->href);
    out.emplace_back(NewTree(doc), reinterpret_cast<xmlNodePtr>(made));
  }
  return out;
}

std::string QualifiedName(xmlNodePtr node) {
  if (node == nullptr) { return {}; }
  std::string local = Text(node->name);
  if (node->ns != nullptr && node->ns->prefix != nullptr) {
    return Text(node->ns->prefix) + ":" + local;
  }
  return local;
}

bool SameName(xmlNodePtr node, std::string_view qualified) {
  return QualifiedName(node) == qualified;
}

bool SameLocalName(xmlNodePtr node, std::string_view local, std::string_view uri) {
  if (node == nullptr || Text(node->name) != local) { return false; }
  const std::string held = node->ns == nullptr ? std::string{} : Text(node->ns->href);
  return held == uri;
}

bool Attachable(xmlNodePtr target, xmlNodePtr node) {
  if (target == nullptr || node == nullptr) { return false; }
  if (node->type == XML_DOCUMENT_NODE) { return false; }
  for (xmlNodePtr walk = target; walk != nullptr; walk = walk->parent) {
    if (walk == node) { return false; }
  }
  return true;
}

void Adopt(const XmlHandle &parent, const XmlHandle &child) {
  xmlNodePtr target = NodeOf(parent);
  xmlNodePtr node = NodeOf(child);
  if (!Attachable(target, node)) { return; }
  xmlUnlinkNode(node);
  if (target->type == XML_DOCUMENT_NODE) {
    auto *doc = reinterpret_cast<xmlDocPtr>(target);
    if (node->type == XML_ELEMENT_NODE && xmlDocGetRootElement(doc) == nullptr) {
      xmlDocSetRootElement(doc, node);
    } else {
      xmlAddChild(target, node);
    }
  } else {
    xmlAddChild(target, node);
  }
  if (parent.tree != child.tree) {
    xmlSetTreeDoc(node, DocOf(parent));
    KeepAlive(parent.tree, child.tree);
    KeepAlive(child.tree, parent.tree);
  }
}

}
