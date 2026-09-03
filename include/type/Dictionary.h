#pragma once

#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/List.h"

#include <map>

/// \file
/// \brief AL `Dictionary of [TKey, TValue]`.

namespace agiru {

/// \brief AL `Dictionary of [TKey, TValue]`.
///
/// \tparam TKey   The key type.
/// \tparam TValue The value type.
///
/// \note ORDERED BY KEY RATHER THAN BY INSERTION, and that is a decision rather than an accident:
///       `Keys()` and `Values()` hand out lists, AL code walks them with `foreach`, and a run over
///       a dictionary has to give the same answer twice for the same data. Determinism is
///       compulsory here (CLAUDE.md), and a hash order is not deterministic across runs.
template <typename TKey, typename TValue> class Dictionary {
public:
  /// \brief An empty dictionary.
  Dictionary() = default;

  /// \brief AL `Dictionary.Add(Key, Value)`.
  /// \param key   The key.
  /// \param value The value.
  /// \throws Error when the key is already there, as AL does.
  void Add(const TKey &key, const TValue &value) {
    if (!entries_.try_emplace(key, value).second) {
      throw Error("the dictionary already holds that key");
    }
  }

  /// \brief AL `Dictionary.Set(Key, Value)` -- adds or replaces.
  /// \param key   The key.
  /// \param value The value.
  void Set(const TKey &key, const TValue &value) { entries_.insert_or_assign(key, value); }

  /// \brief AL `Dictionary.ContainsKey(Key)`.
  /// \param key The key.
  /// \return True when the dictionary holds it.
  [[nodiscard]] Boolean ContainsKey(const TKey &key) const { return entries_.contains(key); }

  /// \brief AL `Dictionary.Count()`.
  /// \return How many entries the dictionary holds.
  [[nodiscard]] Integer Count() const { return static_cast<Integer>(entries_.size()); }

  /// \brief AL `Dictionary.Get(Key)`.
  /// \param key The key.
  /// \return The value.
  /// \throws Error when the dictionary does not hold the key.
  [[nodiscard]] const TValue &Get(const TKey &key) const {
    const auto at = entries_.find(key);
    if (at == entries_.end()) { throw Error("the dictionary holds no such key"); }
    return at->second;
  }

  /// \brief AL `Ok := Dictionary.Get(Key, Value)`.
  /// \param key   The key.
  /// \param value Receives the value.
  /// \return True when the key was there; `value` is untouched otherwise.
  /// \note NOT `[[nodiscard]]`, because the RESULT IS NOT THE PRODUCT. AL writes
  ///       `Expected.Get(Key, ExpectedValue);` as a statement and reads the out parameter; the
  ///       Boolean says only whether it was there, and AL lets a caller discard any result at all.
  Boolean Get(const TKey &key, TValue &value) const {
    const auto at = entries_.find(key);
    if (at == entries_.end()) { return false; }
    value = at->second;
    return true;
  }

  /// \brief AL `Dictionary.Remove(Key)`.
  /// \param key The key.
  /// \return True when an entry was removed.
  /// \note NOT `[[nodiscard]]`, because the RESULT IS NOT THE PRODUCT. AL writes
  ///       `Expected.Get(Key, ExpectedValue);` as a statement and reads the out parameter; the
  ///       Boolean says only whether it was there, and AL lets a caller discard any result at all.
  Boolean Remove(const TKey &key) { return entries_.erase(key) != 0; }

  /// \brief AL `Dictionary.Keys()`.
  /// \return The keys, in key order.
  [[nodiscard]] List<TKey> Keys() const {
    List<TKey> keys;
    for (const auto &[key, value] : entries_) { keys.Add(key); }
    return keys;
  }

  /// \brief AL `Dictionary.Values()`.
  /// \return The values, in their keys' order.
  [[nodiscard]] List<TValue> Values() const {
    List<TValue> values;
    for (const auto &[key, value] : entries_) { values.Add(value); }
    return values;
  }

  /// \brief Compares two dictionaries.
  /// \param o The other dictionary.
  /// \return True when they hold the same entries.
  [[nodiscard]] bool operator==(const Dictionary &o) const = default;

private:
  std::map<TKey, TValue> entries_;
};

}
