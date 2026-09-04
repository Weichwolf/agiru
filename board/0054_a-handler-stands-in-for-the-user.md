Type: root
State: open
Area: rt, gen

# A handler stands in for the user, and a named one that never ran fails the test

`HandlerFunctions` does not occur in `include/`, `src/rt/` or `src/gen/` -- not once, measured
2026-09-04. The attribute is parsed nowhere, no handler is registered, and `Confirm`, `Message` and
`StrMenu` are door refusals. So every test that raises a dialog dies on the dialog rather than on
its own claim.

## The population, over the 78 UT codeunits of the milestone

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

**54 of the 266 need no page runtime** -- Message, Confirm, StrMenu and Hyperlink are text in and
text out. They are the part of this item that can be finished before phase 2 and they sit in some
thirty codeunits.

## What the platform documents

`devenv-creating-handler-methods.md` tabulates all twelve kinds with their signatures, and
`attributes/devenv-handlerfunctions-attribute.md` gives the rules. Four of them decide the shape:

- **A handler must live in the SAME test codeunit as the test method.** So the table is per
  codeunit and never global, and the generator can emit it beside the codeunit it belongs to --
  `constexpr`, in `.rodata`, like every other piece of object metadata.
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
