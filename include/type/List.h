#pragma once

#include "runtime/Error.h"
#include "type/Boolean.h"
#include "type/Integer.h"

#include <concepts>
#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

/// \file
/// \brief AL `List of [T]` -- an ordered collection, indexed from one.

namespace agiru {

/// \brief AL `List of [T]`.
///
/// \tparam T The element type.
///
/// \note EVERY INDEX HERE COUNTS FROM ONE, which `list-indexof-method.md` states outright --
/// "returns
///       the one-based index of the first occurrence" -- and `list-get-integer-method.md` confirms
///       from the other side: "this method will raise an error if the index is outside the valid
///       range". A list that quietly answered for index 0 would be off by one everywhere.
template <typename T> class List {
public:
  /// \brief An empty list.
  List() = default;

  /// \brief A list of elements that convert to this one's.
  ///
  /// \tparam U The other element type.
  /// \param other The other list.
  ///
  /// \note AL HAS ONE `List of [Text]` AND THIS TREE HAS TWO SPELLINGS OF ITS ELEMENT.
  ///       `Text.Split` returns the runtime's own text -- `StringValue.h` cannot name `Text<0>`,
  ///       which is declared in a header that includes IT -- and an AL variable is declared
  ///       `List of [Text]`, which is `List<Text<0>>`. The two are the same list in AL, so the
  ///       conversion is elementwise and implicit, not a cast the generator has to write.
  template <typename U>
    requires(!std::is_same_v<U, T>) && std::convertible_to<const U &, T>
  List(const List<U> &other) {
    for (::agiru::Integer at = 1; at <= other.Count(); ++at) {
      values_.emplace_back(other.Get(at));
    }
  }

  /// \brief AL `List.Add(Value)`.
  /// \param value The element to append.
  void Add(const T &value) { values_.push_back(value); }

  /// \brief AL `List.AddRange(List)`.
  /// \param other The list whose elements are appended.
  void AddRange(const List &other) {
    values_.insert(values_.end(), other.values_.begin(), other.values_.end());
  }

  /// \brief AL `List.Count()`.
  /// \return How many elements the list holds.
  [[nodiscard]] Integer Count() const { return static_cast<Integer>(values_.size()); }

  /// \brief AL `List.Contains(Value)`.
  /// \param value The element to look for.
  /// \return True when the list holds it.
  [[nodiscard]] Boolean Contains(const T &value) const { return At(value) != values_.end(); }

  /// \brief AL `List.IndexOf(Value)`.
  /// \param value The element to look for.
  /// \return Its ONE-BASED position, or 0 when the list does not hold it.
  [[nodiscard]] Integer IndexOf(const T &value) const {
    const auto at = At(value);
    return at == values_.end() ? 0 : static_cast<Integer>(at - values_.begin()) + 1;
  }

  /// \brief AL `List.LastIndexOf(Value)`.
  /// \param value The element to look for.
  /// \return The one-based position of its last occurrence, or 0.
  [[nodiscard]] Integer LastIndexOf(const T &value) const {
    for (std::size_t i = values_.size(); i > 0; --i) {
      if (values_[i - 1] == value) { return static_cast<Integer>(i); }
    }
    return 0;
  }

  /// \brief AL `List.Get(Index)`.
  /// \param index The one-based position.
  /// \return The element there.
  /// \throws Error when the index is outside the list, which the page says it must.
  [[nodiscard]] const T &Get(Integer index) const { return values_.at(Checked(index)); }

  /// \brief AL `Ok := List.Get(Index, Value)`.
  /// \param index The one-based position.
  /// \param value Receives the element.
  /// \return True when the index is inside the list; `value` is untouched otherwise.
  /// \note NOT `[[nodiscard]]`, because the RESULT IS NOT THE PRODUCT. AL writes
  ///       `Expected.Get(Key, ExpectedValue);` as a statement and reads the out parameter; the
  ///       Boolean says only whether it was there, and AL lets a caller discard any result at all.
  Boolean Get(Integer index, T &value) const {
    if (!Inside(index)) { return false; }
    value = values_[static_cast<std::size_t>(index) - 1];
    return true;
  }

  /// \brief AL `List.Set(Index, Value)` -- replaces the element there.
  /// \param index The one-based position.
  /// \param value The replacement.
  /// \throws Error when the index is outside the list.
  void Set(Integer index, const T &value) { values_.at(Checked(index)) = value; }

  /// \brief AL `List.Insert(Index, Value)` -- puts an element at that position.
  /// \param index The one-based position it will occupy.
  /// \param value The element.
  /// \throws Error when the index is neither inside the list nor one past its end.
  void Insert(Integer index, const T &value) {
    if (index < 1 || static_cast<std::size_t>(index) > values_.size() + 1) { Refuse(index); }
    values_.insert(values_.begin() + (static_cast<std::size_t>(index) - 1), value);
  }

  /// \brief AL `List.RemoveAt(Index)`.
  /// \param index The one-based position.
  /// \throws Error when the index is outside the list.
  void RemoveAt(Integer index) {
    values_.erase(values_.begin() + static_cast<std::ptrdiff_t>(Checked(index)));
  }

  /// \brief AL `List.Remove(Value)` -- removes the first occurrence.
  /// \param value The element.
  /// \return True when one was removed.
  /// \note NOT `[[nodiscard]]`, because the RESULT IS NOT THE PRODUCT. AL writes
  ///       `Expected.Get(Key, ExpectedValue);` as a statement and reads the out parameter; the
  ///       Boolean says only whether it was there, and AL lets a caller discard any result at all.
  Boolean Remove(const T &value) {
    const auto at = At(value);
    if (at == values_.end()) { return false; }
    values_.erase(at);
    return true;
  }

  /// \brief AL `List.RemoveRange(Index, Count)`.
  /// \param index The one-based position of the first element to remove.
  /// \param count How many.
  /// \throws Error when the range is not entirely inside the list.
  void RemoveRange(Integer index, Integer count) {
    if (count < 0 || !Inside(index) ||
        static_cast<std::size_t>(index) + static_cast<std::size_t>(count) > values_.size() + 1) {
      Refuse(index);
    }
    const auto first = values_.begin() + (static_cast<std::ptrdiff_t>(index) - 1);
    values_.erase(first, first + static_cast<std::ptrdiff_t>(count));
  }

  /// \brief AL `List.GetRange(Index, Count)`.
  /// \param index The one-based position of the first element.
  /// \param count How many.
  /// \return A list of those elements.
  /// \throws Error when the range is not entirely inside the list.
  [[nodiscard]] List GetRange(Integer index, Integer count) const {
    if (count < 0 || !Inside(index) ||
        static_cast<std::size_t>(index) + static_cast<std::size_t>(count) > values_.size() + 1) {
      Refuse(index);
    }
    List taken;
    const auto first = values_.begin() + (static_cast<std::ptrdiff_t>(index) - 1);
    taken.values_.assign(first, first + static_cast<std::ptrdiff_t>(count));
    return taken;
  }

  /// \brief AL `List.Reverse()` -- turns the list around in place.
  void Reverse() {
    for (std::size_t i = 0, j = values_.size(); i + 1 < j; ++i, --j) {
      std::swap(values_[i], values_[j - 1]);
    }
  }

  /// \brief The elements, for a `foreach`.
  /// \return An iterator to the first.
  [[nodiscard]] auto begin() const { return values_.begin(); }

  /// \brief The end of the elements, for a `foreach`.
  /// \return An iterator past the last.
  [[nodiscard]] auto end() const { return values_.end(); }

  /// \brief Compares two lists.
  /// \param o The other list.
  /// \return True when they hold the same elements in the same order.
  [[nodiscard]] bool operator==(const List &o) const = default;

private:
  [[nodiscard]] auto At(const T &value) const {
    auto at = values_.begin();
    while (at != values_.end() && !(*at == value)) { ++at; }
    return at;
  }

  [[nodiscard]] auto At(const T &value) {
    auto at = values_.begin();
    while (at != values_.end() && !(*at == value)) { ++at; }
    return at;
  }

  [[nodiscard]] bool Inside(Integer index) const {
    return index >= 1 && static_cast<std::size_t>(index) <= values_.size();
  }

  [[noreturn]] void Refuse(Integer index) const {
    throw Error("the list index " + std::to_string(index) + " is outside 1.." +
                std::to_string(values_.size()));
  }

  [[nodiscard]] std::size_t Checked(Integer index) const {
    if (!Inside(index)) { Refuse(index); }
    return static_cast<std::size_t>(index) - 1;
  }

  std::vector<T> values_;
};

}
