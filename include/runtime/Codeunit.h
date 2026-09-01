#pragma once

#include "meta/Ids.h"

#include <string_view>

/// \file
/// \brief The base every generated AL codeunit stands on.

/// \brief The platform half of a codeunit run. Not part of the door's vocabulary.
namespace agiru::detail {

/// \brief Refuses a run, naming the codeunit and the item that will make it possible.
/// \param name The codeunit's AL name.
/// \throws Error always.
[[noreturn]] void RefuseRun(std::string_view name);

} // namespace agiru::detail

namespace agiru {

/// \brief WHY AN EVENT PUBLISHER'S BODY IS EMPTY, AND WHY ITS PARAMETERS HAVE NO NAMES.
///
/// A procedure marked `[IntegrationEvent]` or `[BusinessEvent]` is a PUBLISHER. AL gives it an
/// empty body on purpose: the platform does not run the body, it fires every subscriber when the
/// procedure is CALLED. So the dispatch belongs at the call site and the definition genuinely has
/// nothing to do -- which is why the generated definition drops the parameter names. They stay on
/// the DECLARATION, where a reader and a subscriber both need them.
///
/// \note Until event dispatch exists, calling one of these does nothing, and that is the correct
///       behaviour for a publisher with no subscribers -- not a stub. What is missing is the
///       registry that would let there BE a subscriber.

/// \brief The declaration belonging to a generated codeunit.
///
/// \tparam T The generated codeunit class.
///
/// The generator specialises this beside the class, so that the class itself carries nothing but
/// what AL wrote: its procedures and its variables. The number and the name live here, the same way
/// a table's field and key tables do.
template <typename T> struct CodeunitTraits;

/// \brief What every AL codeunit can do, without the generated class saying any of it.
///
/// \tparam Derived The generated codeunit class.
///
/// AL CODE NEVER NAMES A DISPATCHER. It writes `CashFlowCheck.Run(Line)` or
/// `Codeunit.Run(Codeunit::"X", Rec)`, and the platform finds the object, opens a transaction
/// boundary around it and calls its `OnRun` trigger. This base is that platform half.
///
/// \note The base holds NO data, for the same reason `Table` holds none: a generated codeunit is a
///       plain class whose members are exactly the variables its `.al` declares.
template <typename Derived> class Codeunit {
public:
  /// \brief The codeunit's AL number.
  /// \return The number AL declared.
  [[nodiscard]] static constexpr CodeunitId Id() { return CodeunitTraits<Derived>::kId; }

  /// \brief The codeunit's AL name.
  /// \return The name AL declared, spaces and punctuation included.
  [[nodiscard]] static constexpr std::string_view Name() { return CodeunitTraits<Derived>::kName; }

  /// \brief AL `Codeunit.Run()` -- calls `OnRun` inside a transaction boundary.
  ///
  /// \return True when `OnRun` completed; false when it raised.
  ///
  /// \warning NOT IMPLEMENTED, AND IT REFUSES RATHER THAN PRETENDING. `Codeunit.Run` is where AL's
  ///          error handling lives: an error inside rolls the database back to the point the run
  ///          began and returns false, which is why `if not Codeunit.Run(...) then` is the AL idiom
  ///          for "try this" and why `asserterror` can read what happened. Returning true after
  ///          calling `OnRun` directly would make every one of those tests pass for the wrong
  ///          reason. It needs a nested savepoint on the session's pinned connection (board:0021).
  [[nodiscard]] bool Run() { detail::RefuseRun(Name()); }

  /// \brief AL `Codeunit.Run(Record)` -- the same, with a record handed to the object first.
  ///
  /// \tparam Record The record type the codeunit expects.
  /// \param  rec    The record.
  /// \return True when `OnRun` completed; false when it raised.
  ///
  /// \warning Refused for the reason Run() gives (board:0021).
  template <typename Record> [[nodiscard]] bool Run(Record &rec) {
    static_cast<void>(rec);
    detail::RefuseRun(Name());
  }

private:
  /// \brief Only the generated codeunit itself may construct the base.
  ///
  /// Private rather than protected, and the friend is the reason: a protected constructor would let
  /// any class name `Codeunit<Something>` as a base, including one that is not that Something. The
  /// CRTP then reaches into a type it is not part of. This way the pairing is checked.
  constexpr Codeunit() = default;
  friend Derived;
};

} // namespace agiru
