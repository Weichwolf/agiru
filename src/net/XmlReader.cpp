#include "dotnet/XmlReader.h"

#include "runtime/ErrorValue.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/StringValue.h"
#include "type/XmlHandle.h"

#include "XmlEngine.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include <libxml/globals.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlerror.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlstring.h>

namespace agiru::dotnet {

namespace {

constexpr std::size_t kDeclarationOpen = 5;
constexpr std::int32_t kXmlDeclaration = 17;
constexpr std::size_t kFileReadBlock = 4096;

xmlTextReaderPtr Held(const std::shared_ptr<void> &reader) {
  return static_cast<xmlTextReaderPtr>(reader.get());
}

void Quiet([[maybe_unused]] void *context, [[maybe_unused]] xmlErrorPtr error) {}

std::string Bytes(const ::agiru::InStream &stream) {
  auto &input = const_cast<::agiru::InStream &>(stream);
  return input.ReadBytes(input.Length());
}

void CopyDeclaration(xmlDocPtr source, xmlDocPtr target) {
  xmlFree(const_cast<xmlChar *>(target->version));
  target->version = xmlStrdup(source->version);
  xmlFree(const_cast<xmlChar *>(target->encoding));
  target->encoding = xmlStrdup(source->encoding);
  target->standalone = source->standalone;
}

void AppendReaderNode(xmlDocPtr doc, xmlNodePtr parent, xmlNodePtr source, int kind) {
  if (parent->type == XML_DOCUMENT_NODE) {
    const bool rootExists = xmlDocGetRootElement(doc) != nullptr;
    const bool badRoot = kind == XML_READER_TYPE_ELEMENT && rootExists;
    const bool badDtd =
        kind == XML_READER_TYPE_DOCUMENT_TYPE && (rootExists || doc->intSubset != nullptr);
    const bool badText = kind == XML_READER_TYPE_TEXT || kind == XML_READER_TYPE_CDATA ||
                         kind == XML_READER_TYPE_ENTITY_REFERENCE;
    if (badRoot || badDtd || badText || kind == XML_READER_TYPE_ATTRIBUTE) {
      throw Error("XmlDocument.Load: the current node cannot be a document child");
    }
  }
  xmlNodePtr copy =
      kind == XML_READER_TYPE_DOCUMENT_TYPE
          ? reinterpret_cast<xmlNodePtr>(xmlCopyDtd(reinterpret_cast<xmlDtdPtr>(source)))
          : xmlDocCopyNode(source, doc, kind == XML_READER_TYPE_ELEMENT ? 2 : 1);
  if (copy == nullptr) {
    throw Error("XmlDocument.Load: node type " + std::to_string(kind) + " cannot be copied");
  }
  if (xmlAddChild(parent, copy) == nullptr) {
    xmlFreeNode(copy);
    throw Error("XmlDocument.Load: the current node cannot be attached");
  }
  if (kind == XML_READER_TYPE_DOCUMENT_TYPE) { doc->intSubset = reinterpret_cast<xmlDtdPtr>(copy); }
}

}

struct XmlReader::State {
  std::shared_ptr<std::string> text;
  std::shared_ptr<void> reader;
  bool over = false;
  bool declarationPending = false;
  bool atDeclaration = false;
  std::string declaration;
};

StringReader StringReader::Binder::operator()(std::string_view text) const {
  class StringReader out;
  out.text_ = std::string(text);
  return out;
}

XmlReader XmlReader::Over(std::string text) {
  XmlReader out;
  out.state_ = std::make_shared<State>();
  State &state = *out.state_;
  {
    const std::string_view head(text);
    if (head.starts_with("<?xml") && head.size() > kDeclarationOpen &&
        (head[kDeclarationOpen] == ' ' || head[kDeclarationOpen] == '\t' ||
         head[kDeclarationOpen] == '\r' || head[kDeclarationOpen] == '\n')) {
      const std::size_t close = head.find("?>");
      state.declarationPending = close != std::string_view::npos;
      if (state.declarationPending) {
        std::string_view inside = head.substr(kDeclarationOpen, close - kDeclarationOpen);
        while (!inside.empty() && inside.front() == ' ') { inside.remove_prefix(1); }
        state.declaration = std::string(inside);
      }
    }
  }
  state.text = std::make_shared<std::string>(std::move(text));
  xmlTextReaderPtr reader = xmlReaderForMemory(state.text->data(),
                                               static_cast<int>(state.text->size()),
                                               nullptr,
                                               nullptr,
                                               XML_PARSE_NOENT | XML_PARSE_NONET);
  if (reader == nullptr) { throw Error("XmlReader.Create: the data is not XML"); }
  xmlTextReaderSetStructuredErrorHandler(reader, &Quiet, nullptr);
  state.reader = std::shared_ptr<void>(
      reader, [](void *held) { xmlFreeTextReader(static_cast<xmlTextReaderPtr>(held)); });
  return out;
}

XmlReader XmlReader::Create(std::string_view path,
                            [[maybe_unused]] const XmlReaderSettings &settings) {
  const std::unique_ptr<FILE, decltype(&std::fclose)> file(
      std::fopen(std::string(path).c_str(), "rb"), &std::fclose);
  if (file == nullptr) {
    throw Error("XmlReader.Create: the file '" + std::string(path) + "' cannot be read");
  }
  std::string text;
  char buffer[kFileReadBlock];
  while (std::feof(file.get()) == 0 && std::ferror(file.get()) == 0) {
    const std::size_t got = std::fread(buffer, 1, sizeof buffer, file.get());
    text.append(buffer, got);
    if (got != sizeof buffer) { break; }
  }
  if (std::ferror(file.get()) != 0) {
    throw Error("XmlReader.Create: the file '" + std::string(path) + "' cannot be read");
  }
  return Over(std::move(text));
}

XmlReader XmlReader::Create(const ::agiru::InStream &stream,
                            [[maybe_unused]] const XmlReaderSettings &settings) {
  return Over(Bytes(stream));
}

XmlReader XmlReader::Create([[maybe_unused]] const ::agiru::OutStream &stream,
                            [[maybe_unused]] const XmlReaderSettings &settings) {
  throw Error("XmlReader.Create(OutStream): an OutStream is written and not read");
}

XmlReader XmlReader::Create(const StringReader &reader,
                            [[maybe_unused]] const XmlReaderSettings &settings) {
  return Over(std::string(reader.Text()));
}

Boolean XmlReader::Read() {
  if (state_ == nullptr || state_->reader == nullptr || state_->over) { return false; }
  if (state_->declarationPending) {
    state_->declarationPending = false;
    state_->atDeclaration = true;
    return true;
  }
  state_->atDeclaration = false;
  const int step = xmlTextReaderRead(Held(state_->reader));
  if (step == 1) { return true; }
  state_->over = true;
  if (step < 0) { throw Error("XmlReader.Read: the data is not well-formed XML"); }
  return false;
}

void XmlReader::Close() {
  if (state_ == nullptr) { return; }
  state_->over = true;
  state_->declarationPending = false;
  state_->atDeclaration = false;
  state_->reader.reset();
}

Integer XmlReader::Depth() const {
  if (state_ == nullptr || state_->reader == nullptr || state_->atDeclaration) { return 0; }
  return static_cast<Integer>(xmlTextReaderDepth(Held(state_->reader)));
}

::agiru::Text<0> XmlReader::Name() const {
  if (state_ == nullptr || state_->reader == nullptr) { return {}; }
  if (state_->atDeclaration) { return ::agiru::Text<0>{"xml"}; }
  const xmlChar *name = xmlTextReaderConstName(Held(state_->reader));
  return ::agiru::Text<0>{name == nullptr ? std::string{}
                                          : std::string(reinterpret_cast<const char *>(name))};
}

::agiru::Text<0> XmlReader::Value() const {
  if (state_ == nullptr || state_->reader == nullptr) { return {}; }
  if (state_->atDeclaration) { return ::agiru::Text<0>{state_->declaration}; }
  const xmlChar *value = xmlTextReaderConstValue(Held(state_->reader));
  return ::agiru::Text<0>{value == nullptr ? std::string{}
                                           : std::string(reinterpret_cast<const char *>(value))};
}

XmlNodeType XmlReader::NodeType() const {
  if (state_ == nullptr || state_->reader == nullptr) { return {}; }
  if (state_->atDeclaration) { return XmlNodeType{kXmlDeclaration}; }
  xmlTextReaderPtr backend = Held(state_->reader);
  int kind = xmlTextReaderNodeType(backend);
  if (kind == XML_READER_TYPE_SIGNIFICANT_WHITESPACE) {
    xmlNodePtr node = xmlTextReaderCurrentNode(backend);
    if (node != nullptr && xmlNodeGetSpacePreserve(node->parent) != 1) {
      kind = XML_READER_TYPE_WHITESPACE;
    }
  }
  return XmlNodeType{kind};
}

Boolean XmlReader::MoveToFirstAttribute() {
  return state_ != nullptr && !state_->atDeclaration && state_->reader != nullptr &&
         xmlTextReaderMoveToFirstAttribute(Held(state_->reader)) == 1;
}

Boolean XmlReader::MoveToNextAttribute() {
  return state_ != nullptr && state_->reader != nullptr &&
         xmlTextReaderMoveToNextAttribute(Held(state_->reader)) == 1;
}

Boolean XmlReader::MoveToElement() {
  return state_ != nullptr && state_->reader != nullptr &&
         xmlTextReaderMoveToElement(Held(state_->reader)) == 1;
}

Boolean XmlReader::IsEmptyElement() const {
  return state_ != nullptr && state_->reader != nullptr &&
         xmlTextReaderIsEmptyElement(Held(state_->reader)) == 1;
}

Boolean XmlReader::Eof() const {
  return state_ != nullptr && state_->over;
}

void XmlReader::LoadDocument(::agiru::detail::XmlHandle &into, bool preserveWhitespace) const {
  if (state_ == nullptr) { throw Error("XmlDocument.Load: the reader was never created"); }
  std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> owned(
      xmlNewDoc(reinterpret_cast<const xmlChar *>("1.0")), &xmlFreeDoc);
  if (owned == nullptr) { throw Error("XmlDocument.Load: the document cannot be allocated"); }
  xmlDocPtr doc = owned.get();
  auto *tree = ::agiru::detail::NewTree(doc);
  into = ::agiru::detail::XmlHandle(tree, owned.release());
  XmlReader cursor = *this;
  if (state_->reader == nullptr || state_->over) { return; }
  if (cursor.NodeType().Equals(XmlNodeType::None()) && !cursor.Read()) { return; }
  auto *parent = reinterpret_cast<xmlNodePtr>(doc);
  bool declaration = false;
  do {
    if (state_->atDeclaration) {
      declaration = true;
      continue;
    }
    xmlTextReaderPtr backend = Held(state_->reader);
    const int kind = cursor.NodeType().Number();
    if (kind == XML_READER_TYPE_END_ELEMENT) {
      if (parent->type == XML_DOCUMENT_NODE) { return; }
      parent = parent->parent;
      continue;
    }
    if (kind == XML_READER_TYPE_WHITESPACE && !preserveWhitespace) { continue; }
    xmlNodePtr source = xmlTextReaderCurrentNode(backend);
    if (source == nullptr) { throw Error("XmlDocument.Load: the reader has no current node"); }
    if (declaration) {
      CopyDeclaration(source->doc, doc);
      declaration = false;
    }
    AppendReaderNode(doc, parent, source, kind);
    if (kind == XML_READER_TYPE_ELEMENT && !cursor.IsEmptyElement()) { parent = parent->last; }
  } while (cursor.Read());
}

}
