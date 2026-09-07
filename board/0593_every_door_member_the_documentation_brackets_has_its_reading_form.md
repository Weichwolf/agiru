Type: root
Area: net, rt

# Every door member the documentation brackets has its reading form

`methods-auto/` writes a property-shaped member as `[X := ] Type.Member([NewX])` -- BOTH the return
and the parameter bracketed -- and that is TWO signatures: a reader and a writer. The door has the
writer everywhere and the reader in some places. Measured 2026-09-06 over the 135 types: **26
members whose reading form the documentation states and the door lacks**, of which `Page.Editable`
and `Page.Activate` were closed when a generated page called them.

The rest, by type:

| type | members |
|---|---|
| `RecordRef` | `AddLoadFields`, `Ascending`, `ChangeCompany`, `CurrentKeyIndex`, `Delete`, `FilterGroup`, `Find`, `FindSet`, `Mark`, `MarkedOnly`, `Modify`, `Next`, `ReadIsolation`, `SecurityFiltering`, `SetAutoCalcFields`, `SetLoadFields`, `Truncate` |
| `TextBuilder` | `Length` (written; the reader is the same call with no argument) |
| `HttpRequestMessage` | `Content` |
| `ErrorInfo` | `RecordId` |
| `TestHttpResponseMessage` | `ReasonPhrase` |

**The trap this closes:** a call site that READS compiles only if the reader exists, and until it
does the diagnostic names the WRITER's parameter -- `too few arguments to function call, single
argument 'NewEditable' was not specified` -- which points at the door rather than at the AL.

**The measure:** a script over the `## Syntax` line of all 1 876 method pages; a member whose line
starts with `[` and whose first parenthesis is followed by `[` needs both forms. It is the same
sweep that took 51 wrong `[[nodiscard]]`s off the door, run the other way round.

## RE-MEASURED 2026-09-07: 25 OF THE 26 WERE ALREADY READABLE

The list was derived from the documentation and never checked against the door, and the door had
answered it a different way: a DEFAULT ARGUMENT. `RecordRef.Ascending(Boolean SetAscending = {})`
is one declaration that serves both forms, so `Rec.Ascending()` compiles and the item's diagnostic
never appeared. Each of the seventeen `RecordRef` members and `TextBuilder.Length`,
`ErrorInfo.RecordId` and `TestHttpResponseMessage.ReasonPhrase` was compiled with no argument, and
every one of them stands.

**One was real and it is closed:** `HttpRequestMessage.Content()`. The writer carried
`= {}` on a parameter of type `HttpContent`, which the header only FORWARD DECLARES -- a default
argument of an incomplete type is declarable and not callable, so the reading form failed with
`calling 'Content' with incomplete return type` rather than with the arity diagnostic this item
predicted. It is two overloads now, and neither needs the type to be complete at the declaration.

**The finding that survives is about the measure and not about the door.** A sweep over the
documentation says what the door OWES; only a compile says what it HAS, and a default argument is
invisible to the first and decisive for the second. The board's own rule -- a reason ages and a
finding does not -- applies to this item's own table.

**What is left of the item:** the sweep is worth keeping and worth running the other way round --
from the door's declarations to the documentation -- because a default argument that AL does not
declare is a signature this tree invented, and that is the failure mode this measurement cannot see
either.
