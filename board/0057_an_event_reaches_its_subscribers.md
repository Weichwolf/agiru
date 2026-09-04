Type: root
State: open
Area: gen, rt
Tags: navision, semantics, blocker

# An event reaches its subscribers, and a publisher nobody subscribed to says so

`src/gen/CodeunitWriter.cpp:28` knows what a publisher is -- `IsPublisher` matches
`[IntegrationEvent]`, `[BusinessEvent]` and `[InternalEvent]` -- and emits it as a function with
**no body at all**:

```cpp
void SalesPostPrepaymentYesNo::OnPostPrepmtInvoiceYNOnBeforeConfirm(tables::SalesHeader &, Boolean &) {}
```

`[EventSubscriber(...)]` occurs **nowhere in `src/` or `include/`** (measured 2026-09-04): the
attribute is parsed into `ProcedureDecl::attributes` and never read again, so a subscriber is
emitted as an ordinary procedure that nothing calls. `BindSubscription` and `UnbindSubscription`
are door refusals (`src/rt/Builtins.cpp:593`, `:766`).

**So every event in the tree fires into nothing, and nothing says so.** That is the
silent-wrong-data class at the largest scale this tree has: an empty publisher body is a
successful call.

## The population, measured 2026-09-04 over the read roots `apps.json` names

| | |
|---|---:|
| `[IntegrationEvent(...)]` publishers | **23 920** |
| `[InternalEvent(...)]` | 90 |
| `[BusinessEvent(...)]` | **0** -- the read roots declare none |
| of them isolated (third argument `true`) | 10 |
| `[EventSubscriber(...)]` | **3 753**, in 807 files |
| codeunits carrying `EventSubscriberInstance = Manual` | 380 |

| the subscription's `ObjectType` | |
|---|---:|
| Codeunit | 1 865 |
| Table | 1 428 |
| Page | 398 |
| Report | 52 |
| XmlPort | 10 |

**753 of the 3 753 subscribe to a PLATFORM TRIGGER EVENT** -- one the runtime raises itself and no
AL body publishes: `OnAfterDeleteEvent` 142, `OnBeforeInsertEvent` 122, `OnAfterInsertEvent` 98,
`OnAfterModifyEvent` 72, `OnAfterRenameEvent` 67, `OnAfterValidateEvent` 44, `OnModifyRecordEvent`
38, `OnOpenPageEvent` 22, and eleven more. 69 carry an `ElementName`, which for a table event is the
FIELD the validate event belongs to.

**And the `IsHandled` idiom is what makes the silence dangerous rather than merely incomplete.**
1 311 generated sources use it: the caller declares `IsHandled: Boolean`, raises the event, and
skips its own default path if a subscriber set it. With an empty publisher the flag stays false, so
the default path always runs -- which looks right and is exactly wrong wherever an app replaces the
default. `devenv-use-ishandled-pattern.md` is 279 lines about this one shape.

## What the platform documents, and four facts decide the design

`devenv-event-types.md`, `devenv-publishing-events.md`, `devenv-subscribing-to-events.md`,
`devenv-events-isolated.md`, `attributes/devenv-eventsubscriber-attribute.md`.

- **THE ORDER AROUND A DATABASE OPERATION IS TABULATED AND IT INTERLEAVES WITH THE TABLE TRIGGER**,
  which makes this item and board:0029 one lifecycle and not two:

  | # | what runs | example |
  |---|---|---|
  | 1 | trigger event (before) | `OnBeforeDeleteEvent` |
  | 2 | the table trigger | `OnDelete` |
  | 3 | the global table trigger in a codeunit | `OnDatabaseDelete` |
  | 4 | the database operation | the row goes |
  | 5 | trigger event (after) | `OnAfterDeleteEvent` |

- **A TRIGGER EVENT IS PUBLISHED BY THE RUNTIME AND CANNOT BE RAISED FROM AL.** "Trigger events
  don't appear as methods in AL for a table or page object." There are 10 table events and 13 page
  events (`triggers-auto/events/`, 23 files), each with a FIXED signature: `var Rec`, `var xRec`,
  `RunTrigger`, `CurrFieldNo`, `AllowInsert`/`AllowModify`/`AllowDelete`/`AllowClose`. So they need
  board:0042's stored image and board:0029's `RunTrigger` parameter, and they are the reason a
  `tableextension` can react to a table that declares no trigger of its own.
- **BINDING IS A PROPERTY OF THE SUBSCRIBING CODEUNIT.** `EventSubscriberInstance` is
  `StaticAutomatic` (the default -- bound for the life of the session) or `Manual` -- bound only
  between `BindSubscription` and `UnbindSubscription`. 380 codeunits in the read roots are `Manual`,
  and that is what `BindSubscription` exists for.
- **`SkipOnMissingLicense` and `SkipOnMissingPermission` default to `false`, and false means ERROR.**
  "`true` skips the subscriber call ... The default value, `false`, raises an error instead." So the
  subscriber list is not merely filtered by permission -- a missing permission on a subscriber's
  codeunit is a raised error (board:0062).

**An isolated event is a transaction per subscriber**: "the transaction is created before invoking
an event subscriber, then committed afterwards", an error rolls that one back and execution
continues with the next. Only `TableType: Normal` writes roll back; variables, HTTP and single
instance members do not. Ten publishers in the read roots ask for it, and the note beside it is
load-bearing for the posting invariant: **a write transaction must commit before raising an isolated
event, or the event is raised as an ordinary one.**

## What the predecessor paid for, and it answers the two questions that look hardest

`~/Git/openerp/board`, four items, each with its A/B over the test net:

| item | finding | measured |
|---|---|---|
| **WI-1036** | **a codeunit that is ONLY a subscriber is never loaded, so its subscriptions never bind and the event fires into nothing.** 182 of 400 checked (~45 %) are pure subscribers. AL binds at APP LOAD, so eager is the faithful behaviour | 2 968 subscriptions activated, **LOST 0** |
| **WI-1127** | **`BindSubscription` binds to the VARIABLE, not to the method.** The W1 idiom binds once behind `if IsInitialized then exit`, so a per-method reset kills it from method two onward -- and the aggregate then reported 0 instead of the sum, with no error. 144 test codeunits bind on a codeunit global, 99 of them behind `IsInitialized`; 984 bindings are method-local. The binding outlives the METHOD and not the CODEUNIT, or the next codeunit's probe arrives as a second subscriber | 716/874 -> 800/874, **GAINED 84** |
| **WI-1169** | a `TestPage` must raise `OnOpenPageEvent`/`OnClosePageEvent` -- the platform raises page trigger events whichever way the page was opened. 108 subscribers on 55 pages | GAINED 2 |
| **WI-1145** | `UnbindSubscription` returned nothing where AL returns `Boolean` | -- |

**THE ACTIVATION RISK IS ALREADY MEASURED AND IT IS ZERO.** CLAUDE.md classes this as `activation`
-- a dead path that starts running -- and demands an A/B because cases go green over a no-op.
WI-1036 ran exactly that A/B on the same BaseApp: 2 968 newly live subscriptions across 122
codeunits, **no regression**. That does not transfer as a guarantee, but it removes the argument
that switching subscribers on is dangerous in itself.

## The choice

**THE BINDING IS `constexpr` DATA AND THE DISPATCH IS A SORTED LOOKUP, because the set is closed at
translation time.** Every subscription names its publisher by object type, object id, event name and
element name, and the generator has all four in the attribute. So per app, one `constexpr` array of
{ObjectType, ObjectId, event name, element name, instance kind, thunk}, sorted, in `.rodata` -- the
same shape as `kTestMethods` and the field tables. No registry assembled by static initialisers, no
`std::function`, nothing before `main`.

- **A publisher's emitted body raises the event instead of being empty.** It looks up its own
  (object, name) key and calls each bound subscriber in a DECLARED order -- determinism is
  compulsory, so the order is the sorted key and never load order.
- **The `var` parameters are references and the compiler already checks them**, which is what makes
  `IsHandled` work without any further machinery: a subscriber writing the flag writes the caller's
  variable.
- **A SUBSCRIBER'S SIGNATURE IS NOT THE PUBLISHER'S PARAMETER LIST**, and the two flags on the
  publisher decide the difference. `attributes/devenv-integrationevent-attribute.md`:

  | flag | what a subscriber may then declare |
  |---|---|
  | `IncludeSender` | a leading `Sender: Codeunit <publisher>` parameter, "added manually" |
  | `GlobalVarAccess` | trailing parameters matching the publisher's GLOBAL VARIABLES, by name and type |

  Measured over BCApps 2026-09-04: **87 039 publishers are `(false, false)`, 7 211 are
  `(true, false)` and 4 are `(true, true)`.** So the sender parameter is in 7.7 % of the corpus and
  the global-variable form is in four places. A thunk generator that assumed the parameter lists
  match would be right 92 % of the time, which is the worst kind of nearly-right: the 7 211 fail at
  the C++ signature and the 4 fail silently if the shapes happen to line up.

  The `Sender` is the publishing OBJECT, which is `Instance<T>` (board:0037) and free to pass.
  `GlobalVarAccess` reaches the publisher's private state, and the page itself says not to use it --
  "Avoid `GlobalVarAccess`. Use event parameters instead" -- so four call sites decide whether it is
  worth any machinery at all.
- **A subscriber is bound because its APP was linked, not because something named its codeunit.**
  That is WI-1036 in C++ terms and it costs nothing here: the array is emitted per app and the
  linker already decides which apps exist.
- **`Manual` subscribers are the only dynamic half**, and they live in one per-session list keyed by
  the codeunit VARIABLE (WI-1127), released when the variable dies and not when the method returns.
- **The platform trigger events are raised by the runtime**, from the same places board:0029's
  triggers are, in the tabulated order -- before-event, table trigger, operation, after-event.
- **AND SO IS THE SESSION'S OWN LIFECYCLE EVENT.** `devenv-oncompanyopencompleted.md`:
  `OnCompanyOpenCompleted` is a **platform-based isolated event** raised during sign-in when the
  company is opened, and the System Application subscribes to it and raises `OnAfterLogin` -- which
  is what every app is told to subscribe to. So the chain is platform -> System Application -> app,
  and **a runtime that never raises the first link leaves every `OnAfterLogin` subscriber dead**.
  openerp filed exactly that (WI-1216, "the platform does not raise the company-open event"). The
  obsolete `OnCompanyOpen` is the non-isolated ancestor: "a failure in any event subscriber will
  stop the sign-in process", which is why the replacements are isolated.
- **A subscription whose publisher this run does not have is a HOLE WITH A COUNT**, refused at
  translation time with object and event named. It is not a silent drop: a subscription that binds
  to nothing is exactly the defect WI-1036 measured.

## Gate

A publisher with two subscribers calls both, in the declared order, and a `var` parameter written by
the first is seen by the second. `IsHandled` set by a subscriber suppresses the caller's default
branch. A table `OnBeforeInsertEvent` runs before the table's own `OnInsert` and
`OnAfterInsertEvent` after the row is written. A `Manual` subscriber does not run until
`BindSubscription`, runs after it, and stops at `UnbindSubscription`. A subscription naming an
event no object publishes fails the translation with both names in the message.

**Negative control**: emit the publisher body empty again and require the `IsHandled` case to go
red. A gate that only counts subscribers passes over an empty body, which is the state this item
found.

**AND ISOLATION IS SUSPENDED DURING INSTALL AND UPGRADE.** `devenv-events-isolated.md`, re-read in
full 2026-09-04 (board:0071): "When the operation is installing, uninstalling, or upgrading
extensions, **isolated events aren't run isolated. The events run normally instead.** The reason ...
is that these operations require that all operations within them are done in one transaction. So
explicit `Commit` calls can't be made during the operations."

So the `Isolated` flag is not a property of the publisher alone -- it is `Isolated AND NOT
installing`, and board:0070 owns the second half. The consequence is the one that matters for
CLAUDE.md's first invariant: **during an upgrade, a failing subscriber fails the whole upgrade**,
which is the correct behaviour for a schema change and the wrong one for a sign-in. One flag, two
answers, decided by the session's mode rather than by the declaration.

## `IsHandled` IS 180 075 OCCURRENCES AND IT MAKES SUBSCRIBER ORDER OBSERVABLE

`devenv-use-ishandled-pattern.md` (read 2026-09-04, board:0071) is a page arguing AGAINST the
pattern, and everything that makes it a bad pattern is a requirement on the dispatcher:

> According to the pattern's definition, **only one subscriber can handle the event. All other
> subscribers must exit if the event is handled**, so the subscriber code should start by checking
> whether the event was already handled. ... **There's no way to enforce the rule** ... A subscriber
> can break the rules and, for example, **change the value back from True to False**. ... There are
> **over 8,000 `IsHandled` events.**

Measured 2026-09-04 over `~/Git/BCApps/src`: **180 075 occurrences of `IsHandled`**, of which
**35 286 are `var IsHandled: Boolean` declarations**. It is the most-used single idiom this sweep has
counted.

**Three requirements fall out, and this item already owes two of them:**

- **A `var` parameter must survive the whole subscriber chain** -- each subscriber sees what the
  previous one wrote. That is the same `var`-through-dispatch path board:0066's `OnResolveAutoFormat`
  and board:0063's `TargetStream` need; three families, one mechanism.
- **THE ORDER IS OBSERVABLE.** Because the convention is "first handler wins and the rest exit", the
  answer depends on which subscriber runs first -- and BC does not define it. **CLAUDE.md does**:
  anything assembled from concurrent work is combined in a DECLARED order. So agiru dispatches
  subscribers in a declared order (app dependency order, then object id, then declaration order) and
  is MORE defined than the platform. That deviation is in the safe direction and has to be written
  down, because a test that passes here and fails in BC is as much a finding as the reverse.
- **The publisher reads the flag AFTER the chain**, and the documented shape is
  `OnFoo(IsHandled); if not IsHandled then Error(...)` -- so a publisher with no subscribers RAISES,
  which is the opposite of board:0057's usual failure mode. An event that reaches nobody is not
  always silent.

## THE CENSUS, AND IT IS TWO ORDERS OF MAGNITUDE ABOVE THE ESTIMATE

Measured 2026-09-04 over `~/Git/BCApps/src` (board:0575, while counting the report pipeline's eight):

| | count |
|---|---:|
| `[IntegrationEvent(` | **94 269**, across 5 461 files |
| `[EventSubscriber(` | **11 135** |
| `[BusinessEvent(` | **4** |
| `var IsHandled` parameters | **35 392** |
| `.al` files in the tree | 36 673 |

**CLAUDE.md says "the BaseApp wires hundreds of `[EventSubscriber]`s inside itself".** It is
**11 135**, with **94 269** publishers for them to attach to -- seventeen publishers per file that
declares any. The sentence's POINT stands, and stands harder: events are not optional. Its NUMBER is
two orders of magnitude low, and this item is where the real one belongs.

**`[BusinessEvent(` at 4** is the other end of the same scale: the distinction between an integration
event and a business event is real and is not a population.

**`var IsHandled` at 35 392** is board:0516's trap counted directly -- one AL procedure in
seventy-three takes a parameter with that name, and every one is a `var` the generator must not copy.
