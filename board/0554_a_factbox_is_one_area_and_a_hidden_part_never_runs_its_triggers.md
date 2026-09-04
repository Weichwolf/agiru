Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/devenv-adding-a-factbox-to-page.md
Verdict:  fehlt
Class:    activation

# A FactBox is one area, and a hidden part never runs its triggers

board:0553 makes the layout a tree; this is the one area in it whose LOADING ORDER is specified, and
the specification is observable from AL -- so it is behaviour and not presentation.

## What the platform guarantees

**One `area(FactBoxes)` per page, and only on five page types.**

> "There can only be ONE FactBox area control on one page."

Allowed on `Card`, `Document`, `ListPlus`, `List`, `Worksheet` -- and on no other. board:0553's
category table says why: a FactBox shows facts about a CURRENT record, and the five are exactly the
types that have one.

**A FactBox part must target a `CardPart` or a `ListPart`, and anything else is an ERROR.**

> "You can add a part to the FactBox area that displays an existing page of the CardPart or ListPart
> type only. If you attempt to use another page type, you GET AN ERROR."

**That is a translation-time refusal**, because the generator holds both pages' `PageType`. It is one
of the three cases board:0553 names as a legitimate `static_assert` -- the platform refuses, so the
transpiler may.

**`systempart()` is a control kind of its own, with three targets.**

| target | what it is |
|---|---|
| `Links` | user links to a URL or path, stored against the record |
| `Notes` | a user note, stored against the record |
| `Summary` | the Copilot summary, referred to in code as `DefaultSummaryPart` |

**`Summary` is on by DEFAULT** on every `Card`, `Document` and `ListPlus` with a `FactBoxes` area, and
is only ever NAMED in order to hide it -- "there can only be one summary system part per page". So it
is a part that exists without being declared, which is the opposite of every other control.

**THE LOADING SEQUENCE IS SPECIFIED AND IT IS NOT ASYNCHRONOUS.**

1. the hosting page's content loads first and the user can interact with it
2. then the FactBox pane, **each FactBox in sequence starting from the top**
3. a FactBox whose `Visible` evaluates to `false` **is not loaded at all**
4. a FactBox not in view is loaded only when the user scrolls it into view
5. a collapsed pane loads nothing until the user expands it

with four consequences the page states as FAQ answers, each of which a test can see:

- **"Are any FactBox triggers run when the FactBox is hidden?" -- NO.**
- **"How often are triggers run if the pane is expanded, collapsed, and expanded again?" --
  `OnOpenPage` runs only the FIRST time. Once loaded, a FactBox is not loaded again for as long as the
  page remains open.**
- **"Are FactBoxes processed asynchronously?" -- NO.** "This optimization is simply a CONTROLLED
  SEQUENCE in which triggers are run, STILL WITHIN THE SAME SESSION as the hosting page." So no
  thread, no task, no reordering -- CLAUDE.md's determinism commitment is not even at risk here,
  because the platform itself declares the order.
- **"Does `SubPageLink` or `SubPageView` affect the sequence?" -- NO**, and "using properties such as
  `SubPageView` is PREFERRED to writing trigger code to update a FactBox."

**One trap named outright:** "Avoid having triggers on the hosting page that call into a FactBox
because this condition FORCES the FactBox to ignore performance optimizations and load along with the
content of the hosting page." So a host trigger reaching into a part changes WHEN the part loads --
the sequence is not a property of the part alone.

**Role centres are excluded** from the whole optimisation; a part in a `content` area is not, and is
skipped when `Visible` is false.

## What the predecessor measured

Two items, both `done`, both a row-following defect rather than a load-order one:

- **WI-1190** (`ut81 2227->2229, GAINED 2, LOST 0`): *"Ein part mit SubPageLink zeigt die Daten zur
  AKTUELLEN Zeile des Wirts ... Der Link wurde nur beim BAU gesetzt, der Teil danach zwischengespeichert
  -- er blieb auf Zeile 1."* The link was applied once at construction and the part stayed on row one.
  The fix pulls the parts along after `first()` and after `next()`. **Its named trap: if the link
  filters to empty, it was taken back** -- so an empty result must not be treated as "the link did not
  apply".
- **WI-1228** (`2255 -> 2256`): the title blamed a missing `SubPageLink`; the comment records that as
  REFUTED. *"die FactBox wird nicht per SubPageLink gefuellt, sondern per `LoadDataFromRecord` in einen
  temporaeren Puffer (`SourceTableTemporary = true`)."* **So a FactBox is not always link-driven**, and
  an implementation that assumes it is will look for a filter that was never meant to exist. The actual
  cause was `OnNextRecord` returning 0 on a page that declares `OnFindRecord` without it.

**Read the FINDING, not the fix**: the first says the link is re-evaluated on every row change, the
second says the link is one of two fill mechanisms.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`area(` and `systempart(` are CALLS, not properties, so the `Name =` pattern does not apply and the
one used is named instead: `^\s*<kind>\s*\(` for the control, and the argument after `;` for a system
part's target.

| | count |
|---|---:|
| `area(FactBoxes)` | **2 103** |
| `part(` (all areas) | 5 891 |
| `systempart(` | 4 045 |
| `SubPageLink =` | 3 487 |
| `SubPageView =` | 96 |

and the system-part targets: **`Notes` 1 978, `Links` 1 933, `MyNotes` 134** -- summing to 4 045, so
the census is complete.

**`Summary` appears ZERO times**, which confirms the reading above rather than contradicting it: it is
enabled by default and BC's own source never needs to hide it. **`MyNotes` at 134 is a fourth target
the documentation does not list** -- the page tabulates three. That contradiction is recorded, not
resolved; the source declares and the page describes, so `MyNotes` exists.

`SubPageLink` at 3 487 against `SubPageView` at 96 decides the order: the link is the mechanism, the
view is the exception.

## The IST-state

- **`src/gen/PageWriter.cpp:29`** -- `IsPart` accepts only `"part"`. **`systempart` is classified as a
  FIELD** (`PageWriter.cpp:24`), so all 4 045 are emitted onto the wrong list; board:0553 owns that
  correction.
- **The area is gone.** `area` matches no predicate in `Flatten`, so nothing in the generated header
  says which of a page's parts are FactBoxes and which sit in `content`. The five parts of a Sales
  Order page are one undifferentiated vector.
- **`SubPageLink` and `SubPageView` are never read.** `src/gen/PageWriter.cpp:94` (`PartSource`) takes
  a part's TARGET PAGE and nothing else; the properties vector is untouched.
- **There is no load sequence, because there is no loading.** `include/runtime/Page.h` has no part
  lifecycle -- no `Visible` evaluation, no first-time-only rule, no row-change propagation.
- **`Visible` is not evaluated anywhere**, so rule 3 above -- the one with an observable trigger
  consequence -- has no mechanism.

## The choice

**The FactBox area is a `ControlKind::Area` node with `AreaKind::FactBoxes` in board:0553's tree, and
the loading rule is a WALK over that node, in source order.**

```cpp
class Page {
  void LoadParts();          // content first, then the FactBoxes area, top to bottom
  bool loaded_[kMaxParts];   // "not loaded again for as long as the page remains open"
};
```

**Why a flag per part and not a set:** the count is `constexpr` from `PageDef`, the page is one
session's object, and CLAUDE.md counts per-session state in bytes. A `std::set` here is an allocation
on a path that runs on every page open.

**Why source order and not any other:** the documentation says "starting from the top", and CLAUDE.md
requires anything assembled from independent work to be combined in a DECLARED order. The tree already
holds it, so the order costs nothing to honour and would cost a defect to lose.

**The scroll rule is NOT implemented and that is deliberate.** "A FactBox not in view is only loaded
when the user scrolls it into view" is a client-viewport fact, and the server does not know the
viewport. Under phase 2's htmx shape the browser asks for a fragment, so the rule becomes *the
fragment is fetched when requested* -- which is the same observable behaviour arrived at from the other
side. **`Visible = false` is a server-side decision and IS implemented**, because AL evaluates the
expression.

**A part follows the host's current row**, per WI-1190, and the re-evaluation happens after every
change of the host's current record -- not once at construction. **An empty filter result is a result**
and is not taken back.

**Refuse at translation time, once:** a `FactBoxes` part whose target `PageType` is neither `CardPart`
nor `ListPart`. The platform errors, so the transpiler errors, and it is a `static_assert` rather than
a run-time check because both `PageType`s are `constexpr`.

## Ordering

**After board:0553**, which supplies the tree and the area kind -- there is nothing to walk before it.
**Before board:0537**, whose card and list renderers place a FactBox pane.

The trigger rules -- once only, never when hidden -- come with the first part that has an `OnOpenPage`,
because a part loaded twice is indistinguishable from one loaded once until a trigger counts.

## Gate, and its negative control

A `List` page with an `area(FactBoxes)` holding two parts, the first with `Visible = false`:

1. opening the page runs the second part's `OnOpenPage` and **not the first's**
2. the parts load in source order, the host's content before either
3. collapsing and re-expanding runs neither `OnOpenPage` a second time
4. moving to the next host row re-evaluates the visible part's `SubPageLink`, and a link that filters
   to nothing leaves the part EMPTY rather than unfiltered

**The negative control is the trigger COUNTER, not the rendered output.** Remove the `loaded_` flag and
case 3 must go red -- a control that only asserts the part shows the right rows stays green whether the
trigger ran once or three times, which is exactly the defect this rule exists to prevent.

**Second control:** remove the `Visible` check and case 1 must go red. It goes red only if the test
counts the FIRST part's trigger; one that counts the total sees 1 versus 2 and would also catch it, but
one that asserts "the second part is present" is blind to both.

## Class

`activation`. No part lifecycle runs today, so every one of these paths is new and there is nothing to
regress -- but a page whose parts start firing triggers is a page whose host triggers now see different
state, which is why the A/B is over the whole UT suite and not over the page tests.
