Type:     task
Status:   open
Parent:   0057
Area:     gen, rt
Source:   developer/attributes/devenv-eventsubscriber-attribute.md
Verdict:  fehlt
Class:    activation

# An `[EventSubscriber]` binds to its publisher, and its six arguments all decide something

`[EventSubscriber(ObjectType, ObjectId, EventName, ElementName, SkipOnMissingLicense, SkipOnMissingPermission)]`

Every argument is load-bearing:

| argument | what it decides |
|---|---|
| `ObjectType`, `ObjectId` | which object publishes -- the first half of the dispatch key |
| `EventName` | which publisher method, matched by NAME and not by symbol |
| `ElementName` | for a TRIGGER event, the field or control it belongs to; empty for a method event |
| `SkipOnMissingLicense` | **default `false`, and false RAISES** rather than skipping |
| `SkipOnMissingPermission` | the same (board:0062) |

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**11 142 `[EventSubscriber` declarations.** It is the second-largest attribute population after
`[Test]`, and every one of them is a binding that does not happen today.

## The IST-state

`src/al/Parser.cpp:545` parses the attribute into the procedure's raw attribute list. Nothing reads
it: `grep -rn "EventSubscriber" src/gen/ src/rt/` returns nothing (2026-09-04). The 11 142
subscribers are translated as ordinary private methods that no one calls.

## The choice

- The generator emits, beside each codeunit, a `constexpr` array of its subscriptions: publisher
  object type and id, the event name as a `string_view`, the element name, the two skip flags, and
  the address of a thunk that calls the member.
- **Binding is by NAME and stays by name.** The publisher is another object, possibly in another
  app, and AL resolves it at load. A `constexpr` string comparison at startup over one array is the
  faithful shape; resolving it at translation time would break the app boundary the linker enforces.
- The subscriber's own codeunit is instantiated on first dispatch and cached per session
  (board:0037).

**Why not a call-site rewrite.** Turning a publisher into direct calls to its subscribers is
tempting and wrong: `IncludeSender`, `GlobalVarAccess` and the isolated-transaction flag are
properties of the DISPATCH, and an extension may add a subscriber the base app cannot see.

## Ordering

After board:0057's dispatcher exists; before board:0054, because 501 test procedures reach a handler
through an event.

## Gate, and its negative control

A publisher with two subscribers, one of them in another app: both run, in the declared order. A
subscriber whose codeunit the session may not execute RAISES when `SkipOnMissingPermission` is
false and is SKIPPED when it is true.

**The negative control is the skip pair** -- they must differ. A dispatcher that always runs the
subscriber passes the first case and proves nothing about the flags.
