Type:     task
Status:   open
Parent:   0037
Area:     rt, gen
Source:   developer/properties/devenv-singleinstance-property.md, developer/properties/devenv-eventsubscriberinstance-property.md
Verdict:  fehlt
Class:    activation

# A `SingleInstance` codeunit is one object per session, and its subscribers bind with it

**Two pages, one item**: both govern how a codeunit's INSTANCE relates to the session, and the second
is meaningless without the first -- a manual subscriber is bound by `BindSubscription` on an instance,
and which instance depends on this property.

> **SingleInstance** (default **false**): **"all codeunit variables that use this codeunit use the
> SAME INSTANCE ... the same set of internal variables when the code is running on the same client.
> The codeunit remains instantiated UNTIL YOU CLOSE THE COMPANY."**
>
> **EventSubscriberInstance**: `StaticAutomatic` -- **"subscribers are automatically bound to the
> events they subscribe to"**; `Manual` -- **"subscribers are bound to an event only if
> `BindSubscription` is called from the code that raises the event."**

**"Until you close the company" is the lifetime**, and it is not the process: a single-instance
codeunit is per SESSION and per company, so it is `thread_local`-shaped state and never a global.
CLAUDE.md is explicit -- per-session arenas, `thread_local` for what belongs to one session, shared
structures read-only after startup -- and a single-instance codeunit is the one AL construct that
looks like a global and must not be one. With 10 000 sessions a shared instance is a data race with
the answer as the prize.

**And `Manual` subscribers are board:0057's binding half.** A manually-bound subscriber does not fire
until bound and stops when unbound, which is the mechanism the predecessor's snapshot/restore of event
bindings existed for -- and CLAUDE.md says not to port that apparatus, because it solves a Python
problem. What it does not say is that the SEMANTIC is still required, and this item is where that is
recorded.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`SingleInstance =` **866** (all necessarily `true`) · `EventSubscriberInstance =` **1 380**.

**866 single-instance codeunits** is a large per-session footprint if each holds state, and it is
board:0006's "a session costs a known number of bytes" with a number attached.

## The IST-state

`src/gen/CodeunitWriter.cpp` consumes `TableNo` and `Subtype` and nothing else; board:0037 records
that a codeunit member is a lazy handle; board:0057 records the event dispatch state.

## The choice

A bit on the codeunit descriptor, and a per-session instance table -- one entry per single-instance
codeunit, created on first use, destroyed when the session's company changes. Not a static, not a
`Meyers singleton`, not one per process.

`EventSubscriberInstance` selects whether the generator registers the codeunit's subscribers at
startup or leaves them for `BindSubscription`.

## Ordering

Behind board:0037 for the handle and board:0057 for the dispatch. Ahead of anything that measures
per-session bytes.

## Gate, and its negative control

Two variables of a single-instance codeunit in one session share state; the same codeunit in a second
session does not.

**The negative control is the second session** -- a process-wide instance passes the first assertion
and is a cross-tenant data leak, which is the worst failure in this sweep.
