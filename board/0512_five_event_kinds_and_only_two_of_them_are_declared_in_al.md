Type:     task
Status:   open
Parent:   0057
Area:     rt, gen
Source:   developer/devenv-events-in-al.md, developer/devenv-event-types.md
Verdict:  fehlt
Class:    activation

# Five event kinds, and only three of them are declared in AL

**Two pages, one item**: the overview and the type taxonomy. The second is the first's table expanded,
and neither is a task alone.

## The five kinds, and what distinguishes them

| kind | declared by | raised by | contract |
|---|---|---|---|
| **BusinessEvent** | `[BusinessEvent]` on a method | AL code calling it | **"a formal contract that carries an IMPLICIT PROMISE NOT TO CHANGE in future releases"** |
| **IntegrationEvent** | `[IntegrationEvent]` | AL code | no such promise; may expose implementation details |
| **InternalEvent** | `[InternalEvent]` | AL code | **only within the same extension** |
| **Global** | nothing -- **predefined**, published as integration events by named BaseApp codeunits | the BaseApp | |
| **Trigger** | nothing -- **predefined** | **the runtime** | **"can't be raised programmatically"** |

> **"An event publisher method is COMPOSED OF A SIGNATURE ONLY and doesn't execute any code."**
>
> **"Publishing an event doesn't actually do anything apart from making it available for
> subscription."**
>
> **"There can be MULTIPLE SUBSCRIBERS to a single event publisher method. However, a PUBLISHER HAS NO
> KNOWLEDGE OF SUBSCRIBERS, if any."**
>
> **"Trigger events DON'T APPEAR AS METHODS IN AL for a table or page object."**

**The publisher-body-is-empty rule is what makes the C++ shape obvious**: a publisher method is a
dispatch call and nothing else, so `src/gen/CodeunitWriter.cpp:29`'s `IsPublisher` (which already
recognises the three attributes) can emit a body that raises and returns, with the `var` parameters
carried back. board:0196 owns the subscriber side; this is the publisher side and it is smaller than
it looks.

**And "the publisher has no knowledge of subscribers" is the dispatch requirement**: a publisher
cannot be a direct call, and a subscriber list must be reachable from the publisher's identity alone.

## Global events are a NAMED LIST in the BaseApp, not a platform feature

> "Global events are predefined system events **automatically raised by various base application
> codeunits** ... defined as **integration event publishers by local methods** in the following
> codeunits."
>
> | codeunit | events |
> |---|---|
> | **9170** Conf./Personalization Mgt. | `OnRoleCenterOpen`, `OnBeforeLogInStart`, `OnAfterLogInEnd`, `OnBeforeCompanyOpen`, `OnAfterCompanyOpen`, `OnBeforeCompanyClose`, `OnAfterCompanyClose` |
> | **42** TextManagement | `OnBeforeMakeTextFilter`, `OnAfterMakeDateTimeFilter`, `OnAfterMakeDateFilter`, `OnAfterMakeTextFilter`, `OnAfterMakeTimeFilter` |
> | **42** Caption Class | `OnResolveCaptionClass`, `OnAfterCaptionClassResolve` |
> | **44** ReportManagement | `OnAfterGetPrinterName`, `OnAfterDocumentReady`, `OnAfterDocumentPrintReady`, `OnAfterIntermediateDocumentReady`, `OnAfterDocumentDownload`, `OnAfterSetupPrinters`, `OnAfterGetPaperTrayForReport`, `OnAfterHasCustomLayout` |

**So a "global event" is an ordinary integration event in a BaseApp codeunit** -- there is nothing
platform-specific about it, and the runtime must NOT know these names. That is CLAUDE.md's invariant
exactly, and it is the strongest confirmation in this sweep: the page's own explanation is that they
are AL, in codeunits, with ids.

**Three of them are already cited elsewhere in this sweep**: board:0384's `CaptionClass` resolution is
`OnResolveCaptionClass` in codeunit 42, board:0437's `AutoFormat` is the same shape, and
codeunit 42's `TextManagement` events are board:0509's filter-string construction. **What the runtime
owes is the RAISE POINT, not the handler** -- the runtime raises where the platform documentation says
it raises, and the BaseApp's transpiled codeunit does the work.

**And that RAISE POINT is the gap this item names**: the runtime has to call `OnResolveCaptionClass`
without naming codeunit 42. It does so by raising an event identified by publisher object and event
name, both of which come from the generated catalogue -- which is what board:0057 has to build.

## The ten database trigger events, with their exact signatures

> `[EventSubscriber(ObjectType::Table, Database::<Table>, '<Event>', '<Field>', <SkipOnMissingLicense>, <SkipOnMissingPermission>)]`

| event | subscriber signature |
|---|---|
| `OnBeforeInsertEvent` / `OnAfterInsertEvent` | `(var Rec: Record; RunTrigger: Boolean)` |
| `OnBeforeModifyEvent` / `OnAfterModifyEvent` | `(var Rec; var xRec; RunTrigger: Boolean)` |
| `OnBeforeDeleteEvent` / `OnAfterDeleteEvent` | `(var Rec; RunTrigger: Boolean)` |
| `OnBeforeRenameEvent` / `OnAfterRenameEvent` | `(var Rec; var xRec; RunTrigger: Boolean)` |
| `OnBeforeValidateEvent` / `OnAfterValidateEvent` | `(var Rec; var xRec; **CurrFieldNo: Integer**)` |

**Insert and delete carry no `xRec`; modify, rename and validate do.** And the two validate events take
`CurrFieldNo` where the others take `RunTrigger` -- **five signatures, not one**, and the fourth
attribute argument is the FIELD NAME for validate events and empty for the rest.

board:0244-0266 filed these ten with their subscriber counts; **this page is where their signatures
come from**, and a dispatcher with one signature cannot carry them.

`SkipOnMissingLicense` and `SkipOnMissingPermission` are the fifth and sixth attribute arguments --
board:0062's and board:0381's gates, per subscriber.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0196: `[EventSubscriber]` **11 142** declarations. board:0244-0266 measured the per-event
subscriber counts. board:0252/0253 measured the validate events split: table 182/398, page 2/22.

## The IST-state

`src/gen/CodeunitWriter.cpp:29` -- `IsPublisher` recognises `IntegrationEvent`, `BusinessEvent` and
`InternalEvent`. board:0057 records that no dispatch exists. `include/runtime/Table.h:353` calls the
table's own `OnInsert` and no event.

## The choice

One dispatch table keyed by `{ publisher object, event name, element name }`, built at translation
time from the generated catalogue, with **five signature shapes** rather than one -- which C++
expresses as five function-pointer types and a tagged table, not a variadic.

**No AL object name in `src/`**: the global events are found through the catalogue like any other.

## Ordering

board:0057's core. Before board:0384 and board:0437, which are two of its consumers, and before
board:0244-0266, which are its per-event items.

## Gate, and its negative control

A subscriber to `OnAfterInsertEvent` runs after the insert with `Rec` populated and `RunTrigger`
reflecting the call; a subscriber to `OnAfterValidateEvent` receives `xRec` and `CurrFieldNo`.

**The negative control is the validate subscriber's third parameter** -- a dispatcher built on the
insert signature passes `RunTrigger` where `CurrFieldNo` belongs, which is a Boolean where an Integer
is expected and will silently be 0 or 1: a subscriber that branches on the field number then handles
field 1 for every field.
