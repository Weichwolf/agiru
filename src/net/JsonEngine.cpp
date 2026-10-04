#include "JsonEngine.h"

#include "runtime/ErrorValue.h"
#include "type/Decimal.h"
#include "type/JsonHandle.h"

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include <yyjson.h>

namespace agiru::detail {

void JsonRetain(JsonTree *tree) noexcept {
  if (tree != nullptr) { ++tree->uses; }
}

void JsonRelease(JsonTree *tree) noexcept {
  if (tree != nullptr && --tree->uses == 0) { delete tree; }
}

JsonHandle JsonHandleMade(JsonNode value) {
  auto *tree = new JsonTree(std::move(value));
  return JsonHandle{tree, &tree->root.Node()};
}

JsonNode &JsonNodeOf(const JsonHandle &handle) {
  if (handle.node == nullptr) {
    throw Error("this JSON value refers to nothing -- nothing has been read into it");
  }
  return *static_cast<JsonNode *>(handle.node);
}

JsonHandle JsonHandleAt(const JsonHandle &tree, JsonNode &node) {
  return JsonHandle{tree.tree, &node};
}

JsonHandle NewJsonObject() {
  return JsonHandleMade(JsonNode::object());
}

JsonHandle NewJsonArray() {
  return JsonHandleMade(JsonNode::array());
}

JsonHandle NewJsonValue() {
  return JsonHandleMade(JsonNode());
}

void JsonNodeRetain(void *node) noexcept {
  if (node != nullptr) { ++static_cast<JsonNode *>(node)->uses_; }
}

void JsonNodeRelease(void *node) noexcept {
  auto *held = static_cast<JsonNode *>(node);
  if (held != nullptr && --held->uses_ == 0) { delete held; }
}

JsonOwnedNode::JsonOwnedNode(JsonNode value) : node_(new JsonNode(std::move(value))) {
  JsonNodeRetain(node_);
}

JsonOwnedNode::JsonOwnedNode(const JsonOwnedNode &other) : JsonOwnedNode(other.Node()) {}

JsonOwnedNode::JsonOwnedNode(JsonOwnedNode &&other) noexcept
    : node_(std::exchange(other.node_, nullptr)) {}

JsonOwnedNode &JsonOwnedNode::operator=(const JsonOwnedNode &other) {
  if (this != &other) {
    JsonOwnedNode copy(other);
    *this = std::move(copy);
  }
  return *this;
}

JsonOwnedNode &JsonOwnedNode::operator=(JsonOwnedNode &&other) noexcept {
  if (this != &other) {
    JsonNodeRelease(node_);
    node_ = std::exchange(other.node_, nullptr);
  }
  return *this;
}

JsonOwnedNode::~JsonOwnedNode() {
  JsonNodeRelease(node_);
}

JsonNode &JsonOwnedNode::Node() const noexcept {
  return *node_;
}

JsonNode::JsonNode(bool value) : kind_(Kind::Boolean), boolean_(value) {}

JsonNode::JsonNode(std::string value) : kind_(Kind::String), text_(std::move(value)) {}

JsonNode::JsonNode(const JsonNode &other)
    : kind_(other.kind_),
      text_(other.text_),
      boolean_(other.boolean_),
      members_(other.members_),
      children_(other.children_) {}

JsonNode::JsonNode(JsonNode &&other) noexcept
    : kind_(other.kind_),
      text_(std::move(other.text_)),
      boolean_(other.boolean_),
      members_(std::move(other.members_)),
      children_(std::move(other.children_)) {}

JsonNode &JsonNode::operator=(const JsonNode &other) {
  if (this != &other) {
    JsonNode copy(other);
    *this = std::move(copy);
  }
  return *this;
}

JsonNode &JsonNode::operator=(JsonNode &&other) noexcept {
  if (this != &other) {
    kind_ = other.kind_;
    text_ = std::move(other.text_);
    boolean_ = other.boolean_;
    members_ = std::move(other.members_);
    children_ = std::move(other.children_);
  }
  return *this;
}

JsonNode JsonNode::object() {
  JsonNode node;
  node.kind_ = Kind::Object;
  return node;
}

JsonNode JsonNode::array() {
  JsonNode node;
  node.kind_ = Kind::Array;
  return node;
}

JsonNode JsonNode::Number(std::string text) {
  JsonNode node;
  node.kind_ = Kind::Number;
  node.text_ = std::move(text);
  return node;
}

bool JsonNode::is_number_float() const noexcept {
  return is_number() && text_.find_first_of(".eE") != std::string::npos;
}

std::size_t JsonNode::size() const noexcept {
  return is_object() ? members_.size() : children_.size();
}

bool JsonNode::contains(std::string_view key) const {
  return std::ranges::any_of(members_, [key](const Member &member) { return member.first == key; });
}

JsonNode &JsonNode::operator[](std::string_view key) {
  if (!is_object()) { throw Error("JSON node is not an object"); }
  for (auto &[name, value] : members_) {
    if (name == key) { return value.Node(); }
  }
  members_.emplace_back(std::string(key), JsonOwnedNode(JsonNode()));
  return members_.back().second.Node();
}

const JsonNode &JsonNode::operator[](std::string_view key) const {
  for (const auto &[name, value] : members_) {
    if (name == key) { return value.Node(); }
  }
  throw Error("JSON object does not contain the requested member");
}

JsonNode &JsonNode::operator[](std::size_t index) {
  return children_.at(index).Node();
}

const JsonNode &JsonNode::operator[](std::size_t index) const {
  return children_.at(index).Node();
}

void JsonNode::push_back(JsonNode value) {
  if (!is_array()) { throw Error("JSON node is not an array"); }
  children_.emplace_back(std::move(value));
}

void JsonNode::insert(std::size_t index, JsonNode value) {
  if (!is_array() || index > children_.size()) { throw Error("JSON array index is out of range"); }
  children_.insert(children_.begin() + static_cast<std::ptrdiff_t>(index),
                   JsonOwnedNode(std::move(value)));
}

void JsonNode::erase(std::size_t index) {
  if (!is_array() || index >= children_.size()) { throw Error("JSON array index is out of range"); }
  children_.erase(children_.begin() + static_cast<std::ptrdiff_t>(index));
}

std::size_t JsonNode::erase(std::string_view key) {
  return std::erase_if(members_, [key](const Member &member) { return member.first == key; });
}

void JsonNode::RenameMember(std::string_view key, std::string name, JsonNode value) {
  if (key != name && contains(name)) {
    throw Error("JSON object already contains the replacement member");
  }
  for (auto &member : members_) {
    if (member.first == key) {
      JsonOwnedNode replacement(std::move(value));
      member.first = std::move(name);
      member.second = std::move(replacement);
      return;
    }
  }
  throw Error("JSON object does not contain the member to rename");
}

bool JsonNode::operator==(const JsonNode &other) const {
  if (kind_ != other.kind_) { return false; }
  if (is_number()) {
    return Decimal::FromInvariantString(ExactJsonDecimal(text_)) ==
           Decimal::FromInvariantString(ExactJsonDecimal(other.text_));
  }
  if (text_ != other.text_ || boolean_ != other.boolean_ || size() != other.size()) {
    return false;
  }
  for (std::size_t index = 0; index < children_.size(); ++index) {
    if (!(children_[index].Node() == other.children_[index].Node())) { return false; }
  }
  return std::ranges::all_of(members_, [&other](const Member &member) {
    return other.contains(member.first) && member.second.Node() == other[member.first];
  });
}

namespace {

constexpr std::size_t kMaximumJsonDepth = 256;

JsonNode Imported(yyjson_val *value, std::size_t depth) {
  if (depth > kMaximumJsonDepth) { throw Error("JSON nesting exceeds the runtime limit of 256"); }
  if (yyjson_is_null(value)) { return {}; }
  if (yyjson_is_bool(value)) { return JsonNode(yyjson_get_bool(value)); }
  if (yyjson_is_str(value)) {
    return JsonNode(std::string(yyjson_get_str(value), yyjson_get_len(value)));
  }
  if (yyjson_is_raw(value)) {
    return JsonNode::Number(std::string(yyjson_get_raw(value), yyjson_get_len(value)));
  }
  if (yyjson_is_arr(value)) {
    JsonNode array = JsonNode::array();
    yyjson_arr_iter iterator = yyjson_arr_iter_with(value);
    while (yyjson_val *child = yyjson_arr_iter_next(&iterator)) {
      array.push_back(Imported(child, depth + 1));
    }
    return array;
  }
  JsonNode object = JsonNode::object();
  yyjson_obj_iter iterator = yyjson_obj_iter_with(value);
  while (yyjson_val *key = yyjson_obj_iter_next(&iterator)) {
    object[std::string_view(yyjson_get_str(key), yyjson_get_len(key))] =
        Imported(yyjson_obj_iter_get_val(key), depth + 1);
  }
  return object;
}

yyjson_mut_val *Exported(const JsonNode &node, yyjson_mut_doc *document, std::size_t depth) {
  if (depth > kMaximumJsonDepth) { throw Error("JSON nesting exceeds the runtime limit of 256"); }
  if (node.is_null()) { return yyjson_mut_null(document); }
  if (node.is_boolean()) { return yyjson_mut_bool(document, node.Boolean()); }
  if (node.is_string()) {
    return yyjson_mut_strn(document, node.Text().data(), node.Text().size());
  }
  if (node.is_number()) {
    return yyjson_mut_rawn(document, node.Text().data(), node.Text().size());
  }
  if (node.is_array()) {
    yyjson_mut_val *array = yyjson_mut_arr(document);
    if (array == nullptr) { throw std::bad_alloc(); }
    for (const JsonOwnedNode &child : node.Children()) {
      if (!yyjson_mut_arr_add_val(array, Exported(child.Node(), document, depth + 1))) {
        throw std::bad_alloc();
      }
    }
    return array;
  }
  yyjson_mut_val *object = yyjson_mut_obj(document);
  if (object == nullptr) { throw std::bad_alloc(); }
  for (const auto &[name, value] : node.Members()) {
    if (!yyjson_mut_obj_add(object,
                            yyjson_mut_strn(document, name.data(), name.size()),
                            Exported(value.Node(), document, depth + 1))) {
      throw std::bad_alloc();
    }
  }
  return object;
}

}

JsonNode JsonNode::parse(std::string_view text) {
  yyjson_read_err error{};
  const std::unique_ptr<yyjson_doc, decltype(&yyjson_doc_free)> document(
      yyjson_read_opts(
          const_cast<char *>(text.data()), text.size(), YYJSON_READ_NUMBER_AS_RAW, nullptr, &error),
      yyjson_doc_free);
  if (document == nullptr) {
    if (error.code == YYJSON_READ_ERROR_MEMORY_ALLOCATION) { throw std::bad_alloc(); }
    JsonNode invalid;
    invalid.kind_ = Kind::Invalid;
    return invalid;
  }
  return Imported(yyjson_doc_get_root(document.get()), 0);
}

std::string JsonNode::dump(int indent) const {
  if (is_discarded()) { throw Error("cannot serialize invalid JSON"); }
  const std::unique_ptr<yyjson_mut_doc, decltype(&yyjson_mut_doc_free)> document(
      yyjson_mut_doc_new(nullptr), yyjson_mut_doc_free);
  if (document == nullptr) { throw std::bad_alloc(); }
  const yyjson_write_flag flags = indent < 0 ? 0 : YYJSON_WRITE_PRETTY_TWO_SPACES;
  std::size_t length = 0;
  const std::unique_ptr<char, decltype(&std::free)> text(
      yyjson_mut_val_write(Exported(*this, document.get(), 0), flags, &length), std::free);
  if (text == nullptr) { throw Error("JSON serialization failed"); }
  return {text.get(), length};
}

namespace {

std::int64_t JsonNumberExponent(std::string_view &digits) {
  const std::size_t exponentAt = digits.find_first_of("eE");
  std::int64_t exponent = 0;
  if (exponentAt != std::string_view::npos) {
    std::string_view exponentText = digits.substr(exponentAt + 1);
    if (exponentText.starts_with('+')) { exponentText.remove_prefix(1); }
    const auto result =
        std::from_chars(exponentText.data(), exponentText.data() + exponentText.size(), exponent);
    if (result.ec != std::errc{} || result.ptr != exponentText.data() + exponentText.size()) {
      throw Error("JSON number exponent is out of range");
    }
    digits = digits.substr(0, exponentAt);
  }
  return exponent;
}

std::string JsonCoefficient(std::string_view digits) {
  std::string coefficient;
  bool seenPoint = false;
  for (const char character : digits) {
    if (character == '.' && !seenPoint) {
      seenPoint = true;
      continue;
    }
    if (character < '0' || character > '9') { throw Error("JSON value is not an exact number"); }
    coefficient += character;
  }
  if (coefficient.empty()) { throw Error("JSON value is not an exact number"); }
  return coefficient;
}

}

std::string ExactJsonDecimal(std::string_view text) {
  std::string_view digits = text;
  const bool negative = !digits.empty() && digits.front() == '-';
  if (!digits.empty() && (digits.front() == '-' || digits.front() == '+')) {
    digits.remove_prefix(1);
  }
  const std::int64_t exponent = JsonNumberExponent(digits);
  const std::size_t point = digits.find('.');
  std::string coefficient = JsonCoefficient(digits);
  const std::size_t first = coefficient.find_first_not_of('0');
  if (first == std::string::npos) { return "0"; }
  coefficient.erase(0, first);
  constexpr std::int64_t kMaximumDecimalScale = 28;
  constexpr std::int64_t kMaximumDecimalDigits = 29;
  if (exponent < -static_cast<std::int64_t>(digits.size()) - kMaximumDecimalScale ||
      exponent > static_cast<std::int64_t>(digits.size()) + kMaximumDecimalDigits) {
    throw Error("JSON number cannot be represented exactly as Decimal");
  }
  std::int64_t scale =
      (point == std::string_view::npos ? 0 : static_cast<std::int64_t>(digits.size() - point - 1)) -
      exponent;
  while (scale > 0 && coefficient.back() == '0') {
    coefficient.pop_back();
    --scale;
  }
  if (scale > kMaximumDecimalScale ||
      static_cast<std::int64_t>(coefficient.size()) - scale > kMaximumDecimalDigits) {
    throw Error("JSON number cannot be represented exactly as Decimal");
  }
  if (scale < 0) { coefficient.append(static_cast<std::size_t>(-scale), '0'); }
  if (scale > 0) {
    if (static_cast<std::size_t>(scale) >= coefficient.size()) {
      coefficient.insert(0, static_cast<std::size_t>(scale) + 1 - coefficient.size(), '0');
    }
    coefficient.insert(coefficient.size() - static_cast<std::size_t>(scale), ".");
  }
  return (negative ? "-" : "") + coefficient;
}

std::int64_t JsonInteger(std::string_view text) {
  const std::string exact = ExactJsonDecimal(text);
  std::int64_t value = 0;
  const auto parsed = std::from_chars(exact.data(), exact.data() + exact.size(), value);
  if (parsed.ec != std::errc{} || parsed.ptr != exact.data() + exact.size()) {
    throw Error("JSON number cannot be converted to BigInteger without loss of precision");
  }
  return value;
}

JsonNode *FindJsonPath(JsonNode &root, std::string_view path) {
  JsonNode *node = &root;
  if (path.starts_with('$')) { path.remove_prefix(1); }
  while (!path.empty()) {
    if (path.front() == '.') {
      path.remove_prefix(1);
      continue;
    }
    if (path.front() == '[') {
      const std::size_t close = path.find(']');
      if (close == std::string_view::npos || !node->is_array()) { return nullptr; }
      const std::string_view digits = path.substr(1, close - 1);
      std::size_t index = 0;
      const auto parsed = std::from_chars(digits.data(), digits.data() + digits.size(), index);
      if (parsed.ec != std::errc{} || parsed.ptr != digits.data() + digits.size() ||
          index >= node->size()) {
        return nullptr;
      }
      node = &(*node)[index];
      path.remove_prefix(close + 1);
      continue;
    }
    const std::size_t next = path.find_first_of(".[");
    const std::string_view name = path.substr(0, next);
    if (!node->is_object() || !node->contains(name)) { return nullptr; }
    node = &(*node)[name];
    if (next == std::string_view::npos) { break; }
    path.remove_prefix(next);
  }
  return node;
}

}
