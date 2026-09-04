Type:     task
Status:   open
Parent:   0057
Area:     gen, rt
Source:   developer/devenv-publishing-events.md, developer/devenv-raising-events.md
Verdict:  teilweise
Class:    activation

# A publisher is a signature with no body, and no subscribers means no call

**Two pages, one item**: publishing and raising are two halves of one mechanism and each page is a
page and a half. The subscriber half is board:0513's.

## The publisher's restrictions are compile-time and they define the C++ shape

> **"An event publisher method CAN'T INCLUDE ANY CODE except comments."**
>
> **"An event publisher method CAN'T HAVE A RETURN VALUE, VARIABLES, OR TEXT CONSTANTS."**
>
> "You can include as many **parameters of any type** as necessary."
>
> **"If you include the event publisher method in a PAGE object, THE PAGE MUST HAVE A SOURCE TABLE.
> Otherwise, you can't successfully create an event subscriber method to subscribe to the event."**
>
> `[IntegrationEvent(IncludeSender: Boolean, GlobalVarAccess: Boolean)]` ·
> `[BusinessEvent(IncludeSender: Boolean)]`

**Four checkable rules**, all decidable from the declaration and therefore `static_assert`s or
translation errors: no body, no return value, no local variables, no text constants; and a publisher
on a page requires board:0431's `SourceTable`.

**The empty body is what makes the generated shape trivial**: the generator emits a body that raises,
because there is no AL body to preserve. `src/gen/CodeunitWriter.cpp:29` already recognises the three
attributes -- so the generator knows which methods these are and currently emits them as ordinary
empty procedures.

**`IncludeSender` and `GlobalVarAccess` are the two arguments**, and board:0513 records that
`src/al/Parser.cpp:926` ignores everything after the first `(`. `IncludeSender` adds the publishing
object as a first parameter; `GlobalVarAccess` gives subscribers access to the publisher's globals --
which is a capability, not a flag, and it is the one that would let a subscriber read another object's
variables.

## Raising: the call vanishes when nobody listens

> **"If there are NO SUBSCRIBERS to the published event, then the line of code that calls the event
> publisher method IS IGNORED AND NOT EXECUTED."**

**That is not an optimisation, it is semantics**, because the ARGUMENTS are not evaluated either. A
raise whose argument list contains a call -- `Publisher.OnX(ExpensiveLookup())` -- does not perform the
lookup when nothing subscribes. An implementation that evaluates the arguments and then finds an empty
subscriber list is observably different wherever an argument has a side effect.

**And it is a performance property worth having**: with 11 142 subscribers over an unknown number of
publishers, most raises in a running system have no subscriber, and the check is one lookup against a
`constexpr` table -- board:0513's sorted array makes it a binary search or, better, a
generator-resolved constant when the publisher's subscriber set is empty at translation time.

**A raise with a statically empty subscriber set can be emitted as nothing at all.** That is available
because agiru merges extensions at translation time (board:0033) and therefore knows the whole
subscriber set -- which BC does not, since it binds at run time. **A deviation in mechanism, identical
in behaviour**, and it removes the call entirely rather than making it cheap.

> "the subscriber methods are run **one at a time in no particular order**" -- board:0513.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0191 and board:0196 own the attribute counts; board:0196 measured `[EventSubscriber]` at
**11 142**. The publisher counts belong to board:0191's family. **Stated rather than guessed.**

## The IST-state, and it is why this is `teilweise`

`src/gen/CodeunitWriter.cpp:29` -- `IsPublisher` returns true for `IntegrationEvent`, `BusinessEvent`
and `InternalEvent`. **So the generator already identifies publishers**; what it does with them is the
item's first check and is not measured here. board:0057 records that no dispatch exists.

## The choice

A publisher's generated body is one dispatch call and nothing else. The four restrictions become
translation errors. `IncludeSender` prepends the sender parameter, resolved by the generator.

**The empty-subscriber-set elision is applied where the set is statically empty**, and the check is
otherwise a lookup.

**`GlobalVarAccess` is refused until it has a design.** Giving a subscriber access to the publisher's
globals is a cross-object variable binding with no obvious C++ shape, and accepting it silently would
be accepting a declaration and doing nothing with it.

## Ordering

Behind board:0512's dispatch table and board:0513's subscriber array. With board:0191.

## Gate, and its negative control

A raise with two subscribers runs both; a raise with none performs no call AND does not evaluate its
arguments; a publisher with a body fails to transpile.

**The negative control is the argument with a side effect** -- `Publisher.OnX(Counter())` with no
subscribers must leave the counter at zero, and an implementation that evaluates arguments before
checking the subscriber list passes every gate that uses a literal argument.
