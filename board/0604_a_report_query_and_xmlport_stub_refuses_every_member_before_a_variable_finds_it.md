Type:     task
Status:   open
Area:     gen, rt
Source:   an A/B over the three object kinds, 2026-09-07
Class:    activation

**STANDING: the cheap half is done (2026-09-07).** `Reach` returns nothing for `Report`, `Query`
and `XmlPort`, so such a variable reaches the ABSENT surface, which refuses every member --
never a table that happens to share the name. What remains is the expensive half: the stub
itself refusing, so it can carry the object's id and name instead of the absent placeholder.

# A report, query and xmlport stub refuses every member, and only then does a variable find it

**`Reach` resolves a variable in the index of the kind AL declares -- except for three kinds, where
it silently looks in the TABLES.**

```cpp
const TableIndex &index = type == "Codeunit"    ? objects.codeunits
                          : type == "Page"      ? objects.pages
                          : type == "Interface" ? objects.interfaces
                                                : objects.tables;   // Report, Query, XmlPort too
```

`CALTestSuiteXML: XmlPort "CAL Test Suite"` therefore came out as `Instance<CALTestSuite_Table>`,
because BC has a TABLE of that name as well -- the wrong object, silently, and the only reason it
was found at all is that a member it called does not exist on a table.

## Fixing the lookup makes the tree WORSE, and the A/B says by how much

Measured 2026-09-07, each kind added to the mapping on its own, over the 6 898-source slice:

| kind | AL declarations | build errors when the lookup is correct |
|---|---:|---:|
| `Report` | 4 978 | **1 292** |
| `Query` | 784 | **648** |
| `XmlPort` | 124 | **still red** -- `no member named 'Set'`, `'Initialize'` |

**All three are the same activation.** Today those variables fall to `absent::X`, and the absent
surface REFUSES every member, which compiles. The real stub carries the object's id and name and
nothing else -- a report has no translated procedures (board:0063), a query no columns (board:0064),
an xmlport no procedures (board:0065) -- so the correct lookup replaces a type that swallows
everything with a type that swallows nothing.

**Taken back in full**, and the order is the finding: the stub gets the refusal FIRST, the lookup
second. `dotnet::Refused` is the shape that already exists for exactly this -- an object that
answers any member and throws on use, naming what was asked for.

## What that also closes

board:0571 counts 31 report methods, 14 xmlport and 7 query as documented and undeclared. They are
the PLATFORM half; this item is the OBJECT half -- the procedures an AL author wrote on their own
report or xmlport, which no census can enumerate because they are per object. A stub that refuses
any member covers both, and the day a body is translated the refusal is replaced by the body rather
than by a new list.

**One case is already free without any of this**: `CALTestSuite` compiles now, because
`XmlPort.Export()`, `.Import()` and `.ImportFile()` returned `void` while every one of their
documentation pages brackets `[Ok := ]`. That was a door defect and not an activation.
