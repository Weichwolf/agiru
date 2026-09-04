Type:     task
Status:   open
Parent:   0054
Area:     rt, net
Source:   developer/devenv-progress-windows-message-error-and-confirm-methods.md
Verdict:  fehlt
Class:    activation

# A `Message` is asynchronous and an `Error` ends execution

board:0054 is "a handler stands in for the user" and this page is what the handlers stand in for. It
carries one behavioural fact that no method page states and that decides how the dialogs are built.

> **"The `Message` method runs ASYNCHRONOUSLY, which means that the message ISN'T RUN UNTIL THE
> METHOD FROM WHICH IT WAS CALLED ENDS or another method requests user input."**
>
> "The `Error` method is similar to the `Message` method except that **when the user has acknowledged
> the message ... AL EXECUTION ENDS.**"
>
> **"A Cancel button is AUTOMATICALLY ADDED to every dialog window"** and gives the user the
> opportunity to stop the processing.
>
> "Unlike the progress window, the `Message` method doesn't require that you first declare a variable
> of type `Dialog`."

**`Message` is deferred, and that is not a detail.** A procedure that calls `Message` three times and
then raises shows nothing -- the messages never ran. A procedure that calls `Message` and then
`Confirm` shows the message first, because `Confirm` requests user input. So messages queue on the
session and flush at two documented points, and an implementation that displays them immediately
produces a different sequence from BC on every posting routine that reports progress and then fails.

**And `[MessageHandler]` sees that queue, not the call.** board:0054's handler mechanism has to be fed
from the flush point, so a test asserting on a message asserts on what BC would have shown -- which is
the whole reason the asynchrony matters here rather than only in the UI.

**The automatic Cancel button is board:0455's cancellation from the other side**: every dialog can
stop the processing, which means a long-running `Dialog` is a second cancellation channel beside the
report's.

## The four dialog surfaces

| method | behaviour |
|---|---|
| `Message` | asynchronous, OK only, no `Dialog` variable |
| `Error` | acknowledged, then **execution ends** -- and `ErrorInfo` is the modern form |
| `Confirm` | blocks for a yes/no, so it FLUSHES pending messages |
| `Dialog.Open` | a progress window; **backslashes align its fields** and are required there |

> "**Don't use backslashes** to indicate line breaks in `Message`, `Error` or `Confirm` ... **However,
> in `Dialog.Open` you MUST use backslashes** to align fields correctly."

So `\` is a line break in one family and an alignment marker in the other, from the same `Text`
literal. That is a rendering rule, and board:0055's error-text work has to respect it or every
multi-part progress window collapses.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

These are method calls, not property declarations, so this sweep's pattern does not apply. board:0028
owns the builtin census and board:0040 the builtins a test calls; the counts belong there.
**Stated rather than guessed.**

## The IST-state

board:0054 records the handler state; board:0035 the `Dialog` surface. `src/rt/Builtins.cpp` refuses
the door for the dialog family (board:0035's counted refusals).

## The choice

A per-session message QUEUE flushed at two points -- the end of the outermost AL call, and any input
request -- with `Confirm`, `StrMenu` and `Dialog` counting as input requests. `Error` raises after
its own acknowledgement, which for a handler-driven test means the handler runs and then the error
propagates.

**Not an immediate write to a channel.** The asynchrony is observable and the tests observe it.

## Ordering

Behind board:0054's handler mechanism, which is what a test uses instead of a user. Ahead of
board:0030's UI, which is the other consumer of the same queue.

## Gate, and its negative control

A procedure that calls `Message` twice and then `Confirm` delivers both messages BEFORE the
confirmation; a procedure that calls `Message` and then raises delivers none.

**The negative control is the raising procedure** -- an implementation that delivers immediately
shows the message and then the error, which looks more helpful and is not what BC does; a gate that
only checks the happy path passes.
