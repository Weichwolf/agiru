Type:     task
Status:   open
Parent:   0054
Area:     rt
Source:   developer/attributes/devenv-messagehandler-attribute.md
Verdict:  fehlt
Class:    activation

# A `[MessageHandler]` captures the text, and it fires at the METHOD BOUNDARY

```al
[MessageHandler]
procedure H(Message: Text[1024])
```

`Message(Text)` hands its text and returns nothing. **And `Message` is ASYNCHRONOUS**
(`devenv-progress-windows-message-error-and-confirm-methods.md`): "the message isn't run until the
method from which it was called ends or another method requests user input". So the handler does
NOT fire at the call site -- it fires when the calling method returns, in order.

That is the whole difference between this handler and the other three: `Confirm`, `StrMenu` and
`Error` return something and are therefore synchronous; `Message` returns nothing and is deferred.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**1 860 `[MessageHandler` declarations.** Over the milestone's 78 UT codeunits: 19 declarations in
14 codeunits.

## The IST-state

`Message` is a door refusal; the attribute parses and is dropped.

## The choice

The session holds a queue of pending message texts. `Message(Text)` appends; the queue drains when
the calling AL method returns, or when `Confirm` / `StrMenu` / a page asks for input -- whichever
comes first. Each drained text calls the handler and increments its counter (0199).

**Why a queue and not a direct call.** A direct call is one line and gives the wrong ORDER whenever
a method emits a message and then raises: BC shows nothing, agiru would show the message. A test
that asserts on message count sees the difference.

## Ordering

Needs 0199's table. Needs no page runtime.

## Gate, and its negative control

A procedure that calls `Message` twice with work between them: the handler must see both, after the
procedure returns, in order. A procedure that calls `Message` and then raises: the handler must see
NOTHING.

**The negative control is the second case.** A runtime that calls the handler at the call site
passes the first and fails only this one, which is why it is the case the item is written around.
