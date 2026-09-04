Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/devenv-subscribing-to-events.md
Verdict:  fehlt
Class:    activation

# Subscribers run in no particular order, and agiru declares one anyway

board:0196 filed `[EventSubscriber]` from the attribute page. **This page is the semantics**, and it
carries one sentence that collides head-on with a CLAUDE.md invariant.

> **"When an event is raised, the subscriber methods are run ONE AT A TIME IN NO PARTICULAR ORDER.
> YOU CAN'T SPECIFY THE ORDER in which the subscriber methods are called."**

**CLAUDE.md: "DETERMINISM IS COMPULSORY ... Anything assembled from concurrent work is combined in a
DECLARED order, never in completion order."**

**11 142 subscribers, and the platform declines to order them.** So either agiru is non-deterministic
at 11 142 points, or it declares an order BC does not have -- and only the second is compatible with
the invariant. **The order is declared: subscriber object id, then procedure name**, both available at
translation time and both stable across runs and machines.

**That deviation makes agiru stricter than BC**, like board:0500's upgrade-codeunit order, and it has
a consequence worth stating: AL code that depends on subscriber order is broken in BC and works here,
which HIDES a defect rather than causing one. Acceptable, and worth knowing.

## The six attribute arguments, and which are optional

> `[EventSubscriber(ObjectType::<Type>, <Object>, '<Event Name>', '<Element Name>', <SkipOnMissingLicense>, <SkipOnMissingPermission>)]`
>
> | argument | | optional |
> |---|---|---|
> | object type | **`Codeunit`, `Page`, `Report`, `Table`, `XMLPort`** -- five kinds | no |
> | object | an id, or `Codeunit::"MyPublishers"` / `Database::"Customer"` | no |
> | event name | the publisher method's name | no |
> | **element name** | **"only requires a value ... when the object type is `Table` and the event is a VALIDATE trigger event"** | no |
> | `SkipOnMissingLicense` | **"skip the call if the user's license doesn't cover the subscriber codeunit. If `false`, AN ERROR IS THROWN and code execution stops. `false` is the default."** | yes |
> | `SkipOnMissingPermission` | the same for permissions | yes |

**The two skip flags default to FAILING**, not to skipping. A subscriber whose codeunit the user
cannot execute stops the operation -- a posting fails because an unrelated extension's subscriber is
not licensed. That is BC's documented behaviour and it is why the flags exist.

**Five publisher object kinds**, so board:0512's dispatch table is keyed over all five and not over
codeunits alone. And the element name is required only for validate events, which matches board:0512's
finding that those have their own signature.

> "With 2023 release wave 1, the event publisher parameter **supports being an IDENTIFIER** ... prior
> to this release the parameter was **string literals only**."

Both spellings occur in BCApps and the parser must accept both.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0196: `[EventSubscriber]` **11 142** declarations.

## The IST-state

`src/al/Parser.cpp:545` reads every attribute as raw text; `src/al/Parser.cpp:926`'s `HasAttribute`
matches by name and **ignores everything from the first `(`** -- so the six arguments are never
parsed. board:0057 records that no dispatch exists.

## The choice

The generator parses the six arguments and emits one `constexpr` subscriber entry per declaration,
**sorted by `{ object id, procedure name }` at translation time** so the declared order is a property
of the emitted array and of nothing at run time. The two skip flags are two bits, defaulting to
`false` = fail.

**Sorted by the generator, not at startup.** CLAUDE.md names a lazily sorted global as a data race the
catalogue already had once.

## Ordering

Behind board:0512's dispatch shape. board:0057's core.

## Gate, and its negative control

Two subscribers to one event run in object-id order, and in the same order on a second run and on
another machine; a subscriber with `SkipOnMissingPermission = false` and no permission stops the
operation.

**The negative control is the second run** -- a hash-map iteration order is stable within a process
and differs between builds, so a single-run gate passes. A digest over two runs is what the
determinism invariant actually asks for.
