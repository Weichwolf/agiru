#include "dotnet/JObject.h"

#include "dotnet/Generic.h"
#include "runtime/Error.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Decimal.h"
#include "type/Integer.h"
#include "type/JsonHandle.h"
#include "type/Text.h"
#include "type/Variant.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "JsonEngine.h"

namespace agiru::dotnet {

namespace {

using Json = nlohmann::json;

constexpr int kIndent = 2;

std::string Indented(const Json &node) {
  std::string text = node.dump(kIndent);
  std::string out;
  out.reserve(text.size() + (text.size() / 16));
  for (const char c : text) {
    if (c == '\n') { out += '\r'; }
    out += c;
  }
  return out;
}

std::string LeafText(const Json &node) {
  if (node.is_string()) { return node.get<std::string>(); }
  if (node.is_boolean()) { return node.get<bool>() ? "True" : "False"; }
  if (node.is_null()) { return {}; }
  if (node.is_number_integer()) { return std::to_string(node.get<std::int64_t>()); }
  if (node.is_number_unsigned()) { return std::to_string(node.get<std::uint64_t>()); }
  if (node.is_number_float()) {
    return Decimal::FromInvariantString(node.dump()).Trimmed().ToInvariantString();
  }
  return Indented(node);
}

Json FromVariant(const ::agiru::Variant &value) {
  if (value.IsEmpty()) { return Json(); }
  if (const JsonInVariant *held = value.JsonHeld(); held != nullptr) {
    if (held->handle.Empty()) { return Json(); }
    return ::agiru::detail::JsonNodeOf(held->handle);
  }
  if (value.Is<Boolean>()) { return Json(static_cast<bool>(value.Get<Boolean>())); }
  if (value.Is<Integer>()) { return Json(value.Get<Integer>()); }
  if (value.Is<BigInteger>()) { return Json(value.Get<BigInteger>()); }
  if (value.Is<Decimal>()) {
    return Json::parse(value.Get<Decimal>().Trimmed().ToInvariantString());
  }
  if (value.Is<std::string>()) { return Json(value.Get<std::string>()); }
  throw Error(std::string("a JSON value cannot be made of a Variant holding ") +
              std::string(value.HeldName()));
}

JToken TokenAt(const ::agiru::detail::JsonHandle &tree, Json &node) {
  return JToken{::agiru::detail::JsonHandleAt(tree, node)};
}

::agiru::Variant Carried(const JToken &token) {
  return ::agiru::Variant(token);
}

bool FilterMatches(const Json &item, std::string_view name, std::string_view wanted) {
  if (!item.is_object() || !item.contains(std::string(name))) { return false; }
  return LeafText(item[std::string(name)]) == wanted;
}

std::vector<::agiru::Variant> Selected(const ::agiru::detail::JsonHandle &handle,
                                       std::string_view path) {
  std::vector<::agiru::Variant> found;
  if (handle.Empty()) { return found; }
  Json &node = ::agiru::detail::JsonNodeOf(handle);
  std::string_view rest = path;
  if (rest.starts_with("$")) { rest.remove_prefix(1); }
  const std::string_view filter = "[?(@.";
  if (rest.starts_with(filter)) {
    const std::size_t equals = rest.find("==");
    const std::size_t close = rest.find(")]");
    if (equals == std::string_view::npos || close == std::string_view::npos || !node.is_array()) {
      return found;
    }
    std::string_view name = rest.substr(filter.size(), equals - filter.size());
    while (!name.empty() && name.back() == ' ') { name.remove_suffix(1); }
    std::string_view wanted = rest.substr(equals + 2, close - equals - 2);
    while (!wanted.empty() && wanted.front() == ' ') { wanted.remove_prefix(1); }
    while (!wanted.empty() && wanted.back() == ' ') { wanted.remove_suffix(1); }
    if (wanted.size() >= 2 && wanted.front() == '\'' && wanted.back() == '\'') {
      wanted = wanted.substr(1, wanted.size() - 2);
    }
    for (Json &item : node) {
      if (FilterMatches(item, name, wanted)) { found.push_back(Carried(TokenAt(handle, item))); }
    }
    return found;
  }
  const JToken one = JToken{handle}.SelectToken(path);
  if (!one.IsNullObject()) { found.push_back(Carried(one)); }
  return found;
}

}

JToken::JToken(const ::agiru::Variant &value) : handle_(HandleOf(value)) {}

::agiru::detail::JsonHandle JToken::HandleOf(const ::agiru::Variant &value) {
  if (value.IsEmpty()) { return {}; }
  const JsonInVariant *held = value.JsonHeld();
  if (held == nullptr) {
    throw Error(std::string("a JSON token cannot be read out of a Variant holding ") +
                value.HeldName());
  }
  return held->handle;
}

JToken JToken::Parse(std::string_view json) const {
  Json parsed = Json::parse(json, nullptr, false);
  if (parsed.is_discarded()) { throw Error("JToken.Parse: the text is not JSON"); }
  return JToken{::agiru::detail::JsonHandleMade(std::move(parsed))};
}

::agiru::Text<0> JToken::ToString() const {
  if (handle_.Empty()) { return {}; }
  const Json &node = ::agiru::detail::JsonNodeOf(handle_);
  return ::agiru::Text<0>{node.is_structured() ? Indented(node) : LeafText(node)};
}

JToken JToken::DeepClone() const {
  if (handle_.Empty()) { return {}; }
  return JToken{::agiru::detail::JsonHandleMade(::agiru::detail::JsonNodeOf(handle_))};
}

JToken JToken::Root() const {
  if (handle_.Empty()) { return {}; }
  return JToken{::agiru::detail::JsonHandle{handle_.tree, &handle_.tree->root}};
}

::agiru::Text<0> JToken::Path() const {
  return {};
}

::agiru::Boolean JToken::HasValues() const {
  if (handle_.Empty()) { return false; }
  const Json &node = ::agiru::detail::JsonNodeOf(handle_);
  return node.is_structured() && !node.empty();
}

JToken JToken::SelectToken(std::string_view path) const {
  if (handle_.Empty()) { return {}; }
  Json *node = &::agiru::detail::JsonNodeOf(handle_);
  std::string_view rest = path;
  if (rest.starts_with("$")) { rest.remove_prefix(1); }
  while (!rest.empty()) {
    if (rest.front() == '.') {
      rest.remove_prefix(1);
      continue;
    }
    if (rest.front() == '[') {
      const std::size_t close = rest.find(']');
      if (close == std::string_view::npos || !node->is_array()) { return {}; }
      std::size_t at = 0;
      for (const char c : rest.substr(1, close - 1)) {
        if (c < '0' || c > '9') { return {}; }
        at = (at * 10) + static_cast<std::size_t>(c - '0');
      }
      if (at >= node->size()) { return {}; }
      node = &(*node)[at];
      rest.remove_prefix(close + 1);
      continue;
    }
    const std::size_t next = rest.find_first_of(".[");
    const std::string name(rest.substr(0, next));
    if (!node->is_object() || !node->contains(name)) { return {}; }
    node = &(*node)[name];
    if (next == std::string_view::npos) { break; }
    rest.remove_prefix(next);
  }
  return TokenAt(handle_, *node);
}

GenericIEnumerable1 JToken::SelectTokens(std::string_view path,
                                         ::agiru::Boolean errorWhenNoMatch) const {
  std::vector<::agiru::Variant> found = Selected(handle_, path);
  if (found.empty() && errorWhenNoMatch) {
    throw Error("JToken.SelectTokens: no token matches " + std::string(path));
  }
  return GenericIEnumerable1{std::move(found)};
}

::agiru::Variant JToken::Value() const {
  if (handle_.Empty()) { return {}; }
  const Json &node = ::agiru::detail::JsonNodeOf(handle_);
  if (node.is_string()) { return ::agiru::Variant(node.get<std::string>()); }
  if (node.is_boolean()) { return ::agiru::Variant(Boolean{node.get<bool>()}); }
  if (node.is_number_integer() || node.is_number_unsigned()) {
    const auto whole = node.get<std::int64_t>();
    if (whole >= std::numeric_limits<Integer>::min() &&
        whole <= std::numeric_limits<Integer>::max()) {
      return ::agiru::Variant(static_cast<Integer>(whole));
    }
    return ::agiru::Variant(static_cast<BigInteger>(whole));
  }
  if (node.is_number_float()) { return ::agiru::Variant(Decimal::FromInvariantString(node.dump())); }
  if (node.is_null()) { return {}; }
  return Carried(*this);
}

::agiru::Text<0> JToken::Type() const {
  if (handle_.Empty()) { return ::agiru::Text<0>{"None"}; }
  const Json &node = ::agiru::detail::JsonNodeOf(handle_);
  if (node.is_object()) { return ::agiru::Text<0>{"Object"}; }
  if (node.is_array()) { return ::agiru::Text<0>{"Array"}; }
  if (node.is_string()) { return ::agiru::Text<0>{"String"}; }
  if (node.is_boolean()) { return ::agiru::Text<0>{"Boolean"}; }
  if (node.is_number_float()) { return ::agiru::Text<0>{"Float"}; }
  if (node.is_number()) { return ::agiru::Text<0>{"Integer"}; }
  return ::agiru::Text<0>{"Null"};
}

::agiru::Integer JToken::Count() const {
  if (handle_.Empty()) { return 0; }
  const Json &node = ::agiru::detail::JsonNodeOf(handle_);
  return node.is_structured() ? static_cast<Integer>(node.size()) : 0;
}

JToken JToken::Item(const ::agiru::Variant &key) const {
  if (handle_.Empty()) { return {}; }
  Json &node = ::agiru::detail::JsonNodeOf(handle_);
  if (node.is_array()) {
    const auto at = static_cast<std::size_t>(key.Is<Integer>() ? key.Get<Integer>() : 0);
    if (at >= node.size()) { throw Error("JArray.Item: the index is outside the array"); }
    return TokenAt(handle_, node[at]);
  }
  if (node.is_object() && key.Is<std::string>()) {
    const std::string &name = key.Get<std::string>();
    if (!node.contains(name)) { return {}; }
    return TokenAt(handle_, node[name]);
  }
  return {};
}

GenericIEnumerator1 JToken::GetEnumerator() const {
  std::vector<::agiru::Variant> items;
  if (!handle_.Empty()) {
    Json &node = ::agiru::detail::JsonNodeOf(handle_);
    if (node.is_array()) {
      for (Json &item : node) { items.push_back(Carried(TokenAt(handle_, item))); }
    } else if (node.is_object()) {
      for (auto &[name, value] : node.items()) {
        items.push_back(::agiru::Variant(JProperty::Of(handle_, name)));
      }
    }
  }
  return GenericIEnumerator1{std::move(items)};
}

void JToken::Replace(const JToken &token) {
  if (handle_.Empty()) { throw Error("JToken.Replace: the token refers to nothing"); }
  ::agiru::detail::JsonNodeOf(handle_) =
      token.IsNullObject() ? Json() : ::agiru::detail::JsonNodeOf(token.Handle());
}

JToken::operator JObject() const {
  return JObject::Over(handle_);
}

JToken::operator JArray() const {
  return JArray::Over(handle_);
}

JToken::operator JValue() const {
  return JValue::Over(handle_);
}

JToken::operator JProperty() const {
  class JProperty made;
  made = *this;
  return made;
}

JsonChildIterator JToken::begin() const {
  return JsonChildIterator{handle_, 0};
}

JsonChildIterator JToken::end() const {
  if (handle_.Empty()) { return JsonChildIterator{handle_, 0}; }
  const Json &node = ::agiru::detail::JsonNodeOf(handle_);
  return JsonChildIterator{handle_, node.is_structured() ? node.size() : 0};
}

JObject &JsonChildIterator::operator*() {
  Json &node = ::agiru::detail::JsonNodeOf(handle_);
  std::size_t seen = 0;
  for (Json &child : node) {
    if (seen == at_) {
      current_ = JObject::Over(::agiru::detail::JsonHandleAt(handle_, child));
      return current_;
    }
    ++seen;
  }
  throw Error("the JSON child the iterator stands on is gone");
}

JValue JValue::Binder::operator()(const ::agiru::Variant &value) const {
  return JValue::Over(::agiru::detail::JsonHandleMade(FromVariant(value)));
}

JValue JValue::Over(::agiru::detail::JsonHandle handle) {
  class JValue made;
  made.handle_ = std::move(handle);
  return made;
}

class JValue &JValue::operator=(const JToken &token) {
  handle_ = token.Handle();
  return *this;
}

JValue JValue::CreateNull() const {
  return Over(::agiru::detail::JsonHandleMade(Json()));
}

JProperty JProperty::Binder::operator()(std::string_view name,
                                        const ::agiru::Variant &value) const {
  Json object = Json::object();
  object[std::string(name)] = FromVariant(value);
  const ::agiru::detail::JsonHandle owner = ::agiru::detail::JsonHandleMade(std::move(object));
  return JProperty::Of(owner, std::string(name));
}

JProperty JProperty::Of(const ::agiru::detail::JsonHandle &owner, std::string name) {
  class JProperty made;
  made.owner_ = owner;
  made.name_ = std::move(name);
  Json &object = ::agiru::detail::JsonNodeOf(owner);
  if (object.is_object() && object.contains(made.name_)) {
    made.handle_ = ::agiru::detail::JsonHandleAt(owner, object[made.name_]);
  }
  return made;
}

class JProperty &JProperty::operator=(const JToken &token) {
  handle_ = token.Handle();
  owner_ = {};
  name_.clear();
  return *this;
}

class JProperty &JProperty::operator=(const ::agiru::Variant &value) {
  if (const JsonInVariant *held = value.JsonHeld();
      held != nullptr && held->kind == ::agiru::detail::JsonKind::Property) {
    owner_ = held->owner;
    name_ = held->name;
    handle_ = held->handle;
    return *this;
  }
  return *this = JToken(value);
}

JToken JProperty::Value() const {
  return JToken{handle_};
}

JToken JProperty::Value(const JToken &token) {
  if (handle_.Empty()) {
    if (owner_.Empty() || name_.empty()) {
      throw Error("JProperty.Value: the property refers to nothing");
    }
    Json &object = ::agiru::detail::JsonNodeOf(owner_);
    object[name_] = token.IsNullObject() ? Json() : ::agiru::detail::JsonNodeOf(token.Handle());
    handle_ = ::agiru::detail::JsonHandleAt(owner_, object[name_]);
    return Value();
  }
  JToken{handle_}.Replace(token);
  return Value();
}

::agiru::Text<0> JProperty::ToString() const {
  if (name_.empty()) { return JToken::ToString(); }
  const std::string value =
      handle_.Empty() ? std::string("null") : Indented(::agiru::detail::JsonNodeOf(handle_));
  return ::agiru::Text<0>{Json(name_).dump() + ": " + value};
}

void JProperty::Replace(const class JProperty &property) {
  if (owner_.Empty()) { throw Error("JProperty.Replace: the property is in no object"); }
  Json &object = ::agiru::detail::JsonNodeOf(owner_);
  if (!object.is_object()) { throw Error("JProperty.Replace: the owner is not an object"); }
  Json value = property.Value().IsNullObject()
                   ? Json()
                   : Json(::agiru::detail::JsonNodeOf(property.Value().Handle()));
  const std::string newName = property.name_.empty() ? name_ : property.name_;
  if (newName == name_) {
    object[name_] = std::move(value);
  } else {
    Json rebuilt = Json::object();
    for (auto &[key, held] : object.items()) {
      if (key == name_) {
        rebuilt[newName] = std::move(value);
      } else {
        rebuilt[key] = held;
      }
    }
    object = std::move(rebuilt);
    name_ = newName;
  }
  handle_ = ::agiru::detail::JsonHandleAt(owner_, object[name_]);
}

JObject JObject::Binder::operator()() const {
  return JObject::Over(::agiru::detail::NewJsonObject());
}

JObject JObject::Over(::agiru::detail::JsonHandle handle) {
  class JObject made;
  made.handle_ = std::move(handle);
  return made;
}

class JObject &JObject::operator=(const JToken &token) {
  handle_ = token.Handle();
  return *this;
}

class JObject &JObject::operator=(const ::agiru::Variant &value) {
  handle_ = HandleOf(value);
  return *this;
}

JObject JObject::Parse(std::string_view json) const {
  Json parsed = Json::parse(json, nullptr, false);
  if (parsed.is_discarded() || !parsed.is_object()) {
    throw Error("JObject.Parse: the text is not a JSON object");
  }
  return Over(::agiru::detail::JsonHandleMade(std::move(parsed)));
}

void JObject::Add(std::string_view name, const ::agiru::Variant &value) {
  if (handle_.Empty()) { throw Error("JObject.Add: the object refers to nothing"); }
  Json &object = ::agiru::detail::JsonNodeOf(handle_);
  if (!object.is_object()) { throw Error("JObject.Add: the node is not an object"); }
  const std::string key(name);
  if (object.contains(key)) {
    throw Error("Can not add property " + key + " to Newtonsoft.Json.Linq.JObject. Property with the same name already exists on object.");
  }
  object[key] = FromVariant(value);
}

void JObject::Add(const JProperty &property) {
  Add(std::string_view(property.Name()), property.Value().IsNullObject()
                                              ? ::agiru::Variant{}
                                              : Carried(property.Value()));
}

JProperty JObject::Property(std::string_view name) const {
  if (handle_.Empty()) { return {}; }
  Json &object = ::agiru::detail::JsonNodeOf(handle_);
  if (!object.is_object() || !object.contains(std::string(name))) { return {}; }
  return JProperty::Of(handle_, std::string(name));
}

::agiru::Boolean JObject::TryGetValue(std::string_view name, JToken &token) const {
  if (handle_.Empty()) { return false; }
  Json &object = ::agiru::detail::JsonNodeOf(handle_);
  const std::string key(name);
  if (!object.is_object() || !object.contains(key)) { return false; }
  token = TokenAt(handle_, object[key]);
  return true;
}

GenericIEnumerable1 JObject::Properties() const {
  std::vector<::agiru::Variant> items;
  if (!handle_.Empty()) {
    Json &object = ::agiru::detail::JsonNodeOf(handle_);
    if (object.is_object()) {
      for (auto &[name, value] : object.items()) {
        items.push_back(::agiru::Variant(JProperty::Of(handle_, name)));
      }
    }
  }
  return GenericIEnumerable1{std::move(items)};
}

::agiru::Boolean JObject::Remove(std::string_view name) {
  if (handle_.Empty()) { return false; }
  Json &object = ::agiru::detail::JsonNodeOf(handle_);
  if (!object.is_object()) { return false; }
  return object.erase(std::string(name)) != 0;
}

JArray JArray::Binder::operator()() const {
  return JArray::Over(::agiru::detail::NewJsonArray());
}

JArray JArray::Over(::agiru::detail::JsonHandle handle) {
  class JArray made;
  made.handle_ = std::move(handle);
  return made;
}

class JArray &JArray::operator=(const JToken &token) {
  handle_ = token.Handle();
  return *this;
}

class JArray &JArray::operator=(const ::agiru::Variant &value) {
  handle_ = HandleOf(value);
  return *this;
}

JArray JArray::Parse(std::string_view json) const {
  Json parsed = Json::parse(json, nullptr, false);
  if (parsed.is_discarded() || !parsed.is_array()) {
    throw Error("JArray.Parse: the text is not a JSON array");
  }
  return Over(::agiru::detail::JsonHandleMade(std::move(parsed)));
}

void JArray::Add(const ::agiru::Variant &value) {
  if (handle_.Empty()) { throw Error("JArray.Add: the array refers to nothing"); }
  Json &array = ::agiru::detail::JsonNodeOf(handle_);
  if (!array.is_array()) { throw Error("JArray.Add: the node is not an array"); }
  array.push_back(FromVariant(value));
}

void JArray::Merge(const JToken &other) {
  if (handle_.Empty() || other.IsNullObject()) { return; }
  Json &array = ::agiru::detail::JsonNodeOf(handle_);
  const Json &more = ::agiru::detail::JsonNodeOf(other.Handle());
  if (!array.is_array() || !more.is_array()) { return; }
  for (const Json &item : more) { array.push_back(item); }
}

void JArray::Insert(::agiru::Integer index, const ::agiru::Variant &value) {
  if (handle_.Empty()) { throw Error("JArray.Insert: the array refers to nothing"); }
  Json &array = ::agiru::detail::JsonNodeOf(handle_);
  if (!array.is_array() || index < 0 || static_cast<std::size_t>(index) > array.size()) {
    throw Error("JArray.Insert: the index is outside the array");
  }
  array.insert(array.begin() + index, FromVariant(value));
}

void JArray::RemoveAt(::agiru::Integer index) {
  if (handle_.Empty()) { throw Error("JArray.RemoveAt: the array refers to nothing"); }
  Json &array = ::agiru::detail::JsonNodeOf(handle_);
  if (!array.is_array() || index < 0 || static_cast<std::size_t>(index) >= array.size()) {
    throw Error("JArray.RemoveAt: the index is outside the array");
  }
  array.erase(array.begin() + index);
}

}
