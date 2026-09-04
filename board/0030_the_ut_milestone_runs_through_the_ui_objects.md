Type: root
State: open
Area: rt, gen

# The UT milestone runs through the UI objects

Measured 2026-09-02 over `src/Layers/W1/Tests`, on the files the milestone's population lives in
(`*UT.Codeunit.al`, 81 files; the transpiler counts 86 codeunits in them):

| type the file names | UT files | tree-wide files it blocks |
|---|---|---|
| **TestPage** | **41** | 24 |
| Page | 24 | 28 |
| Report | 18 | 20 |
| Notification | 8 | 34 |
| Dialog | 6 | 39 |
| DotNet | 6 | 140 |
| JsonObject | 2 | 31 |
| XmlDocument | 1 | 11 |
| Interface | 1 | 53 |

**Counted per PROCEDURE rather than per file, 2026-09-04: 479 of the 2 291 `[Test]` procedures
name a `TestPage` variable -- 20.9 %.** So the milestone's ceiling without a page runtime is 1 812
of 2 291, and the four codeunits that carry the most of it are `ERM General Journal UT` (94 of 195),
`VAT Return Period UT` (46 of 64), `ERM VAT VIES Lookup UT` (36 of 40) and `Price List Line UT`
(28 of 137). The same measurement puts 501 procedures behind `[HandlerFunctions]` (board:0054), and
202 of the 266 handlers declared take a page or a request page -- so the two ceilings are mostly the
same procedures.

## The same question per PROCEDURE and per TYPE, 2026-09-04

Counting the types a `[Test]` procedure declares in its own `var` block, over the 80 UT codeunits
and 2 305 procedures board:0058's rule gives (the file counts above are over a differently drawn
population, which is what board:0058 exists to stop):

| type a `[Test]` declares | procedures | share | codeunits |
|---|---:|---:|---:|
| **Codeunit** | 467 | 20.3 % | 50 |
| **TestPage** | 428 | 18.6 % | 36 |
| RecordRef | 67 | 2.9 % | 12 |
| OutStream | 26 | 1.1 % | 6 |
| Interface | 19 | 0.8 % | 1 |
| DotNet | 18 | 0.8 % | 3 |
| **Report** | 12 | 0.5 % | 5 |
| InStream | 5 | 0.2 % | 3 |
| Notification | 4 | 0.2 % | 3 |
| FieldRef | 3 | 0.1 % | 3 |
| **Query** | 1 | 0.0 % | 1 |
| **XmlPort** | **0** | -- | -- |

It is a LOWER bound -- a procedure using a codeunit-global variable is not counted -- and it agrees
with the 20.9 % measured the other way for TestPage. Two things follow that the file counts hide:

- **Report, Query and XmlPort are not milestone blockers**, which is why board:0063, board:0064 and
  board:0065 are argued from the TARGET and ranked behind this item and board:0057.
- **XmlPort reaches the milestone through its ID and not through a variable.** `Payment Export
  XMLPort UT` (24 tests), `Payment Export Validation UT` (41) and `Payment Export FX Tables UT` (12)
  write `XMLPORT::"Export Generic CSV"` and hand it to the Data Exchange framework. A count of
  variables says zero; the object still has to exist for the name to resolve.

**The two columns disagree, and the left one is the goal.** `DotNet` blocks more generated files
than anything else in the tree and appears in SIX of the milestone's files; `TestPage` blocks a
third as many files tree-wide and appears in HALF of them. Ordering the work by what the whole tree
fails to compile would put the UI objects last and reach 6 UT files per unit of effort instead of
41.

## What this costs to admit

The `/goal` route reads "filter language and transaction boundary, then temporary records,
FlowFields, remaining field types, then CRONUS, then the tests themselves". **The UI objects are not
in it, and about half the milestone stands on them.** That is not a reason to reorder the route --
the record layer is under the page layer and has to be right first -- but it is a reason to stop
treating pages as something that comes after the milestone.

A `TestPage` is not a page: `testpage-data-type.md` gives it its own surface -- `OpenEdit`, `First`,
`Next`, `GoToRecord`, field access by name, `Action`, and the handler attributes
(`[PageHandler]`, `[ModalPageHandler]`, `[ConfirmHandler]`) that let a test answer a dialog the code
under test raises. Half of that is a TEST harness rather than a UI, and the harness half is what 41
files need.

## The reference this stands on

`~/Git/openerp` reached 97.0 % of the same subset, so it has an answer: `openerp/runtime/` carries
`_page_registry.py`, `filter_page.py` and a `builtins/_recordref.py`, and `openerp/base/**/page/`
holds the generated pages. **Grep there before deriving any of this from scratch** -- in particular
how a `[PageHandler]` is bound to the page it answers, which is the part with no obvious C++ shape.

## What the predecessor already answered, measured 2026-09-02

`~/Git/openerp/openerp/runtime/base/test_page.py` is 4 740 lines and `page.py` is 1 367, and the
module docstring of the first states the dependency that decides the order here:

> field access ... `set_value(v)` / `value(v)` -> `record.validate(field, v)` -- drives the real
> `OnValidate` trigger and its error path.

**A TestPage field write IS a Validate**, which is trigger dispatch (board:0029). So the UI half of
this item cannot start before that one, and the 41 files do not need a page renderer -- they need
`Validate` plus navigation over a record set.

The same file names its own scope boundary, and it is a MEASURED one rather than an opinion: v1 left
"ModalPageHandler dispatch and real action-`OnAction` execution" out, `<action>.invoke()` was a
no-op, and the suite still reached 97.0 %. So the handler machinery is not what most of the 41 files
are waiting for.

**One thing there is NOT to be copied.** That module degrades everything it cannot resolve -- a page
out of scope, a control with no matching field -- to a silent no-op handle, and says so outright:
"activating this wrapper never *removes* a code path that previously survived." That is the
`activation` trade taken in the other direction, and it is why a 97 % number can hide a wrong one:
CLAUDE.md's "a failure is loud" and "accepting a declaration and doing nothing with it is worse than
refusing it" both point the opposite way. Here an unresolvable control is an ERROR naming the page
and the control.

## WHAT THE HARNESS ALREADY IS, read 2026-09-04 (board:0071)

`include/runtime/test/TestPage.h` exists and is a real design rather than a stub: a template that
**derives from the generated page class**, so `SalesOrder."No.".SetValue('X')` resolves because the
page already declares every control as a `TestField`. A page the transpiler never saw falls back to
`UnknownPage`, a class with no members at all, so the call site **fails to compile naming the
control it wanted** -- this item's own "an unresolvable control is an ERROR naming the page and the
control", moved to translation time, which is better than it asked for.

`TestField` has bodies for `SetValueText`, `Value`, `AssertEqualsText`, `AsInteger`, `AsBoolean`,
`Activate`, `Lookup`, `DrillDown`, `Editable`, `Enabled` and `Visible` (`src/rt/TestPage.cpp`), and
every one of them reaches `Unopened()` until a page RUNTIME exists. So what is missing is not the
harness but what it drives.

**Three things `testpage-data-type.md` documents that this item does not carry:**

- **`Trap()`** -- "traps the next test page that is invoked and assigns it to the test page
  variable". It is the ALTERNATIVE to a `[PageHandler]` (board:0054), and a test using it registers
  nothing.
- **`GetValidationError([Integer])` and `ValidationErrorCount()`** -- a TestPage COLLECTS the errors
  its `SetValue` calls raised instead of propagating them, so the page harness needs an error
  channel that is neither `asserterror` nor board:0055's last error.
- **`RunPageBackgroundTask`** runs a codeunit in a CHILD SESSION and waits, and its completion
  triggers do not fire unless asked.
- **`.Value` is ASSIGNABLE**: `devenv-testing-pages.md` writes `CustomerCard.Address.Value :=
  '<address>'` and reads `CustNo := CustomerCard."No.".Value`. agiru's `TestField` has `Value()` and
  `SetValueText()`, so either the generator rewrites the assignment or `Value` becomes a proxy that
  assigns -- and the first is this tree's habit (board:0051). Page parts and subpages are reached
  the same way and are `TestPart` instances.
- **"Test methods and code on test pages run on the Server instance, even though they simulate
  client interactions"** -- so nothing about a TestPage needs a client, which is what makes the
  headless harness faithful rather than a shortcut.

## THE PAGE TYPES ARE FIFTEEN AND THEY ARE NOT ONE MECHANISM

`devenv-page-types-and-layouts.md` (read 2026-09-04, board:0071) gives each type its own behaviour
-- a `Card` shows one entity in FastTabs, a `List` a `Repeater` over a collection, a
`ConfirmationDialog` is a Yes/No dialog, a `RoleCenter` is a collection of parts. Measured over
`Layers/W1`'s 2 657 page objects:

| type | pages | | type | pages |
|---|---:|---|---|---:|
| `List` | **1 265** | | `RoleCenter` | 40 |
| `ListPart` | 349 | | `API` | 21 |
| `Card` | 288 | | `HeadlinePart` | 14 |
| `Worksheet` | 156 | | `ConfirmationDialog` | 14 |
| `CardPart` | 139 | | `UserControlHost` | 1 |
| `Document` | 118 | | `PromptDialog` | 1 |
| `ListPlus` | 93 | | | |
| `StandardDialog` | 91 | | | |
| `NavigatePage` | 67 | | | |

**Four types are 76 % of them** -- `List`, `ListPart`, `Card`, `Worksheet` -- and a `TestPage` over
those four needs a current row, navigation and field access, which is what this item already says the
41 UT files need. `RoleCenter`, `HeadlinePart`, `UserControlHost` and `PromptDialog` are 56 pages
that a headless harness never opens.

The section ORDER is fixed and the parser already knows it: properties, `layout`, `actions`, `views`
(list pages only), then code.

## What is true when this closes

- A `page` object translates, with its source table, its fields and its actions.
- A `TestPage` drives one: open, move, read and write a field by AL name, invoke an action.
- **A `TestPage` raises the PAGE TRIGGER EVENTS the platform raises** -- `OnOpenPageEvent`,
  `OnClosePageEvent`, `OnAfterGetCurrRecordEvent` and the ten others (board:0057). The predecessor
  filed this twice as its own defect (WI-1169, WI-1170): `Page.run` raised them and the TestPage path
  did not, so 108 subscribers on 55 pages were live one way and dead the other. 398 subscriptions in
  the read roots name `ObjectType::Page`.
- A handler attribute binds a procedure to the page it answers, and a page opened with no handler
  bound is an ERROR naming the page rather than a silent pass.
- The count of UT codeunits that translate is a baseline beside the others.

## THE CONTROL TREE IS THE BASE AND PERSONALISATION IS AN OVERLAY, read 2026-09-04 (board:0071)

`ui-personalization-stored.md` says where a user's page adjustments live: **"Roaming personalization
is stored in the Business Central service"** -- server-side, per user, across devices -- while a
short list (pane widths, rows-versus-tiles, pin states) stays in the browser. The page tabulates
which is which, and almost everything structural roams: fields moved, hidden, added, locked,
included in or excluded from Quick Entry; columns reordered, resized, frozen; views added, renamed,
reordered; parts and actions moved or hidden.

**That settles a design question this tree's own rules would otherwise force the wrong way.** A
page's layout is `constexpr` metadata in `.rodata`, shared between processes and never per-session
(CLAUDE.md). So personalisation cannot be merged INTO it: the compiled control tree is the base, and
what a user sees is base + a per-profile customisation (board:0034's `pagecustomization`, a
translation-time object) + a per-user delta read from a table at render time. Three layers, applied
in that order, and only the third is session state.

`ui-customizing-overview.md` adds the layer above them: the **Experience** setting (Essential or
Premium) decides which controls exist for a whole company, through each control's `ApplicationArea`
(board:0067). That one is a FILTER over the base rather than a delta, and it is company-scoped
rather than user-scoped (board:0060).

**And the filter DENIES by default.** `devenv-extending-application-areas.md` (read 2026-09-04):
"**If your extension fails to use `ApplicationArea` in any controls or actions, they won't be visible
when you use an experience tier.**" So a control with no declared area is hidden rather than shown --
the opposite of what an implementer assumes -- and the enabled areas come from the `Application Area
Setup` table, which is BaseApp data the platform consults per company. `ApplicationArea = All` is the
declaration that opts out, and it is what most BaseApp controls carry.

**A TestPage sees the BASE and nothing else**, which is why this is recorded here rather than filed:
no UT test personalises, so the overlay is not milestone work. What it constrains is the renderer's
shape -- a renderer that walks the `constexpr` tree directly, with no seam for a delta, is one that
has to be written twice.

**AND THE OVERLAY CAN ADD A CONTROL THE PAGE NEVER DECLARED.** `ui-personalization-manage.md`: an
administrator customising a profile may "add a table field that's not on the page object" -- drag it
from the **Add field to page** pane -- and afterwards "the added field is locked from editing and
can't be unlocked", while users may then show or hide it like any other. Removing it again makes it
unavailable to every user of that profile at once.

So the delta is not only show / hide / move over the compiled controls: **it may name a FIELD of the
source table that no control exists for.** A renderer whose control tree is closed at translation
time cannot express that. It is still cheap here, because the field metadata is `constexpr` too
(board:0067): a runtime control is either a compiled one or one SYNTHESISED over a `FieldDef`, and
the second kind is exactly what the personalisation layer produces. Naming that seam now costs
nothing; discovering it after the renderer is written costs the renderer.

`ui-personalization-user.md` also fixes what "clear" means, and it is four separate scopes rather
than one -- navigation menu, actions, fields and columns, or all -- which is the shape of the stored
delta as much as it is a user action.

**THE RENDERER'S HTML IS PARTLY SPECIFIED, and that is free to honour if it is known first.**
`ui-accessibility.md` states what BC's own markup does: "on list pages, the columns are defined in
**TH** tags and the column headings are set with **TITLE** attribute inside the tag. Captions for
elements, such as FastTabs, FactBoxes, and fields are included in **heading tags (H1, H2, H3, and
H4)**", images carry `ALT` and links carry `title`. Keyboard navigation is Tab / Shift+Tab between
elements and arrow keys within an action area.

CLAUDE.md's phase 2 is "htmx: the server holds the state and sends HTML fragments", so this is a
specification for the fragments rather than a nice-to-have -- and every one of those attributes has
a source in the `constexpr` metadata already: a column's `TITLE` is its `Caption`, a FastTab's
heading is its `Caption`, an image's `ALT` is its `ToolTip`. Emitting them from the start costs
nothing; retrofitting them means walking the renderer again.
