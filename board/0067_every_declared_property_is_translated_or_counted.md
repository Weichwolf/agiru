Type: arc
State: open
Area: al, gen
Tags: gate, measured

# Every declared property is translated or counted, and none is dropped in silence

board:0034 made the object KINDS a counted population, so a kind with no generator is a hole with a
number rather than a decision. **The properties inside those objects have no such counter**, and
they are the larger population by two orders of magnitude.

Measured 2026-09-04 over the read roots `apps.json` names, counting `Name = value` in a declaration
position where `Name` is one of the 335 documented properties:

| | |
|---|---:|
| properties documented in `properties/` | 335 |
| of them declared in the read roots | **256** |
| declarations | **340 307** |
| **property names `src/gen` asks for** | **7** |

The seven: `Caption`, `Implementation`, `OptionCaption`, `OptionMembers`, `SourceTable`, `Subtype`,
`TableNo`. Everything else in a `properties` block is read by the parser into a token list and never
consulted.

CLAUDE.md is explicit about what that costs: "**accepting a declaration and doing nothing with it is
worse than refusing it**, and `catch (...) {}` is a finding with a counter". A dropped property is
the same shape without the counter.

## The ones that change BEHAVIOUR, with who owns each

Most of the 340 307 are appearance -- `ApplicationArea` 57 358, `ToolTip` 38 516, `Visible` 16 868,
`Image` 12 413 -- and board:0030 owns the page half of those. What is left is short and each row is
a decision somebody has to have made:

| property | declarations | owner |
|---|---:|---|
| `Editable` | 13 558 | board:0030 (page), and a table field's is not a write refusal |
| `AutoFormatType` | 9 690 | board:0066 |
| `TableRelation` | 9 275 | board:0043 |
| `AutoFormatExpression` | 4 803 | board:0066 |
| **`CaptionClass`** | **3 591** | **UNCLAIMED** -- `CaptionClassTranslate` is a stub in `src/rt/Builtins.cpp` |
| `DecimalPlaces` | 3 413 | board:0066 |
| `FieldClass` | 2 712 | board:0047 |
| `Enabled` | 2 410 | board:0045 on a KEY (an index the platform does not maintain), board:0030 on a control |
| `CalcFormula` | 2 212 | board:0047 |
| `AccessByPermission` | 2 180 | board:0062 |
| `BlankZero` | 1 918 | board:0066 |
| `DataItemTableView` | 1 816 | board:0063 |
| **`Clustered`** | **1 771** | **board:0045**, which does not name it yet |
| `Permissions` | 1 460 | board:0062 |
| `TestPermissions` | 1 342 | board:0062 |
| **`ObsoleteState` / `ObsoleteTag` / `ObsoleteReason`** | **2 986** | **board:0069** |
| **`NotBlank`** | **949** | **board:0068** |
| **`MinValue` / `MaxValue`** | **1 243** | **board:0068** |
| `InitValue` | 816 | board:0056 |
| `ExtendedDataType` | 666 | board:0030 |
| `DataItemLink` | 571 | board:0063 |
| `SourceTableView` / `SourceTableTemporary` | 938 | board:0030 |
| **`ValidateTableRelation`** | **457** | board:0043, which names it in prose |
| `EventSubscriberInstance` | 383 | board:0057 |
| **`TableType`** | **237** | **UNCLAIMED** -- `Normal`, `Temporary`, `CRM`, `ExternalSQL`, `MicrosoftGraph`, and only `Normal` is storage as this tree knows it |
| `SingleInstance` | 181 | board:0027 |
| `SumIndexFields` | 176 | board:0019 |
| **`AutoIncrement`** | **151** | **board:0068** |
| `SqlJoinType` | 77 | board:0064 |
| `DataPerCompany` | 71 | board:0060 |

**Three rows were genuinely unowned before this pass** and now have items; `CaptionClass` and
`TableType` remain unclaimed on purpose, with their numbers standing so the choice is visible.

## The attributes are the same question with 24 answers

| attribute | declarations | read by the generator |
|---|---:|---|
| `[Scope(...)]` | 46 070 | no -- a compile-scope declaration, and the one dropped property that costs nothing |
| `[Test]` | 41 586 | **yes** |
| `[IntegrationEvent(...)]` | 23 920 | **yes** -- as a publisher, board:0057 |
| `[HandlerFunctions(...)]` | 17 451 | no -- board:0054 |
| `[EventSubscriber(...)]` | 4 051 | no -- board:0057 |
| **`[TransactionModel(...)]`** | **1 230** | **no -- board:0039** |
| `[NonDebuggable]` | 981 | no, and correctly |
| `[Obsolete(...)]` | 629 | no -- board:0069 |
| `[TryFunction]` | 569 | no -- board:0061 |
| `[SecurityFiltering(...)]` | 291 | no -- board:0062 |
| `[InherentPermissions(...)]` | 117 | no -- board:0062 |
| `[InternalEvent(...)]` | 90 | **yes** |
| `[CommitBehavior(...)]` | 78 | no -- board:0012 |
| `[RunOnClient]` | 61 | no |

Four of 24 attribute kinds are read. The rest are parsed into `ProcedureDecl::attributes` and
discarded there.

## The choice

- **The transpiler counts, per property and per attribute, how many declarations it DROPPED**, and
  prints the ranking in the run summary the way it already prints untranslated object kinds. That
  is the whole of this item's mechanism: it turns 340 307 silent declarations into a list with the
  biggest one at the top.
- **A property is dropped by DECISION, in a list beside the counter** -- `ApplicationArea` and
  `ToolTip` are dropped because the page renderer does not exist yet, not because nobody looked.
  A property in neither list -- not translated, not deliberately dropped -- is a translation
  WARNING naming the object, the element and the property.
- **The counter is the denominator the property items are measured against**, so board:0043,
  board:0047, board:0066, board:0068 and board:0069 each close a row of it and the number falls.

## Gate

The run summary lists dropped properties by count. A property added to the AL corpus that neither
list knows produces a warning naming it. **Negative control**: remove `Caption` from the seven the
generator reads and require the count of dropped `Caption` declarations to appear at the top of the
ranking -- a counter that only sees what it already handles is the blind gate CLAUDE.md names.
