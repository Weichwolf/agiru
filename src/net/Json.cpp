#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Byte.h"
#include "type/Char.h"
#include "type/Date.h"
#include "type/DateTime.h"
#include "type/Decimal.h"
#include "type/Duration.h"
#include "type/Integer.h"
#include "type/JsonArray.h"
#include "type/JsonHandle.h"
#include "type/JsonObject.h"
#include "type/JsonToken.h"
#include "type/JsonValue.h"
#include "type/List.h"
#include "type/SecretText.h"
#include "type/Stream.h"
#include "type/Text.h"
#include "type/Time.h"

#include "JsonEngine.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

namespace agiru::detail {

void JsonRetain(JsonTree *tree) noexcept {
  if (tree != nullptr) { ++tree->uses; }
}

void JsonRelease(JsonTree *tree) noexcept {
  if (tree != nullptr && --tree->uses == 0) { delete tree; }
}

JsonHandle JsonHandleMade(nlohmann::ordered_json value) {
  auto *tree = new JsonTree{.root = std::move(value), .uses = 0};
  return JsonHandle{tree, &tree->root};
}

nlohmann::ordered_json &JsonNodeOf(const JsonHandle &handle) {
  if (handle.node == nullptr) {
    throw Error("this JSON value refers to nothing -- nothing has been read into it");
  }
  return *static_cast<nlohmann::ordered_json *>(handle.node);
}

JsonHandle JsonHandleAt(const JsonHandle &tree, nlohmann::ordered_json &node) {
  return JsonHandle{tree.tree, &node};
}

JsonHandle NewJsonObject() {
  return JsonHandleMade(nlohmann::ordered_json::object());
}

JsonHandle NewJsonArray() {
  return JsonHandleMade(nlohmann::ordered_json::array());
}

JsonHandle NewJsonValue() {
  return JsonHandleMade(nlohmann::ordered_json());
}

}

namespace agiru {

namespace {

using Json = nlohmann::ordered_json;

Json &Node(const detail::JsonHandle &handle) {
  return detail::JsonNodeOf(handle);
}

Json FromDecimal(const Decimal &value) {
  const std::string text = value.ToInvariantString();
  return Json::parse(text, nullptr, false, false);
}

template <typename T> Json Valued(const T &value);

template <> Json Valued<BigInteger>(const BigInteger &value) {
  return Json(static_cast<std::int64_t>(value));
}

template <> Json Valued<Boolean>(const Boolean &value) {
  return Json(static_cast<bool>(value));
}

template <> Json Valued<Byte>(const Byte &value) {
  return Json(static_cast<std::int64_t>(value));
}

template <> Json Valued<Char>(const Char &value) {
  return Json(Encoded(value));
}

template <> Json Valued<Date>(const Date &value) {
  return Json(value.ToInvariantString());
}

template <> Json Valued<DateTime>(const DateTime &value) {
  return Json(value.ToInvariantString());
}

template <> Json Valued<Decimal>(const Decimal &value) {
  return FromDecimal(value);
}

template <> Json Valued<Duration>(const Duration &value) {
  return Json(static_cast<std::int64_t>(value.Milliseconds()));
}

template <> Json Valued<Integer>(const Integer &value) {
  return Json(static_cast<std::int64_t>(value));
}

template <> Json Valued<Time>(const Time &value) {
  return Json(value.ToInvariantString());
}

Json Textual(std::string_view value) {
  return Json(std::string(value));
}

std::string TextOf(const Json &node) {
  if (node.is_string()) { return node.get<std::string>(); }
  if (node.is_null()) { return {}; }
  return node.dump();
}

}

}

namespace agiru {

namespace {

Boolean AddTo(const detail::JsonHandle &handle, std::string_view key, Json value) {
  Json &node = Node(handle);
  if (!node.is_object()) { return false; }
  const std::string name(key);
  if (node.contains(name)) { return false; }
  node[name] = std::move(value);
  return true;
}

Boolean ReplaceIn(const detail::JsonHandle &handle, std::string_view key, Json value) {
  Json &node = Node(handle);
  if (!node.is_object()) { return false; }
  const std::string name(key);
  if (!node.contains(name)) { return false; }
  node[name] = std::move(value);
  return true;
}

void AppendVoid(const detail::JsonHandle &handle, Json value);

Boolean AppendTo(const detail::JsonHandle &handle, Json value) {
  Json &node = Node(handle);
  if (!node.is_array()) { return false; }
  node.push_back(std::move(value));
  return true;
}

bool WithinArray(const Json &node, Integer index) {
  return node.is_array() && index >= 0 && static_cast<std::size_t>(index) < node.size();
}

Boolean InsertInto(const detail::JsonHandle &handle, Integer index, Json value) {
  Json &node = Node(handle);
  if (!node.is_array() || index < 0 || static_cast<std::size_t>(index) > node.size()) {
    return false;
  }
  node.insert(node.begin() + index, std::move(value));
  return true;
}

Boolean SetIn(const detail::JsonHandle &handle, Integer index, Json value) {
  Json &node = Node(handle);
  if (!WithinArray(node, index)) { return false; }
  node[static_cast<std::size_t>(index)] = std::move(value);
  return true;
}

void AppendVoid(const detail::JsonHandle &handle, Json value) {
  static_cast<void>(AppendTo(handle, std::move(value)));
}

Integer IndexIn(const detail::JsonHandle &handle, const Json &value) {
  const Json &node = Node(handle);
  if (!node.is_array()) { return -1; }
  for (std::size_t at = 0; at < node.size(); ++at) {
    if (node[at] == value) { return static_cast<Integer>(at); }
  }
  return -1;
}

}

}

namespace agiru {

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::BigInteger Value) {
  return AddTo(Handle_, Key, Valued<BigInteger>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Boolean Value) {
  return AddTo(Handle_, Key, Valued<Boolean>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Byte Value) {
  return AddTo(Handle_, Key, Valued<Byte>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Char Value) {
  return AddTo(Handle_, Key, Valued<Char>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Date Value) {
  return AddTo(Handle_, Key, Valued<Date>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::DateTime Value) {
  return AddTo(Handle_, Key, Valued<DateTime>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Decimal Value) {
  return AddTo(Handle_, Key, Valued<Decimal>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Duration Value) {
  return AddTo(Handle_, Key, Valued<Duration>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Integer Value) {
  return AddTo(Handle_, Key, Valued<Integer>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, const ::agiru::JsonArray &Value) {
  return AddTo(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, const ::agiru::JsonObject &Value) {
  return AddTo(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, const ::agiru::JsonToken &Value) {
  return AddTo(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, const ::agiru::JsonValue &Value) {
  return AddTo(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, std::string_view Value) {
  return AddTo(Handle_, Key, Textual(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Time Value) {
  return AddTo(Handle_, Key, Valued<Time>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::BigInteger Value) {
  return ReplaceIn(Handle_, Key, Valued<BigInteger>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Boolean Value) {
  return ReplaceIn(Handle_, Key, Valued<Boolean>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Byte Value) {
  return ReplaceIn(Handle_, Key, Valued<Byte>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Char Value) {
  return ReplaceIn(Handle_, Key, Valued<Char>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Date Value) {
  return ReplaceIn(Handle_, Key, Valued<Date>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::DateTime Value) {
  return ReplaceIn(Handle_, Key, Valued<DateTime>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Decimal Value) {
  return ReplaceIn(Handle_, Key, Valued<Decimal>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Duration Value) {
  return ReplaceIn(Handle_, Key, Valued<Duration>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Integer Value) {
  return ReplaceIn(Handle_, Key, Valued<Integer>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, const ::agiru::JsonArray &Value) {
  return ReplaceIn(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, const ::agiru::JsonObject &Value) {
  return ReplaceIn(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, const ::agiru::JsonToken &Value) {
  return ReplaceIn(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, const ::agiru::JsonValue &Value) {
  return ReplaceIn(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, std::string_view Value) {
  return ReplaceIn(Handle_, Key, Textual(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Time Value) {
  return ReplaceIn(Handle_, Key, Valued<Time>(Value));
}

void JsonArray::Add(::agiru::BigInteger Value) {
  AppendVoid(Handle_, Valued<BigInteger>(Value));
}

void JsonArray::Add(::agiru::Boolean Value) {
  AppendVoid(Handle_, Valued<Boolean>(Value));
}

void JsonArray::Add(::agiru::Byte Value) {
  AppendVoid(Handle_, Valued<Byte>(Value));
}

void JsonArray::Add(::agiru::Char Value) {
  AppendVoid(Handle_, Valued<Char>(Value));
}

void JsonArray::Add(::agiru::Date Value) {
  AppendVoid(Handle_, Valued<Date>(Value));
}

void JsonArray::Add(::agiru::DateTime Value) {
  AppendVoid(Handle_, Valued<DateTime>(Value));
}

void JsonArray::Add(::agiru::Decimal Value) {
  AppendVoid(Handle_, Valued<Decimal>(Value));
}

void JsonArray::Add(::agiru::Duration Value) {
  AppendVoid(Handle_, Valued<Duration>(Value));
}

void JsonArray::Add(::agiru::Integer Value) {
  AppendVoid(Handle_, Valued<Integer>(Value));
}

void JsonArray::Add(const ::agiru::JsonArray &Value) {
  AppendVoid(Handle_, Node(Value.Handle_));
}

void JsonArray::Add(const ::agiru::JsonObject &Value) {
  AppendVoid(Handle_, Node(Value.Handle_));
}

void JsonArray::Add(const ::agiru::JsonToken &Value) {
  AppendVoid(Handle_, Node(Value.Handle_));
}

void JsonArray::Add(const ::agiru::JsonValue &Value) {
  AppendVoid(Handle_, Node(Value.Handle_));
}

void JsonArray::Add(std::string_view Value) {
  AppendVoid(Handle_, Textual(Value));
}

void JsonArray::Add(::agiru::Time Value) {
  AppendVoid(Handle_, Valued<Time>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::BigInteger Value) {
  return IndexIn(Handle_, Valued<BigInteger>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Boolean Value) {
  return IndexIn(Handle_, Valued<Boolean>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Byte Value) {
  return IndexIn(Handle_, Valued<Byte>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Char Value) {
  return IndexIn(Handle_, Valued<Char>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Date Value) {
  return IndexIn(Handle_, Valued<Date>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::DateTime Value) {
  return IndexIn(Handle_, Valued<DateTime>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Decimal Value) {
  return IndexIn(Handle_, Valued<Decimal>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Duration Value) {
  return IndexIn(Handle_, Valued<Duration>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Integer Value) {
  return IndexIn(Handle_, Valued<Integer>(Value));
}

::agiru::Integer JsonArray::IndexOf(const ::agiru::JsonArray &Value) {
  return IndexIn(Handle_, Node(Value.Handle_));
}

::agiru::Integer JsonArray::IndexOf(const ::agiru::JsonObject &Value) {
  return IndexIn(Handle_, Node(Value.Handle_));
}

::agiru::Integer JsonArray::IndexOf(const ::agiru::JsonToken &Value) {
  return IndexIn(Handle_, Node(Value.Handle_));
}

::agiru::Integer JsonArray::IndexOf(const ::agiru::JsonValue &Value) {
  return IndexIn(Handle_, Node(Value.Handle_));
}

::agiru::Integer JsonArray::IndexOf(std::string_view Value) {
  return IndexIn(Handle_, Textual(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Time Value) {
  return IndexIn(Handle_, Valued<Time>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::BigInteger Value) {
  return InsertInto(Handle_, Index, Valued<BigInteger>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Boolean Value) {
  return InsertInto(Handle_, Index, Valued<Boolean>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Byte Value) {
  return InsertInto(Handle_, Index, Valued<Byte>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Char Value) {
  return InsertInto(Handle_, Index, Valued<Char>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Date Value) {
  return InsertInto(Handle_, Index, Valued<Date>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::DateTime Value) {
  return InsertInto(Handle_, Index, Valued<DateTime>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Decimal Value) {
  return InsertInto(Handle_, Index, Valued<Decimal>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Duration Value) {
  return InsertInto(Handle_, Index, Valued<Duration>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Integer Value) {
  return InsertInto(Handle_, Index, Valued<Integer>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, const ::agiru::JsonArray &Value) {
  return InsertInto(Handle_, Index, Node(Value.Handle_));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, const ::agiru::JsonObject &Value) {
  return InsertInto(Handle_, Index, Node(Value.Handle_));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, const ::agiru::JsonToken &Value) {
  return InsertInto(Handle_, Index, Node(Value.Handle_));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, const ::agiru::JsonValue &Value) {
  return InsertInto(Handle_, Index, Node(Value.Handle_));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, std::string_view Value) {
  return InsertInto(Handle_, Index, Textual(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Time Value) {
  return InsertInto(Handle_, Index, Valued<Time>(Value));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::BigInteger Result) {
  return SetIn(Handle_, Index, Valued<BigInteger>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Boolean Result) {
  return SetIn(Handle_, Index, Valued<Boolean>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Byte Result) {
  return SetIn(Handle_, Index, Valued<Byte>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Char Result) {
  return SetIn(Handle_, Index, Valued<Char>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Date Result) {
  return SetIn(Handle_, Index, Valued<Date>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::DateTime Result) {
  return SetIn(Handle_, Index, Valued<DateTime>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Decimal Result) {
  return SetIn(Handle_, Index, Valued<Decimal>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Duration Result) {
  return SetIn(Handle_, Index, Valued<Duration>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Integer Result) {
  return SetIn(Handle_, Index, Valued<Integer>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, const ::agiru::JsonArray &Result) {
  return SetIn(Handle_, Index, Node(Result.Handle_));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, const ::agiru::JsonObject &Result) {
  return SetIn(Handle_, Index, Node(Result.Handle_));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, const ::agiru::JsonToken &Result) {
  return SetIn(Handle_, Index, Node(Result.Handle_));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, const ::agiru::JsonValue &Result) {
  return SetIn(Handle_, Index, Node(Result.Handle_));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, std::string_view Result) {
  return SetIn(Handle_, Index, Textual(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Time Result) {
  return SetIn(Handle_, Index, Valued<Time>(Result));
}

void JsonValue::SetValue(::agiru::BigInteger Value) {
  Node(Handle_) = Valued<BigInteger>(Value);
}

void JsonValue::SetValue(::agiru::Boolean Value) {
  Node(Handle_) = Valued<Boolean>(Value);
}

void JsonValue::SetValue(::agiru::Byte Value) {
  Node(Handle_) = Valued<Byte>(Value);
}

void JsonValue::SetValue(::agiru::Char Value) {
  Node(Handle_) = Valued<Char>(Value);
}

void JsonValue::SetValue(::agiru::Date Value) {
  Node(Handle_) = Valued<Date>(Value);
}

void JsonValue::SetValue(::agiru::DateTime Value) {
  Node(Handle_) = Valued<DateTime>(Value);
}

void JsonValue::SetValue(::agiru::Decimal Value) {
  Node(Handle_) = Valued<Decimal>(Value);
}

void JsonValue::SetValue(::agiru::Duration Value) {
  Node(Handle_) = Valued<Duration>(Value);
}

void JsonValue::SetValue(::agiru::Integer Value) {
  Node(Handle_) = Valued<Integer>(Value);
}

void JsonValue::SetValue(std::string_view Value) {
  Node(Handle_) = Textual(Value);
}

void JsonValue::SetValue(::agiru::Time Value) {
  Node(Handle_) = Valued<Time>(Value);
}

}

namespace agiru {

Boolean JsonObject::Contains(std::string_view Key) {
  const Json &node = Node(Handle_);
  return node.is_object() && node.contains(std::string(Key));
}

Boolean JsonObject::Get(std::string_view Key, JsonToken &Result) {
  Json &node = Node(Handle_);
  const std::string name(Key);
  if (!node.is_object() || !node.contains(name)) { return false; }
  Result.Handle_ = detail::JsonHandle{Handle_.tree, &node[name]};
  return true;
}

Boolean JsonObject::Remove(std::string_view Key) {
  Json &node = Node(Handle_);
  const std::string name(Key);
  if (!node.is_object() || !node.contains(name)) { return false; }
  node.erase(name);
  return true;
}

::agiru::List<std::string> JsonObject::Keys() {
  ::agiru::List<std::string> keys;
  const Json &node = Node(Handle_);
  if (node.is_object()) {
    for (const auto &one : node.items()) { keys.Add(one.key()); }
  }
  return keys;
}

::agiru::List<::agiru::JsonToken> JsonObject::Values() {
  ::agiru::List<::agiru::JsonToken> values;
  Json &node = Node(Handle_);
  if (node.is_object()) {
    for (auto &one : node.items()) {
      JsonToken token;
      token.Handle_ = detail::JsonHandle{Handle_.tree, &one.value()};
      values.Add(token);
    }
  }
  return values;
}

Boolean JsonObject::ReadFrom(std::string_view String) {
  Json parsed = Json::parse(String, nullptr, false);
  if (parsed.is_discarded() || !parsed.is_object()) { return false; }
  Handle_ = detail::NewJsonObject();
  Node(Handle_) = std::move(parsed);
  return true;
}

Boolean JsonObject::WriteTo(::agiru::Text<0> &String) {
  String = ::agiru::Text<0>{Node(Handle_).dump()};
  return true;
}

Integer JsonArray::Count() {
  const Json &node = Node(Handle_);
  return node.is_array() ? static_cast<Integer>(node.size()) : 0;
}

Boolean JsonArray::Get(Integer Index, JsonToken &Result) {
  Json &node = Node(Handle_);
  if (!WithinArray(node, Index)) { return false; }
  Result.Handle_ = detail::JsonHandle{Handle_.tree, &node[static_cast<std::size_t>(Index)]};
  return true;
}

Boolean JsonArray::RemoveAt(Integer Index) {
  Json &node = Node(Handle_);
  if (!WithinArray(node, Index)) { return false; }
  node.erase(node.begin() + Index);
  return true;
}

Boolean JsonArray::ReadFrom(std::string_view String) {
  Json parsed = Json::parse(String, nullptr, false);
  if (parsed.is_discarded() || !parsed.is_array()) { return false; }
  Handle_ = detail::NewJsonArray();
  Node(Handle_) = std::move(parsed);
  return true;
}

Boolean JsonArray::WriteTo(::agiru::Text<0> &String) {
  String = ::agiru::Text<0>{Node(Handle_).dump()};
  return true;
}

JsonValue JsonToken::AsValue() {
  JsonValue value;
  value.Handle_ = Handle_;
  return value;
}

JsonObject JsonToken::AsObject() {
  JsonObject object;
  object.Handle_ = Handle_;
  return object;
}

JsonArray JsonToken::AsArray() {
  JsonArray array;
  array.Handle_ = Handle_;
  return array;
}

Boolean JsonToken::IsValue() {
  const Json &node = Node(Handle_);
  return !node.is_object() && !node.is_array();
}

Boolean JsonToken::IsObject() {
  return Node(Handle_).is_object();
}

Boolean JsonToken::IsArray() {
  return Node(Handle_).is_array();
}

Boolean JsonToken::WriteTo(::agiru::Text<0> &String) {
  String = ::agiru::Text<0>{Node(Handle_).dump()};
  return true;
}

Boolean JsonValue::IsNull() {
  return Node(Handle_).is_null();
}

::agiru::Text<0> JsonValue::AsText() {
  return TextOf(Node(Handle_));
}

std::string JsonValue::AsCode() {
  return TextOf(Node(Handle_));
}

Integer JsonValue::AsInteger() {
  const Json &node = Node(Handle_);
  if (node.is_number()) { return static_cast<Integer>(node.get<std::int64_t>()); }
  if (node.is_boolean()) { return node.get<bool>() ? 1 : 0; }
  return static_cast<Integer>(::agiru::Round(Decimal::FromInvariantString(TextOf(node)), 1));
}

BigInteger JsonValue::AsBigInteger() {
  const Json &node = Node(Handle_);
  return node.is_number() ? static_cast<BigInteger>(node.get<std::int64_t>())
                          : static_cast<BigInteger>(AsInteger());
}

Decimal JsonValue::AsDecimal() {
  const Json &node = Node(Handle_);
  if (node.is_string()) { return Decimal::FromInvariantString(node.get<std::string>()); }
  return Decimal::FromInvariantString(node.dump());
}

Boolean JsonValue::AsBoolean() {
  const Json &node = Node(Handle_);
  if (node.is_boolean()) { return node.get<bool>(); }
  if (node.is_number()) { return node.get<std::int64_t>() != 0; }
  const std::string text = TextOf(node);
  return text == "true" || text == "True";
}

namespace {

Boolean SelectIn(const detail::JsonHandle &handle, std::string_view path, JsonToken &into) {
  Json *node = &Node(handle);
  std::string_view rest = path;
  if (rest.starts_with("$")) { rest.remove_prefix(1); }
  while (!rest.empty()) {
    if (rest.front() == '.') {
      rest.remove_prefix(1);
      continue;
    }
    if (rest.front() == '[') {
      const std::size_t close = rest.find(']');
      if (close == std::string_view::npos || !node->is_array()) { return false; }
      const std::string digits(rest.substr(1, close - 1));
      std::size_t at = 0;
      for (const char c : digits) {
        if (c < '0' || c > '9') { return false; }
        at = (at * 10) + static_cast<std::size_t>(c - '0');
      }
      if (at >= node->size()) { return false; }
      node = &(*node)[at];
      rest.remove_prefix(close + 1);
      continue;
    }
    const std::size_t next = rest.find_first_of(".[");
    const std::string name(rest.substr(0, next));
    if (!node->is_object() || !node->contains(name)) { return false; }
    node = &(*node)[name];
    if (next == std::string_view::npos) { break; }
    rest.remove_prefix(next);
  }
  into.Handle_ = detail::JsonHandle{handle.tree, node};
  return true;
}

JsonToken FoundAt(const detail::JsonHandle &handle, std::string_view key, bool &found) {
  JsonToken token;
  Json &node = Node(handle);
  const std::string name(key);
  found = node.is_object() && node.contains(name);
  if (found) { token.Handle_ = detail::JsonHandle{handle.tree, &node[name]}; }
  return token;
}

[[noreturn]] void NoSuchKey(std::string_view key) {
  throw Error("JsonObject: it holds no key '" + std::string(key) + "'");
}

}

Boolean JsonObject::SelectToken(std::string_view Path, JsonToken &Result) {
  return SelectIn(Handle_, Path, Result);
}

Boolean JsonArray::SelectToken(std::string_view Path, JsonToken &Result) {
  return SelectIn(Handle_, Path, Result);
}

::agiru::Text<0> JsonObject::GetText(std::string_view Key, Boolean DefaultIfNotFound) {
  bool found = false;
  JsonToken token = FoundAt(Handle_, Key, found);
  if (!found) {
    if (!DefaultIfNotFound) { NoSuchKey(Key); }
    return {};
  }
  return token.AsValue().AsText();
}

Integer JsonObject::GetInteger(std::string_view Key, Boolean DefaultIfNotFound) {
  bool found = false;
  JsonToken token = FoundAt(Handle_, Key, found);
  if (!found) {
    if (!DefaultIfNotFound) { NoSuchKey(Key); }
    return 0;
  }
  return token.AsValue().AsInteger();
}

Decimal JsonObject::GetDecimal(std::string_view Key, Boolean DefaultIfNotFound) {
  bool found = false;
  JsonToken token = FoundAt(Handle_, Key, found);
  if (!found) {
    if (!DefaultIfNotFound) { NoSuchKey(Key); }
    return Decimal{};
  }
  return token.AsValue().AsDecimal();
}

Boolean JsonObject::GetBoolean(std::string_view Key, Boolean DefaultIfNotFound) {
  bool found = false;
  JsonToken token = FoundAt(Handle_, Key, found);
  if (!found) {
    if (!DefaultIfNotFound) { NoSuchKey(Key); }
    return false;
  }
  return token.AsValue().AsBoolean();
}

JsonObject JsonObject::GetObject(std::string_view Key, Boolean DefaultIfNotFound) {
  bool found = false;
  JsonToken token = FoundAt(Handle_, Key, found);
  if (!found) {
    if (!DefaultIfNotFound) { NoSuchKey(Key); }
    JsonObject empty;
    empty.Handle_ = detail::NewJsonObject();
    return empty;
  }
  return token.AsObject();
}

JsonArray JsonObject::GetArray(std::string_view Key, Boolean DefaultIfNotFound) {
  bool found = false;
  JsonToken token = FoundAt(Handle_, Key, found);
  if (!found) {
    if (!DefaultIfNotFound) { NoSuchKey(Key); }
    JsonArray empty;
    empty.Handle_ = detail::NewJsonArray();
    return empty;
  }
  return token.AsArray();
}

}
