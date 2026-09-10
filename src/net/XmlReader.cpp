#include "dotnet/XmlReader.h"

#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/Text.h"

#include <libxml/xmlreader.h>

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace agiru::dotnet {

namespace {

constexpr std::size_t kDeclarationOpen = 5;
constexpr std::int32_t kXmlDeclaration = 17;

xmlTextReaderPtr Held(const std::shared_ptr<void> &reader) {
  return static_cast<xmlTextReaderPtr>(reader.get());
}

void Quiet(void *, xmlErrorPtr) {}

std::string Bytes(const ::agiru::InStream &stream) {
  auto &input = const_cast<::agiru::InStream &>(stream);
  return input.ReadBytes(input.Length());
}

}

StringReader StringReader::Binder::operator()(std::string_view text) const {
  class StringReader out;
  out.text_ = std::string(text);
  return out;
}

XmlReader XmlReader::Over(std::string text) {
  XmlReader out;
  {
    const std::string_view head(text);
    if (head.starts_with("<?xml")) {
      const std::size_t close = head.find("?>");
      out.declarationPending_ = close != std::string_view::npos;
      if (out.declarationPending_) {
        std::string_view inside = head.substr(kDeclarationOpen, close - kDeclarationOpen);
        while (!inside.empty() && inside.front() == ' ') { inside.remove_prefix(1); }
        out.declaration_ = std::string(inside);
      }
    }
  }
  out.text_ = std::make_shared<std::string>(std::move(text));
  xmlTextReaderPtr reader = xmlReaderForMemory(out.text_->data(),
                                               static_cast<int>(out.text_->size()),
                                               nullptr,
                                               nullptr,
                                               XML_PARSE_NOENT | XML_PARSE_NONET);
  if (reader == nullptr) { throw Error("XmlReader.Create: the data is not XML"); }
  xmlTextReaderSetStructuredErrorHandler(reader, &Quiet, nullptr);
  out.reader_ = std::shared_ptr<void>(reader, [](void *held) {
    xmlFreeTextReader(static_cast<xmlTextReaderPtr>(held));
  });
  return out;
}

XmlReader XmlReader::Create(std::string_view path, const XmlReaderSettings &) {
  std::string text;
  if (FILE *file = std::fopen(std::string(path).c_str(), "rb"); file != nullptr) {
    char buffer[4096];
    for (std::size_t got = std::fread(buffer, 1, sizeof buffer, file); got != 0;
         got = std::fread(buffer, 1, sizeof buffer, file)) {
      text.append(buffer, got);
    }
    std::fclose(file);
  } else {
    throw Error("XmlReader.Create: the file '" + std::string(path) + "' cannot be read");
  }
  return Over(std::move(text));
}

XmlReader XmlReader::Create(const ::agiru::InStream &stream, const XmlReaderSettings &) {
  return Over(Bytes(stream));
}

XmlReader XmlReader::Create(const ::agiru::OutStream &, const XmlReaderSettings &) {
  throw Error("XmlReader.Create(OutStream): an OutStream is written and not read");
}

XmlReader XmlReader::Create(const StringReader &reader, const XmlReaderSettings &) {
  return Over(std::string(reader.Text()));
}

Boolean XmlReader::Read() {
  if (reader_ == nullptr || over_) { return false; }
  if (declarationPending_) {
    declarationPending_ = false;
    atDeclaration_ = true;
    return true;
  }
  atDeclaration_ = false;
  const int step = xmlTextReaderRead(Held(reader_));
  if (step == 1) { return true; }
  over_ = true;
  if (step < 0) { throw Error("XmlReader.Read: the data is not well-formed XML"); }
  return false;
}

void XmlReader::Close() {
  over_ = true;
  reader_.reset();
}

Integer XmlReader::Depth() const {
  if (atDeclaration_) { return 0; }
  return reader_ == nullptr ? 0 : static_cast<Integer>(xmlTextReaderDepth(Held(reader_)));
}

::agiru::Text<0> XmlReader::Name() const {
  if (atDeclaration_) { return ::agiru::Text<0>{"xml"}; }
  if (reader_ == nullptr) { return {}; }
  const xmlChar *name = xmlTextReaderConstName(Held(reader_));
  return ::agiru::Text<0>{name == nullptr ? std::string{} : std::string(reinterpret_cast<const char *>(name))};
}

::agiru::Text<0> XmlReader::Value() const {
  if (atDeclaration_) { return ::agiru::Text<0>{declaration_}; }
  if (reader_ == nullptr) { return {}; }
  const xmlChar *value = xmlTextReaderConstValue(Held(reader_));
  return ::agiru::Text<0>{value == nullptr ? std::string{} : std::string(reinterpret_cast<const char *>(value))};
}

XmlNodeType XmlReader::NodeType() const {
  if (atDeclaration_) { return XmlNodeType{kXmlDeclaration}; }
  return reader_ == nullptr ? XmlNodeType{} : XmlNodeType{xmlTextReaderNodeType(Held(reader_))};
}

Boolean XmlReader::MoveToFirstAttribute() {
  return !atDeclaration_ && reader_ != nullptr &&
         xmlTextReaderMoveToFirstAttribute(Held(reader_)) == 1;
}

Boolean XmlReader::MoveToNextAttribute() {
  return reader_ != nullptr && xmlTextReaderMoveToNextAttribute(Held(reader_)) == 1;
}

Boolean XmlReader::MoveToElement() {
  return reader_ != nullptr && xmlTextReaderMoveToElement(Held(reader_)) == 1;
}

Boolean XmlReader::IsEmptyElement() const {
  return reader_ != nullptr && xmlTextReaderIsEmptyElement(Held(reader_)) == 1;
}

Boolean XmlReader::Eof() const {
  return over_;
}

}
