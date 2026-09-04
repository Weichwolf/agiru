Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/devenv-designing-role-centers.md, developer/devenv-cues-action-tiles.md, developer/devenv-create-role-center-headline.md, developer/devenv-role-center-behaviors.md, developer/devenv-simple-role-center-example.md
Verdict:  fehlt
Class:    activation

# A role centre is six areas, and its action order is source order

**Five pages, one item**: the role centre, its cues, its headline, its behaviours and the worked
example. CLAUDE.md names role centres with cues as a core expectation, and board:0429 measured
**`PageType = RoleCenter` 211** and `HeadlinePart` 16.

## Six area controls, and their order in the code does not matter -- except where it does

> | control | |
> |---|---|
> | `area(Sections)` | **top-level navigation menus**, expanding to links; targets open **in the content area** |
> | `area(embedding)` | **navigation bar** -- a flat second-level list; targets open **in the content area** |
> | `area(creation)` | actions **first**, with a **plus icon** |
> | `area(processing)` | actions **after creation**, groupable |
> | `area(reporting)` | actions **last**, with a **report icon** |
> | `layout` + `part` | the content area |
>
> **"The ORDER OF THE `area()` CONTROLS IN THE PAGE CODE ISN'T IMPORTANT. However, the ORDER OF THE
> INDIVIDUAL ACTIONS AND GROUPS IS IMPORTANT because they appear IN THE ORDER IN WHICH THEY APPEAR IN
> PAGE CODE."**

**Areas are a fixed sequence; actions within them are source order.** So the renderer sorts by a
`constexpr` area rank and then by declaration index -- and the declaration index must survive the
generator, which board:0033's extension merge complicates: an extension's actions have their own source
order and must land somewhere deterministic.

**That is board:0513's problem again** -- BC does not say where an extension's actions go, and
determinism requires an answer.

**Two more placement rules, both conditional on CONTENT:**

> "if the **FIRST PART in the content area is a HEADLINE part**, the actions area is automatically
> positioned **either to the right of the Headline part or after it, DEPENDING ON BROWSER WINDOW
> SIZE**. If the first part isn't a Headline, the actions area appears **directly after the navigation
> area**."
>
> **"If a WELCOME BANNER is displayed, the banner HIDES THE ACTION BAR."**

So the action bar's position depends on the first part's TYPE and on the viewport -- a responsive rule,
not a layout property.

## A cue is a table FIELD, not a computed control

> "Cues display data that is contained in a **TABLE FIELD**. This data can be raw data or calculated
> data."
>
> **"You can ONLY base Cues on INTEGER AND DECIMAL data types. Other data types aren't supported and
> won't display."**
>
> A cue may be a **FlowField** -- logic in the `CalcFormula` (board:0340) -- **or a Normal field**,
> where "you'll typically add the logic ... to an AL trigger or method. Unlike a FlowField, a Normal
> field enables you to **extract data from other objects such as QUERIES.**"
>
> Implementation: **a table object with a field**, and **a page object that contains the field**.

**So a cue needs a TABLE**, and the BaseApp's cue tables exist for nothing else -- rows whose fields are
FlowFields over the real data. That is why board:0510's FlowField work and board:0498's page background
tasks both land on role centres: the cue's value is an aggregate, computed per render, and BC moved it
to a child session for exactly that reason.

**The Integer/Decimal restriction is a `static_assert`** once board:0339's `FieldClass` and the field's
type are in the metadata.

## The wide layout has two documented consequences

> The `wide` layout (board:0428's `CuegroupLayout`) is **"only supported in the web client"** -- always
> true here -- and **"the `Caption` and `CaptionML` properties of the `cuegroup` control are IGNORED
> when the layout is wide."**
>
> **"wide groups that PRECEDE all normal groups appear in their own section spanning the entire
> width"**; wide groups placed after normal ones behave like normal ones.

**A declared caption that is deliberately ignored** -- the fourth in this sweep, after board:0374,
board:0422 and board:0487. And the wide layout's effect depends on its POSITION relative to the normal
groups, which is source order again.

> **"Modifying actions in Cue groups on page extensions ISN'T SUPPORTED."**

A merge restriction for board:0033.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0429: `RoleCenter` **211**, `HeadlinePart` **16**, `CardPart` **271**, `ListPart` **817**.
board:0428: `CueGroupLayout` **10**. board:0482: `RoleCenter` on a profile **59**.

**211 role centres and 59 profiles pointing at one** -- so most role centres are reachable through a
profile and board:0482 is how.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; no areas, no parts, no cues. board:0047 records
that FlowFields are not computed, so a cue would have no value.

## The choice

Six `constexpr` area ranks on the action descriptor, actions carrying their declaration index, and the
content area as a list of part references. The cue is an ordinary bound control whose field happens to
be Integer or Decimal; **nothing about a cue is special except its rendering**, which is what makes it
implementable before board:0498's background tasks.

**The extension-merge order is declared** -- base page's actions, then extensions by app id, each in
source order -- which is board:0513's rule applied to a second place.

## Ordering

Behind board:0537's control tree and board:0510's FlowFields. board:0498's background tasks make cues
fast and are not required for them to be correct.

## Gate, and its negative control

A role centre renders its six areas in the documented sequence and its actions in source order; a cue
over a `Sum` FlowField shows the total; a wide cue group placed first spans the width and ignores its
caption.

**The negative control is the wide group's caption** -- it must NOT render, and an implementation that
draws it produces a heading BC does not show, which no "the number is right" gate can see.

## `area(Sections)` IS THE NAVIGATION MENU, AND IT IS ROLE-CENTRE ONLY

`devenv-adding-menus-to-navigation-pane.md` (read 2026-09-04, routed here) names the control that
carries the role centre's navigation:

> "The top-level navigation area is referred to as the NAVIGATION MENU ... You define the navigation
> menu by using an **`area(Sections)`** control in the page code."

Its children are `group` and `action`, nested to any depth -- *"you can also group `action()` controls
in SUBMENUS. This enables you to create a logical hierarchy"* -- and each action's target is a
`RunObject`, which *"can include page extensions and other objects like reports, XMLports, and
codeunits."*

**So the navigation menu is a tree of groups over actions whose targets are OBJECTS OF ANY KIND**, and
the pages it opens *"will open in the CONTENT AREA of the Role Center"* rather than as a new window.

**Measured 2026-09-04 over `~/Git/BCApps/src`, per file: 204 files declare `area(Sections)` and ALL
204 are `PageType = RoleCenter`.** Checked by reading each file's first `PageType` rather than
assumed -- board:0553's area census lists `sections` 204 among fifteen area arguments with no
attribution, and this settles which page type it belongs to. Against 211 `PageType = RoleCenter`
declarations, so almost every role centre has one.

The neighbouring role-centre areas, per file: `Creation` 216, `RoleCenter` 162, `Embedding` 145.
