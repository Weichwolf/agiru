#pragma once

#include "type/JsonHandle.h"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace agiru::detail {

class JsonNode;

class JsonOwnedNode {
public:
  explicit JsonOwnedNode(JsonNode value);
  JsonOwnedNode(const JsonOwnedNode &other);
  JsonOwnedNode(JsonOwnedNode &&other) noexcept;
  JsonOwnedNode &operator=(const JsonOwnedNode &other);
  JsonOwnedNode &operator=(JsonOwnedNode &&other) noexcept;
  ~JsonOwnedNode();
  [[nodiscard]] JsonNode &Node() const noexcept;

private:
  JsonNode *node_;
};

class JsonNode {
public:
  enum class Kind : std::uint8_t { Null, Boolean, String, Number, Object, Array, Invalid };
  using Member = std::pair<std::string, JsonOwnedNode>;

  JsonNode() = default;
  explicit JsonNode(bool value);
  explicit JsonNode(std::string value);

  template <std::integral T> explicit JsonNode(T value) : JsonNode(Number(std::to_string(value))) {}

  JsonNode(const JsonNode &other);
  JsonNode(JsonNode &&other) noexcept;
  JsonNode &operator=(const JsonNode &other);
  JsonNode &operator=(JsonNode &&other) noexcept;

  [[nodiscard]] static JsonNode object();
  [[nodiscard]] static JsonNode array();
  [[nodiscard]] static JsonNode Number(std::string text);
  [[nodiscard]] static JsonNode parse(std::string_view text);
  [[nodiscard]] std::string dump(int indent = -1) const;

  [[nodiscard]] bool is_null() const noexcept { return kind_ == Kind::Null; }

  [[nodiscard]] bool is_boolean() const noexcept { return kind_ == Kind::Boolean; }

  [[nodiscard]] bool is_string() const noexcept { return kind_ == Kind::String; }

  [[nodiscard]] bool is_number() const noexcept { return kind_ == Kind::Number; }

  [[nodiscard]] bool is_number_float() const noexcept;

  [[nodiscard]] bool is_number_integer() const noexcept {
    return is_number() && !is_number_float();
  }

  [[nodiscard]] bool is_object() const noexcept { return kind_ == Kind::Object; }

  [[nodiscard]] bool is_array() const noexcept { return kind_ == Kind::Array; }

  [[nodiscard]] bool is_structured() const noexcept { return is_object() || is_array(); }

  [[nodiscard]] bool is_discarded() const noexcept { return kind_ == Kind::Invalid; }

  [[nodiscard]] std::size_t size() const noexcept;

  [[nodiscard]] bool empty() const noexcept { return size() == 0; }

  [[nodiscard]] bool contains(std::string_view key) const;

  [[nodiscard]] const std::vector<Member> &Members() const noexcept { return members_; }

  [[nodiscard]] const std::vector<JsonOwnedNode> &Children() const noexcept { return children_; }

  [[nodiscard]] const std::string &Text() const noexcept { return text_; }

  [[nodiscard]] bool Boolean() const noexcept { return boolean_; }

  JsonNode &operator[](std::string_view key);
  const JsonNode &operator[](std::string_view key) const;
  JsonNode &operator[](std::size_t index);
  const JsonNode &operator[](std::size_t index) const;
  void push_back(JsonNode value);
  void insert(std::size_t index, JsonNode value);
  void erase(std::size_t index);
  std::size_t erase(std::string_view key);
  void RenameMember(std::string_view key, std::string name, JsonNode value);
  [[nodiscard]] bool operator==(const JsonNode &other) const;

private:
  friend void JsonNodeRetain(void *node) noexcept;
  friend void JsonNodeRelease(void *node) noexcept;
  std::size_t uses_ = 0;
  Kind kind_ = Kind::Null;
  std::string text_;
  bool boolean_ = false;
  std::vector<Member> members_;
  std::vector<JsonOwnedNode> children_;
};

struct JsonTree {
  explicit JsonTree(JsonNode value) : root(std::move(value)) {}

  JsonOwnedNode root;
  long uses = 0;
};

[[nodiscard]] JsonNode &JsonNodeOf(const JsonHandle &handle);
[[nodiscard]] JsonHandle JsonHandleMade(JsonNode value);
[[nodiscard]] JsonHandle JsonHandleAt(const JsonHandle &tree, JsonNode &node);
[[nodiscard]] std::string ExactJsonDecimal(std::string_view text);
[[nodiscard]] std::int64_t JsonInteger(std::string_view text);
[[nodiscard]] JsonNode *FindJsonPath(JsonNode &root, std::string_view path);

}
