Type:     task
Status:   open
Parent:   0553
Area:     gen, rt
Source:   developer/devenv-arranging-fields-on-fasttab.md, developer/devenv-arranging-fields-using-grid-and-fixed-controls.md, developer/devenv-arrange-fields-in-rows-and-columns-using-gridlayout-control.md, developer/devenv-arrange-fields-in-rows-and-columns-using-fixedlayout-control.md
Verdict:  fehlt
Class:    activation

# A FastTab is a group, and the column split is source order

**Four pages, one item**: how fields land on a FastTab by default, and the two controls that override
it. board:0553 keeps the tree; **this is what the renderer does with a `group` once it has one.**

## A FastTab is not a control kind

> "A FastTab IS A GROUP CONTROL directly within the `content` area of a card, document, or task page."

So there is no `fasttab()` in the layout grammar -- board:0553's census counts 32 177 `group(` and the
FastTabs are the subset of them one level under `area(content)`. **The kind is positional**, which is
one more reason the tree cannot be flattened: a `group` inside a `group` is not a FastTab and renders
differently.

## The default layout, and it is source order

> "By default, a FastTab is divided into TWO COLUMNS ... Fields are automatically distributed between
> the left and right columns **based on their ORDER in the `layout` section**. Starting with the first
> field and working downward, fields are positioned in the left column and then in the right column.
> The area occupied by the fields in each column is AS EQUAL AS POSSIBLE. Field captions are
> positioned to the LEFT of fields."

**And the rule changes when the children are groups rather than fields:**

> "When you group fields on a FastTab, **the GROUPS are distributed evenly between the left and right
> columns. FIELDS AREN'T.**"

Two different distributions depending on the child kind, in one container. A renderer that balanced
by field count in both cases gets the grouped case wrong -- the groups are dealt out evenly, the
fields are balanced by AREA.

**The column count is responsive and not declared**: one column when narrow, "more than two columns to
take advantage of wider screens".

**And the initial collapsed state is NOT a developer decision.**

> "Business Central AUTOMATICALLY DETERMINES whether FastTabs are initially displayed as expanded or
> collapsed ... the first two parts or FastTabs are automatically expanded. All other parts or FastTabs
> are shown as collapsed ... **developers CAN'T SPECIFY the starting state.**"

The first two, expanded; everything after, collapsed. A positional rule with no property behind it,
and board:0554's FactBox rule says a collapsed thing does not load -- so the FastTab's position in
source order decides whether its parts run their triggers on open.

**A group with no caption is STRUCTURAL:**

> "If a group doesn't specify the `CaptionML` property or this is set to an empty value, it's
> considered to be a group used ONLY FOR STRUCTURAL PURPOSES. This includes FastTabs. Structural
> FastTabs look and behave differently -- for example, they CAN'T BE COLLAPSED by users unless they
> include the **Show more** action."

So the presence of a caption changes the container's behaviour, not just its label.

## `grid` and `fixed` are the manual override, and their placement is load-bearing

> "The Grid and the Fixed control MUST BE PLACED IN THE `group` control in an `area(content)` subtype
> ... **If the Grid or Fixed control is NOT placed in a Group control, it will INHERIT PROPERTIES AS IF
> IT WERE A TYPICAL GROUP CONTROL and NONE of the Grid or Fixed properties will apply to it.**"

**A silent degradation, not an error** -- and decidable at translation time, because the parent's kind
is in the tree. It joins board:0560's `IndentationControls` in the small set of "declared and
discarded" cases the generator must COUNT rather than refuse.

The required nesting is exact: `area(content)` > `group` > `grid`|`fixed` > `group` > `field`. "The
Grid or Fixed control must have Group controls as DIRECT CHILDREN and Field controls as children of
THOSE Group controls."

| | `grid` | `fixed` |
|---|---|---|
| arrangement | row-by-row or column-by-column | column-by-column only |
| captions | shown or hidden, before or above the field | headings on rows and columns; **none per field** |
| spanning | rows and columns | **none** |
| appearance | shaded, bordered | neither |

**And a `fixed` overrides `Editable`:**

> "Fields in a fixed layout are NOT EDITABLE **even if the `Editable` property is set to `true`**.
> However, if the field drills down to a page where the field source is defined, then you can modify
> the field."

A container overriding a field's own property. `Editable` is declared 51 886 times, so the rule is
narrow but the property it overrides is not.

`grid` accepts nested `group`, `repeater`, `cuegroup`, `fixed`, `grid` and `part`; it does NOT accept
add-ins, chart parts or system parts. The page discourages the nesting it permits -- "we do not
recommend it because of lower UI performance" -- which is advice and not a rule.

## What the web client drops, with a count

Three declarations are documented as unsupported in the web client, and BC's source carries them
anyway:

| declaration | count | web client |
|---|---:|---|
| `GridLayout = Rows` | **13** | **"The `Rows` layout on the grid control itself is not supported."** |
| `RowSpan =` | 6 | **"not supported"** |
| `ColumnSpan =` | 7 | **"not supported"** |

**26 declarations that do nothing**, and the general rule above them: *"Arranging fields in rows only
works in the Windows client. In the Web client, fields can only be arranged in COLUMNS."* The Windows
client does not exist any more; agiru's target is a browser. **So `GridLayout = Rows` renders as
`Columns` and the spans are ignored** -- the same shape as board:0560's 31 `IndentationControls`, and
the same treatment: counted, not refused.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

| | count | values |
|---|---:|---|
| `Caption =` | 288 491 | |
| `Editable =` | 51 886 | |
| **`Importance =`** | **16 781** | `Additional` 12 490, `Promoted` 4 128, `Standard` 163 |
| `ShowCaption =` | 8 636 | **`false` 8 595**, `true` 41 |
| `group(` | 32 177 | |
| `fixed(` | 230 | |
| `grid(` | 53 | |
| `GridLayout =` | 32 | `Columns` 19, **`Rows` 13** |
| `ColumnSpan =` | 7 | |
| `RowSpan =` | 6 | |
| **`CaptionML =`** | **3** | |

**`CaptionML` is declared THREE times in 2.56 million lines**, against `Caption` at 288 491. The
structural-group rule is written against `CaptionML`, so **read literally almost every group in BC is
structural** -- which is plainly not the intent. The rule means "has no caption", and `Caption` is how
BC writes one. **The documentation names a property the source has abandoned**, and that is recorded
rather than resolved: the two spellings are the same property with and without a language tag, and the
sensible reading is that the rule applies to both.

`Importance` at 16 781 is the largest number in this item and it decides the order: 12 490 fields are
`Additional` -- hidden behind **Show more** -- and 4 128 are `Promoted` onto the collapsed summary
line. **So on a collapsed FastTab, three quarters of what is declared is not visible**, and a renderer
that ignores `Importance` shows every field at once on every card in the product.

`ShowCaption` is 99.5 % `false`: it is only ever used to REMOVE a caption.

`grid(` at 53 against `fixed(` at 230 says the "new and preferred" control is outnumbered four to one
by the one it replaced.

## The IST-state

- **`group`, `grid` and `fixed` are dropped** by `Flatten` (`src/gen/PageWriter.cpp:52`) -- they match
  no predicate, so the FastTab structure that decides all of the above does not survive translation.
- **No property is emitted** (board:0553), so `Importance`, `ShowCaption`, `Caption`, `Editable`,
  `GridLayout`, `RowSpan` and `ColumnSpan` reach nothing.
- **A FastTab cannot even be identified**, because identifying one requires knowing a `group`'s parent
  is an `area(content)` -- and the parent is exactly what flattening destroys.

## The choice

**No new structure: a FastTab is `ControlKind::Group` whose parent is `AreaKind::Content`**, and the
renderer asks the tree rather than a flag.

```cpp
constexpr bool IsFastTab(const ControlDef &parent, const ControlDef &self) {
  return self.kind == ControlKind::Group && parent.kind == ControlKind::Area &&
         parent.area == AreaKind::Content;
}
```

**Why derived and not a stored flag:** the documentation defines it positionally, and a stored flag is
a second source that can disagree with the tree. It is also `constexpr`, so the question costs nothing
at run time.

**The column split is a rendering-time pass over the children, and it BRANCHES ON CHILD KIND** -- deal
groups out evenly, balance fields by area. Two code paths because the documentation states two rules,
not one path with a parameter.

**The initial expand state is positional and computed at render time**: the first two FastTabs or
parts expanded, the rest collapsed. `constexpr`-known from the tree, so it can be a compile-time
constant per page -- but it is deliberately NOT a property, because AL has none and inventing one
would let a generated file say something no `.al` file can.

**`Importance` decides three renderings and it is the first thing built here**, at 16 781
declarations: `Promoted` onto the summary line, `Standard` in the body, `Additional` behind
**Show more**.

**Refuse nothing; count two things.** A `grid` or `fixed` whose parent is not a `group` -- documented
as silently degrading -- and a `GridLayout = Rows`, `RowSpan` or `ColumnSpan` the web client drops.
Both are `constexpr` facts and both are conditions BC's own source is in, so a refusal would reject
the BaseApp. The generator prints the counts, which is how CLAUDE.md's "accepting a declaration and
doing nothing with it is worse than refusing it" is honoured where refusing is not available.

**A `fixed` makes its fields read-only regardless of `Editable`.** One line in the renderer, and it is
listed here because it is a property being overridden rather than combined -- the only such case met
in this sweep.

## Ordering

**Inside board:0553**, after the tree exists. **Before board:0537's card renderer**, which draws
FastTabs.

`Importance` 16 781 first; the default two-column split second, because it is what every card page
without a `grid` does; `grid` and `fixed` last, at 53 and 230.

## Gate, and its negative control

A `Card` page with one `group` under `area(content)` holding four fields and one nested `group`:

1. the four fields are split between two columns in SOURCE ORDER, left column filled first
2. a field with `Importance = Additional` does not render until **Show more**
3. a field with `Importance = Promoted` renders on the collapsed summary line
4. a second FastTab is expanded and a THIRD is collapsed, with no property saying so
5. a `grid` placed directly under `area(content)` renders as a plain group, and the generator's
   discarded-property counter reports it
6. a field inside a `fixed` is read-only although it declares `Editable = true`

**The negative control is case 1 with GROUPS instead of fields.** Replace the four fields with four
groups: they must be dealt out evenly, two and two, and NOT balanced by area. An implementation with
one distribution rule passes case 1 and fails only this, and it is the case the documentation states
in a single sentence that is easy to read past.

**Case 5 is the blind-gate guard**: a counter of 0 over a page that contains the misplacement means
the check never ran, and cases 1 through 4 are green regardless.

## Class

`activation`. Nothing renders today. The risk is concentrated in `Importance`: 12 490 fields currently
have no rendering at all, and giving them one that hides them behind **Show more** is the difference
between a card that shows everything and a card that shows what BC shows -- so a TestPage that finds a
field by name must still find an `Additional` one, and that is the first thing to check when
board:0540's surface meets this.
