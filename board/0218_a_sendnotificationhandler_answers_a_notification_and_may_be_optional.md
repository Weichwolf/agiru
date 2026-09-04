Type:     task
Status:   open
Parent:   0054
Area:     rt
Source:   developer/attributes/devenv-sendnotificationhandler-attribute.md
Verdict:  fehlt
Class:    activation

# A `[SendNotificationHandler]` answers a notification, and it is the one handler that may be OPTIONAL

```al
[SendNotificationHandler([HandlerIsOptional: Boolean])]
procedure SendNotificationHandler(var TheNotification: Notification): Boolean
```

Two things make this handler unlike the others:

- **It takes an argument.** `[SendNotificationHandler(true)]` declares the handler OPTIONAL, which
  exempts it from board:0199's rule that every named handler must be called at least once. Only this
  handler and `[RecallNotificationHandler]` may be optional.
- **It returns a Boolean**, and it receives the `Notification` by `var`, so it may read the
  notification's data (`GetData`) and its actions before answering.

The attribute is legal only inside a `Subtype = Test` codeunit and the method must be global.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**369 `[SendNotificationHandler` declarations.** Over the milestone's 78 UT codeunits: 5
declarations in 5 codeunits.

## The IST-state

`include/type/Notification.h` and `src/net/Notification.cpp` exist; `Notification.Send` is a door
refusal. The attribute parses and is dropped, and the optional flag has nowhere to go.

## The choice

A table entry with kind `SendNotification`, no object id, plus **one bit for the optional flag** --
the only handler entry that needs a field beyond kind, id, name and thunk. `Notification.Send`
consults it by kind, calls the handler, and takes its Boolean as the answer.

The counter in 0199 skips an entry whose optional bit is set. **That bit is the whole reason this
item is separate from 0194**: an optional handler that never runs must NOT fail the test, and a
runner that counts all handlers alike turns 369 declarations into spurious failures.

## Ordering

Needs 0199's table. Needs no page runtime -- a notification is text and data, not a page.

## Gate, and its negative control

A test naming an optional handler that never runs: it must PASS. The same test with the handler
declared non-optional: it must FAIL.

**The negative control is that pair.** A runner that ignores the argument passes one of them and
fails the other, whichever way it guessed.
