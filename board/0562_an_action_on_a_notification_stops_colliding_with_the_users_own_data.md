Type:     bug
Status:   open
Parent:   0028
Area:     net, rt
Source:   developer/devenv-notifications-developing.md
Verdict:  teilweise
Class:    silent-wrong-data

# An action on a notification stops colliding with the user's own data

`Notification` is one of the few AL types this tree has already BUILT, and reading the page that
specifies it turned up three defects in `src/net/Notification.cpp` and one in the door.

## The defect

`src/net/Notification.cpp:16`:

```cpp
void Notification::AddAction(std::string_view caption,
                             Integer codeunitId,
                             std::string_view methodName) {
  SetData("Action" + std::to_string(actions_),
          std::string(caption) + "|" + std::to_string(codeunitId) + "|" + std::string(methodName));
  ++actions_;
}
```

**The action list is stored in the USER'S key-value store.** `SetData` and `GetData` are an AL-visible
map -- "the data is defined as text in a key-value pair" -- and `AddAction` writes into it under
`Action0`, `Action1`, ... Three consequences, all silent:

1. **`SetData('Action0', x)` overwrites the first action**, and `AddAction` afterwards overwrites `x`.
   The AL never sees an error; the notification simply carries the wrong thing.
2. **`GetData('Action0')` returns `"caption|5|Method"`** to AL code that set nothing under that key.
3. **`HasData('Action0')` returns `true`** (`include/type/Notification.h:98`) for a key the AL never
   set -- and `HasData` exists precisely so AL can ask that question.

**The encoding is the second defect**: `caption + "|" + id + "|" + method`, with no escaping. **A
caption containing `|` produces an action nothing can parse back**, and captions are free AL text.

**And `AddAction` enforces no limit.** The page: *"A LocalScope notification can have UP TO 3 ACTIONS.
A GlobalScope notification can have up to 2."* `actions_` counts and never checks.

**The door already declares the not-wiring and that part is fine.**
`include/type/Notification.h:101`: *"THE ACTION IS RECORDED AND NOT WIRED. Invoking it is the client's
half, and there is no client; recording it lets a test see that the action was offered."* That is a
stated decision and it stands. **Recording it in the user's namespace is not part of that decision.**

## Two more, from the same page

**`Send()` does not send.** `src/net/Notification.cpp:24`:

```cpp
void Notification::Send() {
  if (id_.IsNull()) { id_ = Guid::Create(); }
}
```

It assigns an id and returns. **No `SendNotificationHandler` is invoked** -- `grep -rn
SendNotificationHandler src/ include/` finds nothing -- so board:0218's handler has no caller and
**311 `Notification.Send` call sites do nothing observable.** A UT case that sends a notification and
expects its handler to fire cannot pass, and cannot fail loudly either.

**`Recall()` is an empty body** (`src/net/Notification.cpp:28`), so board:0211's
`RecallNotificationHandler` likewise has no caller, over **253 call sites**.

**And both have the wrong SIGNATURE.** `notification-send-method.md` and
`notification-recall-method.md`:

> `[Ok := ] Notification.Send()`
> **"`true` if the notification was sent; otherwise `false`. If you omit this optional return value
> and the operation does not execute successfully, A RUNTIME ERROR WILL OCCUR."**

`void Send()` and `void Recall()` (`Notification.h:115`, `:120`) cannot carry that. **This is
CLAUDE.md's named `value context` trap exactly** -- AL decides at consumption-versus-discard whether a
failure throws or yields `false` -- on a type that already exists, so it is not a design question but
a signature that is wrong today.

**One documented overload is missing.** The door's own `\brief` says
`AddAction(Caption, CodeunitId, MethodName [, Tooltip])` and only the three-argument form is declared.
`methods-auto/notification/` carries both files --
`notification-addaction-string-integer-string-method.md` and
`-string-integer-string-string-method.md` -- so this is a gap the completeness counter should already
be showing.

## What the page specifies that nothing here implements yet

- **Notifications appear in a bar in CHRONOLOGICAL ORDER**, and "validation errors on the page will be
  shown FIRST" -- so the bar is two ordered lists, not one.
- **They live for the PAGE INSTANCE**, until dismissed or acted on.
- **A subpage's notifications appear in the HOST's bar** -- a FactBox or part sends into the page
  above it (board:0554's parts).
- **`Send` must be the last call**: "the `Send` method call should be the last statement ... after any
  `AddAction` or `SetData` calls." So the notification is a value that is BUILT and then handed over,
  which is what the current type already is.
- **`AddAction` names a method BY STRING** on a codeunit id. That is a dynamic dispatch AL has and C++
  does not, and CLAUDE.md forbids the runtime knowing any AL object -- so the resolution has to be a
  generated per-codeunit table of `void(Notification&)` entries, the same shape the `OnValidate` map
  already has.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

| | count |
|---|---:|
| a variable of type `Notification` | **731** |
| `.Message :=` | 511 |
| `.SetData(` | 488 |
| `.AddAction(` | 487 |
| `NotificationScope::` | **321, every one `LocalScope`** |
| `Notification.Send` | 311 |
| `.GetData(` | 310 |
| `.Recall(` | 253 |

`.Send(` alone is 656 across the tree and is NOT separable -- `HttpClient`, `Email` and others declare
a `Send`. The 311 figure is the qualified `Notification.Send` form only and is therefore a LOWER
bound on the call sites; no interpolation is offered for the rest.

**`GlobalScope` is declared zero times**, which agrees with the page: *"GlobalScope is currently not
supported. This will be implemented in a future release."* So the unsupported scope has no call site
and the enumerator exists for completeness only.

**487 `AddAction` call sites are what makes the key collision matter.** Every one of them writes into
the same map 488 `SetData` calls write into.

## The choice

**Actions go in their own member, and `SetData` gets its namespace back.**

```cpp
struct NotificationAction {
  std::string caption;
  Integer codeunit;
  std::string method;
};
std::vector<NotificationAction> actions_;   // replaces the Action<n> keys
```

**Why a vector and not the dictionary with a reserved prefix:** a reserved prefix is a rule the AL
cannot see and BC's own code could walk into -- `SetData('Action1', ...)` is legal AL. A separate
member cannot collide at all, and the encoding problem disappears with it: no separator, no escaping,
no caption that breaks parsing.

**Why not `std::array<NotificationAction, 3>`:** the limit is 3 for `LocalScope` and 2 for
`GlobalScope`, and it is a RUN-TIME limit on a run-time call. A fixed array would have to pick one,
and the scope can be set after the actions are added. `reserve(3)` is the right compromise -- the
allocation is bounded and the check stays where the platform puts it.

**`Send` and `Recall` return `Boolean`**, and the discard context raises. board:0028's value-context
machinery decides how; the signature is what this item changes.

**`Send` invokes the handler** through board:0218's registration, `Recall` through board:0211's. Both
handlers already have items; what they lack is a caller.

**The fourth `AddAction` overload is declared**, with the tooltip.

## Ordering

**The key collision first**, because it is the only one of the four that produces WRONG DATA rather
than no data -- 487 call sites writing into a map 488 others read. **The signatures second**, since
they are a compile-time change with a `-Werror` blast radius. **The handlers third**, with
board:0218 and board:0211. **The action limit last**, at zero known call sites that exceed it.

## Gate, and its negative control

1. `SetData('Action0', 'mine')` then `AddAction('a', 1, 'M')` then `GetData('Action0')` returns
   **`'mine'`**
2. `HasData('Action0')` is **false** on a notification carrying one action and no such data
3. `AddAction('a|b', 1, 'M')` round-trips the caption `a|b` intact
4. a fourth `AddAction` on a `LocalScope` notification raises
5. `Send` invokes the registered `SendNotificationHandler` exactly once
6. `if not MyNotification.Send() then` compiles and takes the `false` branch when sending fails

**The negative control is case 2.** Move the actions to their own member but leave `AddAction` also
mirroring them into `data_` "for compatibility" -- cases 1, 3, 4, 5 and 6 all stay green and only case
2 goes red. It is the case that proves the user's namespace is actually clean rather than merely
overwritten in the right order.

**Case 5's counter is the blind-gate guard**: a handler invocation count of 0 over a test that sends
one notification is an ABORT and not a pass, which is exactly the state today.

## Class

`silent-wrong-data` for the collision and the encoding: `GetData` returns a value, nothing throws, and
the value belongs to something else. The missing handlers are closer to `activation` -- 311 sends that
currently do nothing would start reaching AL handler code -- and that half carries the A/B.
