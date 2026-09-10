# 0683 A member reached on a refusal is folded by the generator

**The finding.** `dotnet::Refused` carried 75 hand-written `static Refused <Name>;` members --
`Worksheet`, `Groups`, `Item`, `Result`, ... -- one for every member the BaseApp reaches on the
RESULT of a refused .NET member. It is exactly the failure mode CLAUDE.md names: a list somebody
has to remember to fill. Two codeunits could not be compiled because of names it lacked
(`WorksheetPart`, `ResponseUri`, `ToLowerInvariant`), and 10 UT cases died with them.

**The choice.** The GENERATOR knows the whole chain, so it folds it: an expression whose root
names a .NET variable nothing rebuilds and which reaches TWO or more members becomes one refusal,
`::agiru::dotnet::Refused({.type = "WorksheetWriter", .member = "Worksheet.WorksheetPart.WorksheetCommentsPart"})()`.
58 generated sources carry such a fold after the change.

- **Two members and not one.** A single member is a real member of the generated absent struct and
  may stand on the left of an assignment; folding it would turn an lvalue into a value. The chain
  is the case that has no member to stand on.
- **Which types refuse is READ FROM THE DOOR, never listed.** `RebuiltDotNet()` scans
  `include/dotnet/` for the classes it declares -- the scan the transpiler already did for its
  absent-type header, moved next to the other door scans in `src/gen/Door.cpp` so the generator
  shares it.
- **The refusal still names the type and the whole path**, so the message says more than before
  rather than less.

**Beside it:** `StreamReader.ReadLine()` is a statement in AL as often as it is a value, so the
door's `[[nodiscard]]` was wrong.

**Still open, found while doing this:** `apps/base/system/io/table/CSVBuffer.h` names `Globals<>`
and includes no `runtime/Codeunit.h`; compiled alone it fails, and only the unity build hides it.
A generated file includes what it names -- that is its own item.

**Measured.** Chain 100, A/B against chain 99.

**Chain 99 named three more of the same kind, and they are in this round:**

- **A synthesized dataitem is NOT a variable and a synthesized element IS.** A report's inner
  dataitem `Sales Cycle Stage` under `Opportunity Entry` became a Record variable that shadowed the
  OUTER dataitem's field of that name -- AL has no variable there at all, only a record scope. An
  xmlport's `textelement` is a variable in AL and must keep winning, so the order is the page kind's:
  local, field, global for a report; local, global, field for an xmlport.
- **`new StreamReader(x)` over a .NET stream nothing rebuilds is a REFUSAL**, not a translation
  that cannot compile -- `CSVFile.OpenRead(...)` and a `MemoryStream` both arrive that way.
- **`Globals<>` belongs to `runtime/Codeunit.h`** and the door table did not say so, so a table
  with a var block compiled only inside a unity group.
