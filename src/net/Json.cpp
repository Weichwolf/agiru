#include "runtime/ErrorValue.h"
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
#include "type/StringValue.h"
#include "type/Text.h"
#include "type/Time.h"

#include "JsonEngine.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

namespace agiru {

namespace {

using Json = detail::JsonNode;

Json &Node(const detail::JsonHandle &handle) {
  return detail::JsonNodeOf(handle);
}

Json FromDecimal(const Decimal &value) {
  const std::string text = value.ToInvariantString();
  return Json::Number(text);
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
  if (node.is_string()) { return node.Text(); }
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
  node.insert(static_cast<std::size_t>(index), std::move(value));
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

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::BigInteger Value) const {
  return AddTo(Handle_, Key, Valued<BigInteger>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Boolean Value) const {
  return AddTo(Handle_, Key, Valued<Boolean>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Byte Value) const {
  return AddTo(Handle_, Key, Valued<Byte>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Char Value) const {
  return AddTo(Handle_, Key, Valued<Char>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Date Value) const {
  return AddTo(Handle_, Key, Valued<Date>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::DateTime Value) const {
  return AddTo(Handle_, Key, Valued<DateTime>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Decimal Value) const {
  return AddTo(Handle_, Key, Valued<Decimal>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Duration Value) const {
  return AddTo(Handle_, Key, Valued<Duration>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Integer Value) const {
  return AddTo(Handle_, Key, Valued<Integer>(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, const ::agiru::JsonArray &Value) const {
  return AddTo(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, const ::agiru::JsonObject &Value) const {
  return AddTo(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, const ::agiru::JsonToken &Value) const {
  return AddTo(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, const ::agiru::JsonValue &Value) const {
  return AddTo(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, std::string_view Value) const {
  return AddTo(Handle_, Key, Textual(Value));
}

::agiru::Boolean JsonObject::Add(std::string_view Key, ::agiru::Time Value) const {
  return AddTo(Handle_, Key, Valued<Time>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::BigInteger Value) const {
  return ReplaceIn(Handle_, Key, Valued<BigInteger>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Boolean Value) const {
  return ReplaceIn(Handle_, Key, Valued<Boolean>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Byte Value) const {
  return ReplaceIn(Handle_, Key, Valued<Byte>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Char Value) const {
  return ReplaceIn(Handle_, Key, Valued<Char>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Date Value) const {
  return ReplaceIn(Handle_, Key, Valued<Date>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::DateTime Value) const {
  return ReplaceIn(Handle_, Key, Valued<DateTime>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Decimal Value) const {
  return ReplaceIn(Handle_, Key, Valued<Decimal>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Duration Value) const {
  return ReplaceIn(Handle_, Key, Valued<Duration>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Integer Value) const {
  return ReplaceIn(Handle_, Key, Valued<Integer>(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, const ::agiru::JsonArray &Value) const {
  return ReplaceIn(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, const ::agiru::JsonObject &Value) const {
  return ReplaceIn(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, const ::agiru::JsonToken &Value) const {
  return ReplaceIn(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, const ::agiru::JsonValue &Value) const {
  return ReplaceIn(Handle_, Key, Node(Value.Handle_));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, std::string_view Value) const {
  return ReplaceIn(Handle_, Key, Textual(Value));
}

::agiru::Boolean JsonObject::Replace(std::string_view Key, ::agiru::Time Value) const {
  return ReplaceIn(Handle_, Key, Valued<Time>(Value));
}

void JsonArray::Add(::agiru::BigInteger Value) const {
  AppendVoid(Handle_, Valued<BigInteger>(Value));
}

void JsonArray::Add(::agiru::Boolean Value) const {
  AppendVoid(Handle_, Valued<Boolean>(Value));
}

void JsonArray::Add(::agiru::Byte Value) const {
  AppendVoid(Handle_, Valued<Byte>(Value));
}

void JsonArray::Add(::agiru::Char Value) const {
  AppendVoid(Handle_, Valued<Char>(Value));
}

void JsonArray::Add(::agiru::Date Value) const {
  AppendVoid(Handle_, Valued<Date>(Value));
}

void JsonArray::Add(::agiru::DateTime Value) const {
  AppendVoid(Handle_, Valued<DateTime>(Value));
}

void JsonArray::Add(::agiru::Decimal Value) const {
  AppendVoid(Handle_, Valued<Decimal>(Value));
}

void JsonArray::Add(::agiru::Duration Value) const {
  AppendVoid(Handle_, Valued<Duration>(Value));
}

void JsonArray::Add(::agiru::Integer Value) const {
  AppendVoid(Handle_, Valued<Integer>(Value));
}

void JsonArray::Add(const ::agiru::JsonArray &Value) const {
  AppendVoid(Handle_, Node(Value.Handle_));
}

void JsonArray::Add(const ::agiru::JsonObject &Value) const {
  AppendVoid(Handle_, Node(Value.Handle_));
}

void JsonArray::Add(const ::agiru::JsonToken &Value) const {
  AppendVoid(Handle_, Node(Value.Handle_));
}

void JsonArray::Add(const ::agiru::JsonValue &Value) const {
  AppendVoid(Handle_, Node(Value.Handle_));
}

void JsonArray::Add(std::string_view Value) const {
  AppendVoid(Handle_, Textual(Value));
}

void JsonArray::Add(::agiru::Time Value) const {
  AppendVoid(Handle_, Valued<Time>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::BigInteger Value) const {
  return IndexIn(Handle_, Valued<BigInteger>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Boolean Value) const {
  return IndexIn(Handle_, Valued<Boolean>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Byte Value) const {
  return IndexIn(Handle_, Valued<Byte>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Char Value) const {
  return IndexIn(Handle_, Valued<Char>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Date Value) const {
  return IndexIn(Handle_, Valued<Date>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::DateTime Value) const {
  return IndexIn(Handle_, Valued<DateTime>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Decimal Value) const {
  return IndexIn(Handle_, Valued<Decimal>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Duration Value) const {
  return IndexIn(Handle_, Valued<Duration>(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Integer Value) const {
  return IndexIn(Handle_, Valued<Integer>(Value));
}

::agiru::Integer JsonArray::IndexOf(const ::agiru::JsonArray &Value) const {
  return IndexIn(Handle_, Node(Value.Handle_));
}

::agiru::Integer JsonArray::IndexOf(const ::agiru::JsonObject &Value) const {
  return IndexIn(Handle_, Node(Value.Handle_));
}

::agiru::Integer JsonArray::IndexOf(const ::agiru::JsonToken &Value) const {
  return IndexIn(Handle_, Node(Value.Handle_));
}

::agiru::Integer JsonArray::IndexOf(const ::agiru::JsonValue &Value) const {
  return IndexIn(Handle_, Node(Value.Handle_));
}

::agiru::Integer JsonArray::IndexOf(std::string_view Value) const {
  return IndexIn(Handle_, Textual(Value));
}

::agiru::Integer JsonArray::IndexOf(::agiru::Time Value) const {
  return IndexIn(Handle_, Valued<Time>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::BigInteger Value) const {
  return InsertInto(Handle_, Index, Valued<BigInteger>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Boolean Value) const {
  return InsertInto(Handle_, Index, Valued<Boolean>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Byte Value) const {
  return InsertInto(Handle_, Index, Valued<Byte>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Char Value) const {
  return InsertInto(Handle_, Index, Valued<Char>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Date Value) const {
  return InsertInto(Handle_, Index, Valued<Date>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::DateTime Value) const {
  return InsertInto(Handle_, Index, Valued<DateTime>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Decimal Value) const {
  return InsertInto(Handle_, Index, Valued<Decimal>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Duration Value) const {
  return InsertInto(Handle_, Index, Valued<Duration>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Integer Value) const {
  return InsertInto(Handle_, Index, Valued<Integer>(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, const ::agiru::JsonArray &Value) const {
  return InsertInto(Handle_, Index, Node(Value.Handle_));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, const ::agiru::JsonObject &Value) const {
  return InsertInto(Handle_, Index, Node(Value.Handle_));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, const ::agiru::JsonToken &Value) const {
  return InsertInto(Handle_, Index, Node(Value.Handle_));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, const ::agiru::JsonValue &Value) const {
  return InsertInto(Handle_, Index, Node(Value.Handle_));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, std::string_view Value) const {
  return InsertInto(Handle_, Index, Textual(Value));
}

::agiru::Boolean JsonArray::Insert(::agiru::Integer Index, ::agiru::Time Value) const {
  return InsertInto(Handle_, Index, Valued<Time>(Value));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::BigInteger Result) const {
  return SetIn(Handle_, Index, Valued<BigInteger>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Boolean Result) const {
  return SetIn(Handle_, Index, Valued<Boolean>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Byte Result) const {
  return SetIn(Handle_, Index, Valued<Byte>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Char Result) const {
  return SetIn(Handle_, Index, Valued<Char>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Date Result) const {
  return SetIn(Handle_, Index, Valued<Date>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::DateTime Result) const {
  return SetIn(Handle_, Index, Valued<DateTime>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Decimal Result) const {
  return SetIn(Handle_, Index, Valued<Decimal>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Duration Result) const {
  return SetIn(Handle_, Index, Valued<Duration>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Integer Result) const {
  return SetIn(Handle_, Index, Valued<Integer>(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, const ::agiru::JsonArray &Result) const {
  return SetIn(Handle_, Index, Node(Result.Handle_));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, const ::agiru::JsonObject &Result) const {
  return SetIn(Handle_, Index, Node(Result.Handle_));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, const ::agiru::JsonToken &Result) const {
  return SetIn(Handle_, Index, Node(Result.Handle_));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, const ::agiru::JsonValue &Result) const {
  return SetIn(Handle_, Index, Node(Result.Handle_));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, std::string_view Result) const {
  return SetIn(Handle_, Index, Textual(Result));
}

::agiru::Boolean JsonArray::Set(::agiru::Integer Index, ::agiru::Time Result) const {
  return SetIn(Handle_, Index, Valued<Time>(Result));
}

void JsonValue::SetValue(::agiru::BigInteger Value) {
  Handle_ = detail::JsonHandleMade(Textual(std::to_string(Value)));
}

void JsonValue::SetValue(::agiru::Boolean Value) {
  Handle_ = detail::JsonHandleMade(Valued<Boolean>(Value));
}

void JsonValue::SetValue(::agiru::Byte Value) {
  Handle_ = detail::JsonHandleMade(Valued<Byte>(Value));
}

void JsonValue::SetValue(::agiru::Char Value) {
  Handle_ = detail::JsonHandleMade(Json(static_cast<std::int32_t>(Value)));
}

void JsonValue::SetValue(::agiru::Date Value) {
  Handle_ = detail::JsonHandleMade(Valued<Date>(Value));
}

void JsonValue::SetValue(::agiru::DateTime Value) {
  Handle_ = detail::JsonHandleMade(Valued<DateTime>(Value));
}

void JsonValue::SetValue(::agiru::Decimal Value) {
  Handle_ = detail::JsonHandleMade(Textual(Value.ToInvariantString()));
}

void JsonValue::SetValue(::agiru::Duration Value) {
  Handle_ = detail::JsonHandleMade(Valued<Duration>(Value));
}

void JsonValue::SetValue(::agiru::Integer Value) {
  Handle_ = detail::JsonHandleMade(Valued<Integer>(Value));
}

void JsonValue::SetValue(std::string_view Value) {
  Handle_ = detail::JsonHandleMade(Textual(Value));
}

void JsonValue::SetValue(::agiru::Time Value) {
  Handle_ = detail::JsonHandleMade(Valued<Time>(Value));
}

}

namespace agiru {

Boolean JsonObject::Contains(std::string_view Key) const {
  const Json &node = Node(Handle_);
  return node.is_object() && node.contains(std::string(Key));
}

Boolean JsonObject::Get(std::string_view Key, JsonToken &Result) const {
  Json &node = Node(Handle_);
  const std::string name(Key);
  if (!node.is_object() || !node.contains(name)) { return false; }
  Result.Handle_ = detail::JsonHandle{Handle_.tree, &node[name]};
  return true;
}

Boolean JsonObject::Remove(std::string_view Key) const {
  Json &node = Node(Handle_);
  const std::string name(Key);
  if (!node.is_object() || !node.contains(name)) { return false; }
  node.erase(name);
  return true;
}

::agiru::List<std::string> JsonObject::Keys() const {
  ::agiru::List<std::string> keys;
  const Json &node = Node(Handle_);
  if (node.is_object()) {
    for (const auto &[key, value] : node.Members()) { keys.Add(key); }
  }
  return keys;
}

::agiru::List<::agiru::JsonToken> JsonObject::Values() const {
  ::agiru::List<::agiru::JsonToken> values;
  const Json &node = Node(Handle_);
  if (node.is_object()) {
    for (const auto &[key, value] : node.Members()) {
      JsonToken token;
      token.Handle_ = detail::JsonHandle{Handle_.tree, &value.Node()};
      values.Add(token);
    }
  }
  return values;
}

Boolean JsonObject::ReadFrom(std::string_view String) {
  Json parsed = Json::parse(String);
  if (parsed.is_discarded() || !parsed.is_object()) { return false; }
  Handle_ = detail::NewJsonObject();
  Node(Handle_) = std::move(parsed);
  return true;
}

Boolean JsonObject::WriteTo(::agiru::Text<0> &String) const {
  String = ::agiru::Text<0>{Node(Handle_).dump()};
  return true;
}

Integer JsonArray::Count() const {
  const Json &node = Node(Handle_);
  return node.is_array() ? static_cast<Integer>(node.size()) : 0;
}

Boolean JsonArray::Get(Integer Index, JsonToken &Result) const {
  Json &node = Node(Handle_);
  if (!WithinArray(node, Index)) { return false; }
  Result.Handle_ = detail::JsonHandle{Handle_.tree, &node[static_cast<std::size_t>(Index)]};
  return true;
}

Boolean JsonArray::RemoveAt(Integer Index) const {
  Json &node = Node(Handle_);
  if (!WithinArray(node, Index)) { return false; }
  node.erase(static_cast<std::size_t>(Index));
  return true;
}

Boolean JsonArray::ReadFrom(std::string_view String) {
  Json parsed = Json::parse(String);
  if (parsed.is_discarded() || !parsed.is_array()) { return false; }
  Handle_ = detail::NewJsonArray();
  Node(Handle_) = std::move(parsed);
  return true;
}

Boolean JsonArray::WriteTo(::agiru::Text<0> &String) const {
  String = ::agiru::Text<0>{Node(Handle_).dump()};
  return true;
}

JsonValue JsonToken::AsValue() const {
  JsonValue value;
  value.Handle_ = Handle_;
  return value;
}

JsonObject JsonToken::AsObject() const {
  JsonObject object;
  object.Handle_ = Handle_;
  return object;
}

JsonArray JsonToken::AsArray() const {
  JsonArray array;
  array.Handle_ = Handle_;
  return array;
}

Boolean JsonToken::IsValue() const {
  const Json &node = Node(Handle_);
  return !node.is_object() && !node.is_array();
}

Boolean JsonToken::IsObject() const {
  return Node(Handle_).is_object();
}

Boolean JsonToken::IsArray() const {
  return Node(Handle_).is_array();
}

Boolean JsonToken::WriteTo(::agiru::Text<0> &String) const {
  String = ::agiru::Text<0>{Node(Handle_).dump()};
  return true;
}

Boolean JsonValue::IsNull() const {
  return Node(Handle_).is_null();
}

::agiru::Text<0> JsonValue::AsText() const {
  return TextOf(Node(Handle_));
}

std::string JsonValue::AsCode() const {
  return TextOf(Node(Handle_));
}

Integer JsonValue::AsInteger() const {
  const Json &node = Node(Handle_);
  if (!node.is_number()) { throw Error("JsonValue.AsInteger requires a number"); }
  const std::int64_t value = detail::JsonInteger(node.Text());
  if (value < std::numeric_limits<Integer>::min() || value > std::numeric_limits<Integer>::max()) {
    throw Error("JSON number does not fit an Integer");
  }
  return static_cast<Integer>(value);
}

BigInteger JsonValue::AsBigInteger() const {
  const Json &node = Node(Handle_);
  if (!node.is_number() && !node.is_string()) {
    throw Error("JsonValue.AsBigInteger requires a number or string");
  }
  return detail::JsonInteger(node.Text());
}

Decimal JsonValue::AsDecimal() const {
  const Json &node = Node(Handle_);
  if (!node.is_number() && !node.is_string()) {
    throw Error("JsonValue.AsDecimal requires a number or string");
  }
  return Decimal::FromInvariantString(detail::ExactJsonDecimal(node.Text()));
}

Boolean JsonValue::AsBoolean() const {
  const Json &node = Node(Handle_);
  if (!node.is_boolean()) { throw Error("JsonValue.AsBoolean requires a Boolean"); }
  return node.Boolean();
}

namespace {

Boolean SelectIn(const detail::JsonHandle &handle, std::string_view path, JsonToken &into) {
  Json *node = detail::FindJsonPath(Node(handle), path);
  if (node == nullptr) { return false; }
  into.Handle_ = detail::JsonHandleAt(handle, *node);
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

Boolean JsonObject::SelectToken(std::string_view Path, JsonToken &Result) const {
  return SelectIn(Handle_, Path, Result);
}

Boolean JsonArray::SelectToken(std::string_view Path, JsonToken &Result) const {
  return SelectIn(Handle_, Path, Result);
}

::agiru::Text<0> JsonObject::GetText(std::string_view Key, Boolean DefaultIfNotFound) const {
  bool found = false;
  const JsonToken token = FoundAt(Handle_, Key, found);
  if (!found) {
    if (!DefaultIfNotFound) { NoSuchKey(Key); }
    return {};
  }
  return token.AsValue().AsText();
}

Integer JsonObject::GetInteger(std::string_view Key, Boolean DefaultIfNotFound) const {
  bool found = false;
  const JsonToken token = FoundAt(Handle_, Key, found);
  if (!found) {
    if (!DefaultIfNotFound) { NoSuchKey(Key); }
    return 0;
  }
  return token.AsValue().AsInteger();
}

Decimal JsonObject::GetDecimal(std::string_view Key, Boolean DefaultIfNotFound) const {
  bool found = false;
  const JsonToken token = FoundAt(Handle_, Key, found);
  if (!found) {
    if (!DefaultIfNotFound) { NoSuchKey(Key); }
    return Decimal{};
  }
  return token.AsValue().AsDecimal();
}

Boolean JsonObject::GetBoolean(std::string_view Key, Boolean DefaultIfNotFound) const {
  bool found = false;
  const JsonToken token = FoundAt(Handle_, Key, found);
  if (!found) {
    if (!DefaultIfNotFound) { NoSuchKey(Key); }
    return false;
  }
  return token.AsValue().AsBoolean();
}

JsonObject JsonObject::GetObject(std::string_view Key, Boolean DefaultIfNotFound) const {
  bool found = false;
  const JsonToken token = FoundAt(Handle_, Key, found);
  if (!found) {
    if (!DefaultIfNotFound) { NoSuchKey(Key); }
    JsonObject empty;
    empty.Handle_ = detail::NewJsonObject();
    return empty;
  }
  return token.AsObject();
}

JsonArray JsonObject::GetArray(std::string_view Key, Boolean DefaultIfNotFound) const {
  bool found = false;
  const JsonToken token = FoundAt(Handle_, Key, found);
  if (!found) {
    if (!DefaultIfNotFound) { NoSuchKey(Key); }
    JsonArray empty;
    empty.Handle_ = detail::NewJsonArray();
    return empty;
  }
  return token.AsArray();
}
}
