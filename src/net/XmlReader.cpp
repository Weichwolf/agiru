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
#include <cstring>
#include <memory>
#include <span>
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
constexpr std::size_t kDoctypeOpen = std::string_view("<!DOCTYPE").size();
constexpr std::int32_t kXmlDeclaration = 17;
constexpr std::size_t kFileReadBlock = 4096;

class MarkupBytes {
public:
  explicit MarkupBytes(std::string_view bytes) : bytes_(bytes) {
    if (bytes.starts_with(std::string_view("\xff\xfe\0\0", 4))) {
      width_ = 4;
      begin_ = 4;
    } else if (bytes.starts_with(std::string_view("\0\0\xfe\xff", 4))) {
      width_ = 4;
      low_ = 3;
      begin_ = 4;
    } else if (bytes.starts_with("\xff\xfe")) {
      width_ = 2;
      begin_ = 2;
    } else if (bytes.starts_with("\xfe\xff")) {
      width_ = 2;
      low_ = 1;
      begin_ = 2;
    } else if (bytes.starts_with("\xef\xbb\xbf")) {
      begin_ = 3;
    } else if (bytes.size() >= 4 && bytes.substr(0, 4) == std::string_view("<\0\0\0", 4)) {
      width_ = 4;
    } else if (bytes.size() >= 4 && bytes.substr(0, 4) == std::string_view("\0\0\0<", 4)) {
      width_ = 4;
      low_ = 3;
    } else if (bytes.starts_with(std::string_view("<\0", 2))) {
      width_ = 2;
    } else if (bytes.starts_with(std::string_view("\0<", 2))) {
      width_ = 2;
      low_ = 1;
    }
  }

  [[nodiscard]] std::size_t Begin() const { return begin_; }

  [[nodiscard]] std::size_t Width() const { return width_; }

  void Append(std::string &bytes, std::string_view text) const {
    for (const char character : text) {
      const std::size_t offset = bytes.size();
      bytes.resize(offset + width_, '\0');
      bytes[offset + low_] = character;
    }
  }

  [[nodiscard]] char At(std::size_t offset) const {
    if (offset > bytes_.size() || width_ > bytes_.size() - offset) { return '\0'; }
    for (std::size_t index = 0; index < width_; ++index) {
      if (index != low_ && bytes_[offset + index] != '\0') { return '\xff'; }
    }
    return bytes_[offset + low_];
  }

  [[nodiscard]] bool Is(std::size_t offset, std::string_view text) const {
    for (const char character : text) {
      if (At(offset) != character) { return false; }
      offset += width_;
    }
    return true;
  }

  [[nodiscard]] std::size_t After(std::size_t offset, std::string_view terminal) const {
    while (offset < bytes_.size()) {
      if (Is(offset, terminal)) { return offset + terminal.size() * width_; }
      offset += width_;
    }
    return bytes_.size();
  }

private:
  std::string_view bytes_;
  std::size_t width_ = 1;
  std::size_t low_ = 0;
  std::size_t begin_ = 0;
};

bool XmlSpace(char character) {
  return character == ' ' || character == '\t' || character == '\r' || character == '\n';
}

void SkipSpace(const MarkupBytes &source, std::size_t &offset) {
  while (XmlSpace(source.At(offset))) { offset += source.Width(); }
}

bool SkipQuoted(const MarkupBytes &source, std::size_t &offset, std::size_t size) {
  const char quote = source.At(offset);
  if (quote != '\'' && quote != '"') { return false; }
  offset += source.Width();
  while (offset < size) {
    const char character = source.At(offset);
    offset += source.Width();
    if (character == quote) { return true; }
  }
  return false;
}

std::size_t DoctypeHeaderEnd(const MarkupBytes &source, std::size_t start, std::size_t size) {
  std::size_t offset = start + kDoctypeOpen * source.Width();
  if (!XmlSpace(source.At(offset))) { return start; }
  SkipSpace(source, offset);
  const std::size_t name = offset;
  while (offset < size && !XmlSpace(source.At(offset)) && source.At(offset) != '[' &&
         source.At(offset) != '>') {
    offset += source.Width();
  }
  if (offset == name) { return start; }
  SkipSpace(source, offset);
  if (source.At(offset) == '[' || source.At(offset) == '>') { return offset; }
  const bool publicId = source.Is(offset, "PUBLIC");
  if (!publicId && !source.Is(offset, "SYSTEM")) { return start; }
  offset += std::string_view("SYSTEM").size() * source.Width();
  if (!XmlSpace(source.At(offset))) { return start; }
  SkipSpace(source, offset);
  if (!SkipQuoted(source, offset, size)) { return start; }
  if (publicId) {
    if (!XmlSpace(source.At(offset))) { return start; }
    SkipSpace(source, offset);
    if (!SkipQuoted(source, offset, size)) { return start; }
  }
  SkipSpace(source, offset);
  return source.At(offset) == '[' || source.At(offset) == '>' ? offset : start;
}

std::size_t DoctypeEnd(const MarkupBytes &source, std::size_t start, std::size_t size) {
  std::size_t offset = DoctypeHeaderEnd(source, start, size);
  if (offset == start) { return start; }
  if (source.At(offset) == '>') { return offset + source.Width(); }
  char quote = '\0';
  for (offset += source.Width(); offset < size; offset += source.Width()) {
    const char character = source.At(offset);
    if (quote != '\0') {
      if (character == quote) { quote = '\0'; }
    } else if (source.Is(offset, "<!--")) {
      offset = source.After(offset, "-->") - source.Width();
    } else if (source.Is(offset, "<?")) {
      offset = source.After(offset, "?>") - source.Width();
    } else if (character == '\'' || character == '"') {
      quote = character;
    } else if (character == ']') {
      offset += source.Width();
      SkipSpace(source, offset);
      return source.At(offset) == '>' ? offset + source.Width() : start;
    }
  }
  return start;
}

void RetainBytes(std::string &text, std::size_t begin, std::size_t end, std::size_t &written) {
  const std::size_t count = end - begin;
  if (begin != written && count != 0) {
    const std::span bytes(text);
    std::memmove(bytes.subspan(written).data(), bytes.subspan(begin).data(), count);
  }
  written += count;
}

std::string ApplyDtdPolicy(std::string &text, const XmlReaderSettings &settings) {
  const auto processing = settings.DtdProcessing().Number();
  if (processing == 2) { return {}; }
  if (processing != 0 && processing != 1) {
    throw Error("XmlReader.Create: invalid DtdProcessing value");
  }
  const MarkupBytes source(text);
  std::size_t offset = source.Begin();
  std::size_t retained = 0;
  std::size_t segment = 0;
  while (offset < text.size()) {
    if (XmlSpace(source.At(offset))) {
      offset += source.Width();
    } else if (source.Is(offset, "<!--")) {
      offset = source.After(offset, "-->");
    } else if (source.Is(offset, "<?")) {
      offset = source.After(offset, "?>");
    } else if (source.Is(offset, "<!DOCTYPE")) {
      if (processing == 0) {
        RetainBytes(text, segment, offset, retained);
        text.resize(retained);
        source.Append(text, "<dtd-policy-boundary/>");
        return "XmlReader.Read: DTD processing is prohibited";
      }
      const std::size_t end = DoctypeEnd(source, offset, text.size());
      if (end == offset) {
        RetainBytes(text, segment, offset, retained);
        text.resize(retained);
        source.Append(text, "<dtd-policy-boundary/>");
        return "XmlReader.Read: the DTD is not well-formed XML";
      }
      RetainBytes(text, segment, offset, retained);
      segment = end;
      offset = end;
    } else {
      break;
    }
  }
  RetainBytes(text, segment, text.size(), retained);
  text.resize(retained);
  return {};
}

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
  std::string policyFailure;
  std::string declaration;
};

StringReader StringReader::Binder::operator()(std::string_view text) const {
  class StringReader out;
  out.text_ = std::string(text);
  return out;
}

XmlReader XmlReader::Over(std::string text, const XmlReaderSettings &settings) {
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
  state.policyFailure = ApplyDtdPolicy(text, settings);
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

XmlReader XmlReader::Create(std::string_view path, const XmlReaderSettings &settings) {
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
  return Over(std::move(text), settings);
}

XmlReader XmlReader::Create(const ::agiru::InStream &stream, const XmlReaderSettings &settings) {
  return Over(Bytes(stream), settings);
}

XmlReader XmlReader::Create([[maybe_unused]] const ::agiru::OutStream &stream,
                            [[maybe_unused]] const XmlReaderSettings &settings) {
  throw Error("XmlReader.Create(OutStream): an OutStream is written and not read");
}

XmlReader XmlReader::Create(const StringReader &reader, const XmlReaderSettings &settings) {
  return Over(std::string(reader.Text()), settings);
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
  if (step == 1) {
    if (!state_->policyFailure.empty() &&
        xmlTextReaderNodeType(Held(state_->reader)) == XML_READER_TYPE_ELEMENT) {
      state_->over = true;
      state_->reader.reset();
      throw Error(state_->policyFailure);
    }
    return true;
  }
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
