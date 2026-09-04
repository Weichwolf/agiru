Type:     task
Status:   open
Parent:   0057
Area:     gen, rt
Source:   developer/attributes/devenv-integrationevent-attribute.md
Verdict:  deklariert
Class:    activation

# An `[IntegrationEvent]` exposes the publisher's globals when it says so

`[IntegrationEvent(IncludeSender: Boolean, GlobalVarAccess: Boolean [, Isolated: Boolean])]`

Three arguments, and the middle one has no counterpart in the other publisher kinds:

| argument | what it decides |
|---|---|
| `IncludeSender` | "whether global METHODS in the object that contains the event publisher method are exposed to event subscriber methods" |
| **`GlobalVarAccess`** | **"whether global VARIABLES in the object ... are accessible to event subscriber methods"** |
| `Isolated` | a transaction per subscriber (board:0057) |

**`GlobalVarAccess` is the one that constrains the C++ shape.** A subscriber reaching the
publisher's globals means the subscriber runs with a reference to the publishing INSTANCE and reads
its members -- so the publishing codeunit cannot be a stateless free function, and its globals
cannot be locals.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**94 269 `[IntegrationEvent` declarations** -- by far the largest attribute population in the tree
and the mechanism the BaseApp is built on.

## The IST-state

`src/gen/CodeunitWriter.cpp:29` recognises it in `IsPublisher` and emits an empty body. The three
arguments are parsed as part of the raw attribute string and never read; nothing raises.

## The choice

As board:0191: the emitted body calls board:0057's dispatcher with the object id, the method name
and the three flags as `constexpr` arguments. `GlobalVarAccess = true` additionally passes `*this`
as the sender EVEN WHEN `IncludeSender` is false, because the subscriber needs the instance to reach
a global variable.

**The two flags are therefore not independent in the emission**, and reading them as if they were is
the mistake: a subscriber with `GlobalVarAccess` and no `IncludeSender` still needs the instance.

## Ordering

After 0196 and board:0057's dispatcher. This is the item that makes the other 94 268 publishers
work, so it is the one that decides the dispatcher's cost -- board:0009's code-locality concern
applies at this population.

## Gate, and its negative control

A publisher with a global variable and `GlobalVarAccess = true`: the subscriber reads the value the
publisher set. The same with `GlobalVarAccess = false`: the subscriber must not compile against it.

**The negative control is the false case**, and it must fail at BUILD time -- if it fails at run
time the access was never restricted.
