#pragma once

#include "runtime/Error.h"
#include "type/Integer.h"

#include <array>
#include <cstddef>
#include <string>

/// \file
/// \brief AL's `array[N] of T` -- a fixed run of values, indexed from ONE.

namespace agiru {

/// \brief AL `array[N] of T`.
///
/// \tparam T The element type.
/// \tparam N How many, which AL writes in the declaration.
///
/// \note AL HAS NO NAME FOR THIS TYPE, so neither does the name here: `array` is a keyword and not
///       an identifier, and `Array` would read as a type AL declares. What matters is the SHAPE --
///       `A[1]` is the first element, because AL indexes from one and reading `A[0]` is an error
///       rather than the element before it.
///
/// \note THE DIMENSION IS PART OF THE SIGNATURE. `ERMDimensionShortcuts` declares `CreateDimSet`
///       over an `array[6] of Record "Dimension Value"` and again over one record; without the
///       dimension in the type, C++ sees one member declared twice.
template <typename T, std::size_t N> class AlArray;

/// \brief AL's array as a `var` PARAMETER sees it -- the elements and how many, without the size
///        in the type.
///
/// \tparam T The element type.
///
/// \note IT IS THE SAME SHAPE `Text<0>` HAS AND FOR THE SAME REASON. AL passes an `array[7]` to a
///       parameter declared `array[17]` -- `IncDocAttachmentOverviewUT` does, and BC compiles it --
///       so the declared dimension is a CAPACITY and not part of the type. A `var` parameter must
///       therefore bind an array of any size, and only a base can do that.
///
/// \note IT REFERS AND DOES NOT OWN, which is what `var` means: the callee writes into the
///       CALLER's array. The sized one below owns the storage and points this at it.
template <typename T> class AlArray<T, 0> {
public:
  /// \brief The element at an AL index.
  /// \param index The ONE-BASED position.
  /// \return The element.
  /// \throws Error when the index is outside 1..Length().
  T &operator[](Integer index) { return At(index); }

  /// \brief The element at an AL index.
  /// \param index The ONE-BASED position.
  /// \return The element.
  /// \throws Error when the index is outside 1..Length().
  const T &operator[](Integer index) const { return const_cast<AlArray *>(this)->At(index); }

  /// \brief AL `ArrayLen(A)`.
  /// \return How many elements this array holds.
  [[nodiscard]] constexpr Integer Length() const { return static_cast<Integer>(count_); }

protected:
  /// \brief Refers to storage somebody else owns.
  /// \param values Where the elements are.
  /// \param count  How many there are.
  constexpr AlArray(T *values, std::size_t count) : values_(values), count_(count) {}

  AlArray(const AlArray &) = default;
  AlArray(AlArray &&) = default;
  AlArray &operator=(const AlArray &) = default;
  AlArray &operator=(AlArray &&) = default;

  /// \brief Not deleted through this type, which is why it is protected.
  ~AlArray() = default;

  /// \brief Points at another owner's storage.
  /// \param values Where the elements are.
  /// \param count  How many there are.
  constexpr void Refer(T *values, std::size_t count) {
    values_ = values;
    count_ = count;
  }

private:
  T &At(Integer index) {
    if (index < 1 || static_cast<std::size_t>(index) > count_) {
      throw Error("the array index " + std::to_string(index) + " is outside 1.." +
                  std::to_string(count_));
    }
    return values_[static_cast<std::size_t>(index) - 1];
  }

  T *values_;
  std::size_t count_;
};

template <typename T, std::size_t N> class AlArray : public AlArray<T, 0> {
public:
  /// \brief An array of the declared size, empty.
  AlArray() : AlArray<T, 0>(nullptr, N) { this->Refer(held_.data(), N); }

  /// \brief A copy, pointing at ITS OWN storage.
  /// \param other The array copied.
  /// \note THE BASE'S POINTER IS NOT COPIED, IT IS REMADE. A defaulted copy would leave two arrays
  ///       referring to one buffer, and the second write would land in the first array.
  AlArray(const AlArray &other) : AlArray<T, 0>(nullptr, N), held_(other.held_) {
    this->Refer(held_.data(), static_cast<std::size_t>(other.Length()));
  }

  /// \brief Takes another array's elements, keeping its own storage.
  /// \param other The array copied.
  /// \return This array.
  AlArray &operator=(const AlArray &other) {
    if (this != &other) {
      held_ = other.held_;
      this->Refer(held_.data(), static_cast<std::size_t>(other.Length()));
    }
    return *this;
  }

  AlArray(AlArray &&other) noexcept : AlArray(static_cast<const AlArray &>(other)) {}

  /// \brief Takes another array's elements.
  /// \param other The array moved from.
  /// \return This array.
  AlArray &operator=(AlArray &&other) noexcept {
    return *this = static_cast<const AlArray &>(other);
  }

  ~AlArray() = default;

  /// \brief An array of ANOTHER size, which is what AL hands a by-value parameter.
  ///
  /// \tparam M The other array's size.
  /// \param other The other array.
  ///
  /// \note AL DOES NOT CHECK THE DIMENSION AT A CALL, AND THE SOURCE IS WHERE THAT IS DECLARED.
  ///       `IncDocAttachmentOverviewUT` passes an `array[7] of FieldRef` and an `array[5]` to a
  ///       parameter declared `array[17] of FieldRef`, and BC compiles it.
  ///
  /// \warning THE LENGTH TRAVELS WITH THE VALUE, WHICH IS WHAT MAKES THE CALLEE SAFE. That same
  ///          body loops `for I := 1 to ArrayLen(FieldRefArray)`, and if the length were the
  ///          parameter's 17 it would read ten elements the caller never filled.
  template <std::size_t M>
    requires(M != N && M != 0)
  AlArray(const AlArray<T, M> &other) : AlArray<T, 0>(nullptr, N) {
    const auto taken =
        static_cast<std::size_t>(other.Length()) < N ? static_cast<std::size_t>(other.Length()) : N;
    for (std::size_t item = 0; item < taken; ++item) {
      held_[item] = other[static_cast<Integer>(item) + 1];
    }
    this->Refer(held_.data(), taken);
  }

private:
  std::array<T, N> held_{};
};

/// \brief AL `ArrayLen(A)` -- how many elements the declaration gave it.
///
/// \tparam T The element type.
/// \tparam N How many.
/// \param array The array.
/// \return The count.
///
/// \note IT SHADOWS THE DOOR'S REFUSING `ArrayLen(Any)`, and deliberately: the array's own length
///       is known at translation time, so the answer is a constant rather than a refusal. The
///       generic one stays for what is genuinely an `Any`.
template <typename T, std::size_t N>
[[nodiscard]] constexpr Integer ArrayLen(const AlArray<T, N> &array) {
  return array.Length();
}

/// \brief AL `X[i]` -- the element at a ONE-BASED index.
///
/// \tparam Container What is being indexed.
/// \tparam Index     The index type.
/// \param container The array, list or string.
/// \param index     The ONE-BASED position.
/// \return The element.
///
/// \note IT IS A FREE FUNCTION BECAUSE THE EMITTER CANNOT KNOW WHAT IT IS INDEXING. `X[i]` in AL
///       is an array, a `List`, a `Dictionary` or a string depending on a declaration the body
///       writer does not resolve, so it writes the call and the OVERLOAD SET decides -- which is a
///       compiler's job and not a generator's.
template <typename Container, typename Index>
[[nodiscard]] decltype(auto) At(Container &container, Index index) {
  return container[index];
}

}
