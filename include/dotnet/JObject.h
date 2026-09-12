#pragma once

#include "dotnet/Generic.h"
#include "dotnet/Refused.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/JsonHandle.h"
#include "type/Text.h"
#include "type/Variant.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

/// \file
/// \brief Newtonsoft's `JObject` family, REBUILT on the JSON tree the AL `JsonObject` types hold.
///
/// `codeunit 5459 "JSON Management"` is the BaseApp's hub for these (235 declarations of `JObject`
/// alone, board:0035, board:0714): a token is a REFERENCE into a tree, the way Newtonsoft's is,
/// and every class here is a view of one node -- `JObject` an object, `JArray` an array, `JValue`
/// a leaf, `JProperty` a name inside an object, `JToken` any of them. A copy of a variable is a
/// second reference to the same node; `DeepClone` is the copy that is not.

namespace agiru::dotnet {

class JArray;
class JObject;
class JProperty;
class JValue;
class JsonChildIterator;

/// \brief How a `JToken` is seen, for the `Variant` that carries one.
using JsonKind = ::agiru::detail::JsonKind;

/// \brief Newtonsoft `JToken`: a reference to one node of a JSON tree, or to nothing.
class JToken {
public:
  /// \brief The binder behind `JToken.Parse(text)`; a `JToken` is never constructed by AL itself.
  struct Binder {};

  /// \brief The kind this class stands for. \see JsonKind
  static constexpr JsonKind kJsonKind = JsonKind::Token;

  /// \brief A null reference.
  JToken() = default;

  /// \brief A token over a node. \param handle The node.
  explicit JToken(::agiru::detail::JsonHandle handle) noexcept : handle_(std::move(handle)) {}

  /// \brief A token over what a Variant carries (`JToken := Variant`).
  /// \param value The Variant, which holds a JSON node or refuses.
  explicit(false) JToken(const ::agiru::Variant &value);

  /// \brief The node a Variant carries, for the classes that cannot construct.
  /// \param value The Variant. \return The node; empty for an empty Variant.
  /// \throws Error when the Variant holds something that is not JSON.
  [[nodiscard]] static ::agiru::detail::JsonHandle HandleOf(const ::agiru::Variant &value);

  /// \brief The node behind this reference. \return It; empty for a null reference.
  [[nodiscard]] const ::agiru::detail::JsonHandle &Handle() const noexcept { return handle_; }

  /// \brief AL `IsNull(JToken)`. \return Whether this refers to nothing.
  [[nodiscard]] bool IsNullObject() const noexcept { return handle_.Empty(); }

  /// \brief `JToken.Parse(json)`: a new tree read from text. \param json The text.
  /// \return The root. \throws Error when the text is not JSON.
  [[nodiscard]] JToken Parse(std::string_view json) const;

  /// \brief `JToken.ToString()`: an object or array as INDENTED JSON, a string as its own text,
  ///        a number or boolean as .NET renders it, null as the empty string.
  /// \return The text.
  [[nodiscard]] ::agiru::Text<0> ToString() const;

  /// \brief `JToken.DeepClone()`: a copy of the subtree, in a tree of its own. \return It.
  [[nodiscard]] JToken DeepClone() const;

  /// \brief `JToken.Root`: the root of the tree this node lives in. \return It.
  [[nodiscard]] JToken Root() const;

  /// \brief `JToken.Path`: the JSONPath of this node. \return It.
  [[nodiscard]] ::agiru::Text<0> Path() const;

  /// \brief `JToken.HasValues`: whether an object or array holds anything. \return It.
  [[nodiscard]] ::agiru::Boolean HasValues() const;

  /// \brief `JToken.SelectToken(path)`: the node a JSONPath names, or null.
  /// \param path The path, with or without the leading `$`. \return The node.
  [[nodiscard]] JToken SelectToken(std::string_view path) const;

  /// \brief `JToken.SelectTokens(path [, errorWhenNoMatch])`: every node a JSONPath names.
  /// \param path The path. \param errorWhenNoMatch Whether an empty answer raises. \return Them.
  [[nodiscard]] GenericIEnumerable1 SelectTokens(std::string_view path,
                                                 ::agiru::Boolean errorWhenNoMatch = {}) const;

  /// \brief `JToken.Value`: what a leaf holds, as the Variant AL reads it -- a string, a number,
  ///        a boolean; an object or array as itself.
  /// \return The value.
  [[nodiscard]] ::agiru::Variant Value() const;

  /// \brief `JToken.Type`: the token's kind by name -- `Object`, `Array`, `Property`, `String`,
  ///        `Integer`, `Float`, `Boolean`, `Null`. \return The name.
  [[nodiscard]] ::agiru::Text<0> Type() const;

  /// \brief `JToken.Count`: the members of an object or the items of an array. \return It.
  [[nodiscard]] ::agiru::Integer Count() const;

  /// \brief `JToken.Item(index)` / `JToken.Item(name)`: a child by position or by name.
  /// \param key The index or the name. \return The child, or null.
  [[nodiscard]] JToken Item(const ::agiru::Variant &key) const;

  /// \brief `JToken.GetEnumerator()`: the children, one Variant each. \return The enumerator.
  [[nodiscard]] GenericIEnumerator1 GetEnumerator() const;

  /// \brief `JToken.Replace(token)`: this node's content becomes the other's. \param token It.
  void Replace(const JToken &token);

  /// \brief `JToken.Remove()`: not carried yet.
  ::agiru::dotnet::Refused Remove{{.type = "JToken", .member = "Remove"}};

  /// \brief AL `JObject := JToken` handed as an ARGUMENT: the same node, seen as an object.
  /// \return The object.
  [[nodiscard]] operator JObject() const; // NOLINT(*-explicit-constructor)
  /// \brief The same node, seen as an array. \return The array.
  [[nodiscard]] operator JArray() const; // NOLINT(*-explicit-constructor)
  /// \brief The same node, seen as a leaf. \return The leaf.
  [[nodiscard]] operator JValue() const; // NOLINT(*-explicit-constructor)
  /// \brief The same node, seen as a nameless property. \return The property.
  [[nodiscard]] operator JProperty() const; // NOLINT(*-explicit-constructor)

  /// \brief AL `foreach Item in Token`: the first child. \return The iterator.
  [[nodiscard]] JsonChildIterator begin() const;
  /// \brief AL `foreach Item in Token`: past the last child. \return The iterator.
  [[nodiscard]] JsonChildIterator end() const;

protected:
  ::agiru::detail::JsonHandle handle_; ///< The node.
};

/// \brief Newtonsoft `JValue`: a leaf -- a string, a number, a boolean, a date or null.
class JValue : public JToken {
public:
  /// \brief The binder behind `JValue.JValue(value)`.
  struct Binder {
    /// \brief A leaf holding a value. \param value The value. \return The leaf.
    [[nodiscard]] class JValue operator()(const ::agiru::Variant &value) const;
  };

  static constexpr JsonKind kJsonKind = JsonKind::Value; ///< \see JsonKind

  /// \brief The constructor, spelled the way AL spells it: `JValue := JValue.JValue(x)`. THE
  ///        CLASS DECLARES NO CONSTRUCTOR OF ITS OWN, because C++ refuses a member named like
  ///        its class beside one; `Over` is the factory instead.
  Binder JValue{}; // NOLINT(misc-non-private-member-variables-in-classes)

  /// \brief A leaf over a node. \param handle The node. \return The leaf.
  [[nodiscard]] static class JValue Over(::agiru::detail::JsonHandle handle);

  /// \brief `JValue := <absent .NET member>`: the member refuses, the way it does everywhere.
  /// \tparam R The refusal. \param refused The member. \return Never.
  template <typename R>
    requires requires { typename R::IsAlRefusal; }
  class JValue &operator=(const R &refused) {
    static_cast<void>(static_cast<::agiru::Integer>(refused));
    return *this;
  }

  /// \brief `JValue := JToken`: the same node, seen as a leaf. \param token The token.
  /// \return This.
  class JValue &operator=(const JToken &token);

  /// \brief `JValue.CreateNull()`: a null leaf. \return It.
  [[nodiscard]] class JValue CreateNull() const;
};

/// \brief Newtonsoft `JProperty`: a name inside an object, with the value it names.
class JProperty : public JToken {
public:
  /// \brief The binder behind `JProperty.JProperty(name, value)`.
  struct Binder {
    /// \brief A property not yet in any object. \param name Its name. \param value Its value.
    /// \return The property.
    [[nodiscard]] class JProperty operator()(std::string_view name,
                                             const ::agiru::Variant &value) const;
  };

  static constexpr JsonKind kJsonKind = JsonKind::Property; ///< \see JsonKind

  /// \brief The constructor, spelled the way AL spells it; the class declares no constructor of
  ///        its own for the reason `JValue` gives.
  Binder JProperty{}; // NOLINT(misc-non-private-member-variables-in-classes)

  /// \brief A property of an object. \param owner The object. \param name The member's name.
  /// \return The property, whose node is the member's value.
  [[nodiscard]] static class JProperty Of(const ::agiru::detail::JsonHandle &owner,
                                          std::string name);

  /// \brief `JProperty := <absent .NET member>`: the member refuses, the way it does everywhere.
  /// \tparam R The refusal. \param refused The member. \return Never.
  template <typename R>
    requires requires { typename R::IsAlRefusal; }
  class JProperty &operator=(const R &refused) {
    static_cast<void>(static_cast<::agiru::Integer>(refused));
    return *this;
  }

  /// \brief `JProperty := JToken`: a property standing on a value token with no name, which is
  ///        what `SelectToken` hands a `JProperty` variable; its `Value` is then the token itself.
  /// \param token The token. \return This.
  class JProperty &operator=(const JToken &token);

  /// \brief `JProperty := Variant`. \param value The Variant. \return This.
  class JProperty &operator=(const ::agiru::Variant &value);

  /// \brief `JProperty.Name`. \return The name.
  [[nodiscard]] ::agiru::Text<0> Name() const { return name_; }

  /// \brief `JProperty.Value`: the value token, live in its object. \return It.
  [[nodiscard]] JToken Value() const;

  /// \brief `JProperty.Value := token`: the member takes the token's content.
  /// \param token The new value. \return The value.
  JToken Value(const JToken &token);

  /// \brief `JProperty.ToString()`: `"name": value`. \return The text.
  [[nodiscard]] ::agiru::Text<0> ToString() const;

  /// \brief `JProperty.Replace(property)`: in the owning object, this name goes and the other
  ///        property's name and value take its place. \param property The replacement.
  void Replace(const class JProperty &property);

  /// \brief The object this property lives in. \return Its node; empty when detached.
  [[nodiscard]] const ::agiru::detail::JsonHandle &Owner() const noexcept { return owner_; }

private:
  ::agiru::detail::JsonHandle owner_;
  std::string name_;
};

/// \brief Newtonsoft `JObject`: an object node.
class JObject : public JToken {
public:
  /// \brief The binder behind `JObject.JObject()`.
  struct Binder {
    /// \brief A new, empty object. \return It.
    [[nodiscard]] class JObject operator()() const;
  };

  static constexpr JsonKind kJsonKind = JsonKind::Object; ///< \see JsonKind

  /// \brief The constructor, spelled the way AL spells it; the class declares no constructor of
  ///        its own for the reason `JValue` gives.
  Binder JObject{}; // NOLINT(misc-non-private-member-variables-in-classes)

  /// \brief An object over a node. \param handle The node. \return The object.
  [[nodiscard]] static class JObject Over(::agiru::detail::JsonHandle handle);

  /// \brief `JObject := <absent .NET member>`: the member refuses, the way it does everywhere.
  /// \tparam R The refusal. \param refused The member. \return Never.
  template <typename R>
    requires requires { typename R::IsAlRefusal; }
  class JObject &operator=(const R &refused) {
    static_cast<void>(static_cast<::agiru::Integer>(refused));
    return *this;
  }

  /// \brief `JObject := JToken` (`JObject := JArray.Item(i)`): the same node, seen as an object.
  /// \param token The token. \return This.
  class JObject &operator=(const JToken &token);

  /// \brief `JObject := Variant` (an enumerator's `Current`). \param value The Variant.
  /// \return This.
  class JObject &operator=(const ::agiru::Variant &value);

  /// \brief `JObject.Parse(json)`: a new object read from text. \param json The text.
  /// \return It. \throws Error when the text is not a JSON object.
  [[nodiscard]] class JObject Parse(std::string_view json) const;

  /// \brief `JObject.Add(name, value)`: a member; an existing name raises, as Newtonsoft does.
  /// \param name The name. \param value The value -- a scalar, a token, an object, an array.
  void Add(std::string_view name, const ::agiru::Variant &value);

  /// \brief `JObject.Add(property)`: the property's name and value become a member.
  /// \param property The property.
  void Add(const JProperty &property);

  /// \brief `JObject.Property(name)`: the member as a property, or null. \param name The name.
  /// \return The property.
  [[nodiscard]] class JProperty Property(std::string_view name) const;

  /// \brief `JObject.TryGetValue(name, token)`: the member's value when there is one.
  /// \param name The name. \param token Where the value lands. \return Whether it was there.
  ::agiru::Boolean TryGetValue(std::string_view name, JToken &token) const;

  /// \brief `JObject.Properties()`: every member as a `JProperty`, in document order.
  /// \return The enumerable.
  [[nodiscard]] GenericIEnumerable1 Properties() const;

  /// \brief `JObject.Remove(name)`: the member goes. \param name The name.
  /// \return Whether it was there.
  ::agiru::Boolean Remove(std::string_view name);
};

/// \brief Newtonsoft `JArray`: an array node.
class JArray : public JToken {
public:
  /// \brief The binder behind `JArray.JArray()`.
  struct Binder {
    /// \brief A new, empty array. \return It.
    [[nodiscard]] class JArray operator()() const;
  };

  static constexpr JsonKind kJsonKind = JsonKind::Array; ///< \see JsonKind

  /// \brief The constructor, spelled the way AL spells it; the class declares no constructor of
  ///        its own for the reason `JValue` gives.
  Binder JArray{}; // NOLINT(misc-non-private-member-variables-in-classes)

  /// \brief An array over a node. \param handle The node. \return The array.
  [[nodiscard]] static class JArray Over(::agiru::detail::JsonHandle handle);

  /// \brief `JArray := <absent .NET member>`: the member refuses, the way it does everywhere.
  /// \tparam R The refusal. \param refused The member. \return Never.
  template <typename R>
    requires requires { typename R::IsAlRefusal; }
  class JArray &operator=(const R &refused) {
    static_cast<void>(static_cast<::agiru::Integer>(refused));
    return *this;
  }

  /// \brief `JArray := JToken`. \param token The token. \return This.
  class JArray &operator=(const JToken &token);

  /// \brief `JArray := Variant`. \param value The Variant. \return This.
  class JArray &operator=(const ::agiru::Variant &value);

  /// \brief `JArray.Parse(json)`: a new array read from text. \param json The text. \return It.
  [[nodiscard]] class JArray Parse(std::string_view json) const;

  /// \brief `JArray.Add(value)`: appends. \param value A scalar, a token, an object, an array.
  void Add(const ::agiru::Variant &value);

  /// \brief `JArray.Merge(array)`: the other's items are appended. \param other The other.
  void Merge(const JToken &other);

  /// \brief `JArray.Insert(index, value)`. \param index Where. \param value What.
  void Insert(::agiru::Integer index, const ::agiru::Variant &value);

  /// \brief `JArray.RemoveAt(index)`. \param index Which.
  void RemoveAt(::agiru::Integer index);
};

/// \brief What `foreach Item in Token` walks: an array's items, or an object's member values,
///        each seen as a `JObject` (a leaf item is an object with nothing in it).
class JsonChildIterator {
public:
  /// \brief An iterator standing on one child. \param handle The parent. \param at Which child.
  JsonChildIterator(::agiru::detail::JsonHandle handle, std::size_t at)
      : handle_(std::move(handle)), at_(at) {}

  /// \brief The child the iterator stands on. \return It, as an object.
  [[nodiscard]] JObject &operator*();

  /// \brief The next child. \return This.
  JsonChildIterator &operator++() {
    ++at_;
    return *this;
  }

  /// \brief Whether two iterators stand on different children. \param o The other.
  /// \return Whether they differ.
  [[nodiscard]] bool operator!=(const JsonChildIterator &o) const { return at_ != o.at_; }

private:
  ::agiru::detail::JsonHandle handle_;
  std::size_t at_;
  JObject current_;
};

/// \brief Newtonsoft `JsonConvert`: named by `JSON Management` for the XML conversions and by
///        `Business Chart Impl.` for a .NET object's serialisation; every member REFUSES by name
///        today, because each needs a type this runtime does not carry (`Formatting`,
///        `BusinessChartData`).
class JsonConvert {
public:
  /// \brief The binder AL never calls; it marks the class as rebuilt.
  struct Binder {};

  /// \brief `JsonConvert.SerializeXmlNode(...)`: not carried yet.
  ::agiru::dotnet::Refused SerializeXmlNode{{.type = "JsonConvert", .member = "SerializeXmlNode"}};
  /// \brief `JsonConvert.DeserializeXmlNode(...)`: not carried yet.
  ::agiru::dotnet::Refused DeserializeXmlNode{
      {.type = "JsonConvert", .member = "DeserializeXmlNode"}};
  /// \brief `JsonConvert.SerializeObject(object)`: not carried yet.
  ::agiru::dotnet::Refused SerializeObject{{.type = "JsonConvert", .member = "SerializeObject"}};
};

}
