Type:     task
Status:   open
Parent:   0030
Area:     gen
Source:   developer/devenv-api-pagetype.md, developer/devenv-creating-and-interacting-with-odatav4-bound-action.md, developer/devenv-creating-and-interacting-with-odatav4-unbound-action.md
Verdict:  fehlt
Class:    activation

# An API page is a page, and its parts are navigation properties

board:0429 lists `API` among `PageType`'s nineteen values and quotes the line that decides this item's
scope: **"pages of this type are used to generate web service endpoints and CANNOT BE SHOWN IN THE
USER INTERFACE."**

**So an API page has no renderer and it still has to transpile**: it is 374 page objects with a source
table, a repeater, fields, triggers and AL procedures, and CLAUDE.md's scope sentence admits no
exception for a kind that happens to have no UI.

## The scope position, stated rather than assumed

**The OBJECT is in scope. The HTTP endpoint is not a phase 1-3 target.**

CLAUDE.md's three phases are the UT suite, the UI with `TestPage`, and the whole AL test suite. An
OData v4 surface with `$metadata`, webhooks and versioned routes is none of them. **What an API page
must do here is compile, carry its properties, and run its `OnOpenPage` and its `[ServiceEnabled]`
procedures when a test calls them** -- which is what a UT case can reach.

That is a deliberate boundary and it is recorded so it is a decision rather than an omission. It also
follows the tree's own rule about libraries: an OData router is not written from scratch either, and
nothing is being written for it now.

## An UNBOUND action needs nothing from the transpiler

The two OData action pages are not symmetric and the asymmetry is the finding.

**A BOUND action is a declaration**: a `[ServiceEnabled]` procedure on a page, taking a
`var WebServiceActionContext`, which it fills with `SetObjectType`, `SetObjectId`, `AddEntityKey` and
`SetResultCode`. board:0219 owns the attribute and board:0192 the `Caption` that names it.

**An UNBOUND action is not a declaration at all.** It is *"a codeunit with a procedure with the
desired business logic"* -- no attribute, no context parameter, no property -- reached at
`POST /ODataV4/{serviceName}_{procedureName}`. **Publishing is a registration step in the client, not
something the AL says**, so an ordinary public codeunit procedure already IS one.

So the unbound half of the subject needs nothing from the transpiler beyond what a public procedure
already gets, and it is recorded here so the page is not read a second time looking for the missing
mechanism.

## `EntityName` is a PAGE property and a PART property, and that is the structure

**Measured, and it refuted the first hypothesis.** `EntityName` is **854 declarations over 489
FILES**, against 374 `PageType = API` pages and 113 `QueryType = API` queries -- 487, plus exactly two
non-API pages (`SalesDocumentEntity.Page.al`, `PurchaseDocumentEntity.Page.al`), so **489 exactly**.

The excess 365 looked like the preprocessor duplicating declarations across `#if not CLEANxx`
branches, which board:0552 measures at 9 946 `#if` in the tree. **That was checked and is wrong: of
the 140 files declaring `EntityName` more than once, FOUR contain a preprocessor directive at all.**

The real answer is in `APIV2Items.Page.al`, which declares it eight times:

```AL
page 30008 "APIV2 - Items"
{
    EntityName = 'item';
    EntitySetName = 'items';
    PageType = API;
    ...
        part(inventoryPostingGroup; "APIV2 - Inventory Post. Group")
        {
            EntityName = 'inventoryPostingGroup';
            EntitySetName = 'inventoryPostingGroups';
        }
        part(picture; "APIV2 - Pictures") { EntityName = 'picture'; ... }
```

**A `part` on an API page is an OData NAVIGATION PROPERTY and carries its own entity name.** So the
page's control tree is also the entity graph -- board:0553's tree again, in a third role after layout
and after board:0554's FactBox loading. **A flattened control list loses the association between a
part and its entity name**, which is the same defect in a third place.

## What the platform refuses, and both are decidable

> "This page type **CAN'T BE EXTENDED** by creating a page extension object. Instead, you must create
> a new API by adding a page object."
>
> "Bound actions **cannot be added by extending** an existing page that has been exposed as a web
> service."

**Both are translation-time refusals**, because the extended page's `PageType` is `constexpr` and the
generator holds it: a `pageextension` over a `PageType = API` page is an error, and so is a
`[ServiceEnabled]` procedure added by an extension.

**And the naming rules are checkable too**, with the platform's own severities:

> "camelCase for naming attributes, tables, and `APIPublisher`, `APIGroup`, `EntityName`, and
> `EntitySetName`. Alphanumeric characters allowed (A-Z + a-z + 0-9) ... `APIVersion` follows the
> pattern `vX.Y` or `beta`. **At design time, the compiler shows WARNINGS on casing violations and
> ERRORS on naming violations.**"

So a non-alphanumeric `EntityName` is an error and a `PascalCase` one is a warning -- **two severities
the transpiler can reproduce exactly, and the distinction matters**: an error is a `static_assert`, a
warning is a counter. Getting it the other way round would reject BC's own pages over casing.

`APIVersion` matching `vX.Y|beta` is a `constexpr` string check, and **it is a list**:
`APIVersion = 'v2.0', 'v1.0';`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

| | count |
|---|---:|
| `InsertAllowed =` | 2 101 |
| `DeleteAllowed =` | 1 863 |
| `DelayedInsert =` | 1 048 |
| `ModifyAllowed =` | 1 020 |
| `EntityName =` / `EntitySetName =` | **854** each, over **489 files** |
| `APIVersion =` | 481 |
| `ODataKeyFields =` | 349 |
| `APIPublisher =` / `APIGroup =` | **295** each |
| `[ServiceEnabled]` | 131 |
| `: WebServiceActionContext` | 134 |

`APIVersion` values: **`'v2.0'` 168, `'v1.0'` 132, `'v0.5', 'v1.0'` 98, `'beta'` 69, `'v0.1'` 10,
`'v0.5'` 3, `'v1.0', 'v2.0', 'beta'` 1** -- so the multi-version form is 99 of 481 and the pattern
must accept a comma-separated list.

`APIPublisher`: **`'microsoft'` 283, `'mock'` 12.** Every one in the tree, and 12 of them in test
code.

**295 `APIPublisher` against 374 API pages leaves 79 pages declaring none**, and what they take
instead is not settled here -- the property page and the concept page both call it required-looking
without saying what a page without one does. **Named as unsettled rather than inferred from the
default of a neighbouring property.**

## The IST-state

- **`PageType` is never read** (board:0553), so `API` is indistinguishable from `Card` in the output.
- **`EntityName` and every other API property is dropped** with all page-control properties.
- **`WebServiceActionContext` and `WebServiceActionResultCode` exist in the door and refuse** --
  `src/rt/Door.cpp:2979`-`:3008`, seven methods. board:0219 owns `[ServiceEnabled]` and board:0192 the
  `Caption` attribute that names an OData action.
- **board:0368's `DataAccessIntent` and board:0369's `ChangeTrackingAllowed` apply only to `API`**, so
  they are waiting on the same `PageType`.

## The choice

**Nothing new: `PageDef` gains the API properties as `constexpr` members and `ControlDef` gains
`entityName`**, because a part carries one.

```cpp
struct PageDef {
  ...
  PageType type;
  std::string_view apiPublisher, apiGroup, entityName, entitySetName;
  std::span<const std::string_view> apiVersions;   // a LIST
};
```

**Why on `ControlDef` too and not only on the page:** 365 of the 854 declarations are on parts, and
the association between a part and its entity name is the navigation graph. Putting it only on the
page would keep the name and lose which part it names.

**Two `static_assert`s and one counter**, matching the platform's two severities:

- a `pageextension` extending a `PageType = API` page -- **error**
- an `EntityName` with a non-alphanumeric character, or an `APIVersion` not matching `v<digits>.<digits>`
  or `beta` -- **error**
- an `EntityName` that is not camelCase -- **a counter**, never a refusal

**The `ReadIsolation` idiom is not special-cased.** The page documents
`Rec.ReadIsolation := IsolationLevel::ReadCommitted` in `OnOpenPage` as how an API exposes only
committed data. That is ordinary AL over board:0012's fourth dial, at 1 034 call sites tree-wide, and
it needs nothing from this item -- worth recording because it looks like an API feature and is not.

## Ordering

**Inside board:0553**, after the tree and the properties. **After board:0429's `PageType` enum**,
without which none of the three refusals can be expressed.

The four CRUD-gating properties -- `InsertAllowed` 2 101, `DeleteAllowed` 1 863, `ModifyAllowed`
1 020 -- are the largest numbers here and they are NOT API-specific: they gate an ordinary page too,
so they come with board:0537's renderer rather than with this item.

## Gate, and its negative control

1. a `PageType = API` page transpiles, with `entityName`, `entitySetName` and an `apiVersions` list of
   two
2. a `part` on it carries its OWN `entityName`, distinct from the page's
3. a `pageextension` over it **fails to transpile**
4. `APIVersion = 'v2'` (no minor) **fails to transpile**; `APIVersion = 'beta'` does not
5. `EntityName = 'Item'` transpiles and increments the casing counter

**The negative control is case 5 against case 4.** Make the casing rule an error and case 5 goes red
while 1 through 4 stay green -- and case 5 going red means BC's own pages stop translating the moment
one of them capitalises. It is the case that proves the two severities were kept apart rather than
merged into whichever was implemented first.

**Case 2 is the second control**: keep `entityName` only on the page and case 2 goes red while
everything else passes -- the 365 part-level declarations would be silently attributed to the page.

## Class

`activation`. No page property is emitted today, so nothing regresses. The refusals are the risk: case
3 must fire on a `pageextension` over an API page and on nothing else, and `make apps` over the whole
tree is the A/B for that before any of the metadata is used.

## THE TWO API OBJECT KINDS, SIDE BY SIDE

`devenv-api.md` (read 2026-09-04, routed here) tabulates the choice this item and board:0550 split
between them:

| | API page | API query |
|---|---|---|
| operations | **read AND write** | **read only** |
| webhooks | supported | -- |
| extensible | **no** | **no** |
| tables | **one** | **several** |

So the two kinds are not two spellings of one thing: a page is one table with CRUD, a query is a join
with none. **Both refuse extension**, which makes board:0567's first `static_assert` apply to
`QueryType = API` as well -- 113 query objects on top of the 374 pages.

`devenv-connect-apps-tips.md` (read 2026-09-04, routed here) is the client side of the same surface --
`GET` shapes, the `Accept-Language` header, OData `$batch`, and `DataAccessIntent` on a GET
(board:0368). It is downstream of this item's scope decision -- the HTTP endpoint is not a phase 1-3
target -- and is recorded so the page is not read twice.
