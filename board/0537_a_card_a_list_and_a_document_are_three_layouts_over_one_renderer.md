Type:     task
Status:   open
Parent:   0030
Area:     rt, gen
Source:   developer/devenv-designing-card-pages.md, developer/devenv-designing-list-pages.md, developer/devenv-designing-navigate-pages.md, developer/devenv-designing-parts.md, developer/devenv-designing-cardparts.md, developer/devenv-designing-listparts.md, developer/devenv-designing-multilist-pages.md, developer/devenv-simple-card-page-example.md, developer/devenv-simple-list-page-example.md
Verdict:  fehlt
Class:    activation

# A card, a list and a document are three layouts over one renderer

**Nine pages, one item**: the three main page types, the three part types, the multilist, and the two
worked examples. board:0429 measured the distribution -- `List` 2 740, `Card` 923, `ListPart` 817,
`Document` 429, **4 909 of 6 891 pages, 71 %** -- and said four renderers cover seven pages in ten.
**These are those four renderers' specifications.**

## What each type is, and what the client supplies

> **Card**: "view, create, and modify records (master and reference data)".
> **Document**: "the computerized counterpart to paper-based documents ... **in addition to fields, it
> also includes a PART that includes another page, called a SUB-PAGE.**"
> **List**: "records from an underlying table, **either as rows and columns or as individual TILES**"
> (board:0529's `fieldgroup(Brick)`).
> **NavigatePage**: a wizard. **"In the client, the `NavigatePage` page DOESN'T have an action bar.
> It's designed to have buttons at the bottom"** -- Back, Next, Finish.

**The system actions are the platform's and cannot be changed:**

> "**System actions** ... the ability to edit the record, create a new record, and delete the current
> record. **The actions are only active if the `Editable` property is set to `true`.** **These actions
> appear on ALL PAGES; YOU CANNOT REMOVE THEM OR ADD OTHER ACTIONS.**"

**So a page has actions the AL never declares**, and their enabled state comes from board:0400's
`Editable` -- which board:0434 says is read from the CARD page when a list declares a `CardPageId`.
Three items meet here and the renderer has to resolve all three before it draws one button.

**The action bar has three standard menus** -- **Actions**, **Navigate**, **Report** -- plus promoted
categories (board:0477). So board:0425's placement enumerator has a fourth dimension: which of the
three standard menus.

## A `NavigatePage` uses a temporary source, and the documentation says why

> `SourceTableTemporary = true` -- **"because Business Central AUTOMATICALLY STORES ALL MODIFICATIONS
> TO DATABASE TABLES AS SOON AS USERS MOVE FOCUS to another field or close the page. Using a temporary
> table lets users EXIT THE ASSISTED SETUP AT ANY POINT, WITHOUT SAVING** the changes they made so
> far."

**That sentence is the single most consequential statement about page behaviour in this sweep**: a page
bound to a real table WRITES ON FOCUS CHANGE. Not on Finish, not on OK -- on leaving a field.

board:0404's `DelayedInsert` is the same fact for the INSERT half; this is the MODIFY half, and it is
the default for every page in the system. **A renderer that batched changes until the user confirmed
would be safer, more familiar, and wrong** -- and every AL trigger that reacts to a modify would fire
at a different time.

**And each wizard step is a `group()` control at the root of `area(Content)`** -- so the steps are not
separate pages; one page with N groups and a visible-group index. board:0401's `Visible` is how a step
is shown or hidden.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0429: `List` **2 740**, `Card` **923**, `ListPart` **817**, `Document` **429**, `CardPart`
**271**, `NavigatePage` **159**. board:0431: `SourceTable` **6 295**, `SourceTableTemporary` **680**.
board:0434: `CardPageId` **599**.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone (board:0431); no page type, no controls, no
actions, no renderer.

## The choice

**One control-tree walker, four layout strategies** -- not four renderers. The tree, the bindings and
the triggers are shared; what differs is how the walker arranges an `area(Content)`: one record's
fields (Card), a repeater (List), a card plus a part (Document), one visible group of many
(NavigatePage).

**The write-on-focus-change behaviour lands in the field's input path**, not in the page's save path,
and it is what board:0317's UI-time constraint checks hang off.

The three standard menus and the system actions are `constexpr` in the renderer, since AL cannot
declare them.

## Ordering

board:0030's core, behind board:0429's page type. The four layouts in population order: List, Card,
ListPart, Document.

## Gate, and its negative control

A card page renders one record's fields and its system actions; typing in a field and moving focus
WRITES the record; a `NavigatePage` shows one group at a time with Back/Next/Finish and writes nothing
until its code does.

**The negative control is the focus change on a card page** -- an implementation that saves on OK
passes every "the value is right" gate and changes when every `OnModify` trigger in the BaseApp fires.
