#pragma once

#include "runtime/Error.h"

#include <compare>
#include <string>
#include <string_view>
#include <type_traits>

/// \file
/// \brief What a .NET member that has not been rebuilt answers with: nothing, loudly.

namespace agiru::dotnet {

/// \brief A .NET member the runtime carries by name and cannot yet perform.
///
/// \note IT IS A MEMBER AND NOT A METHOD, and that is what lets one shape serve both. AL reads
///       `UserInfo.ObjectId` as a property and calls `Doc.SelectNodes(x)` as a method, and the
///       generator cannot tell them apart from the call site -- a `dotnet` package declares a type
///       and no members at all. An OBJECT that is both callable and convertible is the same at
///       either site, and refuses at either.
///
/// \warning EVERY PATH THROUGH IT THROWS, and the message names the type and the member. That is
///          the whole point: the predecessor let an unbuilt .NET class fall to a nil value whose
///          every operation was a silent no-op, and its own comments record what that cost --
///          "jeder Aufruf lieferte NilValue, jede Abfrage las falsch -- und zwar STILL". A wrong
///          answer that looks like an answer is the defect this tree is arranged against.
/// \brief Which member of which type, in ONE value.
///
/// Two adjacent strings are two strings a caller can swap in silence, and this one is written 1 908
/// times by a generator -- so the pair is a value with named fields rather than two arguments.
struct Named {
  std::string_view type;   ///< The .NET type's AL alias.
  std::string_view member; ///< The member's name.
};

/// \brief A .NET member this runtime has not rebuilt, callable in any shape and refusing all of
///        them.
class Refused {
public:
  /// \brief A member of a .NET type that is not rebuilt.
  /// \param named The type and the member, in one value so they cannot be swapped.
  constexpr explicit Refused(Named named) : named_(named) {}

  /// \brief Refuses a call of any shape.
  /// \tparam Arguments Whatever the caller passed.
  /// \param arguments The arguments, read only to be discarded -- what is refused is the CALL.
  /// \return Never.
  /// \throws Error always.
  template <typename... Arguments> Refused operator()(Arguments &&...arguments) const {
    (static_cast<void>(arguments), ...);
    Throw();
  }

  /// \brief Refuses to become a value of any type.
  /// \tparam T The type the caller wants.
  /// \return Never.
  /// \throws Error always.
  ///
  /// \note IT DOES NOT CONVERT TO A STANDARD STRING, and that is what keeps it unambiguous.
  ///       `Code<50> = Obj.Member` had two viable conversions -- one to `Code<50>` and one to
  ///       `std::string_view`, which `Code` also assigns from -- and two user-defined conversions
  ///       to two different parameters is ambiguous rather than wrong. Excluding the standard
  ///       spellings leaves exactly the AL type, which is the one AL means.
  template <typename T>
    requires(!std::is_same_v<T, std::string> && !std::is_same_v<T, std::string_view>)
  operator T() const {
    Throw();
  }

  /// \brief Refuses an assignment.
  /// \tparam T The type the caller assigned.
  /// \return Never.
  /// \throws Error always.
  template <typename T> Refused &operator=(const T &) { Throw(); }

  /// \brief Refuses to stand on either side of a `+`.
  ///
  /// \tparam T The other operand's type.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  ///
  /// \note AL CONCATENATES AND ADDS WITH THE SAME OPERATOR, and a refusal has to stand in both.
  ///       Without it `"Profile:" + Absent.ProfileID` was a COMPILE error naming `std::string` and
  ///       `Refused` -- which says nothing about the absent object, and stops a whole translation
  ///       unit over a member that would have refused at run time anyway. Friends rather than
  ///       members, so the refusal reaches the left-hand side too.
  template <typename T> friend Refused operator+(const Refused &left, const T &right) {
    static_cast<void>(right);
    left.Throw();
  }

  /// \brief Refuses a `+` with the refusal on the right.
  /// \tparam T The left operand's type.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  template <typename T> friend Refused operator+(const T &left, const Refused &right) {
    static_cast<void>(left);
    right.Throw();
  }

  /// \brief Refuses to be ordered against anything.
  /// \tparam T The other operand's type.
  /// \param left  The refusal.
  /// \param right The other operand.
  /// \return Never.
  /// \throws Error always.
  /// \note AL COMPARES WHATEVER IT HAS, and a member of an object this run does not carry stands
  ///       in a comparison as readily as in a sum. `<=>` gives all six at once, so the refusal is
  ///       written once rather than six times.
  template <typename T>
  friend std::strong_ordering operator<=>(const Refused &left, const T &right) {
    static_cast<void>(right);
    left.Throw();
  }

  /// \brief Refuses to be compared for equality.
  /// \tparam T The other operand's type.
  /// \param left  The refusal.
  /// \param right The other operand.
  /// \return Never.
  /// \throws Error always.
  template <typename T> friend bool operator==(const Refused &left, const T &right) {
    static_cast<void>(right);
    left.Throw();
  }

private:
  [[noreturn]] void Throw() const {
    throw Error("the .NET member " + std::string(named_.type) + "." + std::string(named_.member) +
                " is named by AL and not rebuilt here (board:0035)");
  }

  Named named_;
};

}
