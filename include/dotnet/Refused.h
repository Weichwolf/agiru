#pragma once

#include "runtime/Error.h"

#include <string>
#include <string_view>

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

class Refused {
public:
  /// \brief A member of a .NET type that is not rebuilt.
  /// \param named The type and the member, in one value so they cannot be swapped.
  constexpr explicit Refused(Named named) : named_(named) {}

  /// \brief Refuses a call of any shape.
  /// \tparam Arguments Whatever the caller passed.
  /// \return Never.
  /// \throws Error always.
  template <typename... Arguments> Refused operator()(Arguments &&.../*unused*/) const { Throw(); }

  /// \brief Refuses to become a value of any type.
  /// \tparam T The type the caller wants.
  /// \return Never.
  /// \throws Error always.
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions) -- the conversion must
  // be implicit or `Text x := Obj.Member` would not compile, which is the shape AL writes.
  template <typename T> operator T() const { Throw(); }

  /// \brief Refuses an assignment.
  /// \tparam T The type the caller assigned.
  /// \return Never.
  /// \throws Error always.
  template <typename T> Refused &operator=(const T & /*unused*/) { Throw(); }

private:
  [[noreturn]] void Throw() const {
    throw Error("the .NET member " + std::string(named_.type) + "." + std::string(named_.member) +
                " is named by AL and not rebuilt here (board:0035)");
  }

  Named named_;
};

} // namespace agiru::dotnet
