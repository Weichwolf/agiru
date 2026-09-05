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
struct RefusedResult;

class Refused {
public:
  /// \brief Marks this as a REFUSAL rather than a value, for whoever has to keep out of its way.
  ///
  /// \note TWO REFUSALS COMPARED TO EACH OTHER WERE AMBIGUOUS. `Absent.Status = Rec.Type::X` puts
  ///       a refused .NET member beside an absent enumeration, and both types answer every
  ///       operator, so neither reading was better. The other refusal looks for this marker and
  ///       stands aside; this one then refuses, which is what both would have done.
  using IsAlRefusal = void;

  /// \brief A member of a .NET type that is not rebuilt.
  /// \param named The type and the member, in one value so they cannot be swapped.
  constexpr explicit Refused(Named named) : named_(named) {}

  /// \brief Refuses a call of any shape.
  /// \tparam Arguments Whatever the caller passed.
  /// \param arguments The arguments, read only to be discarded -- what is refused is the CALL.
  /// \return Never.
  /// \throws Error always.
  template <typename... Arguments> RefusedResult operator()(Arguments &&...arguments) const;

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

  /// \brief Refuses to be walked with `foreach`.
  /// \return Never.
  /// \throws Error always.
  ///
  /// \note AL WALKS A .NET COLLECTION WITH `foreach`, and without a `begin` the diagnostic named
  ///       the range rather than the member -- `foreach Str in DotNetString.Split(...)` stopped a
  ///       translation unit over a member that would have refused at run time anyway.
  [[noreturn]] const Refused *begin() const { Throw(); }

  /// \brief The other half of the range, which is never reached.
  /// \return Never.
  /// \throws Error always.
  [[noreturn]] const Refused *end() const { Throw(); }

  /// \brief Refuses to be rendered as text.
  /// \return Never.
  /// \throws Error always.
  ///
  /// \note IT EXISTS SO THAT `StrSubstNo` REFUSES RATHER THAN FAILING TO COMPILE. `AsText` asks
  ///       for a `ToText()` before it reaches `Format`, and without one a message that names a
  ///       .NET member is a compile error inside a template -- a diagnostic pointing at the
  ///       runtime instead of at the AL line that wrote it.
  [[nodiscard]] std::string ToText() const { Throw(); }

  /// \brief AL `-Query.Column`, `not Query.Column`: a unary operator on a refused member refuses
  ///        the same way, when it is reached.
  /// \return Never.
  [[noreturn]] Refused operator-() const { Throw(); }

  /// \copydoc operator-
  [[noreturn]] Refused operator!() const { Throw(); }

  /// \copydoc operator-
  [[noreturn]] Refused operator+() const { Throw(); }

  /// \brief Refuses to state a declared length, which `MaxStrLen` asks of anything text-like.
  /// \return Never.
  /// \throws Error always.
  [[nodiscard]] std::size_t Max() const { Throw(); }

  /// \brief Refuses an assignment.
  /// \tparam T The type the caller assigned.
  /// \return Never.
  /// \throws Error always.
  template <typename T> Refused &operator=(const T &) { Throw(); }

  /// \brief Refuses to stand on either side of `a + b`.
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
  template <typename T>
    requires(!std::is_base_of_v<Refused, std::remove_cvref_t<T>>)
  friend Refused operator+(const Refused &left, const T &right) {
    static_cast<void>(right);
    left.Throw();
  }

  /// \brief Refuses `a + b` with the refusal on the right.
  /// \tparam T The left operand's type.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  template <typename T>
    requires(!std::is_base_of_v<Refused, std::remove_cvref_t<T>>)
  friend Refused operator+(const T &left, const Refused &right) {
    static_cast<void>(left);
    right.Throw();
  }

  /// \brief Refuses `a + b` between two refusals.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  ///
  /// \note IT IS THE EXACT OVERLOAD THAT KEEPS THE PAIR ABOVE UNAMBIGUOUS. With only the two
  ///       templates, `Refused - Refused` matched both equally well and the diagnostic named the
  ///       operator rather than the .NET member behind it.
  friend Refused operator+(const Refused &left, const Refused &right) {
    static_cast<void>(right);
    left.Throw();
  }

  /// \brief Refuses to stand on either side of `a - b`.
  /// \tparam T The other operand's type.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  template <typename T>
    requires(!std::is_base_of_v<Refused, std::remove_cvref_t<T>>)
  friend Refused operator-(const Refused &left, const T &right) {
    static_cast<void>(right);
    left.Throw();
  }

  /// \brief Refuses `a - b` with the refusal on the right.
  /// \tparam T The left operand's type.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  template <typename T>
    requires(!std::is_base_of_v<Refused, std::remove_cvref_t<T>>)
  friend Refused operator-(const T &left, const Refused &right) {
    static_cast<void>(left);
    right.Throw();
  }

  /// \brief Refuses `a - b` between two refusals.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  ///
  /// \note IT IS THE EXACT OVERLOAD THAT KEEPS THE PAIR ABOVE UNAMBIGUOUS. With only the two
  ///       templates, `Refused - Refused` matched both equally well and the diagnostic named the
  ///       operator rather than the .NET member behind it.
  friend Refused operator-(const Refused &left, const Refused &right) {
    static_cast<void>(right);
    left.Throw();
  }

  /// \brief Refuses to stand on either side of `a * b`.
  /// \tparam T The other operand's type.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  template <typename T>
    requires(!std::is_base_of_v<Refused, std::remove_cvref_t<T>>)
  friend Refused operator*(const Refused &left, const T &right) {
    static_cast<void>(right);
    left.Throw();
  }

  /// \brief Refuses `a * b` with the refusal on the right.
  /// \tparam T The left operand's type.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  template <typename T>
    requires(!std::is_base_of_v<Refused, std::remove_cvref_t<T>>)
  friend Refused operator*(const T &left, const Refused &right) {
    static_cast<void>(left);
    right.Throw();
  }

  /// \brief Refuses `a * b` between two refusals.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  ///
  /// \note IT IS THE EXACT OVERLOAD THAT KEEPS THE PAIR ABOVE UNAMBIGUOUS. With only the two
  ///       templates, `Refused - Refused` matched both equally well and the diagnostic named the
  ///       operator rather than the .NET member behind it.
  friend Refused operator*(const Refused &left, const Refused &right) {
    static_cast<void>(right);
    left.Throw();
  }

  /// \brief Refuses to stand on either side of `a / b`.
  /// \tparam T The other operand's type.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  template <typename T>
    requires(!std::is_base_of_v<Refused, std::remove_cvref_t<T>>)
  friend Refused operator/(const Refused &left, const T &right) {
    static_cast<void>(right);
    left.Throw();
  }

  /// \brief Refuses `a / b` with the refusal on the right.
  /// \tparam T The left operand's type.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  template <typename T>
    requires(!std::is_base_of_v<Refused, std::remove_cvref_t<T>>)
  friend Refused operator/(const T &left, const Refused &right) {
    static_cast<void>(left);
    right.Throw();
  }

  /// \brief Refuses `a / b` between two refusals.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  ///
  /// \note IT IS THE EXACT OVERLOAD THAT KEEPS THE PAIR ABOVE UNAMBIGUOUS. With only the two
  ///       templates, `Refused - Refused` matched both equally well and the diagnostic named the
  ///       operator rather than the .NET member behind it.
  friend Refused operator/(const Refused &left, const Refused &right) {
    static_cast<void>(right);
    left.Throw();
  }

  /// \brief Refuses to stand on either side of `a < b`.
  /// \tparam T The other operand's type.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  template <typename T>
    requires(!std::is_base_of_v<Refused, std::remove_cvref_t<T>>)
  friend std::strong_ordering operator<=>(const Refused &left, const T &right) {
    static_cast<void>(right);
    left.Throw();
  }

  /// \brief Refuses `a < b` with the refusal on the right.
  /// \tparam T The left operand's type.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  template <typename T>
    requires(!std::is_base_of_v<Refused, std::remove_cvref_t<T>>)
  friend std::strong_ordering operator<=>(const T &left, const Refused &right) {
    static_cast<void>(left);
    right.Throw();
  }

  /// \brief Refuses `a < b` between two refusals.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  ///
  /// \note IT IS THE EXACT OVERLOAD THAT KEEPS THE PAIR ABOVE UNAMBIGUOUS. With only the two
  ///       templates, `Refused - Refused` matched both equally well and the diagnostic named the
  ///       operator rather than the .NET member behind it.
  friend std::strong_ordering operator<=>(const Refused &left, const Refused &right) {
    static_cast<void>(right);
    left.Throw();
  }

  /// \brief Refuses to stand on either side of `a = b`.
  /// \tparam T The other operand's type.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  template <typename T>
    requires(!std::is_base_of_v<Refused, std::remove_cvref_t<T>>)
  friend bool operator==(const Refused &left, const T &right) {
    static_cast<void>(right);
    left.Throw();
  }

  /// \brief Refuses `a = b` with the refusal on the right.
  /// \tparam T The left operand's type.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  template <typename T>
    requires(!std::is_base_of_v<Refused, std::remove_cvref_t<T>>)
  friend bool operator==(const T &left, const Refused &right) {
    static_cast<void>(left);
    right.Throw();
  }

  /// \brief Refuses `a = b` between two refusals.
  /// \param left  The left operand.
  /// \param right The right operand.
  /// \return Never.
  /// \throws Error always.
  ///
  /// \note IT IS THE EXACT OVERLOAD THAT KEEPS THE PAIR ABOVE UNAMBIGUOUS. With only the two
  ///       templates, `Refused - Refused` matched both equally well and the diagnostic named the
  ///       operator rather than the .NET member behind it.
  friend bool operator==(const Refused &left, const Refused &right) {
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

/// \brief What a refused .NET call RETURNS: a value that refuses everything, and that carries the
///        members AL reaches next in the same expression.
///
/// \note `Match.Groups.Item(i).Captures.Count` is one AL expression over four .NET members, and
///       the transpiler names only the FIRST on the stub type; the rest hang off the result of a
///       call it has already refused. So the result carries the members BCApps reaches that way
///       (measured 2026-09-05 over the UT census), each a refusal of its own, and a name outside
///       this list is a compile error naming the member -- which is the census's next entry.
struct RefusedResult : Refused {
  using Refused::Refused;
  Refused Groups{{.type = "<result>", .member = "Groups"}};             ///< The chained member.
  Refused Captures{{.type = "<result>", .member = "Captures"}};         ///< The chained member.
  Refused Item{{.type = "<result>", .member = "Item"}};                 ///< The chained member.
  Refused Result{{.type = "<result>", .member = "Result"}};             ///< The chained member.
  Refused Name{{.type = "<result>", .member = "Name"}};                 ///< The chained member.
  Refused Value{{.type = "<result>", .member = "Value"}};               ///< The chained member.
  Refused Count{{.type = "<result>", .member = "Count"}};               ///< The chained member.
  Refused Length{{.type = "<result>", .member = "Length"}};             ///< The chained member.
  Refused Reference{{.type = "<result>", .member = "Reference"}};       ///< The chained member.
  Refused LastChild{{.type = "<result>", .member = "LastChild"}};       ///< The chained member.
  Refused FirstChild{{.type = "<result>", .member = "FirstChild"}};     ///< The chained member.
  Refused WorkbookPart{{.type = "<result>", .member = "WorkbookPart"}}; ///< The chained member.
  Refused Workbook{{.type = "<result>", .member = "Workbook"}};         ///< The chained member.
  Refused Success{{.type = "<result>", .member = "Success"}};           ///< The chained member.
  Refused Index{{.type = "<result>", .member = "Index"}};               ///< The chained member.
  Refused Key{{.type = "<result>", .member = "Key"}};                   ///< The chained member.
  Refused Keys{{.type = "<result>", .member = "Keys"}};                 ///< The chained member.
  Refused Values{{.type = "<result>", .member = "Values"}};             ///< The chained member.
  Refused Attributes{{.type = "<result>", .member = "Attributes"}};     ///< The chained member.
  Refused InnerText{{.type = "<result>", .member = "InnerText"}};       ///< The chained member.
  Refused OuterXml{{.type = "<result>", .member = "OuterXml"}};         ///< The chained member.
  Refused ChildNodes{{.type = "<result>", .member = "ChildNodes"}};     ///< The chained member.
  Refused ParentNode{{.type = "<result>", .member = "ParentNode"}};     ///< The chained member.
  Refused Text{{.type = "<result>", .member = "Text"}};                 ///< The chained member.
  Refused ToString{{.type = "<result>", .member = "ToString"}};         ///< The chained member.
  Refused GetType{{.type = "<result>", .member = "GetType"}};           ///< The chained member.
  Refused Equals{{.type = "<result>", .member = "Equals"}};             ///< The chained member.
  Refused Dispose{{.type = "<result>", .member = "Dispose"}};           ///< The chained member.
};

template <typename... Arguments> RefusedResult Refused::operator()(Arguments &&...arguments) const {
  (static_cast<void>(arguments), ...);
  Throw();
}

}
