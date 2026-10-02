#include "dotnet/Generic.h"

#include "runtime/ErrorValue.h"
#include "type/BigInteger.h"
#include "type/Boolean.h"
#include "type/Decimal.h"
#include "type/Guid.h"
#include "type/Integer.h"
#include "type/StringValue.h"
#include "type/Variant.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <variant>

namespace agiru::dotnet {

const Variant &GenericList1::Item(Integer index) const {
  if (index < 0 || static_cast<std::size_t>(index) >= items_.size()) {
    throw Error("the index " + std::to_string(index) + " is outside a list of " +
                std::to_string(items_.size()) + ", which .NET counts from zero");
  }
  return items_[static_cast<std::size_t>(index)];
}

Boolean GenericList1::Contains(const Variant &item) const {
  return std::ranges::find(items_, item) != items_.end();
}

GenericDictionary2 GenericDictionary2::Binder::operator()() const {
  return (*this)(0);
}

GenericDictionary2 GenericDictionary2::Binder::operator()(Integer capacity) const {
  if (capacity < 0) { throw Error("Dictionary capacity must be nonnegative"); }
  GenericDictionary2 made;
  made.store_ = std::make_shared<Store>();
  return made;
}

GenericDictionary2::Store &GenericDictionary2::Held() const {
  if (store_ == nullptr) { throw Error("Dictionary operation on a null reference"); }
  return *store_;
}

GenericDictionary2::Key GenericDictionary2::KeyOf(const Variant &key) {
  if (key.Is<std::monostate>()) { throw Error("Dictionary key cannot be null"); }
  if (key.Is<Boolean>()) { return key.Get<Boolean>(); }
  if (key.Is<Integer>()) { return key.Get<Integer>(); }
  if (key.Is<BigInteger>()) { return key.Get<BigInteger>(); }
  if (key.Is<Decimal>()) { return key.Get<Decimal>(); }
  if (key.Is<Text<0>>()) { return std::string(key.Get<Text<0>>().Value()); }
  if (key.Is<Guid>()) { return key.Get<Guid>(); }
  throw Error("Dictionary CLR boxing for this key type is not implemented (board:0035)");
}

void GenericDictionary2::Add(const Variant &key, const Variant &value) {
  Store &store = Held();
  Key boxed = KeyOf(key);
  if (store.entries.contains(boxed)) { throw Error("Dictionary already contains this typed key"); }
  if (store.entries.size() >= static_cast<std::size_t>(std::numeric_limits<Integer>::max())) {
    throw Error("Dictionary Count exceeds Int32");
  }
  store.entries.emplace(std::move(boxed), value);
  ++store.version;
}

Boolean GenericDictionary2::ContainsKey(const Variant &key) const {
  return Held().entries.contains(KeyOf(key));
}

const Variant &GenericDictionary2::Item(const Variant &key) const {
  const Entries &entries = Held().entries;
  const auto found = entries.find(KeyOf(key));
  if (found == entries.end()) { throw Error("Dictionary contains no value for this typed key"); }
  return found->second;
}

Boolean GenericDictionary2::TryGetValue(const Variant &key, Variant &value) const {
  const Entries &entries = Held().entries;
  const auto found = entries.find(KeyOf(key));
  if (found == entries.end()) {
    value = Variant{};
    return false;
  }
  value = found->second;
  return true;
}

Integer GenericDictionary2::Count() const {
  return static_cast<Integer>(Held().entries.size());
}

Boolean GenericDictionary2::Remove(const Variant &key) {
  return Held().entries.erase(KeyOf(key)) != 0;
}

void GenericDictionary2::Clear() {
  Held().entries.clear();
}

GenericDictionary2::Iterator::Iterator(std::shared_ptr<Store> store, Entries::const_iterator at)
    : store_(std::move(store)), version_(store_->version) {
  Set(at);
}

void GenericDictionary2::Iterator::Set(Entries::const_iterator at) {
  if (at == store_->entries.end()) {
    at_.reset();
    current_ = {};
    return;
  }
  at_ = at->first;
  Variant key = std::visit([](const auto &value) { return Variant{value}; }, at->first);
  current_ = {std::move(key), at->second};
}

void GenericDictionary2::Iterator::Validate() const {
  if (store_->version != version_) { throw Error("Dictionary changed during enumeration"); }
}

GenericDictionary2::Entry GenericDictionary2::Iterator::operator*() const {
  Validate();
  if (!at_.has_value()) { throw Error("Dictionary enumerator stands on no entry"); }
  return current_;
}

GenericDictionary2::Iterator &GenericDictionary2::Iterator::operator++() {
  Validate();
  if (!at_.has_value()) { throw Error("Dictionary enumerator is past the last entry"); }
  Set(store_->entries.upper_bound(*at_));
  return *this;
}

bool GenericDictionary2::Iterator::operator==(const Iterator &other) const {
  Validate();
  other.Validate();
  return store_ == other.store_ && at_ == other.at_;
}

GenericDictionary2::Iterator GenericDictionary2::begin() const {
  return {store_, Held().entries.begin()};
}

GenericDictionary2::Iterator GenericDictionary2::end() const {
  return {store_, Held().entries.end()};
}

const Variant &ArrayList::Item(Integer index) const {
  if (index < 0 || static_cast<std::size_t>(index) >= items_.size()) {
    throw Error("the index " + std::to_string(index) + " is outside a list of " +
                std::to_string(items_.size()) + ", which .NET counts from zero");
  }
  return items_[static_cast<std::size_t>(index)];
}

Boolean ArrayList::Contains(const Variant &item) const {
  return std::ranges::find(items_, item) != items_.end();
}

}
