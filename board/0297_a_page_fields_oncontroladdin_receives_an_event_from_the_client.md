Type:     task
Status:   open
Parent:   0034
Area:     rt, gen
Source:   developer/triggers-auto/pagefield/devenv-oncontroladdin-pagefield-trigger.md, developer/triggers-auto/pagefieldextension/devenv-oncontroladdin-pagefieldextension-trigger.md
Verdict:  fehlt
Class:    activation

# A page field's `OnControlAddIn` receives an event the client sent to the server

```al
trigger OnControlAddIn(Index: Integer; Data: Text)
```

"Executed when a control add-in **on the client** sends event information to the server-side business
logic." `Index` is "an integer identifier that a control add-in sends with the event" and `Data`
carries its payload as text.

**It is the only page trigger whose caller is outside the process.** Every other trigger in this
family is raised by the page runtime; this one is raised by a browser control posting to the server.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnControlAddIn(` on a page field or extension: **73 declarations.**

## The IST-state

No page runtime, and **ControlAddIn has no generator at all** -- board:0034's object-kind table
records it as one of the kinds with no writer. So neither the add-in nor the trigger that receives
its events exists.

## The choice

Deferred with board:0034, and named here so the dependency is visible: the trigger is trivial once
there is a transport, and the transport is the whole of the ControlAddIn object.

**What it constrains in phase 2**: CLAUDE.md's htmx design has the server holding the state and
sending HTML fragments, which is a transport in the same direction -- so whatever carries a control
add-in's event is the same channel a page's own interactions use, and designing that channel without
this trigger in view means designing it twice.

## Ordering

Blocked on board:0034 (ControlAddIn has no generator) and board:0030.

## Gate, and its negative control

A control add-in that posts an event with index 3 and a payload: the trigger receives both.

**The negative control is the index** -- a transport that delivers the payload and drops the index
leaves a page with several add-in events unable to tell them apart, and a single-event test passes.
