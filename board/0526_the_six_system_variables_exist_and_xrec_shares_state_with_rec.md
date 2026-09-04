Type:     task
Status:   open
Parent:   0042
Area:     rt, gen
Source:   developer/devenv-system-defined-variables.md
Verdict:  teilweise
Class:    silent-wrong-data

# The six system-defined variables exist, and `xRec` shares state with `Rec`

board:0042 is "`xRec` is the record before the change" -- and CLAUDE.md records what it cost the
predecessor: **four rounds** (openerp WI-781, WI-1078, WI-1137, WI-1156). **This page is the platform's
own statement**, and it contains one sentence that explains all four.

## The six variables

| variable | |
|---|---|
| `Rec` | the current record **including the changes** |
| `xRec` | **the original values before the changes** |
| `CurrPage` | the current page |
| `CurrReport` | the current report |
| `RequestOptionsPage` | the request page of the current report |
| `CurrFieldNo` | the field number of the current field. **"Retained for compatibility reasons."** |

**`CurrFieldNo` is marked legacy** and `include/runtime/Table.h:1375` already implements it as a
`ValidatingField` scope. So the tree has it; the page says AL keeps it only for compatibility, which
means nothing new is expected of it.

## The sentence that explains the predecessor's four rounds

> **"NOTE: AVOID MODIFICATIONS TO THE `xRec` VARIABLE, because the record MIGHT SHARE SOME OF THE
> UNDERLYING STATE WITH THE `Rec` VARIABLE for performance and compatibility reasons, and CHANGES CAN
> UNEXPECTEDLY PROPAGATE TO THE `Rec` VARIABLE."**

**`xRec` is not a copy.** It shares state with `Rec`, unspecified which, and writing through it can
change `Rec`. That is the behaviour, not an implementation detail -- Microsoft documents it as a
caveat to AL authors rather than as a promise, which means:

- **an implementation that makes `xRec` a full copy is MORE correct than BC**, and AL code written
  against the shared behaviour then behaves differently;
- **an implementation that shares state must decide WHICH state**, and the page does not say.

**CLAUDE.md's rule applies directly**: `~/Git/openerp/board` "is AUTHORITATIVE ABOUT THE QUESTION AND
NEVER ABOUT THE ANSWER", and its four `xRec` rounds are the question. **This page is the reason there
were four**: the semantics are underspecified in the platform itself.

**The decision this item takes: `xRec` is a full copy, and the deviation is recorded.** A shared-state
`xRec` cannot be specified from the documentation, cannot be tested against a specification that does
not exist, and its only observable consequence is that a write through `xRec` corrupts `Rec` -- which
the documentation tells authors not to do. Copying is deterministic, specifiable and strictly safer,
and it is the one place where being stricter than BC removes a hazard rather than adding one.

**What it costs is the copy**, and board:0018's eight-byte budget applies: `xRec` is a second record
instance per validate, per page row. board:0006 measures per-session bytes and this is one of its
entries.

`include/runtime/Table.h:1373` already takes a `BeforeImage` -- so the copy is what the tree does
today, and this item's first task is to confirm it and record it as a deviation rather than an
accident.

## `CurrPage` is a handle to the page's runtime state

> "You can access the CONTROLS of the page through `CurrPage` and set the **DYNAMIC PROPERTIES** of the
> page and its controls."
>
> **"`CurrPage.Editable` reflects the RUNTIME value of the `Editable` property, which can be changed at
> design time, PROGRAMMATICALLY, or BY THE USER when switching view modes."**
>
> **"When the View mode on a page is `false`, then the Edit, New, and Delete modes are `true`."**
>
> `CurrPage.Update([SaveRecord])` -- **"save the current record and then update the controls."**

board:0400 filed `Editable` and recorded that `CurrPage.Editable` mixes the declaration with the page
MODE, View excluded. **This page adds the mode arithmetic**: not-View implies Edit, New and Delete are
all true. So the mode is not an enumeration of four; it is View versus the rest.

And `CurrPage.Update(SaveRecord)` is board:0474's `UpdatePropagation` trigger -- the documentation
there says "add a `CurrPage.Update()` call ... to have the property take effect".

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

These are variables, not declarations; this sweep's pattern does not count them. CLAUDE.md records
`SetRange` at 55 402 as the comparable order of magnitude for a builtin. **Stated rather than
guessed** -- and `xRec`'s count is worth taking, because it sizes the per-session copy.

## The IST-state, and it is why this is `teilweise`

`include/runtime/Table.h:1373` -- `Validate` takes a `BeforeImage image(&before)` and restores on
throw. `include/runtime/Table.h:1375` -- `ValidatingField current(no)` implements `CurrFieldNo`.
`src/rt/Table.cpp:356` -- `PushBefore`/`PopBefore`/`CurrentBefore` maintain a before-image stack.

**So `xRec` and `CurrFieldNo` exist for the validate path.** Whether they exist on the INSERT, MODIFY,
DELETE and RENAME paths -- where board:0512's five event signatures also carry `xRec` -- is this
item's check and is not measured here.

## The choice

`xRec` is a full copy, taken where the platform documentation says the trigger sees one, and never
shared. `CurrPage` is a handle carrying the mode; `CurrReport` and `RequestOptionsPage` land with
board:0063.

## Ordering

board:0042's core. `xRec` on the modify/rename paths is needed by board:0512's dispatch, which passes
it to five of the ten trigger events.

## Gate, and its negative control

Inside `OnModify`, `xRec` holds the values before the change and `Rec` after; a write to `xRec` does
NOT change `Rec`.

**The negative control is the write to `xRec`** -- BC's own documentation says it MIGHT propagate, so
this assertion is the deviation made visible. A shared-state implementation fails it, which is the
point: the gate records which behaviour agiru has.
