Type:     task
Status:   open
Parent:   0553
Area:     gen, rt
Source:   developer/devenv-repeater-controls.md, developer/devenv-indented-hierarchy-lists.md, developer/devenv-creating-flowfields-and-flowfilters.md
Verdict:  fehlt
Class:    activation

# A repeater is flat, and the hierarchy is an integer column

board:0553 makes the page's LAYOUT a tree. **This is the one place where a tree in the UI is NOT a
tree in the layout**: a repeater's rows are flat, and the parent-child structure is an integer per
row.

## What the platform guarantees

> "A row that's indented from a row above is considered a *child* of that row."
>
> "The `IndentationColumn` property ... You set the property to either a FIELD in the page's source
> table OR TO A VARIABLE. The important thing is that property resolves to an INTEGER. This integer
> determines the indentation level."

So the hierarchy is DATA and the renderer never nests anything. Two kinds:

| | fixed | collapsible |
|---|---|---|
| properties | `IndentationColumn`, `IndentationControls` | `IndentationColumn`, `ShowAsTree`, `TreeInitialState` |
| which column indents | the ONE named by `IndentationControls` | **the LEFT-MOST VISIBLE column** |
| rows hidden | never | by collapsing a parent |

**And the collapsible kind IGNORES `IndentationControls` outright:**

> "Unlike fixed indented lists, a collapsible hierarchy ALWAYS INDENTS THE LEFT-MOST VISIBLE COLUMN in
> the repeater. **The `IndentationControls` property is IGNORED.** If users customize the page by
> moving another column first, the moved column will be indented instead."

**The indented column therefore depends on RUNTIME PERSONALISATION STATE**, not on the declaration.
That is the only layout decision met in this sweep that a `constexpr` cannot hold, and it is worth
naming as such rather than discovering it when the `constexpr` is already written.

## The repeater's own limits

- **One repeater, at the BEGINNING of `area(Content)`.** "List pages are designed for using a single
  `repeater()` control, which must be defined at the beginning of the content area. If you include
  more than one repeater or another control like a group or grid, the page might NOT BEHAVE AS
  EXPECTED."
- **A repeater may not contain a Part or a FlowFilter field** -- "the Web client doesn't support
  displaying `repeater` controls that contain other Parts or FlowFilter fields." board:0510 already
  records that a FlowFilter cannot be a control's `SourceExpression` at all; this is the same refusal
  from the container's side.
- **The field ORDER is the column order**, stated outright: "the order of the field controls
  determines the order in which they appear on the page." board:0553's tree carries it.

## THIS SETTLES board:0553'S OPEN CONTRADICTION

board:0553 recorded that `devenv-page-types-and-layouts.md` states as `IMPORTANT` that entity-oriented
pages must not carry a repeater, measured **30 that do** (13 `Card`, 8 `ListPlus`, 6 `CardPart`,
3 `Document`), four of them in `Layers/W1/BaseApp`, and left the contradiction unresolved -- concluding
only that a `static_assert` would be wrong.

**This page names the enforcement and it is a WARNING:**

> "In fact, if you use a repeater on an entity-oriented page, you'll get **UICop Warning AW0008**."

A UICop warning is an analyzer finding, not a compiler error. **So the platform loads those 30 pages,
which is exactly what the measurement showed, and the conclusion board:0553 reached from the count is
confirmed by a citation rather than only inferred from it.** The item's reading stands and its
"contradiction recorded, not resolved" becomes "resolved: the documentation's `IMPORTANT` is an
analyzer rule".

**And the page supplies the workaround BC itself uses**: "create a `ListPart` page that contains a
`repeater` control, and then use the `ListPart` in the entity-oriented page" -- which is what the 271
`CardPart` and 817 `ListPart` pages are for.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

| | count |
|---|---:|
| `repeater(` | 4 387 |
| `ShowCaption =` | 8 636 |
| `CardPageId =` | 599 |
| `Width =` | 248 |
| **`IndentationColumn =`** | **181** |
| `IndentationControls =` | 168 |
| **`ShowAsTree =`** | **46**, all `true` |
| `FreezeColumn =` | 37 |
| `TreeInitialState =` | 10 -- `CollapseAll` 9, `ExpandAll` 1 |

So **46 collapsible hierarchies and about 135 fixed ones**, and `TreeInitialState` is declared on ten
of the 46 -- the other 36 take its default.

**31 OF THE 46 COLLAPSIBLE PAGES DECLARE `IndentationControls`, WHICH IS DOCUMENTED AS IGNORED.**
Counted per file over pages carrying both properties. Among them `ALTestTool.Page.al` -- the test
runner's own page -- `PermissionSetTree.Page.al`, `AssistedSetup.Page.al` and `ManualSetup.Page.al`.

**That is BC's own source declaring something that does nothing, 31 times**, and it decides the
implementation: agiru must IGNORE the property on a collapsible repeater, must not honour it, and must
not refuse it. CLAUDE.md says accepting a declaration and doing nothing with it is worse than refusing
it -- **this is the documented exception to that rule**, and it costs a counter rather than a
`static_assert`: the generator reports how many `IndentationControls` it discarded, so the number is
visible instead of silent.

## The IST-state

- **No page property is emitted at all** (board:0553): `src/gen/PageWriter.cpp` never reads a control's
  `properties` vector, so none of the nine properties above exists in the output.
- **The repeater control itself is dropped** -- `Flatten` (`PageWriter.cpp:52`) matches no container
  kind, so all 4 387 disappear and their `field` children land in the page's one flat list.
- **There is no renderer**, so there is nothing to indent.

## The choice

**The repeater is a `ControlKind::Repeater` node in board:0553's tree with four extra members, and the
indentation is evaluated PER ROW at render time.**

```cpp
struct RepeaterDef {
  FieldNo indentationColumn;   // 0 when the source is a page variable
  ControlIndex indentationControl;  // fixed lists only; kInvalid on a tree
  bool showAsTree;
  TreeInitialState initialState;
};
```

**Why `FieldNo` and not a value:** the column is a field of the SOURCE TABLE, so it is read from the
row being rendered, not from the page. A page VARIABLE is the documented alternative -- "you can
achieve the same results using a variable" -- and that case is `indentationColumn == 0` with the value
read from the page object instead; both resolve at translation time, so which of the two applies is
`constexpr` and only the value is not.

**Why the indented column is NOT `constexpr` on a tree:** the left-most VISIBLE column depends on the
viewer's personalisation. So `indentationControl` is `constexpr` and used only when
`!showAsTree`; the tree case computes it from the rendered column order. **A single member that tried
to serve both would be a `constexpr` that is sometimes wrong**, which is worse than two.

**The hierarchy is never materialised.** Rows arrive from board:0045's cursor in key order with an
integer each; the renderer emits an indent level and, for a tree, a collapse toggle. **Building a
parent-child structure in memory would hold the result set**, which is the one thing CLAUDE.md's
streaming requirement forbids -- 100 million rows is ordinary and the indentation column is the reason
the flat form works.

**Collapsing is a VIEW state and belongs to the viewer, not the record.** Under phase 2's htmx shape a
collapsed parent means its children's rows are not requested, which is the same behaviour reached from
the fragment side.

**Refuse at translation time, twice**, because the web client refuses both: a `part` inside a
`repeater`, and a `field` inside a `repeater` whose source is a `FieldClass = FlowFilter` field. Both
are `constexpr` facts -- the control kind and the field class -- and the platform does not merely warn
about them.

**Do NOT refuse a repeater on an entity-oriented page.** UICop AW0008 is a warning and 30 BaseApp
pages carry one.

## Ordering

**Inside board:0553**, after the tree and the control properties exist. **Before board:0537's list
renderer**, which draws the rows this describes the shape of.

`IndentationColumn` at 181 against `repeater(` at 4 387 says the plain flat repeater comes first and
the hierarchy is 4 % of it. `ShowAsTree` at 46 is last of the three.

## Gate, and its negative control

A list page over a table with an `Indent` column holding 0, 1, 1, 0:

1. with `IndentationColumn = Indent` and `IndentationControls = Name`, the **Name** column is indented
   on rows two and three and no other column moves
2. with `ShowAsTree = true` added, the LEFT-MOST column is indented instead and
   `IndentationControls` has NO effect
3. `TreeInitialState = CollapseAll` renders two rows, not four
4. a `part` inside the repeater fails to transpile
5. a `Card` page carrying a repeater TRANSPILES

**The negative control is case 2, and it is the whole reason this item is separate from board:0553.**
Honour `IndentationControls` unconditionally -- the obvious implementation, and the one the 31 BaseApp
declarations invite -- and case 2 goes red while 1, 3, 4 and 5 all stay green. A gate that checks only
that indentation happens cannot tell the two kinds apart.

**Case 5 is the second control**, against the over-correction: refusing the repeater on a `Card` page
turns 30 BaseApp pages into translation errors, and every other case stays green while it does.

## Class

`activation`. Nothing renders today, so there is nothing to regress. The risk is case 4's refusal
firing on a page it should not, which is why the A/B is `make apps` over the whole tree before any
rendering is measured.
