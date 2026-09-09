#pragma once

#include "runtime/Error.h"
#include "type/Enum.h"

#include <concepts>
#include <string>
#include <utility>

/// \file
/// \brief AL `Interface` -- a variable that holds whichever codeunit an enum value names.

namespace agiru {

/// \brief AL `Interface <I>` -- the codeunit an enum value bound to it.
///
/// \tparam I The generated interface class.
///
/// \note THE ENUM DECIDES WHICH CODEUNIT, AND AL SAYS SO IN DATA. `enum 800 "X" implements "Y"`
///       gives each value an `Implementation = "Y" = "Z";`, so assigning the value to an interface
///       variable is a LOOKUP and not a cast. The generated enum's header carries that lookup as
///       `ImplementationOf`, found by argument-dependent lookup, which is why this template names
///       no enum and no codeunit.
///
/// \note A COPY HOLDS NOTHING, for the reason `Instance` does: two AL variables are two variables,
///       and a shared pointer would let one free what the other reads.
template <typename I> class Implementation {
public:
  /// \brief A variable nothing has been assigned to.
  Implementation() = default;

  /// \brief A copy holds nothing of its own.
  /// \param o The other, whose codeunit is not shared.
  Implementation(const Implementation &o) { static_cast<void>(o); }

  /// \brief Lets go of this one's codeunit.
  /// \param o The other, whose codeunit is not shared.
  /// \return This variable.
  Implementation &operator=(const Implementation &o) {
    if (this != &o) { Forget(); }
    return *this;
  }

  /// \brief Takes the other's codeunit.
  /// \param o The other.
  Implementation(Implementation &&o) noexcept : held_(o.held_), free_(o.free_) {
    o.held_ = nullptr;
    o.free_ = nullptr;
  }

  /// \brief Takes the other's codeunit.
  /// \param o The other.
  /// \return This variable.
  Implementation &operator=(Implementation &&o) noexcept {
    if (this != &o) {
      Forget();
      held_ = o.held_;
      free_ = o.free_;
      o.held_ = nullptr;
      o.free_ = nullptr;
    }
    return *this;
  }

  /// \brief Frees the codeunit, if one was bound.
  ~Implementation() { Forget(); }

  /// \brief AL `Variable := <enum value>` -- binds the codeunit that value names.
  ///
  /// \tparam E The enumeration, which must name an implementation for this interface.
  /// \param value The value.
  /// \return This variable.
  template <typename E>
    requires requires(E v) { ImplementationOf(v, static_cast<I *>(nullptr)); }
  Implementation &operator=(E value) {
    Forget();
    held_ = ImplementationOf(value, static_cast<I *>(nullptr));
    free_ = [](I *held) { delete held; };
    return *this;
  }

  /// \brief AL `Variable := <enum field>` -- the same, for a value still in its `Enum` wrapper.
  ///
  /// \tparam E The enumeration.
  /// \param value The value.
  /// \return This variable.
  ///
  /// \note A FIELD CARRIES ITS `Enum<E>` AND A LITERAL DOES NOT. `X := GLSetup."Document Retention
  ///       Period"` hands the wrapper, `X := DocsRetentionPeriodDef::Default` the bare value, and
  ///       AL writes both.
  template <typename E>
    requires requires(E v) { ImplementationOf(v, static_cast<I *>(nullptr)); }
  Implementation &operator=(const Enum<E> &value) {
    return *this = value.Value();
  }

  /// \brief AL `IPrice := PriceCodeunit` -- an interface variable assigned a codeunit that
  ///        implements the interface refers to THAT instance from now on, and does not own it.
  /// \tparam C The codeunit, derived from the interface.
  /// \param codeunit The instance the caller holds.
  /// \brief AL returns a CODEUNIT where an Interface is declared: `exit(Result)` from a procedure
  ///        returning `Interface "ISFTP File"`. The local dies with the frame, so the interface
  ///        takes its own copy of the instance, state and all.
  /// \tparam C The codeunit's class, an implementation of `I`.
  /// \param codeunit The instance copied.
  template <typename C>
    requires std::derived_from<C, I> && (!std::same_as<C, I>)
  Implementation(const C &codeunit)
      : held_(new C(codeunit)), free_([](I *held) { delete static_cast<C *>(held); }) {}

  /// \brief AL `Impl := CodeunitVar`: the interface takes its OWN COPY of the instance.
  ///
  /// \warning IT WAS A BORROWED REFERENCE, and `SalesLine.GetLineWithPrice` assigns a LOCAL
  ///          codeunit and returns: the reference outlived its frame and the next call through
  ///          the interface read a dead vtable (SIGSEGV, API Setup UT and two more, 2026-09-09).
  ///          BC keeps a codeunit instance alive as long as anything refers to it; a copy is the
  ///          nearest this runtime has without shared instances, and what diverges is state the
  ///          variable changes AFTER the assignment -- which the BaseApp does not rely on here.
  template <typename C>
    requires std::derived_from<C, I> && (!std::same_as<C, I>)
  Implementation &operator=(const C &codeunit) {
    Forget();
    held_ = new C(codeunit);
    free_ = [](I *held) { delete static_cast<C *>(held); };
    return *this;
  }

  /// \brief AL `exit(Rec.Implementation)` from a procedure returning the interface: an enum
  ///        value becomes the interface variable it names, the same way the assignment does.
  /// \tparam E The enumeration that implements the interface.
  /// \param value The value.
  template <typename E>
    requires requires(E v) { ImplementationOf(v, static_cast<I *>(nullptr)); }
  Implementation(const Enum<E> &value) {
    *this = value;
  }

  /// \brief The same from a bare enumerator.
  /// \tparam E The enumeration.
  /// \param value The enumerator.
  template <typename E>
    requires requires(E v) { ImplementationOf(v, static_cast<I *>(nullptr)); }
  Implementation(E value) {
    *this = value;
  }

  /// \brief The codeunit bound to this variable.
  /// \return It.
  /// \throws Error when nothing was assigned, which AL calls "the interface is not initialized".
  I *operator->() const {
    if (held_ == nullptr) {
      throw Error("this interface variable has no implementation assigned yet");
    }
    return held_;
  }

  /// \brief The codeunit bound to this variable.
  /// \return It.
  /// \throws Error when nothing was assigned.
  I &operator*() const { return *operator->(); }

private:
  /// THE FREEING FUNCTION IS CAPTURED WHERE THE IMPLEMENTATION IS MADE, the way `Instance<T>`
  /// does it: a holder may see the interface only forward-declared, and `delete` on an incomplete
  /// type is what clang-19 refuses (`-Wdelete-incomplete`, Reminder Action, 2026-09-09).
  void Forget() {
    if (held_ != nullptr && free_ != nullptr) { free_(held_); }
    held_ = nullptr;
    free_ = nullptr;
  }

  I *held_ = nullptr;
  void (*free_)(I *) = nullptr;
};

}
