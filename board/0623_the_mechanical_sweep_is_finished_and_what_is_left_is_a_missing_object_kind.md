Type:     root
Status:   open
Area:     gen
Source:   the mechanical sweep over the whole board, 2026-09-08
Class:    measurement

# The mechanical sweep is finished, and what is left is a missing object KIND

**THE FOUR MECHANICAL CLASSES WERE SWEPT AND EACH HAS A NUMBER NOW.** This item exists so the next
round does not sweep them again: the work that is left on this board is not mechanical, and the
reason is the same for almost all of it.

## 1. A property that has to reach the metadata as `constexpr` data -- DONE

Every member of `FieldDef`, `KeyDef`, `TableDef`, `PageDef`, `ControlDef`, `ActionDef`,
`EnumValueDef` and `CodeunitDef` is filled by the generator. **The single exception is
`PageDef::views`** (board:0352's `OrderBy` over a named view).

The last three landed in this round -- `ExternalName` (3 903 declarations), `OptionOrdinalValues`
(310), `SqlTimestamp` (2) -- and `AssignmentCompatibility` (573) reaches `EnumTraits`.

## 2. A documented method missing from the door -- DONE, with a reason

**1 251 of 1 253 documented methods over 93 types**, and the two are `TestPart.Enabled` and
`TestPart.Visible`, which may NOT be added: `TestPage<P>` reaches the page's controls through `P`,
so a method of either name hides a control of that name, and `Enabled` is one of the commonest
control names there is. They need `TestPart` as a type of its own, which is board:0030's work.

## 3. A name that deviates from AL's own spelling -- NONE LEFT

`scripts/al_surface.py` compares per type following base classes and reports no `DOOR_NAME` or
`DOOR_METHOD` deviation.

## 4. A declaration the transpiler drops in silence -- MEASURED, AND IT IS ONE KIND

**106 documented properties appear in BCApps and are never read by the generator**, and the sweep
had to be done TWICE: the first pass reported 137, and 31 of those were the same property under
another casing -- `Tooltip` for `ToolTip`, `ShortCutKey` for `ShortcutKey`, `LookupPageID` for
`LookupPageId`, `ExtendedDatatype` for `ExtendedDataType`. AL is case-insensitive and so is
`Find(properties, name)`; **the pass, not the generator, was the thing that could not tell a naming
defect from a gap.** That is CLAUDE.md's own rule, and it cost the first reading of this sweep.

**Of the 106 that stand, all but one belong to an object kind that has NO GENERATOR:**

| kind | properties never read | the heaviest |
|---|---|---|
| Report | 19 | `DataItemTableView` 7 706, `DataItemLink` 2 024, `RequestFilterFields` 1 940 |
| XmlPort | 15 | `XmlName` 3 577, `NamespacePrefix` 2 473, `MinOccurs` 887 |
| Query | 4 | `QueryType` 346, `SqlJoinType` 216, `Method` 261 |
| PermissionSet / Entitlement | 4 | `Assignable` 1 116, `IncludedPermissionSets` 968 |
| ControlAddIn | 1 | `Multiplicity` 83 |

**SO "THE TRANSPILER DROPS A DECLARATION IN SILENCE" IS NOT A HUNDRED SMALL ITEMS. IT IS FOUR
MISSING OBJECT KINDS**, and board:0033, board:0034, board:0063, board:0064 and board:0065 own them.
A property of a report cannot reach metadata that does not exist.

## And the one that is not a kind

`AutoCalcField` (238) applies to an XmlPort field and a Report column, so it is in the table above
after all. Nothing outside those five kinds is left.

## STANDING: THE SWEEP IS A SCRIPT AND A BASELINE (2026-09-08)

`scripts/dropped_properties.py` counts documented properties the generator never reads, folded for
case, with the KIND from each page's own "Applies to" section beside it. `test/property-baseline`
holds the number and may only fall.

**101 -> 93 in this round**, and every one that was closed belonged to a kind that has a generator:

| property | declarations | where it went |
|---|---|---|
| `Filters` | 179 | `ControlDef::filters` |
| `OrderBy` | 113 | `ControlDef::orderBy` |
| `Multiplicity` | 83 | `ControlDef::multiplicity` |
| `RequiredTestIsolation` | 64 | `CodeunitDef::requiredTestIsolation` |
| `TestHttpRequestPolicy` | 45 | `CodeunitDef::testHttpRequestPolicy` |
| `ODataEDMType` | 34 | `ControlDef::odataEdmType` |
| `UnknownValueImplementation` | 9 | `EnumTraits::kUnknownValueImplementation` |
| `ExternalSchema` | 2 | `TableDef::externalSchema` |

**AND THE FIRST TWO NEEDED THE PARSER, not the writer.** A page's `views` section was SKIPPED --
`ParsePageBody` skips any identifier followed by a brace -- so the section reached no AST at all
and nothing said so. `PageObject::views` is parsed as controls now (a view IS a control: it carries
a caption, filters and an order), and `PageDef::views` is the last member of any `Def` that the
generator did not fill.

**NOTHING IS LEFT ON A KIND THAT HAS A GENERATOR.** The 93 that stand are Report, XmlPort, Query,
PermissionSet, Entitlement, ControlAddIn and Profile -- five object kinds with no generator, which
is board:0033 and board:0034's count and not a hundred small items.

## What proves it

The sweep is a script and it belongs in the tree rather than in this item: a count of documented
properties the generator never reads, folded for case, with the kind beside each. It may only fall,
and it falls by a whole column when a kind gets its generator.
