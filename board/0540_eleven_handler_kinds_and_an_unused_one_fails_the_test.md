Type:     task
Status:   open
Parent:   0054
Area:     rt, gen
Source:   developer/devenv-creating-handler-methods.md
Verdict:  fehlt
Class:    activation

# Eleven handler kinds, and a declared handler that is never called fails the test

board:0054 is "a handler stands in for the user" and board:0199 measured `[HandlerFunctions]` at
**47 994**. **This page is the complete list and the rule that makes it strict.**

## The eleven handlers and their exact signatures

| attribute | handles | signature |
|---|---|---|
| `MessageHandler` | `Message` | `(Message: Text[1024])` |
| `ConfirmHandler` | `Confirm` | `(Question: Text[1024]; var Reply: Boolean)` |
| `StrMenuHandler` | `StrMenu` | `(Options: Text[1024]; var Choice: Integer; Instruction: Text[1024])` |
| `PageHandler` | pages **not run modally** | `(var Page: Page <id>)` or `(var Page: TestPage <id>)` |
| `ModalPageHandler` | pages **run modally** | `(var Page: Page <id>; var Response: Action)` |
| `ReportHandler` | a report | `(var Report: Report <id>)` |
| `RequestPageHandler` | a report's request page | `(var RequestPage: TestRequestPage)` |
| `FilterPageHandler` | a `FilterPageBuilder` page | `(var Record1: RecordRef[, ...]): Boolean` |
| `HyperlinkHandler` | hyperlinks | `(Hyperlink: Text[1024])` |
| `SendNotificationHandler` | `Send` | `(TheNotification: Notification): Boolean` |
| `RecallNotificationHandler` | `Recall` | `(TheNotification: Notification): Boolean` |
| `SessionSettingsHandler` | `RequestSessionUpdate` | `(var SessionSettings: SessionSettings): Boolean` |

board:0194, board:0198, board:0201 and their neighbours filed several of these from the attribute
pages; **this page is where the signatures come from**, and they are not uniform: four return
`Boolean`, three take a `var` out-parameter, and the two page handlers differ by one parameter.

**`ModalPageHandler` returns the user's `Action` through a `var`** -- that is how a test says OK or
Cancel, and it is board:0516's `IsHandled` shape again: a `var` that must be a reference or the dialog
always reads Cancel.

## The rule that makes the mechanism strict

> **"EVERY handler method that you enter in the `HandlerFunctions` attribute of a test method MUST BE
> CALLED AT LEAST ONE TIME in the test method. If you run a test method that has a handler method
> listed that ISN'T CALLED, THEN THE TEST FAILS."**

**A declared handler that never fires is a FAILURE, not a no-op.** So the runner tracks, per test, which
of the declared handlers were invoked, and fails the test if any was not.

**That is a strong property and it is free to get wrong**: an implementation that simply registers
handlers and lets unused ones sit passes every test whose handler does fire, and silently turns
"the dialog I expected never appeared" into a pass. **With 47 994 declarations, that is the single
most load-bearing assertion in the UT suite** -- it is what makes a handler-driven test prove that the
UI interaction happened at all.

## `ReportHandler` suppresses `RequestPageHandler`

> **"If you create a `ReportHandler` method, then that method REPLACES ALL CODE FOR RUNNING THE REPORT,
> INCLUDING THE REQUEST PAGE, and a `RequestPageHandler` ISN'T CALLED.** Only create a
> `RequestPageHandler` if you aren't using a `ReportHandler`."

**So the two are mutually exclusive at run time**, and declaring both means the request-page handler is
never called -- which by the rule above **fails the test**. Two rules interacting, and the combination
is a `static_assert` candidate: both attributes on one test is a declaration that cannot pass.

## Handlers run on the server

> **"Test methods and code on test pages RUN ON THE SERVER INSTANCE, even though they simulate client
> interactions."**

So a `TestPage` is not a client; board:0030's UI proof runs the same page object twice -- once through
the `TestPage` surface and once over HTTP -- and CLAUDE.md says both must give the same answer.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0199: `[HandlerFunctions]` **47 994**. board:0062: **3** `[Test]` procedures call
`LibraryLowerPermissions` against 2 984 in the wider suite. **The per-handler-kind counts are attribute
declarations and belong to board:0190's family** -- stated rather than guessed, and they say which of
the eleven the milestone needs first.

## The IST-state

board:0054 records the handler state. `src/gen/CodeunitWriter.cpp:77` reads `[Test]`;
`src/al/Parser.cpp:926` ignores attribute arguments, so **the handler NAMES inside
`[HandlerFunctions('A,B')]` are not parsed** -- which is the first thing this item needs.

## The choice

A per-test handler registry: the generator emits, per `[Test]` procedure, the list of declared handler
names resolved to function pointers, and the runner installs them, counts invocations, and **fails the
test when any declared handler was not called.**

**Eleven signature shapes, not one** -- a tagged table, as board:0512's dispatch needs for its five.

**The dialog family calls into the registry** rather than the registry patching the dialog family:
`Message`, `Confirm`, `StrMenu` and the rest ask "is a handler installed" and, if so, call it instead of
rendering. That is one branch in board:0496's message queue.

## Ordering

board:0054's core. Behind board:0496's dialogs, which are what the handlers stand in for, and
board:0493's runner, which owns the per-test lifecycle.

## Gate, and its negative control

A test declaring `[HandlerFunctions('MessageHandler')]` whose code raises a `Message` passes; the same
test with a `Message` that never happens **FAILS**.

**The negative control is the second half** -- an implementation that registers handlers without
counting invocations passes both, and the whole point of the mechanism is lost silently across 47 994
declarations.
