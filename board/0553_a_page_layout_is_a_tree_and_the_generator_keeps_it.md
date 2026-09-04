Type:     arc
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/devenv-page-types-and-layouts.md
Verdict:  teilweise
Class:    activation

# A page layout is a tree, and the generator keeps it

CLAUDE.md's phase 2 rests on one sentence: *"a page's layout is already `constexpr` metadata -- so a
renderer walks the control tree and there is one source rather than a template beside a model."*

**It is not yet metadata and there is no tree.** `src/gen/PageWriter.cpp:52` flattens the layout into
three vectors and every container disappears on the way. This item is the sentence made true.

## What the platform guarantees

`devenv-page-types-and-layouts.md` is the page CONTAINER model, where board:0429 is the `PageType`
value and board:0537 the three layouts. Four things it states that no property page says:

**Two principal categories, and the category decides what an action acts ON.**

| category | types | what an action affects |
|---|---|---|
| entity-oriented | `Card`, `Document`, `ListPlus`, `CardPart` | the entity the page title names |
| collection-oriented | `List`, `Worksheet`, `ListPart` | **the SELECTED ROWS** |

board:0539 places an action in one of six areas; this says what the action receives when it runs, and
the two answers are different objects. A dialog type (`StandardDialog`, `ConfirmationDialog`) can be
either -- "the title caption and actions are suited for both".

**A page is three AREAS and they are not interchangeable**: `content` with the full layout grammar,
`FactBoxes` limited to a list of parts, and the header (title, action bar, filters, views). Measured
below, `area()` takes fifteen distinct arguments.

**SIZING DEPENDS ON A SECTION'S POSITION IN ITS PARENT, not only on its kind.** Three behaviours --
size to content, size to content within limits, size to fill -- and which one applies is positional:

> "When a ListPart is embedded as the LAST part on the page, it EXPANDS to fill space."
> "The Document page type allows the FIRST ListPart on the page to use extra vertical space."

A flat list of parts cannot answer either question. The ORDER and the NESTING are the layout.

**And a `CardPart` or `ListPart` may not embed another page** -- the recursion has depth one.

## The structural rules, and the source breaking them

The page states two rules as `IMPORTANT`:

> "Because entity-oriented pages represent a *single* entity ... **don't use a `Repeater` group** in
> the construction of entity-oriented pages."
> "List pages **must contain a single `Repeater` group**."

**Both are false as invariants over BC's own source, and that is worth more than the rules.** Measured
2026-09-04 over `~/Git/BCApps/src`, per FILE, restricted to the 6 961 files holding exactly one `page`
object, `PageType` by `^\s*PageType\s*=\s*(\w+)` and a repeater by `^\s*repeater\s*\(`:

| `PageType` | files | with a repeater |
|---|---:|---:|
| `List` | 2 742 | 2 722 |
| `Card` | 923 | **13** |
| `ListPart` | 817 | 803 |
| `Document` | 429 | **3** |
| `Worksheet` | 412 | 407 |
| `API` | 374 | 366 |
| `CardPart` | 271 | **6** |
| `RoleCenter` | 211 | 0 |
| `StandardDialog` | 190 | 28 |
| `NavigatePage` | 159 | 19 |
| `UserControlHost` | 157 | 0 |
| `ListPlus` | 152 | **8** |
| (none declared) | 70 | 6 |
| `ConfirmationDialog` | 25 | 1 |
| `HeadlinePart` | 16 | 0 |
| `PromptDialog` | 9 | 0 |
| `ConfigurationDialog` | 4 | 0 |

**Thirty entity-oriented pages carry a repeater, and twenty `List` pages carry none.** Four of the
thirty are in `Layers/W1/BaseApp` -- `ApprovalUserOverview`, `ConsProcessDetails`,
`PurchaseDocumentsDueToday`, `SalesInvoicesDueNextWeek` -- so this is not localisation noise that
`scope.json` filters away.

**RESOLVED by board:0560, with a citation.** `devenv-repeater-controls.md` names the enforcement:
*"if you use a repeater on an entity-oriented page, you'll get **UICop Warning AW0008**."* A UICop
warning is an ANALYZER finding, not a compiler error -- so the platform loads all 30, which is exactly
what the count showed, and the conclusion below is confirmed rather than only inferred. The
documentation's `IMPORTANT` is an analyzer rule. **And the page supplies BC's own workaround**: put the
repeater in a `ListPart` and embed that, which is what the 817 `ListPart` pages are for.

**The reading below was reached from the count alone, before that citation was found, and it holds.**
The documentation says "don't" and "must"; the source says otherwise in 50 places and the platform
loads all of them. The reading that fits both is that
these are DESIGN GUIDANCE with a degraded rendering behind them -- the page itself says "some of the
repeater's features might not work properly, and it might not get the expected size", which is a
description of what happens, not a refusal. **So this is NOT a `static_assert` and not a generator
refusal**, and that is the finding: the instinct CLAUDE.md encourages -- anything decidable at
translation time is a `static_assert` -- would here reject fifty pages the platform accepts. A
`static_assert` is right for what the platform REFUSES, and this is not one of those.

The measurement is per FILE and boolean, so it cannot distinguish two repeaters from one; the
`List`-must-have-exactly-one half of the rule is therefore only half measured, and that is said rather
than rounded.

## The control-kind census, measured 2026-09-04 over `~/Git/BCApps/src`

Container kinds by `^\s*<kind>\s*\(` -- **not** the `Name =` property pattern, because a control is a
call and not a property, and the pattern is named here so the number can be reproduced:

| control kind | count | in `Flatten` |
|---|---:|---|
| `group(` | 32 177 | **dropped** |
| `part(` | 5 891 | `parts` |
| `repeater(` | 4 387 | **dropped** |
| `systempart(` | 4 045 | `fields` -- **wrong list** |
| `label(` | 363 | `fields` |
| `cuegroup(` | 279 | **dropped** |
| `fixed(` | 230 | **dropped** |
| `usercontrol(` | 225 | `fields` |
| `grid(` | 53 | **dropped** |

and `area(` by its argument: `content` 8 383, `processing` 2 977, `promoted` 2 300, `factboxes` 2 103,
`navigation` 1 910, `reporting` 488, `creation` 216, `sections` 204, `rolecenter` 162, `embedding` 145,
`systemactions` 13, `prompt` 7, `promptoptions` 5, `prompting` 5, `promptguide` 2 -- **18 920 areas,
all dropped.**

## The IST-state

**The AST already carries everything.** `src/al/Ast.h:104`:

```cpp
struct PageControl {
  std::string kind;                     // area, group, repeater, field, part, ...
  std::string name;
  std::vector<Token> source;
  std::vector<Property> properties;     // Caption, ToolTip, Visible, Editable, ...
  std::vector<ProcedureDecl> triggers;
  std::vector<PageControl> children;    // THE TREE
};
```

**The writer throws three of the six away.** `src/gen/PageWriter.cpp:52`:

```cpp
void Flatten(const std::vector<al::PageControl> &controls, Controls &into) {
  for (const al::PageControl &control : controls) {
    const std::string kind = Lowered(control.kind);
    if (!control.name.empty() && IsField(kind)) { into.fields.push_back(&control); }
    ...
    Flatten(control.children, into);
  }
}
```

Consequences, each a line in the generated header:

- **`kind` survives only as which of three vectors the control landed in.** `IsField`
  (`PageWriter.cpp:24`) accepts `field`, `usercontrol`, `label`, `systempart`, `chartpart` --
  **so 4 045 `systempart`s are emitted as FIELDS**, and a system part is a part with a platform
  target, not a field on the source table. That is a categorisation defect with a number.
- **A container kind matches no predicate**, so `area`, `group`, `repeater`, `cuegroup`, `grid` and
  `fixed` are visited for their children and then vanish -- 59 979 controls that decide the layout.
- **`properties` is never read.** No `Caption`, `ToolTip` (board:0385), `Visible`, `Editable`,
  `ApplicationArea`, `Importance` reaches the generated header.
- **`PageType` is never read** -- `grep -n PageType src/gen/PageWriter.cpp` is empty. board:0429's
  nineteen values have no representation, so nothing downstream can branch on the category above.
- **There is no `include/meta/PageDef.h`.** `include/meta/` holds `Declare.h`, `EnumDef.h`, `Ids.h`,
  `TableDef.h`. A table's layout is `constexpr` data; a page's is not, and that asymmetry is the whole
  of this item.

What the header DOES carry (`PageWriter.cpp:275`) is `kId`, `kName`, `Rec`, the variables, the labels,
the procedures, the control triggers, and a `<page>_Controls` template of three flat member lists --
which is exactly the TestPage surface board:0540 needs and nothing a renderer needs.

## The choice

**`PageDef` beside `TableDef`, and it is a TREE in `.rodata`.**

```cpp
struct ControlDef {
  ControlKind kind;                 // Area, Group, Repeater, Field, Part, SystemPart, ...
  AreaKind area;                    // meaningful when kind == Area
  std::string_view name;
  std::string_view caption;
  std::string_view toolTip;
  FieldNo field;                    // 0 when the control has no source field
  PageId part;                      // 0 when the control embeds no page
  std::span<const ControlDef> children;
};

struct PageDef {
  PageId id;
  std::string_view name;
  PageType type;
  TableId source;
  std::span<const ControlDef> layout;
  std::span<const ControlDef> actions;
};
```

**Why a `span` of children and not an index into a flat array:** the renderer's walk is the tree's own
shape, and a `constexpr std::span` over a `constexpr` array costs a pointer and a length in `.rodata`
with no relocation the loader has to resolve per process. An index scheme buys nothing and spends the
reader.

**Why the tree and not a flat list plus a depth column:** the sizing rules above are "the LAST part",
"the FIRST ListPart" -- questions about a node's position among its SIBLINGS. A depth column answers
them only by rescanning, and the rescan is the tree written badly.

**Why `ControlKind` as an enum and not the AL string:** `Flatten`'s three predicates already lower and
compare strings at generation time; an enum moves the same decision to a `constexpr` value and makes
the renderer's `switch` exhaustive, which `-Werror` then checks. The nine kinds above plus `area` and
`field` are the whole set the source uses, and a kind the parser does not know must ABORT rather than
be dropped -- CLAUDE.md: accepting a declaration and doing nothing with it is worse than refusing it,
and this item exists because that happened 59 979 times.

**What stays:** the `_Controls` template. It is board:0540's TestPage surface and it is addressed by
NAME, which is what a test does; the renderer walks the tree, which is what a client does. Two
consumers, one source, no duplication -- the flat lists become a `constexpr` view derived from the
tree rather than the only thing generated.

**`static_assert` where the platform REFUSES and nowhere else.** Three are safe, because they are
errors the AL compiler itself raises: a `CardPart`/`ListPart` embedding a part, more than one
`area(FactBoxes)`, and a `FactBoxes` part whose target is not a `CardPart` or `ListPart`
(board:0554). The repeater rules are not, per the measurement above.

## Ordering

**Before board:0537 and board:0538**, which describe what a renderer does with a card, a list and a
role centre -- there is nothing for either to walk until this lands. **After board:0429**, whose
`PageType` enum `PageDef::type` is.

board:0540's TestPage keeps working throughout, because the flat lists are preserved as a view. That
is deliberate: this is an `activation` change to the generated header shape, and one that cannot break
the existing surface is one that can be measured on its own.

## Gate, and its negative control

A page with `area(content)` holding a `group` holding a `repeater` holding two fields emits a
`PageDef` whose layout is one `Area` node with one `Group` child with one `Repeater` child with two
`Field` grandchildren -- **four levels, in source order**, and a `static_assert` on the depth and on
each level's `kind` proves it at translation time rather than at run time.

**The negative control is the nesting.** Remove the `children` span and emit the three flat lists
again: the `static_assert` on depth must FAIL TO COMPILE. A control that compares only the field
NAMES stays green under flattening -- which is what today's generator already produces -- and would
prove nothing.

**Second control, for the categorisation:** a page with one `systempart(Notes; Notes)` must emit a
node of kind `SystemPart`. Today that control passes against `Field`, so the assertion must name the
kind or it is testing that the name survived.

## Class

`activation`. Nothing downstream reads a page's layout today, so the tree starts with no consumer and
cannot regress one; the A/B is over the generated tree compiling (`make apps` to the first error) and
over `test/target/`, whose page image is edited BY HAND to show the nesting -- never overwritten with
what the generator produced.

## `analysisviews` IS A THIRD PAGE SECTION

`devenv-analysis-view-package.md` (read 2026-09-04, routed here) adds a section to the page grammar
beside `layout` and `actions`:

```al
page 50100 ListPageWithAnalysisView
{
    layout { ... }
    analysisviews { view(Name) { DefinitionFile = 'view.json'; } }
}
```

**An analysis view is a JSON file exported from the client and packaged with the app**, referenced by
`DefinitionFile`, and the construct is allowed on a `page`, a `pageextension` AND a
`pagecustomization` -- the third being board:0551's object, which otherwise carries only layout and
actions.

So the tree this item emits has a third top-level span beside layout and actions, and its leaves point
at RESOURCES rather than at controls -- board:0572's subject. Available from 2026 release wave 1.
**Measured since, by board:0574: 19 `analysisviews` sections** -- new, and already in use.

**One piece of advice on the page is a constraint in disguise**: *"avoid using data filters in analysis
views that you package ... the data values you filter on might not exist on the environment where the
extension is installed."* A packaged view carries a filter it cannot guarantee resolves, which is the
same hazard board:0553's `SubPageLink` has and is left to the same mechanism.

## A RICH TEXT CONTROL IS ALONE IN ITS GROUP, AND IT IS HTML

`devenv-richtext-content-controls.md` (read 2026-09-04, routed here) adds a control behaviour with a
structural rule this item's tree has to carry:

> "The Rich Text feature can be applied to **Blob, BigText, and Text** data types without any size
> limits. **A Rich Text control MUST BE PLACED ON ITS OWN IN A GROUP** (for example, in a FastTab).
> When the content is persisted in the database, **it's saved as HTML. Media-like pictures are
> EMBEDDED IN THE HTML content itself** and aren't persisted in a separate table."

**Measured 2026-09-04 over `~/Git/BCApps/src`: the control is `ExtendedDatatype = RichContent`, 23
declarations** -- out of 2 745 `ExtendedDatatype` declarations in all, whose distribution is
`PhoneNo` 1 359, `Email` 831, `URL` 219, `Masked` 104, `Barcode` 96, `Ratio` 67, `Person` 39,
**`RichContent` 23**, `Task` 4, `Document` 2, `None` 1.

**"Alone in a group" is a structural rule over board:0561's FastTab**, and it is decidable: a `group`
containing a `RichContent` field and anything else is a translation error. 23 sites, so the check is
cheap and the population is small.

**The persistence rule matters more than the control.** The value is HTML with images inlined as data,
which is why the documentation says a `Blob` is the right field type -- and why the round trip goes
through `CalcFields` and a stream rather than through the field. **So a rich text field is a `Blob`
that the client renders**, and nothing about it is a new storage mechanism.

**One limitation is stated and is worth carrying**: *"Business Central doesn't currently offer any way
for you to restrict user input to simple formatting only."* There is no sanitisation surface, so
whatever the client produces is what is stored.
