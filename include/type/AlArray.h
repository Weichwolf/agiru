#pragma once

#include "runtime/Error.h"
#include "type/Char.h"
#include "type/Integer.h"
#include "type/StringValue.h"

#include <algorithm>
#include <array>
#include <compare>
#include <concepts>
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
  /// \brief How the view reaches one element of the storage it refers to, typed by whoever owns
  ///        that storage: the owner indexes its OWN element type and hands back the base.
  using Accessor = T &(*)(void *storage, std::size_t index);

  /// \brief A view over an array of another size, or of a DERIVED element type.
  ///
  /// \tparam U The other array's element type: `T` itself, or a class derived from it.
  /// \tparam M The other array's size.
  /// \param other The array referred to.
  ///
  /// \note THIS IS WHAT LETS AL PASS ONE ARRAY TO ANOTHER'S `var` PARAMETER. `MatrixManagement.
  ///       GeneratePeriodMatrixData` takes `var CaptionSet: array[32] of Text[80]` and is handed an
  ///       `array[32] of Text[100]`, and `var PeriodRecords: array[32] of Record Date temporary`
  ///       is handed a plain `array[32] of Record Date` -- BC compiles both, so the parameter here
  ///       is a VIEW over the caller's elements, seen through their base class, and every write
  ///       the callee makes lands in the caller's storage. Nothing is copied.
  template <typename U, std::size_t M>
    requires(M != 0 && (std::same_as<U, T> || std::derived_from<U, T>))
  AlArray(AlArray<U, M> &other) // NOLINT(google-explicit-constructor)
      : storage_(other.Storage_()),
        count_(static_cast<std::size_t>(other.Length())),
        at_(&AlArray<U, M>::template Element_<T>) {}

  /// \brief A view over a view whose elements are a DERIVED type: a `var` parameter handed on.
  /// \tparam U The inner view's element type, derived from `T`.
  /// \param other The inner view, which must outlive this one.
  template <typename U>
    requires(!std::same_as<U, T> && std::derived_from<U, T>)
  AlArray(AlArray<U, 0> &other) // NOLINT(google-explicit-constructor)
      : storage_(&other), count_(static_cast<std::size_t>(other.Length())), at_(&Through_<U>) {}

  /// \brief A view over an array a codeunit holds BY HANDLE, which is how its globals arrive.
  /// \tparam H The handle, whose `operator->` reaches a sized array.
  /// \param handle The handle, which makes the array on first use.
  template <typename H>
    requires requires(H &h) { (*h.operator->()).Storage_(); }
  AlArray(H &handle) // NOLINT(google-explicit-constructor)
      : AlArray(*handle.operator->()) {}

  /// \brief A second view over the same storage, which is what handing a `var` parameter on is.
  AlArray(const AlArray &) = default;

  /// \brief The same, from a view that is going away.
  AlArray(AlArray &&) = default;

  /// \brief A view owns nothing, so there is nothing to do; PUBLIC because a `var` array
  ///        parameter is a view passed by value and the caller's temporary is destroyed.
  ~AlArray() = default;

  /// \brief AL `A := B` on a `var` array parameter: the ELEMENTS are copied into the storage this
  ///        view refers to, which is the caller's array.
  /// \param other The array copied from.
  /// \return This view.
  /// \note THE VIEW IS NOT REBOUND. A reference parameter stands for the caller's array for the
  ///       whole call, so an assignment writes through it rather than pointing it elsewhere.
  AlArray &operator=(const AlArray &other) {
    if (this == &other) { return *this; }
    const std::size_t shared = std::min(count_, static_cast<std::size_t>(other.Length()));
    for (std::size_t item = 0; item < shared; ++item) {
      At(static_cast<Integer>(item) + 1) = other[static_cast<Integer>(item) + 1];
    }
    return *this;
  }

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
  /// \param storage Whose elements these are.
  /// \param count   How many there are.
  /// \param at      How one of them is reached.
  constexpr AlArray(void *storage, std::size_t count, Accessor at)
      : storage_(storage), count_(count), at_(at) {}

  AlArray &operator=(AlArray &&) = default;

  /// \brief Points at another owner's storage.
  /// \param storage Whose elements these are.
  /// \param count   How many there are.
  /// \param at      How one of them is reached.
  constexpr void Refer(void *storage, std::size_t count, Accessor at) {
    storage_ = storage;
    count_ = count;
    at_ = at;
  }

private:
  template <typename U> static T &Through_(void *storage, std::size_t index) {
    return static_cast<T &>(
        (*static_cast<AlArray<U, 0> *>(storage))[static_cast<Integer>(index) + 1]);
  }

  T &At(Integer index) {
    if (index < 1 || static_cast<std::size_t>(index) > count_) {
      throw Error("the array index " + std::to_string(index) + " is outside 1.." +
                  std::to_string(count_));
    }
    return at_(storage_, static_cast<std::size_t>(index) - 1);
  }

  void *storage_;
  std::size_t count_;
  Accessor at_;
};

template <typename T, std::size_t N> class AlArray : public AlArray<T, 0> {
public:
  /// \brief An array of the declared size, empty.
  AlArray() : AlArray<T, 0>(this, N, &Element_<T>), first_(held_.data()) {}

  /// \brief A copy, pointing at ITS OWN storage.
  /// \param other The array copied.
  /// \note THE BASE'S POINTER IS NOT COPIED, IT IS REMADE. A defaulted copy would leave two arrays
  ///       referring to one buffer, and the second write would land in the first array.
  AlArray(const AlArray &other) : AlArray<T, 0>(this, N, &Element_<T>), first_(held_.data()) {
    Take(other);
  }

  /// \brief The same conversion one dimension down: `array[10, 100]` into `array[10, 10]`
  ///        (`PaymentExportXMLPortUT`) differs in the ELEMENT type, and each row converts through
  ///        the bound-changing constructor below.
  /// \tparam U The argument's element type, itself an `AlArray` of other bounds.
  /// \tparam M The argument's outer bound.
  /// \param other The argument.
  /// \brief Takes the elements of an array whose SIZE the caller does not carry.
  /// \param other The array, as AL hands one on from a `var array` parameter.
  ///
  /// \note AL DECLARES A SIZE AND PASSES ONE ON WITHOUT IT. A `var array of Integer` parameter is
  ///       the unsized view here, and the procedure it is handed to declares `array[10]`; the
  ///       elements are copied, which is what passing by value means in AL too.
  AlArray(const AlArray<T, 0> &other) // NOLINT(google-explicit-constructor)
      : AlArray<T, 0>(this, N, &Element_<T>), first_(held_.data()) {
    Take(other);
  }

  template <typename U, std::size_t M>
    requires(!std::same_as<U, T> && M != 0 && std::constructible_from<T, const U &>)
  AlArray(const AlArray<U, M> &other) // NOLINT(google-explicit-constructor)
      : AlArray<T, 0>(this, N, &Element_<T>), first_(held_.data()) {
    Take(other);
  }

  /// \brief The elements of a VIEW over another element type -- a `var array of Code` parameter
  ///        handed on to a by-value `array[10] of Code[20]`.
  /// \tparam U The view's element type.
  /// \param other The view.
  template <typename U>
    requires(!std::same_as<U, T> && std::constructible_from<T, const U &>)
  AlArray(const AlArray<U, 0> &other) // NOLINT(google-explicit-constructor)
      : AlArray<T, 0>(this, N, &Element_<T>), first_(held_.data()) {
    Take(other);
  }

  /// \brief Takes another array's elements, keeping its own storage.
  /// \param other The array copied.
  /// \return This array.
  AlArray &operator=(const AlArray &other) {
    if (this != &other) { Take(other); }
    return *this;
  }

  AlArray(AlArray &&other) noexcept : AlArray(static_cast<const AlArray &>(other)) {}

  /// \brief Takes another array's elements.
  /// \param other The array moved from.
  /// \return This array.
  AlArray &operator=(AlArray &&other) noexcept {
    return *this = static_cast<const AlArray &>(other);
  }

  ~AlArray() { delete[] spill_; }

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
  AlArray(const AlArray<T, M> &other) // NOLINT(google-explicit-constructor)
      : AlArray<T, 0>(this, N, &Element_<T>), first_(held_.data()) {
    Take(other);
  }

  /// \brief Where the elements are, for a view over this array.
  /// \return This array, which is what an accessor is given back.
  [[nodiscard]] void *Storage_() { return this; }

  /// \brief One element, seen as a base of its own type.
  /// \tparam B The base -- `T` itself, or a class it derives from.
  /// \param storage This array.
  /// \param index The ZERO-BASED position, already checked by the caller.
  /// \return The element.
  template <typename B> static B &Element_(void *storage, std::size_t index) {
    return static_cast<B &>(static_cast<AlArray *>(storage)->first_[index]);
  }

private:
  /// \brief Takes every element the other array holds, keeping the OTHER's length.
  ///
  /// \warning A PARAMETER TAKES THE SHAPE OF THE ARGUMENT, NOT OF ITS DECLARATION (board:0633).
  ///          `PaymentExportXMLPortUT.CreateDataExchFieldForLine` declares `array[10, 10]`, is
  ///          given an `array[10, 100]`, and walks the parameter to column 100 -- BC runs it. So
  ///          when the argument is longer than the declared `N`, the surplus lives in a buffer
  ///          this array owns, and `ArrayLen` and the bound check answer with the argument's
  ///          length. The declared size is the storage for the ordinary case and costs no heap.
  template <typename Source> void Take(const Source &other) {
    const auto length = static_cast<std::size_t>(other.Length());
    T *into = held_.data();
    if (length > N) {
      T *fresh = new T[length];
      delete[] spill_;
      spill_ = fresh;
      into = spill_;
    } else {
      delete[] spill_;
      spill_ = nullptr;
    }
    for (std::size_t item = 0; item < length; ++item) {
      into[item] = T(other[static_cast<Integer>(item) + 1]);
    }
    first_ = into;
    this->Refer(this, length, &Element_<T>);
  }

  std::array<T, N> held_{};
  T *spill_ = nullptr;
  T *first_;
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

/// \brief AL `ArrayLen(X)` where the array is held by a HANDLE, which is how a page keeps one.
/// \tparam H The handle's type.
/// \param handle The handle.
/// \return The number of elements.
template <typename H>
  requires requires(const H &held) { ArrayLen(*held.operator->()); }
[[nodiscard]] Integer ArrayLen(const H &handle) {
  return ArrayLen(*handle.operator->());
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

/// \brief AL `X[i, j, ...]` -- a multidimensional array, indexed one dimension at a time.
/// \tparam Container The array's type.
/// \tparam Index     The first index's type.
/// \tparam Rest      The remaining indices' types.
/// \param container The array.
/// \param index     The first index, one-based.
/// \param rest      The remaining indices, one-based.
/// \return What sits there, as a reference, so `X[i, j] := v` assigns.
///
/// \note AL DECLARES `array[10, 4] of Decimal` AS ONE TYPE and indexes it with one bracket; the
///       runtime nests one `AlArray` per dimension, so the indices peel off one at a time.
template <typename Container, typename Index, typename... Rest>
  requires(sizeof...(Rest) > 0)
[[nodiscard]] decltype(auto) At(Container &container, Index index, Rest... rest) {
  return At(container[index], rest...);
}

/// \brief AL `Text[Index]` -- one character of a string value, one-based.
/// \tparam S A `Text` or `Code`.
/// \param value The string.
/// \param index The one-based position.
/// \return The character there.
/// \throws Error when the position is outside the string, which is what AL raises.
/// \note THE GENERATOR SPELLS EVERY INDEX AS `At(...)`, because AL's `[]` reaches arrays and
///       strings alike; this is the string half of that one spelling.
/// \brief One character POSITION of a text, which AL both reads and writes.
///
/// \note AL ASSIGNS INTO A TEXT BY POSITION -- `DateFormulaAsText[I] := '-'` is 61 call sites in
///       the BaseApp -- so an index that returned the character BY VALUE would compile and throw
///       the assignment away. This stands in for the position itself: it reads as a `Char` and
///       takes a `Char`, a one-character text or a code point.
template <typename S> class CharAt {
public:
  /// \brief Marks this as a text POSITION, so a `Char` can take it as one overload.
  using IsATextPosition = void;

  /// \brief Names a position in a text.
  /// \param value The text. \param index The one-based position.
  CharAt(S &value, Integer index) : value_(&value), index_(index) {}

  /// \brief AL `Text[Index] := OtherText[OtherIndex]` -- one position copied into another.
  /// \tparam O The other text's type.
  /// \param other The other position.
  /// \return This position.
  template <typename O> CharAt &operator=(const CharAt<O> &other) {
    return *this = static_cast<Char>(other);
  }

  /// \brief AL `Text[Index] := Char`.
  /// \param character The character.
  /// \return This position.
  CharAt &operator=(Char character) {
    const std::string encoded = Encoded(character);
    if (encoded.size() == 1) {
      Write(encoded.front());
      return *this;
    }
    std::string text(value_->Value());
    if (index_ >= 1 && static_cast<std::size_t>(index_) == text.size() + 1) {
      text += encoded;
    } else {
      Check(text.size());
      text.replace(static_cast<std::size_t>(index_) - 1, 1, encoded);
    }
    *value_ = std::string_view(text);
    return *this;
  }

  /// \brief AL `Text[Index] := 'x'` -- a one-character text.
  /// \param text The text, whose first character is written.
  /// \return This position.
  /// \throws Error when the text is not exactly one character, which is what AL raises.
  CharAt &operator=(std::string_view text) {
    if (text.size() != 1) {
      throw Error("A text of length " + std::to_string(text.size()) +
                  " does not fit one character position");
    }
    Write(text.front());
    return *this;
  }

  /// \brief AL `Text[Index] := 'x'` where the literal is still an array.
  /// \tparam N The literal's length, one character and its terminator.
  /// \param text The literal.
  /// \return This position.
  template <std::size_t N> CharAt &operator=(const char (&text)[N]) {
    return *this = std::string_view(text, N - 1);
  }

  /// \brief AL `Text[Index] := Integer` -- a code point.
  /// \param code The code point.
  /// \return This position.
  CharAt &operator=(std::int32_t code) {
    Write(static_cast<char>(code));
    return *this;
  }

  /// \brief Reads the character standing there.
  /// \return The character.
  [[nodiscard]] operator Char() const { return Read(); }

  /// \brief Reads the character as its code point -- AL assigns `Text[i]` straight to an Integer.
  /// \return The code point.
  ///
  /// \note IT SITS BESIDE THE ORDERING OVERLOADS AND NOT INSTEAD OF THEM. On its own it turned
  ///       `Text[i] >= '0'` into a pointer comparison; with `<=>` present the character wins the
  ///       comparison and this only carries the value where AL wants a number.
  [[nodiscard]] operator std::int32_t() const { return static_cast<std::int32_t>(Read()); }

  /// \brief Compares the character standing there.
  /// \param other The other character.
  /// \return Whether they are the same.
  [[nodiscard]] bool operator==(Char other) const { return Read() == other; }

  /// \brief AL `Text[Index] > 57` -- a position against a code point, which an Integer now
  ///        converts to `Char` implicitly and would otherwise make ambiguous.
  /// \param code The code point.
  /// \return The ordering.
  [[nodiscard]] std::strong_ordering operator<=>(std::int32_t code) const {
    return static_cast<std::int32_t>(Read()) <=> code;
  }

  /// \brief AL `Text[Index] = 57`.
  /// \param code The code point.
  /// \return Whether the character has that code point.
  [[nodiscard]] bool operator==(std::int32_t code) const {
    return static_cast<std::int32_t>(Read()) == code;
  }

  /// \brief Compares one text position with another.
  /// \tparam O The other text's type.
  /// \param other The other position.
  /// \return Whether the two characters are the same.
  ///
  /// \note WITHOUT IT `Text[I] = Other[J]` WAS AMBIGUOUS WITH ITSELF: each side converts to a
  ///       `Char`, so the comparison and its reversed form were equally good.
  template <typename O> [[nodiscard]] bool operator==(const CharAt<O> &other) const {
    return Read() == static_cast<Char>(other);
  }

  /// \brief Compares against a one-character text, which is how AL writes a character literal.
  /// \param text The text.
  /// \return Whether they are the same.
  [[nodiscard]] bool operator==(std::string_view text) const { return Read() == text; }

  /// \brief Compares against a character literal, which reaches here as an array.
  /// \tparam N The literal's length, one character and its terminator.
  /// \param text The literal.
  /// \return Whether they are the same.
  /// \warning IT BINDS THE ARRAY ITSELF. Without it the literal could become a `Char` or a
  ///          `std::string_view` and the two overloads were equally good.
  template <std::size_t N> [[nodiscard]] bool operator==(const char (&text)[N]) const {
    return Read() == std::string_view(text, N - 1);
  }

  /// \brief Orders against a character literal, which reaches here as an array.
  /// \tparam N The literal's length, one character and its terminator.
  /// \param text The literal.
  /// \return The ordering.
  template <std::size_t N>
  [[nodiscard]] std::strong_ordering operator<=>(const char (&text)[N]) const {
    return Read() <=> std::string_view(text, N - 1);
  }

  /// \brief Orders the character against a one-character text -- AL's `>= '0'`.
  /// \param text The text.
  /// \return The ordering.
  [[nodiscard]] std::strong_ordering operator<=>(std::string_view text) const {
    return Read() <=> text;
  }

  /// \brief Orders the character against another.
  /// \param other The other character.
  /// \return The ordering.
  [[nodiscard]] std::strong_ordering operator<=>(Char other) const { return Read() <=> other; }

private:
  /// \brief AL `Text := OtherText[I]` -- one position, as the one-character text it reads as.
  /// \return The character, encoded.
  [[nodiscard]] std::string ToText() const { return Encoded(Read()); }

  [[nodiscard]] Char Read() const {
    const std::string_view text = value_->Value();
    Check(text.size());
    return static_cast<Char>(
        static_cast<unsigned char>(text[static_cast<std::size_t>(index_) - 1]));
  }

  /// \brief AL `Text[Index] := Char`.
  ///
  /// \note ONE PAST THE END APPENDS. `SeparatorChar[1] := 9` on an empty `Text` and
  ///       `t[i] := c` in a loop that builds a column id are both shipped BaseApp code
  ///       (`DataExchDef.Table.al`, `ExcelBuffer.Table.al`), so an index of `StrLen + 1` extends
  ///       the text by one character; anything further is outside it, as a read would be.
  void Write(char character) {
    std::string text(value_->Value());
    if (index_ >= 1 && static_cast<std::size_t>(index_) == text.size() + 1) {
      text.push_back(character);
    } else {
      Check(text.size());
      text[static_cast<std::size_t>(index_) - 1] = character;
    }
    *value_ = std::string_view(text);
  }

  void Check(std::size_t length) const {
    if (index_ < 1 || static_cast<std::size_t>(index_) > length) {
      throw Error("Index " + std::to_string(index_) + " is outside the text of length " +
                  std::to_string(length));
    }
  }

  S *value_;
  Integer index_;
};

/// \brief AL `Text[Index]` on a text that can be WRITTEN -- the position itself.
/// \tparam S A `Text` or `Code`.
/// \param value The text. \param index The one-based position.
/// \return The position, which reads as a character and takes one.
template <typename S>
  requires std::derived_from<S, StringValue>
[[nodiscard]] CharAt<S> At(S &value, Integer index) {
  return CharAt<S>(value, index);
}

template <typename S>
  requires std::derived_from<S, StringValue>
[[nodiscard]] Char At(const S &value, Integer index) {
  const std::string_view text = value.Value();
  if (index < 1 || static_cast<std::size_t>(index) > text.size()) {
    throw Error("Index " + std::to_string(index) + " is outside the text of length " +
                std::to_string(text.size()));
  }
  return static_cast<Char>(static_cast<unsigned char>(text[static_cast<std::size_t>(index) - 1]));
}

/// \brief AL `ArrayLen(A, Dimension)` -- the length along ONE dimension of a nested array.
///
/// \tparam T The element type, itself an `AlArray` for every dimension past the first.
/// \tparam N The outermost length.
/// \param array     The array.
/// \param dimension Which dimension, ONE-BASED as AL counts them.
/// \return The length along it.
/// \throws Error when the dimension exceeds what the declaration has, which is what AL raises.
template <typename T, std::size_t N>
[[nodiscard]] constexpr Integer ArrayLen(const AlArray<T, N> &array, Integer dimension) {
  if (dimension <= 1) { return array.Length(); }
  if constexpr (requires(const T &inner) { ArrayLen(inner, dimension); }) {
    return ArrayLen(At(array, 1), dimension - 1);
  } else {
    throw Error("ArrayLen: dimension " + std::to_string(dimension) + " of a one-dimensional array");
  }
}
}
