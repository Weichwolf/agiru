#pragma once

#include "meta/Ids.h"
#include "runtime/Error.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>

namespace agiru {

/// \brief Which kind of object publishes an event -- AL's `ObjectType::X` in an
///        `[EventSubscriber]`.
enum class EventObject : std::uint8_t { Codeunit, Table, Page, Report, XmlPort, Query };

/// \brief What a publisher hands its subscribers: the arguments by ADDRESS and by NAME.
///
/// \note A `var` parameter is the caller's variable and a subscriber writing `IsHandled` writes
///       it; a by-value one is copied when the subscriber's parameter is by value. A subscriber
///       binds by NAME, case-folded, because AL lets it declare fewer parameters than the
///       publisher and in any order (board:0057).
struct EventArgs {
  std::span<const std::string_view> names; ///< The publisher's parameter names.
  std::span<void *const> values;           ///< The arguments, one address per name.
  /// \brief The object raising the event, for a subscriber whose first parameter is `Sender`.
  /// \note AL'S RULE, NOT A CONVENTION: a subscriber to a table or codeunit event may declare a
  ///       first parameter named `Sender` of the publisher's type and receives the raising
  ///       instance, which the publisher never lists among its parameters. 13 subscribers in
  ///       `Layers/W1` do (`RecordRestrictionMgt.CustomerCheckSalesPostRestrictions(var Sender:
  ///       Record "Sales Header")`), and the dispatch refused every one as naming an unpublished
  ///       parameter (13 UT cases, 2026-09-09).
  void *sender = nullptr;
};

/// \brief The bound index that means "the sender", which no argument list is long enough to hold.
inline constexpr std::size_t kSenderSlot = static_cast<std::size_t>(-1);

/// \brief One `[EventSubscriber]` procedure, as `constexpr` data beside its codeunit.
struct Subscription {
  EventObject kind;                             ///< The publisher's object kind.
  std::int32_t objectId;                        ///< Its number, 0 when only the name is known.
  std::string_view objectName;                  ///< Its AL name.
  std::string_view event;                       ///< The published method's name.
  std::string_view element;                     ///< The field, for a table trigger event.
  std::span<const std::string_view> parameters; ///< The subscriber's parameter names, in order.
  /// \brief Calls the subscriber on an instance with each parameter bound to
  /// `args.values[bound[i]]`.
  void (*invoke)(void *instance, const EventArgs &args, std::span<const std::size_t> bound);
};

/// \brief A codeunit's subscriptions, registered at start-up the way `TestCatalogue` is.
///
/// \note `EventSubscriberInstance = Manual` dispatches only to instances a `BindSubscription`
///       registered for the session; `StaticAutomatic` (the default) to one instance per session
///       made on first dispatch.
class SubscriptionCatalogue {
public:
  /// \brief Registers the catalogue.
  /// \param id            The subscriber codeunit.
  /// \param name          Its AL name.
  /// \param subscriptions Its subscriptions, in declaration order.
  /// \param manual        Whether its instance is `Manual`.
  /// \param make          Makes an instance, for the automatic kind.
  /// \param free          Frees one.
  SubscriptionCatalogue(CodeunitId id,
                        std::string_view name,
                        std::span<const Subscription> subscriptions,
                        bool manual,
                        void *(*make)(),
                        void (*free)(void *));
  SubscriptionCatalogue(const SubscriptionCatalogue &) = delete;
  SubscriptionCatalogue(SubscriptionCatalogue &&) = delete;
  SubscriptionCatalogue &operator=(const SubscriptionCatalogue &) = delete;
  SubscriptionCatalogue &operator=(SubscriptionCatalogue &&) = delete;
  ~SubscriptionCatalogue() = default;

  /// \return The subscriber codeunit.
  [[nodiscard]] CodeunitId Id() const { return id_; }

  /// \return Its AL name.
  [[nodiscard]] std::string_view Name() const { return name_; }

  /// \return Its subscriptions.
  [[nodiscard]] std::span<const Subscription> Subscriptions() const { return subscriptions_; }

  /// \return Whether its instance is `Manual`.
  [[nodiscard]] bool Manual() const { return manual_; }

  /// \return A new instance of the codeunit.
  [[nodiscard]] void *Make() const { return make_(); }

  /// \param instance One `Make` made.
  void Free(void *instance) const { free_(instance); }

private:
  CodeunitId id_;
  std::string_view name_;
  std::span<const Subscription> subscriptions_;
  bool manual_;
  void *(*make_)();
  void (*free_)(void *);
};

/// \brief AL lets a caller hand an EXPRESSION to a publisher's `var` parameter -- `CurrFieldNo()`
///        into `var CurrFieldNo: Integer` -- because the publisher's own body is empty and a
///        subscriber that writes it writes a copy. This gives the expression an address for the
///        length of the call.
/// \tparam T The value's type.
/// \param value The expression's value.
/// \return A reference to it.
template <typename T> T &Materialised(T &&value) {
  return static_cast<T &>(value);
}

namespace detail {

/// \brief Dispatches one event to every bound subscriber, in subscriber-codeunit order.
/// \param kind       The publisher's object kind.
/// \param objectId   Its number.
/// \param objectName Its AL name.
/// \param event      The published method.
/// \param element    The field, for a table trigger event; empty otherwise.
/// \param args       The arguments.
/// \throws Error when a subscriber names a parameter the publisher does not have.
void Raise(EventObject kind,
           std::int32_t objectId,
           std::string_view objectName,
           std::string_view event,
           std::string_view element,
           const EventArgs &args);

/// \brief Dispatches an ISOLATED event: every subscriber runs in a boundary of its own, and one
///        that raises has its writes discarded while the rest still run.
///
/// `devenv-events-isolated.md`: "an error in an event subscriber doesn't stop the other
/// subscribers", and the platform's sign-in events are of that kind -- with the obsolete
/// `OnCompanyOpen`, "a failure in any event subscriber will stop the sign-in process"
/// (`devenv-oncompanyopencompleted.md`), which is what isolation was introduced to end.
///
/// \param kind       The publisher's object kind.
/// \param objectId   Its number.
/// \param objectName Its AL name.
/// \param event      The published method.
/// \param element    The field, for a table trigger event; empty otherwise.
/// \param args       The arguments.
void RaiseIsolated(EventObject kind,
                   std::int32_t objectId,
                   std::string_view objectName,
                   std::string_view event,
                   std::string_view element,
                   const EventArgs &args);

/// \brief AL `BindSubscription(Codeunit)` for the session.
/// \param id       The codeunit.
/// \param instance The instance to dispatch to.
/// \return False when that instance is already bound, or the codeunit has no subscriptions.
bool BindSubscriptions(CodeunitId id, void *instance);

/// \brief AL `UnbindSubscription(Codeunit)`.
/// \param id       The codeunit.
/// \param instance The instance.
/// \return False when it was not bound.
bool UnbindSubscriptions(CodeunitId id, void *instance);

template <typename T, auto Method, typename... P>
void CallBound(T &unit,
               const EventArgs &args,
               std::span<const std::size_t> bound,
               [[maybe_unused]] void (T::*signature)(P...)) {
  [&]<std::size_t... I>(std::index_sequence<I...>) {
    (unit.*Method)(*static_cast<std::remove_cvref_t<P> *>(
        bound[I] == kSenderSlot ? args.sender : args.values[bound[I]])...); // NOLINT
  }(std::index_sequence_for<P...>{});
}

/// \brief The thunk a `Subscription` carries: casts each bound argument to the subscriber's
///        parameter type, which AL guarantees matches the publisher's, and calls the member.
/// \tparam T      The subscriber codeunit.
/// \tparam Method The subscriber procedure.
template <typename T, auto Method>
void InvokeSubscriber(void *instance, const EventArgs &args, std::span<const std::size_t> bound) {
  CallBound<T, Method>(*static_cast<T *>(instance), args, bound, Method);
}

/// \brief What a publisher's emitted body calls: the arguments by address, the names beside.
/// \param kind       The publisher's object kind.
/// \param objectId   Its number.
/// \param objectName Its AL name.
/// \param event      The published method.
/// \param element    The field, for a table trigger event; empty otherwise.
/// \param names      The publisher's parameter names.
/// \param values     The arguments, one per name.
/// \brief Raises an event from the object raising it, so a `Sender` parameter can receive it.
/// \tparam Values The arguments' types.
/// \param sender The raising object, or `nullptr` when the raise has none.
/// \param kind What kind of object publishes.
/// \param objectId Its number.
/// \param objectName Its name.
/// \param event The event's name.
/// \param element The element, for a trigger event.
/// \param names The publisher's parameter names.
/// \param values The arguments.
/// \brief The addresses an event hands its subscribers, one per argument.
/// \tparam Values The argument types.
/// \param values The arguments; a `const` one is handed over as the subscriber's by-value copy
///        source, which is what the cast below is for.
/// \return The addresses, in order.
template <typename... Values>
std::array<void *, sizeof...(Values)> ArgumentAddresses(Values &...values) {
  return {const_cast<void *>(static_cast<const void *>(&values))...}; // NOLINT
}

template <typename... Values>
void RaiseEventFrom(void *sender,
                    EventObject kind,
                    std::int32_t objectId,
                    std::string_view objectName,
                    std::string_view event,
                    std::string_view element,
                    std::span<const std::string_view> names,
                    Values &...values) {
  std::array<void *, sizeof...(Values)> addresses = ArgumentAddresses(values...);
  Raise(kind,
        objectId,
        objectName,
        event,
        element,
        EventArgs{.names = names, .values = addresses, .sender = sender});
}

/// \brief `RaiseEventFrom` for a publisher declared `[IntegrationEvent(_, _, true)]`: an ISOLATED
///        event, whose subscribers each run in a boundary of their own
///        (`devenv-events-isolated.md`)
///        -- `System Initialization.OnAfterLogin` is one, and a subscriber refusing there stopped
///        every later one, the API binder among them (2026-09-11).
/// \tparam Values The argument types.
/// \param sender The raising object.
/// \param kind The publisher's object kind. \param objectId Its number. \param objectName Its AL
///        name. \param event The published method. \param element The field, for a table trigger
///        event; empty otherwise. \param names The publisher's parameter names.
/// \param values The arguments.
template <typename... Values>
void RaiseIsolatedEventFrom(void *sender,
                            EventObject kind,
                            std::int32_t objectId,
                            std::string_view objectName,
                            std::string_view event,
                            std::string_view element,
                            std::span<const std::string_view> names,
                            Values &...values) {
  std::array<void *, sizeof...(Values)> addresses = ArgumentAddresses(values...);
  RaiseIsolated(kind,
                objectId,
                objectName,
                event,
                element,
                EventArgs{.names = names, .values = addresses, .sender = sender});
}

template <typename... Values>
void RaiseEventOn(EventObject kind,
                  std::int32_t objectId,
                  std::string_view objectName,
                  std::string_view event,
                  std::string_view element,
                  std::span<const std::string_view> names,
                  Values &...values) {
  RaiseEventFrom(nullptr, kind, objectId, objectName, event, element, names, values...);
}

/// \brief `RaiseEventOn` with no element -- what a codeunit's publisher emits.
template <typename... Values>
void RaiseEvent(EventObject kind,
                std::int32_t objectId,
                std::string_view objectName,
                std::string_view event,
                std::span<const std::string_view> names,
                Values &...values) {
  RaiseEventOn(kind, objectId, objectName, event, {}, names, values...);
}

}

}
