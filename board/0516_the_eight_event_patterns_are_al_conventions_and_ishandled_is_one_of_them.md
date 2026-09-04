Type:     task
Status:   open
Parent:   0057
Area:     rt
Source:   developer/types-of-events-for-extensibility.md, developer/devenv-use-ishandled-pattern.md, developer/devenv-use-ishandled-min-req.md
Verdict:  fehlt
Class:    silent-wrong-data

# The eight event patterns are AL conventions, and the runtime owes none of them

**Three pages, one item**: the pattern catalogue and the two `IsHandled` pages, which are one of its
eight entries in detail.

> Eight patterns, ranked by the documentation itself:
>
> | pattern | value | what it is |
> |---|---|---|
> | **Business events** | **very high** | notify that a business event occurred |
> | **OnBefore/OnAfter** | | before or after an operation, a procedure, or a specific line |
> | **Verify events** | | |
> | **Isolated events** | | board:0514 |
> | **Switch events** (manually bound) | medium | **"an event that ONLY MANUALLY BOUND subscribers should subscribe to"** |
> | **OnSkip events** | | |
> | **Handled events** | **low** | **"Subscribers MUST EXIT IF THE EVENT IS ALREADY HANDLED, thus these events DON'T SCALE. Consider using skip events."** |
> | **Discovery events** | **mostly obsoleted** | |

**Every one of these is an AL convention over one mechanism, and that is the item's whole point.** A
`var Handled: Boolean` parameter is an ordinary parameter; a "switch event" is an ordinary event with
`EventSubscriberInstance = Manual` (board:0471); a "verify event" is an ordinary event whose subscriber
raises. **The runtime owes exactly one thing: dispatch that passes `var` parameters by reference and
runs subscribers in order.**

**So this page's task is to establish that nothing here is a runtime feature** -- and to record why
that is worth an item rather than nothing:

**`IsHandled` only works if `var` is a real reference.** CLAUDE.md lists "an out parameter never
written" as an inherited failure mode, with the guard "`var` is a reference and the compiler checks it
-- closed in C++, provided the generator never copies". The handled pattern is where a copy would be
silent: every subscriber sets `Handled := true`, the publisher reads `false`, the default code runs
anyway, and the result is a duplicate posting rather than an error.

**And the pattern's own weakness is documented**: subscribers must exit if already handled, so with
N subscribers the first one wins and the rest are called and return immediately. board:0513's declared
order therefore decides WHICH subscriber handles it -- in BC that is unspecified, and here it is
`{ object id, procedure name }`. **A handled event is the one place where board:0513's deviation is
observable in behaviour rather than only in reproducibility.** That is worth writing down.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

The `Handled` parameter is a naming convention, not a declaration this sweep's pattern can count.
board:0196 measured `[EventSubscriber]` at **11 142**. A count of publisher methods carrying a
`var Handled: Boolean` parameter is a scan of signatures and belongs to this item when it is pulled --
**stated rather than guessed**, and it is the number that says how often the order deviation matters.

## The IST-state

board:0057: no dispatch. `src/gen/BodyWriter.cpp` emits procedure bodies; whether a `var` parameter of
a generated event publisher is emitted as a reference is board:0515's question and this item's
dependency.

## The choice

**Nothing, deliberately** -- and recorded as such, which is the difference between a decision and an
omission. The eight patterns need: `var` by reference, ordered dispatch, manual binding
(board:0471's `EventSubscriberInstance`), and isolation (board:0514). All four are already items.

**What this item delivers is the CHECK**: a gate that exercises the handled pattern end to end, because
it is the pattern that fails silently if any of the four is wrong, and it is the one the BaseApp uses
most for behaviour substitution.

## Ordering

Behind board:0512, board:0513, board:0514 and board:0471 -- it is their joint gate rather than new
work.

## Gate, and its negative control

A publisher raises an event with `var Handled: Boolean`; the first subscriber sets it true; the
publisher sees `true` and skips its default code; the second subscriber is still called and returns
immediately.

**The negative control is the publisher's view of `Handled`** -- if the generator copies the parameter,
every subscriber sets a copy, the publisher reads `false`, and the default code runs IN ADDITION to
the substituted behaviour. Nothing raises. That is a duplicate posting, and it is invisible to any
gate that only asserts the subscriber ran.
