#pragma once

#include "runtime/Error.h"

#include <compare>
#include <concepts>
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
  template <typename... Arguments> Refused operator()(Arguments &&...arguments) const;

  /// \brief The chained member `Groups`, which a body reaches on the RESULT of a refused call.
  /// \brief The chained member `GetString`, which a body reaches on the RESULT of a refused call.
  static Refused GetString;
  static Refused Groups;
  /// \brief The chained member `Captures`, which a body reaches on the RESULT of a refused call.
  static Refused Captures;
  /// \brief The chained member `Item`, which a body reaches on the RESULT of a refused call.
  static Refused Item;
  /// \brief The chained member `Result`, which a body reaches on the RESULT of a refused call.
  static Refused Result;
  /// \brief The chained member `Name`, which a body reaches on the RESULT of a refused call.
  static Refused Name;
  /// \brief The chained member `Value`, which a body reaches on the RESULT of a refused call.
  static Refused Value;
  /// \brief The chained member `Count`, which a body reaches on the RESULT of a refused call.
  static Refused Count;
  /// \brief The chained member `Length`, which a body reaches on the RESULT of a refused call.
  static Refused Length;
  /// \brief The chained member `Reference`, which a body reaches on the RESULT of a refused call.
  static Refused Reference;
  /// \brief The chained member `LastChild`, which a body reaches on the RESULT of a refused call.
  static Refused LastChild;
  /// \brief The chained member `FirstChild`, which a body reaches on the RESULT of a refused call.
  static Refused FirstChild;
  /// \brief The chained member `WorkbookPart`, which a body reaches on the RESULT of a refused
  /// call.
  static Refused WorkbookPart;
  /// \brief The chained member `Workbook`, which a body reaches on the RESULT of a refused call.
  static Refused Workbook;
  /// \brief The chained member `Success`, which a body reaches on the RESULT of a refused call.
  static Refused Success;
  /// \brief The chained member `Index`, which a body reaches on the RESULT of a refused call.
  static Refused Index;
  /// \brief The chained member `Key`, which a body reaches on the RESULT of a refused call.
  static Refused Key;
  /// \brief The chained member `Keys`, which a body reaches on the RESULT of a refused call.
  static Refused Keys;
  /// \brief The chained member `Values`, which a body reaches on the RESULT of a refused call.
  static Refused Values;
  /// \brief The chained member `Attributes`, which a body reaches on the RESULT of a refused call.
  static Refused Attributes;
  /// \brief The chained member `InnerText`, which a body reaches on the RESULT of a refused call.
  static Refused InnerText;
  /// \brief The chained member `OuterXml`, which a body reaches on the RESULT of a refused call.
  static Refused OuterXml;
  /// \brief The chained member `ChildNodes`, which a body reaches on the RESULT of a refused call.
  static Refused ChildNodes;
  /// \brief The chained member `ParentNode`, which a body reaches on the RESULT of a refused call.
  static Refused ParentNode;
  /// \brief The chained member `Text`, which a body reaches on the RESULT of a refused call.
  static Refused Text;
  /// \brief The chained member `ToString`, which a body reaches on the RESULT of a refused call.
  static Refused ToString;
  /// \brief The chained member `GetType`, which a body reaches on the RESULT of a refused call.
  static Refused GetType;
  /// \brief The chained member `Equals`, which a body reaches on the RESULT of a refused call.
  static Refused Equals;
  /// \brief The chained member `Dispose`, which a body reaches on the RESULT of a refused call.
  static Refused Dispose;
  /// \brief The chained member `Expect100Continue`, which a body reaches on the RESULT of a refused
  /// call.
  static Refused Expect100Continue;
  /// \brief The chained member `ContentType`, which a body reaches on the RESULT of a refused call.
  static Refused ContentType;
  /// \brief The chained member `PathAndQuery`, which a body reaches on the RESULT of a refused
  /// call.
  static Refused PathAndQuery;
  /// \brief The chained member `LCID`, which a body reaches on the RESULT of a refused call.
  static Refused LCID;
  /// \brief The chained member `ItemOf`, which a body reaches on the RESULT of a refused call.
  static Refused ItemOf;
  /// \brief The chained member `Id`, which a body reaches on the RESULT of a refused call.
  static Refused Id;
  /// \brief The chained member `Parent`, which a body reaches on the RESULT of a refused call.
  static Refused Parent;
  /// \brief The chained member `ToTitleCase`, which a body reaches on the RESULT of a refused call.
  static Refused ToTitleCase;
  /// \brief The chained member `InnerXml`, which a body reaches on the RESULT of a refused call.
  static Refused InnerXml;
  /// \brief The chained member `ImportStream`, which a body reaches on the RESULT of a refused
  /// call.
  static Refused ImportStream;
  /// \brief The chained member `ImportNode`, which a body reaches on the RESULT of a refused call.
  static Refused ImportNode;
  /// \brief The chained member `HasValue`, which a body reaches on the RESULT of a refused call.
  static Refused HasValue;
  /// \brief The chained member `GetEnumerator`, which a body reaches on the RESULT of a refused
  /// call.
  static Refused GetEnumerator;
  /// \brief The chained member `ExportStream`, which a body reaches on the RESULT of a refused
  /// call.
  static Refused ExportStream;
  /// \brief The chained member `CreateNode`, which a body reaches on the RESULT of a refused call.
  static Refused CreateNode;
  /// \brief The chained member `CreateInStream`, which a body reaches on the RESULT of a refused
  /// call.
  static Refused CreateInStream;
  /// \brief The chained member `CopyTo`, which a body reaches on the RESULT of a refused call.
  static Refused CopyTo;
  /// \brief The chained member `AbsoluteUri`, which a body reaches on the RESULT of a refused call.
  static Refused AbsoluteUri;
  /// \brief The chained member `DataType`, which a body reaches on the RESULT of a refused call.
  static Refused DataType;
  /// \brief The chained member `Clear`, which a body reaches on the RESULT of a refused call.
  static Refused Clear;
  /// \brief The chained member `Add`, which a body reaches on the RESULT of a refused call.
  static Refused Add;
  /// \brief The chained member `Remove`, which a body reaches on the RESULT of a refused call.
  static Refused Remove;
  /// \brief The chained member `Contains`, which a body reaches on the RESULT of a refused call.
  static Refused Contains;
  /// \brief The chained member `Rows`, which a body reaches on the RESULT of a refused call.
  static Refused Rows;
  /// \brief The chained member `Columns`, which a body reaches on the RESULT of a refused call.
  static Refused Columns;
  /// \brief The chained member `TryGetValue`, which a body reaches on the RESULT of a refused call.
  static Refused TryGetValue;
  /// \brief The chained member `IsInline`, which a body reaches on the RESULT of a refused call.
  static Refused IsInline;
  /// \brief The chained member `ToXmlString`, which a body reaches on the RESULT of a refused call.
  static Refused ToXmlString;

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
/// \brief What an AL OBJECT this run does not have still has to answer: the record and
///        codeunit surface a body reaches on it.
///
/// \note A STUB IS NOT ONLY A BAG OF NAMED MEMBERS. A page over an absent table calls
///       `Rec.Find`, a codeunit variable calls `Run`, and neither name appears in the
///       gathered member list because the gatherer only sees `.X` in a BODY. The surface
///       is inherited once rather than emitted 843 times (board:0032, board:0035).
struct AbsentType;

/// \brief Whether a type is a stub for something this runtime does not have.
/// \tparam T The type.
template <typename T>
concept IsAbsent = requires { typename std::remove_cvref_t<T>::IsAnAbsentType; };

/// \brief What every stub for a type this runtime does not have derives from.
///
/// \note IT EXISTS SO THE STUBS INTERCONVERT. AL hands a `JToken` to a parameter declared
///       `JObject`, which is a downcast .NET performs and the stubs cannot -- each is its own
///       struct with no relation to the next, so the call was a hard compile error rather than the
///       refusal everything else about them is. A conversion between two refusals is as refused as
///       either, and the error still arrives at the first member touched.
struct AbsentType {
  /// \brief Marks this as a stub, so one stub converts to another and to nothing else.
  using IsAnAbsentType = void;

  /// \brief Becomes any other stub, because AL downcasts between the types these stand in for.
  /// \tparam T The stub the caller wants.
  /// \return An empty one of those, whose every member refuses the same way this one's do.
  ///
  /// \warning IT IS A CONVERSION OPERATOR AND NOT A CONSTRUCTOR, and that is a language rule
  ///          rather than a preference: a stub carries a member named after its own type often
  ///          enough (`ALAzureAdCodeGrantFlow.ALAzureAdCodeGrantFlow`), and a class with any
  ///          user-declared constructor may not have a data member of its own name.
  template <typename T>
    requires IsAbsent<T>
  [[nodiscard]] operator T() const {
    return T{};
  }
};

struct AbsentObject : AbsentType {
  Refused Find{{.type = "<absent object>", .member = "Find"}};                   ///< The AL member.
  Refused FindFirst{{.type = "<absent object>", .member = "FindFirst"}};         ///< The AL member.
  Refused FindLast{{.type = "<absent object>", .member = "FindLast"}};           ///< The AL member.
  Refused FindSet{{.type = "<absent object>", .member = "FindSet"}};             ///< The AL member.
  Refused Next{{.type = "<absent object>", .member = "Next"}};                   ///< The AL member.
  Refused Get{{.type = "<absent object>", .member = "Get"}};                     ///< The AL member.
  Refused GetBySystemId{{.type = "<absent object>", .member = "GetBySystemId"}}; ///< The AL member.
  Refused Insert{{.type = "<absent object>", .member = "Insert"}};               ///< The AL member.
  Refused Modify{{.type = "<absent object>", .member = "Modify"}};               ///< The AL member.
  Refused Delete{{.type = "<absent object>", .member = "Delete"}};               ///< The AL member.
  Refused DeleteAll{{.type = "<absent object>", .member = "DeleteAll"}};         ///< The AL member.
  Refused ModifyAll{{.type = "<absent object>", .member = "ModifyAll"}};         ///< The AL member.
  Refused Rename{{.type = "<absent object>", .member = "Rename"}};               ///< The AL member.
  Refused Init{{.type = "<absent object>", .member = "Init"}};                   ///< The AL member.
  Refused Reset{{.type = "<absent object>", .member = "Reset"}};                 ///< The AL member.
  Refused SetRange{{.type = "<absent object>", .member = "SetRange"}};           ///< The AL member.
  Refused SetFilter{{.type = "<absent object>", .member = "SetFilter"}};         ///< The AL member.
  Refused GetFilter{{.type = "<absent object>", .member = "GetFilter"}};         ///< The AL member.
  Refused GetFilters{{.type = "<absent object>", .member = "GetFilters"}};       ///< The AL member.
  Refused SetCurrentKey{{.type = "<absent object>", .member = "SetCurrentKey"}}; ///< The AL member.
  Refused SetView{{.type = "<absent object>", .member = "SetView"}};             ///< The AL member.
  Refused GetView{{.type = "<absent object>", .member = "GetView"}};             ///< The AL member.
  Refused SetAscending{{.type = "<absent object>", .member = "SetAscending"}};   ///< The AL member.
  Refused IsEmpty{{.type = "<absent object>", .member = "IsEmpty"}};             ///< The AL member.
  Refused Count{{.type = "<absent object>", .member = "Count"}};                 ///< The AL member.
  Refused CountApprox{{.type = "<absent object>", .member = "CountApprox"}};     ///< The AL member.
  Refused CalcFields{{.type = "<absent object>", .member = "CalcFields"}};       ///< The AL member.
  Refused CalcSums{{.type = "<absent object>", .member = "CalcSums"}};           ///< The AL member.
  Refused TestField{{.type = "<absent object>", .member = "TestField"}};         ///< The AL member.
  Refused FieldError{{.type = "<absent object>", .member = "FieldError"}};       ///< The AL member.
  Refused FieldCaption{{.type = "<absent object>", .member = "FieldCaption"}};   ///< The AL member.
  Refused TableCaption{{.type = "<absent object>", .member = "TableCaption"}};   ///< The AL member.
  Refused FieldName{{.type = "<absent object>", .member = "FieldName"}};         ///< The AL member.
  Refused FieldNo{{.type = "<absent object>", .member = "FieldNo"}};             ///< The AL member.
  Refused Validate{{.type = "<absent object>", .member = "Validate"}};           ///< The AL member.
  Refused TransferFields{
      {.type = "<absent object>", .member = "TransferFields"}};                ///< The AL member.
  Refused CopyFilters{{.type = "<absent object>", .member = "CopyFilters"}};   ///< The AL member.
  Refused CopyFilter{{.type = "<absent object>", .member = "CopyFilter"}};     ///< The AL member.
  Refused SetRecFilter{{.type = "<absent object>", .member = "SetRecFilter"}}; ///< The AL member.
  Refused Mark{{.type = "<absent object>", .member = "Mark"}};                 ///< The AL member.
  Refused MarkedOnly{{.type = "<absent object>", .member = "MarkedOnly"}};     ///< The AL member.
  Refused ClearMarks{{.type = "<absent object>", .member = "ClearMarks"}};     ///< The AL member.
  Refused LockTable{{.type = "<absent object>", .member = "LockTable"}};       ///< The AL member.
  Refused Run{{.type = "<absent object>", .member = "Run"}};                   ///< The AL member.
  Refused RunModal{{.type = "<absent object>", .member = "RunModal"}};         ///< The AL member.
  Refused SetTableView{{.type = "<absent object>", .member = "SetTableView"}}; ///< The AL member.
  Refused SetTempTableView{
      {.type = "<absent object>", .member = "SetTempTableView"}};        ///< The AL member.
  Refused Ascending{{.type = "<absent object>", .member = "Ascending"}}; ///< The AL member.
  Refused RecordId{{.type = "<absent object>", .member = "RecordId"}};   ///< The AL member.
  Refused SystemId{{.type = "<absent object>", .member = "SystemId"}};   ///< The AL member.
  Refused CurrentKeyIndex{
      {.type = "<absent object>", .member = "CurrentKeyIndex"}};                 ///< The AL member.
  Refused ChangeCompany{{.type = "<absent object>", .member = "ChangeCompany"}}; ///< The AL member.
  Refused AddLoadFields{{.type = "<absent object>", .member = "AddLoadFields"}}; ///< The AL member.
  Refused SetLoadFields{{.type = "<absent object>", .member = "SetLoadFields"}}; ///< The AL member.
  Refused LoadFields{{.type = "<absent object>", .member = "LoadFields"}};       ///< The AL member.
  Refused AreFieldsLoaded{
      {.type = "<absent object>", .member = "AreFieldsLoaded"}}; ///< The AL member.
  Refused SetAutoCalcFields{
      {.type = "<absent object>", .member = "SetAutoCalcFields"}};           ///< The AL member.
  Refused HasFilter{{.type = "<absent object>", .member = "HasFilter"}};     ///< The AL member.
  Refused FilterGroup{{.type = "<absent object>", .member = "FilterGroup"}}; ///< The AL member.
  Refused ReadPermission{
      {.type = "<absent object>", .member = "ReadPermission"}}; ///< The AL member.
  Refused WritePermission{
      {.type = "<absent object>", .member = "WritePermission"}};                 ///< The AL member.
  Refused ReadIsolation{{.type = "<absent object>", .member = "ReadIsolation"}}; ///< The AL member.
  Refused SecurityFiltering{
      {.type = "<absent object>", .member = "SecurityFiltering"}};           ///< The AL member.
  Refused IsTemporary{{.type = "<absent object>", .member = "IsTemporary"}}; ///< The AL member.
  Refused Copy{{.type = "<absent object>", .member = "Copy"}};               ///< The AL member.
  Refused Number{{.type = "<absent object>", .member = "Number"}};           ///< The AL member.
  Refused Open{{.type = "<absent object>", .member = "Open"}};               ///< The AL member.
  Refused Close{{.type = "<absent object>", .member = "Close"}};             ///< The AL member.
};

/// \brief What a refused CALL answers with, which is a refusal again.
using RefusedResult = Refused;

inline Refused Refused::GetString{{.type = "<result>", .member = "GetString"}};
inline Refused Refused::Groups{{.type = "<result>", .member = "Groups"}};
inline Refused Refused::Captures{{.type = "<result>", .member = "Captures"}};
inline Refused Refused::Item{{.type = "<result>", .member = "Item"}};
inline Refused Refused::Result{{.type = "<result>", .member = "Result"}};
inline Refused Refused::Name{{.type = "<result>", .member = "Name"}};
inline Refused Refused::Value{{.type = "<result>", .member = "Value"}};
inline Refused Refused::Count{{.type = "<result>", .member = "Count"}};
inline Refused Refused::Length{{.type = "<result>", .member = "Length"}};
inline Refused Refused::Reference{{.type = "<result>", .member = "Reference"}};
inline Refused Refused::LastChild{{.type = "<result>", .member = "LastChild"}};
inline Refused Refused::FirstChild{{.type = "<result>", .member = "FirstChild"}};
inline Refused Refused::WorkbookPart{{.type = "<result>", .member = "WorkbookPart"}};
inline Refused Refused::Workbook{{.type = "<result>", .member = "Workbook"}};
inline Refused Refused::Success{{.type = "<result>", .member = "Success"}};
inline Refused Refused::Index{{.type = "<result>", .member = "Index"}};
inline Refused Refused::Key{{.type = "<result>", .member = "Key"}};
inline Refused Refused::Keys{{.type = "<result>", .member = "Keys"}};
inline Refused Refused::Values{{.type = "<result>", .member = "Values"}};
inline Refused Refused::Attributes{{.type = "<result>", .member = "Attributes"}};
inline Refused Refused::InnerText{{.type = "<result>", .member = "InnerText"}};
inline Refused Refused::OuterXml{{.type = "<result>", .member = "OuterXml"}};
inline Refused Refused::ChildNodes{{.type = "<result>", .member = "ChildNodes"}};
inline Refused Refused::ParentNode{{.type = "<result>", .member = "ParentNode"}};
inline Refused Refused::Text{{.type = "<result>", .member = "Text"}};
inline Refused Refused::ToString{{.type = "<result>", .member = "ToString"}};
inline Refused Refused::GetType{{.type = "<result>", .member = "GetType"}};
inline Refused Refused::Equals{{.type = "<result>", .member = "Equals"}};
inline Refused Refused::Dispose{{.type = "<result>", .member = "Dispose"}};
inline Refused Refused::Expect100Continue{{.type = "<result>", .member = "Expect100Continue"}};
inline Refused Refused::ContentType{{.type = "<result>", .member = "ContentType"}};
inline Refused Refused::PathAndQuery{{.type = "<result>", .member = "PathAndQuery"}};
inline Refused Refused::LCID{{.type = "<result>", .member = "LCID"}};
inline Refused Refused::ItemOf{{.type = "<result>", .member = "ItemOf"}};
inline Refused Refused::Id{{.type = "<result>", .member = "Id"}};
inline Refused Refused::Parent{{.type = "<result>", .member = "Parent"}};
inline Refused Refused::ToTitleCase{{.type = "<result>", .member = "ToTitleCase"}};
inline Refused Refused::InnerXml{{.type = "<result>", .member = "InnerXml"}};
inline Refused Refused::ImportStream{{.type = "<result>", .member = "ImportStream"}};
inline Refused Refused::ImportNode{{.type = "<result>", .member = "ImportNode"}};
inline Refused Refused::HasValue{{.type = "<result>", .member = "HasValue"}};
inline Refused Refused::GetEnumerator{{.type = "<result>", .member = "GetEnumerator"}};
inline Refused Refused::ExportStream{{.type = "<result>", .member = "ExportStream"}};
inline Refused Refused::CreateNode{{.type = "<result>", .member = "CreateNode"}};
inline Refused Refused::CreateInStream{{.type = "<result>", .member = "CreateInStream"}};
inline Refused Refused::CopyTo{{.type = "<result>", .member = "CopyTo"}};
inline Refused Refused::AbsoluteUri{{.type = "<result>", .member = "AbsoluteUri"}};
inline Refused Refused::DataType{{.type = "<result>", .member = "DataType"}};
inline Refused Refused::Clear{{.type = "<result>", .member = "Clear"}};
inline Refused Refused::Add{{.type = "<result>", .member = "Add"}};
inline Refused Refused::Remove{{.type = "<result>", .member = "Remove"}};
inline Refused Refused::Contains{{.type = "<result>", .member = "Contains"}};
inline Refused Refused::Rows{{.type = "<result>", .member = "Rows"}};
inline Refused Refused::Columns{{.type = "<result>", .member = "Columns"}};
inline Refused Refused::TryGetValue{{.type = "<result>", .member = "TryGetValue"}};
inline Refused Refused::IsInline{{.type = "<result>", .member = "IsInline"}};
inline Refused Refused::ToXmlString{{.type = "<result>", .member = "ToXmlString"}};

template <typename... Arguments> Refused Refused::operator()(Arguments &&...arguments) const {
  (static_cast<void>(arguments), ...);
  Throw();
}

}
