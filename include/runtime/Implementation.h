#pragma once

#include "runtime/Error.h"
#include "type/Enum.h"

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
  Implementation(Implementation &&o) noexcept : held_(o.held_) { o.held_ = nullptr; }

  /// \brief Takes the other's codeunit.
  /// \param o The other.
  /// \return This variable.
  Implementation &operator=(Implementation &&o) noexcept {
    if (this != &o) {
      Forget();
      held_ = o.held_;
      o.held_ = nullptr;
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
  void Forget() {
    delete held_;
    held_ = nullptr;
  }

  I *held_ = nullptr;
};

}
