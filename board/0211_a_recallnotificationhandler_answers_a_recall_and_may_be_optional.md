Type:     task
Status:   open
Parent:   0054
Area:     rt
Source:   developer/attributes/devenv-recallnotificationhandler-attribute.md
Verdict:  fehlt
Class:    activation

# A `[RecallNotificationHandler]` answers a recall, and it is the second handler that may be OPTIONAL

```al
[RecallNotificationHandler([HandlerIsOptional: Boolean])]
procedure RecallNotificationHandler(var TheNotification: Notification) : Boolean
```

`Notification.Recall()` withdraws a notification that was sent. Under a test runner this handler
receives it and answers whether the recall succeeded.

**With `[SendNotificationHandler]` it is one of only two handlers that may be declared optional**
(board:0218), which exempts it from board:0199's called-at-least-once rule. That matters here more
than there: a test that sends a notification may or may not recall it depending on a branch, so the
recall handler is frequently declared and legitimately unused.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**103 `[RecallNotificationHandler` declarations**, against 369 `[SendNotificationHandler`.

## The IST-state

`include/type/Notification.h` and `src/net/Notification.cpp` exist; `Recall` is a door refusal. The
attribute parses and is dropped, and the optional flag has nowhere to go.

## The choice

A table entry with kind `RecallNotification`, no object id, and the optional bit -- the same entry
shape 0218 introduces, which is why these two are the pair that justifies the bit existing at all.

**The notification identity is what makes `Recall` more than a no-op**: `Recall` must reach the
handler with the SAME notification instance that `Send` produced, so the runtime holds sent
notifications per session until they are recalled or the session ends.

## Ordering

Needs 0199's table and 0218's optional bit. Needs no page runtime.

## Gate, and its negative control

Send a notification, recall it, and require the recall handler to receive the same instance --
`GetData` must return what `SetData` put there before the send.

**The negative control is the identity**: hand the handler a fresh `Notification` and the assertion
on `GetData` must fail. A runtime that constructs a new notification for the recall passes every
other case.
