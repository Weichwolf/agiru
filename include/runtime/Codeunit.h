#pragma once

#include "meta/Ids.h"
#include "runtime/Error.h"
#include "runtime/Transaction.h"

#include <string_view>

/// \file
/// \brief The base every generated AL codeunit stands on.

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

/// \brief One codeunit held by another, created the first time it is used.
///
/// \tparam T The generated codeunit class, which may be INCOMPLETE here.
///
/// \note IT IS LAZY BECAUSE AL IS. `Background Error Handling Mgt.` holds an
///       `Item Journal Errors Mgt.` and that codeunit holds the first one back -- ordinary AL, and
///       an eager pairing that would not terminate. So a codeunit variable is not a value that
///       exists when its holder does; it is an instance the platform makes on demand, and this is
///       that (board:0037).
///
/// \note THE FREEING FUNCTION IS CAPTURED WHERE THE INSTANCE IS MADE, which is what lets the
///       destructor work on an incomplete type. `delete` needs the definition; a function pointer
///       taken in `operator->` -- instantiated only where `T` is complete -- does not. That is the
///       whole reason a generated header can forward declare what it holds and the include cycle
///       disappears with the containment cycle.
///
/// \warning A COPY HOLDS NOTHING, AND THAT IS AL'S OWN ANSWER. Two codeunit variables in AL are two
///          instances, so a copy must not share the pointer -- but it must EXIST, because a RECORD
///          is copied constantly (`Rec2 := Rec`, every by-value parameter) and a table with a `var`
///          block holds one of these. AL copies a record's FIELDS; its object variables are the
///          copy's own and are made on ITS first use. 412 generated tables carry a `Var_Block`, and
///          a deleted copy constructor made every one of them uncopyable.
template <typename T> class Instance {
public:
  /// \brief An instance that has not been made yet.
  Instance() = default;

  /// \brief A copy that has made nothing yet.
  Instance(const Instance &) {}

  /// \brief Lets go of what this one made; the other's instance is not shared.
  /// \return This handle.
  Instance &operator=(const Instance &) {
    Release();
    return *this;
  }

  /// \brief Takes over another's instance.
  /// \param other The one to take from.
  Instance(Instance &&other) noexcept : held_(other.held_), free_(other.free_) {
    other.held_ = nullptr;
    other.free_ = nullptr;
  }

  /// \brief Takes over another's instance.
  /// \param other The one to take from.
  /// \return This one.
  Instance &operator=(Instance &&other) noexcept {
    if (this != &other) {
      Release();
      held_ = other.held_;
      free_ = other.free_;
      other.held_ = nullptr;
      other.free_ = nullptr;
    }
    return *this;
  }

  /// \brief Frees the instance, if one was ever made.
  ~Instance() { Release(); }

  /// \brief The instance, made on the first call.
  /// \return A pointer to it.
  T *operator->() { return Made(); }

  /// \brief The instance, made on the first call.
  /// \return A pointer to it.
  const T *operator->() const { return const_cast<Instance *>(this)->Made(); }

  /// \brief The instance, made on the first call.
  /// \return A reference to it.
  T &operator*() { return *Made(); }

  /// \brief The instance, made on the first call.
  ///
  /// \return A reference to it.
  ///
  /// \note IT CONVERTS IMPLICITLY BECAUSE AL PASSES THE OBJECT ITSELF. `Copy(TempBuffer, true)`
  ///       hands a record to a parameter that takes a record; the handle is agiru's way of
  ///       DECLARING the member, not a thing AL code ever mentions, so it must disappear at every
  ///       use but the one C++ cannot hide -- reaching through it, which is `->`.
  operator T &() { return *Made(); }

private:
  T *Made() {
    if (held_ == nullptr) {
      held_ = new T();
      free_ = [](void *held) { delete static_cast<T *>(held); };
    }
    return held_;
  }

  void Release() {
    if (free_ != nullptr) { free_(held_); }
    held_ = nullptr;
    // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks): see above.
    free_ = nullptr;
  }

  T *held_ = nullptr;
  void (*free_)(void *) = nullptr;
};

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
// NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility): see runtime/Table.h.
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
  /// \warning THE RETURN VALUE IS THE ERROR HANDLING. `Codeunit.Run` does not propagate: an error
  ///          inside rolls the database back to the point the run began and reports `false`, which
  ///          is why `if not Codeunit.Run(...) then` is AL's idiom for "try this". The text is left
  ///          where `GetLastErrorText()` reads it.
  [[nodiscard]] bool Run() {
    detail::Scope scope;
    try {
      static_cast<Derived *>(this)->OnRun();
    } catch (const Error &e) {
      scope.Discard(e.what());
      return false;
    }
    scope.Keep();
    return true;
  }

  /// \brief AL `Codeunit.Run(Record)` -- the same, with a record handed to the object first.
  ///
  /// \tparam Record The record type the codeunit expects.
  /// \param  rec    The record.
  /// \return True when `OnRun` completed; false when it raised.
  ///
  /// \warning Reports rather than propagates, for the reason Run() gives.
  template <typename Record> [[nodiscard]] bool Run(Record &rec) {
    detail::Scope scope;
    try {
      static_cast<Derived *>(this)->OnRun(rec);
    } catch (const Error &e) {
      scope.Discard(e.what());
      return false;
    }
    scope.Keep();
    return true;
  }

private:
  friend Derived;
};

} // namespace agiru
