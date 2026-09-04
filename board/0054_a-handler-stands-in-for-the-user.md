Type: root
State: open
Area: rt, gen

# A handler stands in for the user, and a named one that never ran fails the test

`HandlerFunctions` does not occur in `include/`, `src/rt/` or `src/gen/` -- not once, measured
2026-09-04. The attribute is parsed nowhere, no handler is registered, and `Confirm`, `Message` and
`StrMenu` are door refusals. So every test that raises a dialog dies on the dialog rather than on
its own claim.

## The population, over the 78 UT codeunits of the milestone

**The denominator below is 78 / 2 291 and nothing in the tree recomputes it** -- three plausible
readings of the milestone's own rule give 87 / 2 392, 80 / 2 305 and 86 / 2 392, so the 21.9 %
carries the same uncertainty as its base. board:0058 makes the rule executable; the shares here are
re-derived once it is.

| | |
|---|---|
| `[Test]` procedures | 2 291 |
| of them carrying `[HandlerFunctions]` | **501 (21.9 %)** |
| naming more than one handler | 81 |
| distinct handler names | 201 |
| handler procedures declared | 266 |

| handler kind | declarations | codeunits |
|---|---|---|
| ModalPageHandler | 147 | 28 |
| ConfirmHandler | 32 | 23 |
| PageHandler | 31 | 11 |
| RequestPageHandler | 24 | 10 |
| MessageHandler | 19 | 14 |
| SendNotificationHandler | 5 | 5 |
| ReportHandler | 4 | 1 |
| StrMenuHandler | 3 | 3 |
| HyperlinkHandler | 1 | 1 |

## `[HttpClientHandler]` IS A HANDLER WITH A DIFFERENT JOB, AND IT MAKES A TEST RUN HERMETIC

`devenv-httpclient-mock-outbound-calls.md` (read 2026-09-04, board:0071) -- and the page notes it is
**on-premises only**, which agiru is:

- The handler takes a `TestHttpRequestMessage` and a `TestHttpResponseMessage` and returns a
  Boolean: **`true` issues the real request, `false` uses the mocked response.**
- **The DEFAULT return value is `false`** -- "ensuring that external service calls are only made
  intentionally. Therefore, an empty handler would still intercept the outbound request and mock a
  default response."
- `TestHttpRequestPolicy` on the codeunit (board:0039, board:0067) decides what an UNHANDLED request
  does: `AllowAllOutboundRequests` (**the default**), `AllowOutboundFromHandler`, or
  **`BlockOutboundRequests`**, which "raises an exception ... useful when you don't want frequent
  test executions in CI/CD pipelines to hit the actual endpoint".

**That is the determinism lever the milestone needs.** CLAUDE.md makes determinism compulsory, and a
suite that can reach the network is not deterministic. `BlockOutboundRequests` is what
`agiru run-tests` should default to, and the two `TestHttp*Message` door types already exist for the
handler to fill.

**AND THE TABLE IS NINE KINDS OF TWELVE.** `attributes/` carries three more handler attributes this
item never counted: **`FilterPageHandler`**, **`SessionSettingsHandler`** and
**`HttpClientHandler`** -- the last of which is what `TestHttpRequestMessage` and
`TestHttpResponseMessage` exist for, and both of those already have door headers. The population
above is therefore a subset and is re-counted when this item is worked (read 2026-09-04,
board:0071).

**54 of the 266 need no page runtime** -- Message, Confirm, StrMenu and Hyperlink are text in and
text out. They are the part of this item that can be finished before phase 2 and they sit in some
thirty codeunits.

## What the platform documents

`devenv-creating-handler-methods.md` tabulates all twelve kinds with their signatures, and
`attributes/devenv-handlerfunctions-attribute.md` gives the rules. Four of them decide the shape:

- **A handler must live in the SAME test codeunit as the test method.** So the table is per
  codeunit and never global, and the generator can emit it beside the codeunit it belongs to --
  `constexpr`, in `.rodata`, like every other piece of object metadata.
- **UNHANDLED UI IS A FAILURE ONLY UNDER A TEST RUNNER**, and that condition is the rule's other
  half. `devenv-testing-application.md`: "If you run a test codeunit **from a test runner codeunit**,
  then any unhandled UI in the test methods of the test codeunit causes a failure of the test. If
  you don't run the test codeunit from a test runner codeunit, then any unhandled UI is displayed as
  it typically would." So the refusal belongs to the RUNNER (board:0039) rather than to `Confirm`
  itself, and a codeunit run outside one behaves like an application.
- **Every non-optional handler named must be CALLED AT LEAST ONCE, or the test FAILS.** That is a
  counter per named handler, checked after the procedure returns and before the transaction is
  decided. Send-notification and recall-notification handlers may be declared optional
  (`[SendNotificationHandler(true)]`) and are then exempt.
- **The parameters of the handled call are the handler's parameters.** `Message(Text)` hands its
  text; `Confirm(Question, Default)` hands the question and takes the reply back through a `var`;
  `StrMenu(Options, Default, Instruction)` hands two texts and takes the choice back.
- **A page or report handler binds to ONE object**, because its parameter is typed to it:
  `procedure H(var Page: TestPage "Customer List")`. So the dispatch key is the PAIR (kind, object
  id) for those, and the kind alone for the four text ones. `ModalPageHandler` has two documented
  signatures -- `(var Page: Page <id>; var Response: Action)` and `(var Page: TestPage <id>)` --
  and a `ReportHandler` REPLACES the whole run including the request page, so a
  `RequestPageHandler` is not called when one is present.

## The choice

The shape the runtime already has fits without inventing anything:

- `TestMethod` gains `std::span<const std::string_view> handlers`, emitted by the generator from
  the attribute -- the same `constexpr` array `kTestMethods` already is.
- A codeunit with any handler procedure emits a second `constexpr` array beside it: kind, object id
  (0 where the kind has none), name, and the address of a thunk that calls the member.
- `TestRunner` installs that table for the duration of one procedure and counts every call. The
  session holds ONE table, because a handler may only be reached from its own codeunit.
- `Message`, `Confirm`, `StrMenu`, `Hyperlink` and the page opener consult it by kind. **No handler
  registered is a FAILURE and not a silence**: the predecessor made an unhandled `Message` a no-op
  and an unhandled `Hyperlink` a no-op, and a test that meant to assert on the text then failed
  further downstream with "Queue underflow" instead of at the call (openerp `_system.py`).

**`GuiAllowed()` MUST ANSWER TRUE INSIDE A TEST, and that is a finding rather than a preference.**
The BaseApp guards hundreds of dialogs with `if GuiAllowed then Confirm(...)`, so a runtime that
answers false skips the guarded branch entirely: the validation never fires and the test reports
"expected an error, got none" -- a failure that points nowhere near the cause. The predecessor paid
for it and wrote the reason down (openerp `_system.py`, `_al_gui_allowed`). It is true because the
handler IS the UI: a test with `[HandlerFunctions]` has a user on the other end, and one without
still has the platform's refusal.

## The gate

A test codeunit of our own with one procedure per kind: a named handler that runs (green), a named
handler that does NOT run (must go red with the platform's own wording), and a dialog with no
handler at all (must go red at the dialog). The negative control is the middle one -- a runner that
forgets the counter passes it, and that is exactly the check that would otherwise be blind.

## `Message` IS ASYNCHRONOUS, AND THAT CHANGES WHEN A HANDLER IS CALLED

`devenv-progress-windows-message-error-and-confirm-methods.md` (read 2026-09-04, board:0071):

> The `Message` method **runs asynchronously**, which means that **the message isn't run until the
> method from which it was called ends or another method requests user input.**

So `Message('a'); DoSomething(); Message('b');` does not show `a`, then work, then `b` -- it works,
then shows both, in order, when the method returns or when something asks the user for input. **A
`MessageHandler` therefore fires at the METHOD BOUNDARY and not at the call site**, and a test that
counts handler invocations at a particular point sees a different number depending on which rule the
runtime implements.

`Error` is the opposite and the page says so: it is "similar to the `Message` method except that when
the user has acknowledged the message ... **AL execution ends**". Synchronous, terminal, and -- with
`Error('')` -- silent (board:0055).

**Which means the four text handlers are not one kind after all:**

| the call | when the handler runs |
|---|---|
| `Message` | **deferred** -- at the end of the calling method, or when input is next requested |
| `Confirm` | immediately -- it returns the user's answer |
| `StrMenu` | immediately -- it returns the choice |
| `Error` | immediately, and execution ends |

Three of the four are synchronous because they RETURN something. `Message` returns nothing, which is
exactly why it can be deferred -- and why implementing it as an immediate call is the natural mistake.

The page adds two smaller rules: **"a Cancel button is automatically added to every dialog window"**
created with the `Dialog` type, so a progress window is cancellable whether or not the AL asks for
it; and backslashes are line breaks ONLY in `Dialog.Open`, never in `Message`, `Error` or `Confirm`,
where "line formatting is completed automatically" -- which matters because BaseApp error texts
contain `\` and a runtime that renders it literally produces a different string from the one a test
compares (board:0055).
