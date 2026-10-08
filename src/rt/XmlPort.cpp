#include "runtime/XmlPort.h"

#include "dotnet/Encoding.h"
#include "dotnet/XmlDocument.h"
#include "dotnet/XmlNode.h"
#include "meta/Ids.h"
#include "meta/TableDef.h"
#include "runtime/ErrorValue.h"
#include "runtime/Report.h"
#include "type/Integer.h"
#include "type/Stream.h"
#include "type/TextEncoding.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru {

namespace {

std::vector<const XmlPortEntry *> &XmlPortEntries() {
  static std::vector<const XmlPortEntry *> entries;
  return entries;
}

std::once_flag &XmlPortsOnce() {
  static std::once_flag once;
  return once;
}

constexpr std::int32_t kWindowsCodePage = 1252;
constexpr std::int32_t kMsDosCodePage = 437;
constexpr std::int32_t kReadBlock = 65536;

std::string Escaped(std::string_view text) {
  std::string out;
  out.reserve(text.size());
  for (const char c : text) {
    switch (c) {
      case '&': out += "&amp;"; break;
      case '<': out += "&lt;"; break;
      case '>': out += "&gt;"; break;
      case '"': out += "&quot;"; break;
      default: out += c;
    }
  }
  return out;
}

std::string Padded(std::string_view text, std::int32_t width) {
  if (width <= 0) { return std::string(text); }
  std::string out(text.substr(0, static_cast<std::size_t>(width)));
  out.resize(static_cast<std::size_t>(width), ' ');
  return out;
}

bool SameWord(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) { return false; }
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(a[i])) !=
        std::tolower(static_cast<unsigned char>(b[i]))) {
      return false;
    }
  }
  return true;
}

}

void RegisterXmlPortEntry(const XmlPortEntry *entry) {
  XmlPortEntries().push_back(entry);
}

const XmlPortEntry *FindXmlPort(XmlPortId id) {
  std::call_once(XmlPortsOnce(), [] {
    std::ranges::sort(XmlPortEntries(), [](const XmlPortEntry *a, const XmlPortEntry *b) {
      return a->id.Value() < b->id.Value();
    });
  });
  const auto found = std::lower_bound(
      XmlPortEntries().begin(),
      XmlPortEntries().end(),
      id.Value(),
      [](const XmlPortEntry *entry, auto number) { return entry->id.Value() < number; });
  if (found == XmlPortEntries().end() || (*found)->id != id) { return nullptr; }
  return *found;
}

void XmlPortOutput::Start(const XmlPortDef &def,
                          std::string_view fieldSeparator,
                          std::string_view recordSeparator,
                          std::string_view fieldDelimiter,
                          std::string_view tableSeparator) {
  Start(def,
        {.field = fieldSeparator,
         .record = recordSeparator,
         .delimiter = fieldDelimiter,
         .table = tableSeparator});
}

void XmlPortOutput::Start(const XmlPortDef &def, const detail::XmlPortSeparators &separators) {
  if (def.format == XmlPortFormat::Xml && def.inlineSchema) {
    throw Error("XmlPort.Export: InlineSchema=true requires XML schema generation");
  }
  def_ = def;
  fieldSeparator_ = std::string(separators.field);
  recordSeparator_ = std::string(separators.record);
  fieldDelimiter_ = std::string(separators.delimiter);
  tableSeparator_ = std::string(separators.table);
  root_ = Node{};
  path_.clear();
  lines_.clear();
  fields_.clear();
  records_ = 0;
  inRecord_ = false;
}

XmlPortOutput::Node *XmlPortOutput::Open_() {
  Node *at = &root_;
  for (const std::size_t index : path_) { at = &at->children[index]; }
  return at;
}

void XmlPortOutput::BeginGroup(std::string_view name) {
  if (def_.format != XmlPortFormat::Xml) { return; }
  Node *open = Open_();
  Node made;
  made.name = std::string(name);
  open->children.push_back(std::move(made));
  path_.push_back(open->children.size() - 1);
}

void XmlPortOutput::EndGroup(std::string_view name) {
  static_cast<void>(name);
  if (def_.format != XmlPortFormat::Xml || path_.empty()) { return; }
  path_.pop_back();
}

void XmlPortOutput::BeginRecord(std::string_view name) {
  if (def_.format == XmlPortFormat::Xml) {
    BeginGroup(name);
    return;
  }
  fields_.clear();
  inRecord_ = true;
}

void XmlPortOutput::EndRecord(std::string_view name) {
  if (def_.format == XmlPortFormat::Xml) {
    EndGroup(name);
    return;
  }
  std::string line;
  for (std::size_t i = 0; i < fields_.size(); ++i) {
    if (i != 0 && def_.format == XmlPortFormat::VariableText) { line += fieldSeparator_; }
    line += fields_[i];
  }
  lines_ += line;
  lines_ += recordSeparator_;
  fields_.clear();
  inRecord_ = false;
  ++records_;
}

void XmlPortOutput::Value(std::string_view name,
                          std::string_view text,
                          bool attribute,
                          std::int32_t width) {
  if (def_.format == XmlPortFormat::Xml) {
    Node *open = Open_();
    if (attribute) {
      open->attributes.emplace_back(std::string(name), std::string(text));
      return;
    }
    Node leaf;
    leaf.name = std::string(name);
    leaf.text = std::string(text);
    leaf.hasText = true;
    open->children.push_back(std::move(leaf));
    return;
  }
  const std::string field = def_.format == XmlPortFormat::FixedText
                                ? Padded(text, width)
                                : fieldDelimiter_ + std::string(text) + fieldDelimiter_;
  if (inRecord_) {
    fields_.push_back(field);
    return;
  }
  lines_ += field;
  lines_ += recordSeparator_;
}

void XmlPortOutput::Content(std::string_view name, std::string_view text, std::int32_t width) {
  static_cast<void>(name);
  static_cast<void>(width);
  if (def_.format != XmlPortFormat::Xml) { return; }
  Node *open = Open_();
  open->text = std::string(text);
  open->hasText = true;
}

void XmlPortOutput::Serialize(const Node &node, std::string &into, int depth) const {
  static_cast<void>(depth);
  into += "<" + node.name;
  for (const auto &[name, value] : node.attributes) {
    into += " " + name + "=\"" + Escaped(value) + "\"";
  }
  if (node.children.empty() && !node.hasText) {
    into += " />";
    return;
  }
  into += ">";
  if (node.hasText) { into += Escaped(node.text); }
  for (const Node &child : node.children) { Serialize(child, into, depth + 1); }
  into += "</" + node.name + ">";
}

std::string XmlPortOutput::Finish() const {
  if (def_.format != XmlPortFormat::Xml) { return lines_; }
  std::string out = R"(<?xml version="1.0" encoding="utf-8" standalone="yes"?>)";
  bool first = true;
  for (const Node &child : root_.children) {
    if (first && (def_.useDefaultNamespace || !def_.namespaces.empty())) {
      Node declared = child;
      std::vector<std::pair<std::string, std::string>> declarations;
      if (def_.useDefaultNamespace && !def_.defaultNamespace.empty()) {
        declarations.emplace_back("xmlns", std::string(def_.defaultNamespace));
      }
      for (const XmlNamespaceDef &space : def_.namespaces) {
        declarations.emplace_back(space.prefix.empty() ? std::string("xmlns")
                                                       : "xmlns:" + std::string(space.prefix),
                                  std::string(space.uri));
      }
      declared.attributes.insert(
          declared.attributes.begin(), declarations.begin(), declarations.end());
      Serialize(declared, out, 0);
    } else {
      Serialize(child, out, 0);
    }
    first = false;
  }
  return out;
}

void XmlPortInput::Load(std::string_view bytes,
                        const XmlPortDef &def,
                        std::string_view fieldSeparator,
                        std::string_view recordSeparator,
                        std::string_view fieldDelimiter) {
  Load(bytes,
       def,
       {.field = fieldSeparator, .record = recordSeparator, .delimiter = fieldDelimiter});
}

void XmlPortInput::Load(std::string_view bytes,
                        const XmlPortDef &def,
                        const detail::XmlPortSeparators &separators) {
  def_ = def;
  root_ = Node{};
  levels_.clear();
  lines_.clear();
  line_ = 0;
  field_ = 0;
  depth_ = 0;
  textFormat_ = def.format != XmlPortFormat::Xml;
  if (textFormat_) {
    ParseLines(bytes, separators);
    return;
  }
  if (def.useLax.has_value()) {
    throw Error(std::string("XmlPort.Import: UseLax=") + (*def.useLax ? "true" : "false") +
                " requires XML schema validation");
  }
  ParseXml(bytes);
}

namespace {

struct TextRecord {
  std::string text;
  std::size_t next;
};

TextRecord ReadTextRecord(std::string_view text, std::size_t at, std::string_view separator) {
  std::size_t end = separator.empty() ? std::string_view::npos : text.find(separator, at);
  if (end == std::string_view::npos && separator == "\r\n") { end = text.find('\n', at); }
  std::string line(text.substr(at, end == std::string_view::npos ? end : end - at));
  if (!line.empty() && line.back() == '\r') { line.pop_back(); }
  const std::size_t next =
      end == std::string_view::npos
          ? text.size() + 1
          : end + (text.substr(end, separator.size()) == separator ? separator.size() : 1);
  return {.text = std::move(line), .next = next};
}

std::optional<std::string> ReadQuotedField(std::string_view line,
                                           std::size_t &from,
                                           const detail::XmlPortSeparators &separators) {
  const auto delimiter = separators.delimiter;
  if (delimiter.empty() || line.substr(from, delimiter.size()) != delimiter) { return {}; }
  const std::size_t close = line.find(delimiter, from + delimiter.size());
  if (close == std::string_view::npos) { return {}; }
  std::string field(line.substr(from + delimiter.size(), close - from - delimiter.size()));
  from = close + delimiter.size();
  const std::size_t next = line.find(separators.field, from);
  from = next == std::string_view::npos ? line.size() + 1 : next + separators.field.size();
  return field;
}

std::vector<std::string> ReadTextFields(std::string_view line,
                                        const detail::XmlPortSeparators &separators) {
  std::vector<std::string> fields;
  std::size_t from = 0;
  while (from <= line.size()) {
    if (auto quoted = ReadQuotedField(line, from, separators)) {
      fields.push_back(std::move(*quoted));
      continue;
    }
    const std::size_t next = line.find(separators.field, from);
    fields.emplace_back(line.substr(from, next == std::string_view::npos ? next : next - from));
    from = next == std::string_view::npos ? line.size() + 1 : next + separators.field.size();
  }
  return fields;
}

}

void XmlPortInput::ParseLines(std::string_view text, const detail::XmlPortSeparators &separators) {
  std::size_t at = 0;
  while (at <= text.size()) {
    TextRecord record = ReadTextRecord(text, at, separators.record);
    at = record.next;
    if (record.text.empty() && at > text.size()) { break; }
    if (def_.format == XmlPortFormat::FixedText || separators.field.empty()) {
      lines_.push_back({std::move(record.text)});
    } else {
      lines_.push_back(ReadTextFields(record.text, separators));
    }
  }
}

void XmlPortInput::ParseXml(std::string_view text) {
  dotnet::XmlDocument document;
  document.LoadXml(text);
  const dotnet::XmlNode element = document.DocumentElement();
  const auto build = [](auto &&self, const dotnet::XmlNode &from, Node &into) -> void {
    into.name = from.LocalName();
    const dotnet::XmlAttributeCollection attributes = from.Attributes();
    const Integer count = attributes.Count();
    for (Integer i = 0; i < count; ++i) {
      const dotnet::XmlAttribute attribute = attributes.Item(i);
      into.attributes.emplace_back(attribute.LocalName(), attribute.Value());
    }
    std::string text;
    for (const dotnet::XmlNode &child : from.ChildNodes().Nodes()) {
      const std::string name(std::string_view(child.Name()));
      if (name.empty()) { continue; }
      if (name.front() == '#') {
        if (name == "#text" || name == "#cdata-section") { text += child.Value(); }
        continue;
      }
      Node made;
      self(self, child, made);
      into.children.push_back(std::move(made));
    }
    into.text = text;
  };
  build(build, element, root_);
  Node holder;
  holder.children.push_back(std::move(root_));
  root_ = std::move(holder);
}

bool XmlPortInput::Enter(std::string_view name) {
  if (textFormat_) {
    if (depth_ == 0) {
      ++depth_;
      return true;
    }
    if (depth_ == 1) {
      if (line_ >= lines_.size()) { return false; }
      field_ = 0;
      ++depth_;
      return true;
    }
    if (field_ >= lines_[line_].size()) { return false; }
    ++depth_;
    return true;
  }
  const Node *parent = levels_.empty() ? &root_ : levels_.back().node;
  std::size_t &next = levels_.empty() ? field_ : levels_.back().next;
  if (const std::size_t colon = name.find(':'); colon != std::string_view::npos) {
    name = name.substr(colon + 1);
  }
  for (std::size_t i = next; i < parent->children.size(); ++i) {
    if (SameWord(parent->children[i].name, name)) {
      next = i + 1;
      levels_.push_back(Level{.node = &parent->children[i], .next = 0});
      return true;
    }
  }
  return false;
}

void XmlPortInput::Leave() {
  if (textFormat_) {
    if (depth_ == 3) {
      ++field_;
    } else if (depth_ == 2) {
      ++line_;
    }
    if (depth_ > 0) { --depth_; }
    return;
  }
  if (!levels_.empty()) { levels_.pop_back(); }
}

std::string XmlPortInput::Text() const {
  if (textFormat_) {
    if (line_ < lines_.size() && field_ < lines_[line_].size()) { return lines_[line_][field_]; }
    return {};
  }
  return levels_.empty() ? std::string{} : levels_.back().node->text;
}

std::string XmlPortInput::Attribute(std::string_view name) const {
  if (textFormat_ || levels_.empty()) { return {}; }
  if (const std::size_t colon = name.find(':'); colon != std::string_view::npos) {
    name = name.substr(colon + 1);
  }
  for (const auto &[key, value] : levels_.back().node->attributes) {
    if (SameWord(key, name)) { return value; }
  }
  return {};
}

namespace detail {

namespace {

dotnet::Encoding EncodingOf(TextEncoding encoding) {
  switch (encoding) {
    case TextEncoding::UTF8: return dotnet::Encoding::Made(dotnet::Encoding::kUtf8, false);
    case TextEncoding::UTF16: return dotnet::Encoding::Made(dotnet::Encoding::kUtf16, true);
    case TextEncoding::Windows: return dotnet::Encoding::Made(kWindowsCodePage, false);
    case TextEncoding::MSDos: return dotnet::Encoding::Made(kMsDosCodePage, false);
  }
  return dotnet::Encoding::Made(dotnet::Encoding::kUtf8, false);
}

}

std::string EncodeForXmlPort(std::string_view text, TextEncoding encoding) {
  const dotnet::Encoding made = EncodingOf(encoding);
  std::string out;
  if (encoding == TextEncoding::UTF16) { out = "\xFF\xFE"; }
  out += made.Encode(text);
  return out;
}

std::string DecodeForXmlPort(std::string_view bytes, TextEncoding encoding) {
  if (bytes.starts_with("\xEF\xBB\xBF")) { bytes.remove_prefix(3); }
  if (encoding == TextEncoding::UTF16 && bytes.starts_with("\xFF\xFE")) { bytes.remove_prefix(2); }
  return EncodingOf(encoding).Decode(bytes);
}

std::string SeparatorText(std::string_view declared) {
  std::string out;
  for (std::size_t i = 0; i < declared.size();) {
    if (declared[i] == '<') {
      const std::size_t close = declared.find('>', i);
      if (close != std::string_view::npos) {
        const std::string_view token = declared.substr(i + 1, close - i - 1);
        if (SameWord(token, "TAB")) {
          out += '\t';
        } else if (SameWord(token, "NewLine")) {
          out += "\r\n";
        } else if (SameWord(token, "None")) {
          static_cast<void>(0);
        } else {
          out += token;
        }
        i = close + 1;
        continue;
      }
    }
    out += declared[i];
    ++i;
  }
  return out;
}

std::string ReadWhole(InStream &stream) {
  std::string out;
  while (!stream.EOS()) {
    const std::string block = stream.ReadBytes(kReadBlock);
    if (block.empty()) { break; }
    out += block;
  }
  return out;
}

void ApplyElementView(void *record, const TableDef &table, std::string_view view) {
  ApplyDataItemView(record, table, view);
}

void AdoptElementView(void *to, const void *from) {
  AdoptTableView(to, from);
}

}

}
