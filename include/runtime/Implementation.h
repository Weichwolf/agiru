#pragma once

#include "runtime/Error.h"
#include "type/Enum.h"

#include <concepts>
#include <cstdint>
#include <string>
#include <string_view>
#include <typeinfo>
#include <utility>

/// \file
/// \brief AL `Interface` -- a variable that holds whichever codeunit an enum value names.

namespace agiru {

namespace detail {

/// \brief How another app makes the codeunit one of its `enumextension` values names.
///
/// \note AN ENUM EXTENSION LIVES IN THE EXTENDING APP, AND SO DOES ITS IMPLEMENTATION. `Mock
///       Source Type - Locations` extends `Price Source Type` with `Test_Location`, implemented by
///       `Mock Price Source - Location` -- a codeunit of the TEST app, which the Base Application's
///       library may not name (an app is a library and the linker holds AL's direction). So the
///       enum's own `ImplementationOf` switch carries what its app can see, and a value an
///       extension declared is registered by the extension's app when it loads, the way a table
///       registers itself in the catalogue. Both functions hand the instance as its INTERFACE
///       pointer, cast to `void *`, because this registry names no interface type.
struct ForeignImplementation {
  void *(*make)();              ///< A new instance, as the interface pointer.
  void *(*clone)(const void *); ///< A copy of one, handed and returned as the interface pointer.
};

/// \brief Puts an extension's implementation into the registry the enum's `default:` consults.
/// \param enumName      The extended enum's AL name.
/// \param ordinal       The value's ordinal.
/// \param interfaceName The interface's AL name.
/// \param held          The dynamic type of the codeunit, for the clone lookup.
/// \param made          The two functions.
void RegisterImplementation(std::string_view enumName,
                            std::int32_t ordinal,
                            std::string_view interfaceName,
                            const std::type_info &held,
                            ForeignImplementation made);

/// \brief The implementation an extension registered for a value, or nothing.
/// \param enumName      The enum's AL name, compared without regard to case.
/// \param ordinal       The value's ordinal.
/// \param interfaceName The interface's AL name, compared the same way.
/// \return The registration, or `nullptr` when no app registered one.
[[nodiscard]] const ForeignImplementation *
FindImplementation(std::string_view enumName, std::int32_t ordinal, std::string_view interfaceName);

/// \brief A copy of a registered implementation, found by the instance's dynamic type -- which is
///        all a clone function of the interface's own signature has in hand.
/// \param held          The dynamic type of the instance (`typeid(*pointer)`).
/// \param interfaceName The interface's AL name.
/// \param instance      The instance, as the interface pointer.
/// \return The copy, as the interface pointer.
/// \throws Error when no app registered that type for that interface.
[[nodiscard]] void *
CloneForeign(const std::type_info &held, std::string_view interfaceName, const void *instance);

/// \brief Registers by existing, the way `RegisterTable` does: the generated source of an enum
///        extension declares one per value and interface, and the app's load runs it.
struct RegisterForeignImplementation {
  /// \brief Registers.
  /// \param enumName      The extended enum's AL name.
  /// \param ordinal       The value's ordinal.
  /// \param interfaceName The interface's AL name.
  /// \param held          The codeunit's dynamic type.
  /// \param made          The two functions.
  RegisterForeignImplementation(std::string_view enumName,
                                std::int32_t ordinal,
                                std::string_view interfaceName,
                                const std::type_info &held,
                                ForeignImplementation made) {
    RegisterImplementation(enumName, ordinal, interfaceName, held, made);
  }

  RegisterForeignImplementation(const RegisterForeignImplementation &) = delete;
  RegisterForeignImplementation(RegisterForeignImplementation &&) = delete;
  RegisterForeignImplementation &operator=(const RegisterForeignImplementation &) = delete;
  RegisterForeignImplementation &operator=(RegisterForeignImplementation &&) = delete;
  ~RegisterForeignImplementation() = default;
};

}

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

  /// \brief A by-value interface PARAMETER refers to the instance the caller holds, so the copy
  ///        carries the instance: `PriceCalculationMgt.GetHandler(LineWithPrice, ...)` takes the
  ///        interface by value and calls through it. A copy that held nothing answered "no
  ///        implementation assigned" on every V15 price line (ERM Document Totals UT,
  ///        2026-09-09). It is a clone until board:0640 shares the instance.
  /// \param o The other, whose instance is cloned.
  Implementation(const Implementation &o)
      : held_(o.held_ == nullptr ? nullptr : o.clone_(o.held_)), clone_(o.clone_), free_(o.free_) {}

  /// \brief Replaces this one's instance with a clone of the other's.
  /// \param o The other.
  /// \return This variable.
  Implementation &operator=(const Implementation &o) {
    if (this != &o) {
      Implementation copy(o);
      *this = std::move(copy);
    }
    return *this;
  }

  /// \brief Takes the other's codeunit.
  /// \param o The other.
  Implementation(Implementation &&o) noexcept : held_(o.held_), clone_(o.clone_), free_(o.free_) {
    o.held_ = nullptr;
    o.clone_ = nullptr;
    o.free_ = nullptr;
  }

  /// \brief Takes the other's codeunit.
  /// \param o The other.
  /// \return This variable.
  Implementation &operator=(Implementation &&o) noexcept {
    if (this != &o) {
      Forget();
      held_ = o.held_;
      clone_ = o.clone_;
      free_ = o.free_;
      o.held_ = nullptr;
      o.clone_ = nullptr;
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
    clone_ = CloneOf(value, static_cast<I *>(nullptr));
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
      : held_(new C(codeunit)),
        clone_([](const I *held) -> I * { return new C(*dynamic_cast<const C *>(held)); }),
        free_([](I *held) { delete dynamic_cast<C *>(held); }) {}

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
    clone_ = [](const I *held) -> I * { return new C(*dynamic_cast<const C *>(held)); };
    free_ = [](I *held) { delete dynamic_cast<C *>(held); };
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

  /// \brief AL `Variable is Interface`: whether the implementation behind this variable also
  ///        implements the other interface.
  /// \tparam J The other interface's class.
  /// \return Whether it does; false when nothing is assigned.
  /// \note IT ASKS THE OBJECT AND NOT THE VARIABLE. A codeunit implementing two interfaces
  ///       derives from both, so the question is the object's own type.
  template <typename J> [[nodiscard]] ::agiru::Boolean Is() const {
    return held_ != nullptr && dynamic_cast<const J *>(held_) != nullptr;
  }

  /// \brief AL `Variable as Interface`: the same implementation, seen through the other one.
  /// \tparam J The other interface's class.
  /// \return A variable that REFERS to this one's instance rather than copying it, which is what
  ///         AL's `as` yields -- a call through either reaches the same object.
  /// \throws Error when the implementation is not of that interface, which is what AL does when
  ///         `as` is written without the `is` in front of it.
  template <typename J> [[nodiscard]] Implementation<J> As() const {
    J *seen = held_ == nullptr ? nullptr : dynamic_cast<J *>(held_);
    if (seen == nullptr) {
      throw Error("the implementation behind this interface variable is not of that interface");
    }
    return Implementation<J>::Borrowed(seen);
  }

  /// \brief A variable that refers to an instance somebody else owns, which is what `as` makes.
  /// \param seen The instance.
  /// \return The variable.
  [[nodiscard]] static Implementation Borrowed(I *seen) {
    Implementation made;
    made.held_ = seen;
    made.clone_ = [](const I *held) -> I * { return const_cast<I *>(held); };
    made.free_ = [](I *held) { static_cast<void>(held); };
    return made;
  }

private:
  /// THE FREEING FUNCTION IS CAPTURED WHERE THE IMPLEMENTATION IS MADE, the way `Instance<T>`
  /// does it: a holder may see the interface only forward-declared, and `delete` on an incomplete
  /// type is what clang-19 refuses (`-Wdelete-incomplete`, Reminder Action, 2026-09-09).
  void Forget() {
    if (held_ != nullptr && free_ != nullptr) { free_(held_); }
    held_ = nullptr;
    clone_ = nullptr;
    free_ = nullptr;
  }

  I *held_ = nullptr;
  I *(*clone_)(const I *) = nullptr;
  void (*free_)(I *) = nullptr;
};

}
