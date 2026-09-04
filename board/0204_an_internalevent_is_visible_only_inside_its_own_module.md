Type:     task
Status:   open
Parent:   0057
Area:     gen, rt
Source:   developer/attributes/devenv-internalevent-attribute.md
Verdict:  deklariert
Class:    activation

# An `[InternalEvent]` may only be subscribed to from inside its own module

`[InternalEvent(IncludeSender: Boolean [, Isolated: Boolean])]` -- "It can only be subscribed to
from within the same module."

That sentence is the whole item. `BusinessEvent` and `IntegrationEvent` bind across app boundaries;
this one does not, and the restriction is what makes it usable as an internal hook without becoming
a contract.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**228 `[InternalEvent` declarations.**

## The IST-state

`src/gen/CodeunitWriter.cpp:29` recognises it in `IsPublisher` and emits an empty body. Nothing
raises, and the module restriction is not expressed anywhere.

## The choice

**The restriction is a LINK-TIME fact here, not a runtime check.** CLAUDE.md: "AN APP IS A LIBRARY
... the point is not build time but DIRECTION: the linker enforces what AL declares." An internal
event's dispatcher entry is emitted with internal linkage in its own app's library, so a subscriber
in another app cannot name it -- the same mechanism that already stops the Base Application from
seeing an extension.

**Why not a run-time module comparison.** It would work, cost a comparison per raise at 228 sites,
and move a check the linker performs for free into the hot path. The C++ shape says the AL rule
exactly; a run-time check would only say it later.

## Ordering

After 0196 and board:0057's dispatcher; alongside 0191 and 0203, which share the emission.

## Gate, and its negative control

A subscriber in the same app binds and runs. **The negative control is a subscriber in a DIFFERENT
app: it must fail to LINK**, naming the event. A run-time refusal there would mean the internal
linkage was not used and the restriction is only advisory.
