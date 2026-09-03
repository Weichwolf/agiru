#pragma once

#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Variant.h"

#include <map>
#include <string>
#include <vector>

/// \file
/// \brief The .NET generic collections, as AL declares them with their type argument erased.

namespace agiru::dotnet {

/// \brief `System.Collections.Generic.List<T>`.
///
/// \note AL LOSES THE TYPE ARGUMENT AND SO DOES THIS. A `dotnet` package declares the type as
///       `List`1` and AL names it `GenericList1` -- one alias for every element type there is. So
///       the element is a `Variant`, which is the same erasure AL itself performs, and a caller
///       that put a Text in gets a Text out.
///
/// \note THE INDEX IS ZERO-BASED, because .NET's is. AL's own `List` is ONE-based
///       (`list-get-method.md`), and mixing them up is the kind of defect that reads correctly and
///       returns the neighbour -- which is why the two types stay apart instead of one wrapping the
///       other.
class GenericList1 {
public:
  /// \brief An empty list.
  GenericList1() = default;

  /// \brief .NET `List.Add(item)`.
  /// \param item The value.
  void Add(const Variant &item) { items_.push_back(item); }

  /// \brief .NET `List.Count`.
  /// \return How many values it holds.
  [[nodiscard]] Integer Count() const { return static_cast<Integer>(items_.size()); }

  /// \brief .NET `List.Item(index)` -- ZERO-based.
  /// \param index The position, counting from zero.
  /// \return The value.
  /// \throws Error when the index is outside the list.
  [[nodiscard]] const Variant &Item(Integer index) const;

  /// \brief .NET `List.Contains(item)`.
  /// \param item The value.
  /// \return True when it is in the list.
  [[nodiscard]] Boolean Contains(const Variant &item) const;

  /// \brief .NET `List.Clear()`.
  void Clear() { items_.clear(); }

private:
  std::vector<Variant> items_;
};

/// \brief `System.Collections.Generic.Dictionary<TKey, TValue>`.
///
/// \note THE KEY IS A TEXT AND THE VALUE A VARIANT, which is what AL's erasure leaves reachable: a
///       `dotnet` alias carries neither argument, and every use measured over BCApps keys by a
///       name.
class GenericDictionary2 {
public:
  /// \brief An empty dictionary.
  GenericDictionary2() = default;

  /// \brief .NET `Dictionary.Add(key, value)`.
  /// \param key   The name.
  /// \param value The value.
  void Add(const std::string &key, const Variant &value) { entries_.insert_or_assign(key, value); }

  /// \brief .NET `Dictionary.ContainsKey(key)`.
  /// \param key The name.
  /// \return True when the dictionary holds it.
  [[nodiscard]] Boolean ContainsKey(const std::string &key) const { return entries_.contains(key); }

  /// \brief .NET `Dictionary.Item(key)`.
  /// \param key The name.
  /// \return The value.
  /// \throws Error when nothing is held under that key.
  [[nodiscard]] const Variant &Item(const std::string &key) const;

  /// \brief .NET `Dictionary.Count`.
  /// \return How many entries it holds.
  [[nodiscard]] Integer Count() const { return static_cast<Integer>(entries_.size()); }

  /// \brief .NET `Dictionary.Remove(key)`.
  /// \param key The name.
  /// \return True when something was removed.
  Boolean Remove(const std::string &key) { return entries_.erase(key) != 0; }

private:
  std::map<std::string, Variant> entries_;
};

}
