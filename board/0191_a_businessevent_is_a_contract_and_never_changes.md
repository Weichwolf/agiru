Type:     task
Status:   open
Parent:   0057
Area:     gen, rt
Source:   developer/attributes/devenv-businessevent-attribute.md
Verdict:  deklariert
Class:    activation

# A `[BusinessEvent]` publishes a contract, and its two arguments decide the dispatch

`[BusinessEvent(IncludeSender: Boolean [, Isolated: Boolean])]`

| argument | what it decides |
|---|---|
| `IncludeSender` | "the firing instance of the object is available as a parameter to subscribers" -- a subscriber may declare a `sender` parameter typed to the publishing codeunit |
| `Isolated` | each subscriber runs in its own transaction, errors roll that one back and the next still runs (board:0057) |

A business event is the kind BC promises not to break: it is a published contract rather than an
internal hook. That matters to the RUNTIME only in that its subscribers must bind exactly as
`IntegrationEvent`'s do -- the difference is a promise to developers, not a dispatch rule.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**4 `[BusinessEvent` declarations**, against 94 269 `[IntegrationEvent`. It is the rarest publisher
kind by four orders of magnitude, and that is the finding: the BaseApp almost never promises.

## The IST-state

`src/gen/CodeunitWriter.cpp:29` recognises it in `IsPublisher`, so the procedure is emitted with an
EMPTY BODY -- correct, because a publisher's body is empty in AL too. What does not exist is the
raise: nothing calls the subscribers, and `IncludeSender` and `Isolated` are parsed and dropped.
Verdict `deklariert` rather than `fehlt` for exactly that reason.

## The choice

The publisher's emitted body becomes a call into board:0057's dispatcher with its own object id and
method name, plus the two flags as `constexpr` arguments. `IncludeSender` decides whether `*this` is
passed; `Isolated` decides whether the dispatcher opens a transaction per subscriber.

**Why the flags are `constexpr` arguments and not a table lookup.** They are known where the
publisher is emitted, and passing them makes the dispatcher's per-subscriber branch a compile-time
constant at each of the 94 273 publisher sites.

## Ordering

After 0196 (subscribers bind) and after board:0057's dispatcher. With 4 sites it is the smallest
publisher kind and the natural first one to make raise.

## Gate, and its negative control

A `[BusinessEvent(true)]` whose subscriber declares a `sender` parameter: the subscriber must
receive the publishing instance. The same event declared `[BusinessEvent(false)]` must not compile a
subscriber with a sender.

**The negative control is the second.** A dispatcher that always passes the sender makes
`IncludeSender` decorative.
