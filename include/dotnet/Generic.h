#pragma once

#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"
#include "type/Variant.h"

#include <concepts>
#include <cstddef>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
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
  GenericDictionary2() = default;

  /// \brief The binder behind the constructor call, `Dict := Dict.Dictionary([capacity])`, the way
  ///        AL spells a .NET constructor; a capacity is accepted and ignored.
  struct Binder {
    /// \brief .NET `new Dictionary<K, V>()` / `new Dictionary<K, V>(capacity)`.
    /// \tparam Arguments A capacity, or nothing.
    /// \return An empty dictionary.
    template <typename... Arguments>
    [[nodiscard]] GenericDictionary2 operator()(Arguments &&...) const {
      return GenericDictionary2{};
    }
  };

  /// \brief `Dict.Dictionary(...)`, the constructor as AL calls it.
  Binder Dictionary;

  /// \brief .NET `Dictionary.Add(key, value)`; a Variant key reads as its text, which is how a
  ///        `foreach` over an `ArrayList` hands keys over.
  /// \param key   The key.
  /// \param value The value.
  void Add(std::string_view key, const Variant &value) {
    entries_.insert_or_assign(std::string(key), value);
  }

  /// \brief .NET `Dictionary.ContainsKey(key)`.
  /// \param key The key.
  /// \return Whether it is there.
  [[nodiscard]] Boolean ContainsKey(std::string_view key) const {
    return entries_.contains(std::string(key));
  }

  /// \brief .NET `Dictionary.Item(key)`.
  /// \param key The key.
  /// \return The value.
  /// \throws Error when the key is not there, as .NET's `KeyNotFoundException` does.
  [[nodiscard]] const Variant &Item(std::string_view key) const;

  /// \brief .NET `Dictionary.Count`.
  /// \return How many entries.
  [[nodiscard]] Integer Count() const { return static_cast<Integer>(entries_.size()); }

  /// \brief .NET `Dictionary.Remove(key)`.
  /// \param key The key.
  /// \return Whether it was there.
  Boolean Remove(std::string_view key) { return entries_.erase(std::string(key)) != 0; }

  /// \brief One entry as AL's `foreach KeyValuePair in Dict` sees it: `.Key` and `.Value`.
  class Entry {
  public:
    /// \brief An entry standing nowhere yet, which an iterator fills on the first read.
    Entry() = default;

    /// \param entry The map's entry.
    explicit Entry(const std::pair<const std::string, Variant> &entry) : entry_(&entry) {}

    /// \brief .NET `KeyValuePair.Key`. \return The key.
    [[nodiscard]] const std::string &Key() const { return entry_->first; }

    /// \brief .NET `KeyValuePair.Value`. \return The value.
    [[nodiscard]] const Variant &Value() const { return entry_->second; }

  private:
    const std::pair<const std::string, Variant> *entry_ = nullptr;
  };

  /// \brief Walks the entries as `Entry` objects, which is what a `foreach` needs.
  class Iterator {
  public:
    /// \param at The map position.
    explicit Iterator(std::map<std::string, Variant>::const_iterator at) : at_(at) {}

    /// \return The entry at this position, held by the iterator so a `foreach` can bind to it.
    [[nodiscard]] const Entry &operator*() const {
      current_ = Entry(*at_);
      return current_;
    }

    /// \return This iterator, moved on.
    Iterator &operator++() {
      ++at_;
      return *this;
    }

    /// \param o The other. \return Whether both stand at the same place.
    [[nodiscard]] bool operator==(const Iterator &o) const { return at_ == o.at_; }

  private:
    std::map<std::string, Variant>::const_iterator at_;
    mutable Entry current_;
  };

  /// \brief AL `foreach KeyValuePair in Dict`: the first entry. \return The iterator.
  [[nodiscard]] Iterator begin() const { return Iterator(entries_.begin()); }

  /// \brief AL `foreach KeyValuePair in Dict`: past the last entry. \return The iterator.
  [[nodiscard]] Iterator end() const { return Iterator(entries_.end()); }

private:
  std::map<std::string, Variant> entries_;
};

/// \brief .NET `System.Collections.ArrayList`, rebuilt: a list of Variants with the members the
///        BaseApp names -- `Add`, `Count`, `Contains`, `Clear`, `Item` and a `foreach` --
///        constructed as AL spells it, `List := List.ArrayList()`, also from a sequence
///        (`SheetNames.ArrayList(Reader.SheetNames())`, `Excel Buffer`). 9 declarations in 4
///        objects (measured 2026-09-10); `Request Page Parameters Helper` could not compile
///        without it (19 UT cases).
class ArrayList {
public:
  /// \brief The binder behind the constructor call. The class has no user-declared constructor,
  ///        because only then may a data member carry the class's own name.
  struct Binder {
    /// \brief `new ArrayList()`.
    /// \return An empty list.
    [[nodiscard]] class ArrayList operator()() const { return ::agiru::dotnet::ArrayList{}; }

    /// \brief `new ArrayList(collection)`: the items of anything that iterates over Variants.
    /// \tparam Source The collection.
    /// \param source The collection, walked once.
    /// \return A list of its items.
    template <typename Source>
      requires requires(const Source &s) {
        { *s.begin() } -> std::convertible_to<Variant>;
      }
    [[nodiscard]] class ArrayList operator()(const Source &source) const {
      ::agiru::dotnet::ArrayList out;
      for (const auto &item : source) { out.Add(item); }
      return out;
    }

    /// \brief `new ArrayList(x)` over something this runtime has not rebuilt: refused when run.
    /// \tparam Source The absent type.
    /// \param source Its stand-in, whose own refusal is what is raised.
    /// \return Never.
    template <typename Source>
      requires(!requires(const Source &s) {
        { *s.begin() } -> std::convertible_to<Variant>;
      })
    [[nodiscard]] class ArrayList operator()(const Source &source) const {
      static_cast<void>(static_cast<Integer>(source));
      return ::agiru::dotnet::ArrayList{};
    }
  };

  /// \brief `List.ArrayList(...)`, the constructor as AL calls it.
  Binder ArrayList;

  /// \brief .NET `ArrayList.Add(item)`.
  /// \param item The item.
  void Add(const Variant &item) { items_.push_back(item); }

  /// \brief .NET `ArrayList.Count`.
  /// \return How many items.
  [[nodiscard]] Integer Count() const { return static_cast<Integer>(items_.size()); }

  /// \brief .NET `ArrayList.Item(index)`, zero-based.
  /// \param index The position.
  /// \return The item.
  /// \throws Error when the index is outside the list.
  [[nodiscard]] const Variant &Item(Integer index) const;

  /// \brief .NET `ArrayList.Contains(item)`.
  /// \param item The item.
  /// \return Whether an equal item is in the list.
  [[nodiscard]] Boolean Contains(const Variant &item) const;

  /// \brief .NET `ArrayList.Clear()`.
  void Clear() { items_.clear(); }

  /// \brief AL `foreach Item in List`: the first item.
  /// \return The iterator.
  [[nodiscard]] std::vector<Variant>::const_iterator begin() const { return items_.begin(); }

  /// \brief AL `foreach Item in List`: past the last item.
  /// \return The iterator.
  [[nodiscard]] std::vector<Variant>::const_iterator end() const { return items_.end(); }

private:
  std::vector<Variant> items_;
};

/// \brief .NET `System.Collections.Generic.IEnumerator<T>`, rebuilt over Variants: what
///        `IEnumerable.GetEnumerator()` hands back and a `while IEnumerator.MoveNext() do X :=
///        IEnumerator.Current` loop walks (`JSON Management` walks a `JObject`'s properties and
///        a `JArray`'s matches this way, board:0714).
class GenericIEnumerator1 {
public:
  /// \brief The binder AL never calls; it marks the class as rebuilt.
  struct Binder {};

  /// \brief An enumerator over nothing, which is what an unassigned variable holds.
  GenericIEnumerator1() = default;

  /// \brief An enumerator over items, standing before the first. \param items The items.
  explicit GenericIEnumerator1(std::vector<Variant> items) : items_(std::move(items)) {}

  /// \brief `IEnumerator.MoveNext()`: steps to the next item. \return Whether there was one.
  ::agiru::Boolean MoveNext() {
    if (at_ >= items_.size()) { return false; }
    ++at_;
    return at_ <= items_.size();
  }

  /// \brief `IEnumerator.Current`: the item `MoveNext` stepped to. \return It.
  /// \throws Error before the first `MoveNext` and after the last.
  [[nodiscard]] const Variant &Current() const {
    if (at_ == 0 || at_ > items_.size()) {
      throw Error("IEnumerator.Current: the enumerator stands on no item");
    }
    return items_[at_ - 1];
  }

  /// \brief `IEnumerator.Reset()`: back before the first item.
  void Reset() { at_ = 0; }

private:
  std::vector<Variant> items_;
  std::size_t at_ = 0;
};

/// \brief .NET `System.Collections.Generic.IEnumerable<T>`, rebuilt over Variants.
class GenericIEnumerable1 {
public:
  /// \brief The binder AL never calls; it marks the class as rebuilt.
  struct Binder {};

  /// \brief An enumerable over nothing.
  GenericIEnumerable1() = default;

  /// \brief An enumerable over items. \param items The items.
  explicit GenericIEnumerable1(std::vector<Variant> items) : items_(std::move(items)) {}

  /// \brief `IEnumerable := List` -- AL hands a .NET collection to a variable declared as the
  ///        interface, `UserRoles := UserInfo.Roles()` (Azure AD Graph Impl.).
  /// \tparam R A range.
  /// \param range The collection.
  /// \return This.
  /// \throws Error when an element is of a type a `Variant` cannot carry, rather than dropping it.
  template <typename R>
    requires(!std::same_as<std::remove_cvref_t<R>, GenericIEnumerable1>) &&
            (!requires { typename std::remove_cvref_t<R>::IsAlRefusal; }) && requires(const R &r) {
              std::ranges::begin(r);
              std::ranges::end(r);
            }
  GenericIEnumerable1 &operator=(const R &range) {
    items_.clear();
    for (const auto &item : range) {
      if constexpr (std::constructible_from<Variant, decltype(item)>) {
        items_.emplace_back(item);
      } else {
        throw Error("an IEnumerable over this .NET type cannot be walked here yet: its elements "
                    "do not travel in a Variant (board:0035)");
      }
    }
    return *this;
  }

  /// \brief `IEnumerable := AbsentType.Member()`: the call refused before the assignment.
  /// \tparam R The refusal. \param refused It. \return This.
  template <typename R>
    requires requires { typename R::IsAlRefusal; }
  GenericIEnumerable1 &operator=(const R &refused) {
    static_cast<void>(refused);
    return *this;
  }

  /// \brief `IEnumerable.GetEnumerator()`. \return An enumerator standing before the first item.
  [[nodiscard]] GenericIEnumerator1 GetEnumerator() const { return GenericIEnumerator1{items_}; }

  /// \brief AL `foreach`: the first item. \return The iterator.
  [[nodiscard]] std::vector<Variant>::const_iterator begin() const { return items_.begin(); }

  /// \brief AL `foreach`: past the last item. \return The iterator.
  [[nodiscard]] std::vector<Variant>::const_iterator end() const { return items_.end(); }

  /// \brief The items, for a caller that builds another collection. \return Them.
  [[nodiscard]] const std::vector<Variant> &Items() const { return items_; }

private:
  std::vector<Variant> items_;
};

}
