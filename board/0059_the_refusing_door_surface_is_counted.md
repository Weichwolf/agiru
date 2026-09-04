Type: arc
State: open
Area: rt, net, build
Tags: gate, measured

# The door's refusing surface is a counted baseline, its denominator is whole, and a refusal names the item that owns it

`test/surface-baseline` reads `1173 1253`: 1 173 of the 1 253 documented AL method NAMES appear in
the door. That number says the surface is 93.6 % complete and it is measured honestly -- but the
tree has **no counter at all** for the other half of the same question, which is how many of those
declarations do anything.

## Measured 2026-09-04

| | |
|---|---:|
| documented method names, `methods-auto/` over 135 types | 1 253 |
| of them reachable in the door (`test/surface-baseline`) | 1 173 |
| `RefuseDoor(...)` call sites in `src/` | **949** |
| `throw Error("... is declared and not implemented yet")` in `include/` | **194** |
| **members that refuse when called** | **1 143** |

The units differ and that is the point: the baseline counts NAMES, the refusals are BODIES
(overloads counted separately). Neither number can be subtracted from the other, and neither is
wrong -- what is missing is that only one of them is written down.

The refusals by AL type, from the text each one carries: `JsonArray` 86, `System` 64, `JsonObject`
64, `File` 59, `XmlElement` 48, `XmlDocument` 38, `JsonValue` 36, `XmlNode` 32, `Database` 28,
`XmlDocumentType` 27, `XmlAttribute` 24, `Session` 22, `ErrorInfo` 21, `TextConst` 20, `Label` 20,
and 60 more types below that.

## THE LABEL ON EVERY ONE OF THEM POINTS AT THE WRONG ITEM

`scripts/door.py:200` and `scripts/gen_builtins.py:194` write `(board:0035)` into every generated
refusal, and board:0035 is **"The .NET surface is derived from its use"** -- an item about the 499
`dotnet::` types the corpus names and the stubs the generator emits for them. It says nothing about
AL's own door.

Counted across the tree: **1 441 references to board:0035, 1 429 of them in `include/`.** It is by an
order of magnitude the most-cited item in the repository, and it is cited for `Record.ChangeCompany`,
`Record.TransferFields`, `Label.*`, `File.*`, `KeyRef.*` -- none of which is a .NET type. A reader
who follows the refusal, which is the whole reason the refusal names an item, arrives somewhere
else.

**Four references in the tree name items that no longer exist.** Closing an item is deleting the
file, so a citation is a pointer that outlives its target unless something checks. The two in the
board are repaired in this round; the two in `include/` need an edit to the door and are recorded
here with what they should say:

| where | what it says | what owns it now |
|---|---|---|
| `include/runtime/Table.h:268` | "The value form belongs to the generator, which knows the context (board:0014)" -- the value-context rule for `Insert` | board:0055 for the discard form of `Get`, board:0056 for `Find`. The same rule, three methods, and no item states it once |
| `include/platform/Field.h:254` | "A `Field` reached through a `RecordRef` therefore still goes to SQL, and that is board:0052" | board:0025 -- a `RecordRef` opened by number reaches a virtual table's rows through the catalogue, which is exactly what that item's entry-with-function-pointers is for |

**The second one is a live defect and not only a pointer**: a `Field` reached through a `RecordRef`
queries a relation that does not exist. The door says so and the item that said what to do about it
is gone.

## AND THE DENOMINATOR ITSELF MISSES 74 DOCUMENTED SIGNATURES

`scripts/al_surface.py` reads the documentation's own file names with
`type_dir.glob(f"{type_dir.name}-*-method.md")`. **The instance surface of an object type is not
named that way.** `methods-auto/report/` holds `report-run-method.md` AND
`reportinstance-break-method.md`, and the glob matches only the first: `reportinstance` does not
have a hyphen after `report`.

Measured 2026-09-04:

| type | signature pages | counted | **never seen** |
|---|---:|---:|---:|
| report | 62 | 24 | **38** |
| xmlport | 21 | 3 | **18** |
| query | 22 | 5 | **17** |
| codeunit | 4 | 3 | 1 |
| | 1 741 | 1 667 | **74** |

`CurrReport.Break`, `CurrReport.Skip`, `Report.Preview`, `Query.Open`, `Query.Read`,
`XmlPort.Import`, `XmlPort.Export` -- the whole instance half of three object kinds -- are outside
the 1 253 the baseline measures against.

**It compounds in the worst direction**: the three kinds whose documented surface is under-counted
are exactly the three with no generator at all (board:0063, board:0064, board:0065), so the
completeness number is least accurate where the tree is emptiest, and it moves UP when the
denominator is repaired only because the numerator was never asked about them.

CLAUDE.md's own guard applies word for word -- "a blind gate: the analysis finds nothing and reports
success because it never ran; a count of 0 over N units is an ABORT, not a pass". Here the count is
not zero, so nothing aborts; a type contributing 3 of 21 pages looks like a type with three methods.

**The repair is to read every `*-method.md` in the directory** and take the method name from what
follows the type prefix, `<type>` or `<type>instance` alike -- and then to ASSERT that every page in
`methods-auto/` was consumed by exactly one type, which is the "it finds itself" guard the door's
type list already has.

## What the references say

Not an AL question. It is CLAUDE.md's own method applied to the one measurement that has escaped it:

> **Every baseline may only SHRINK.** ... a recorded count a commit may lower and never raise holds
> new code to zero.

and, from the same file, the completeness counter's own limit -- "It does NOT measure whether an
existing signature does the right thing". That sentence names the gap; it does not fill it. A
refusal count is precisely the thing that can be recorded and only lowered.

board:0035 asks for the same counter over the `dotnet::` half ("the count of refusing members is a
baseline that may only fall"), which is what makes the omission visible: the derived surface will
have a counter and the hand-written door will not.

## The choice

- **`test/refusal-baseline` beside `test/surface-baseline`**, one number with its population beside
  it, computed by a script over `src/` and `include/` the same way the surface count is computed
  over the doxygen XML. It may only fall.
- **The surface denominator counts every `*-method.md`**, and a page consumed by no type is an
  ABORT. The number will rise from 1 253 when it does, which is the honest direction.
- **The label is this item, not board:0035**: `scripts/door.py` and `scripts/gen_builtins.py` emit
  `(board:0059)`, and a type whose refusals belong to a specific item keeps that item's number --
  `Page.RunModal` already cites board:0030 correctly, which is the shape to generalise.
- **A reference IN CODE to a deleted item is a lint finding.** The check runs over `include/` and
  `src/` only: board prose legitimately names a closed item when it records what that item decided,
  and a check that forbade that would forbid the history CLAUDE.md keeps in `git log`. It is four
  lines, it finds itself, and it goes red on the next closed item that leaves a pointer in the door
  behind.

## Gate

The script prints refusals and population; `make lint` compares against the baseline and refuses a
rise. A negative control: delete one implemented method's body down to a refusal and require the
baseline check to go red. And: put a citation of an item number no board file carries into a door
header, and require the dangling-reference check to name the file and the number.
