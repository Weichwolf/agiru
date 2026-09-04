Type:     task
Status:   open
Parent:   0018
Area:     rt, gen
Source:   developer/devenv-filter-pages-for-filtering-tables.md, developer/devenv-adding-filter-tokens.md, developer/devenv-table-field-text-search.md
Verdict:  fehlt
Class:    activation

# A filter page is built at run time, and a filter token is an event

**Three pages, one item**: the `FilterPageBuilder`, the custom filter tokens the filter pane resolves,
and the text search over a table field. All three extend board:0018's filter language at RUN time
rather than at translation time, which is what makes them one subject.

## `FilterPageBuilder` builds a page from code

> "you can use the `FilterPageBuilder` data type to **create a filter page that enables users to set
> filters on MULTIPLE TABLES** ... **filter pages are GENERATED AT RUNTIME and run in a MODAL DIALOG
> BOX.**"
>
> ```AL
> FilterPage.AddTable(Customer.TableCaption(), Database::Customer);
> FilterPage.AddRecord(Item.TableCaption(), Item);
> FilterPage.AddField(Item.TableCaption(), Item."No.", '>100');
> FilterPage.PageCaption := FilterPageCaption;
> FilterPage.RunModal();
> ```

**A page with no page object** -- the only one in this sweep. Everything else in board:0030 is a
declared control tree; this is a control tree assembled by AL at run time from table and field
identifiers.

**So the renderer needs a second entry point**: not "render page N" but "render this constructed
descriptor". That is a real architectural requirement and it is the reason this item is filed rather
than folded into board:0030.

**And board:0198's `[FilterPageHandler]` is its test seam** -- board:0540 lists its signature,
`(var Record1: RecordRef[, ...]): Boolean`, which is how a test answers a page that has no object to
name.

## A filter token is an event subscriber, and the runtime owes only the raise

> "users can enter **filter tokens**, which are special words that resolve to one or more values ...
> entering **`%mycustomers`** in a Customer No. field will resolve to `1001|1002`."
>
> "Subscribe to the **`OnResolveTextFilterToken`** event associated with the `MakeTextFilter` method
> from the **`Filter Tokens` codeunit**."
>
> ```AL
> [EventSubscriber(ObjectType::Codeunit, Codeunit::"Filter Tokens", 'OnResolveTextFilterToken', '', true, true)]
> local procedure FilterMyAccounts(TextToken: Text; var TextFilter: Text; var Handled: Boolean)
> ```
>
> **"If the filter string must contain MULTIPLE VALUES, you must handle the operators that join them by
> adding the `|` filter symbol."**
>
> **"We recommend that you only modify the filter token you have introduced and PRESERVE THE REST of
> the filter string."**

**This is board:0512's finding again, in its clearest form**: the token resolution is AL, in a BaseApp
codeunit, reached by event. board:0512 lists codeunit 42 `TextManagement`'s `OnBeforeMakeTextFilter`
and `OnAfterMakeTextFilter` among the "global" events -- **and this page names the same mechanism from
the consumer side.**

**So the runtime owes: raise `OnResolveTextFilterToken` before parsing a filter string, and use what
comes back.** It must not know `%mycustomers`, and CLAUDE.md's invariant says so.

**The `var Handled: Boolean` is board:0516's pattern** -- and board:0516's warning applies exactly:
a copied `var` means every token resolver reports unhandled and the raw `%mycustomers` reaches
board:0509's parser as a literal value.

**The subscriber's last two arguments are `true, true`** -- `SkipOnMissingLicense` and
`SkipOnMissingPermission` (board:0513), so a token resolver the user cannot execute is SKIPPED rather
than failing the filter. That is the right default here and it is the opposite of board:0513's global
default of `false`.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0196: `[EventSubscriber]` **11 142**. **The `FilterPageBuilder` and `OnResolveTextFilterToken`
counts are method calls and subscriber declarations rather than properties** -- stated rather than
guessed.

## The IST-state

board:0018 records the filter state; `src/rt/Filter.cpp` and `src/rt/Where.cpp` exist. board:0057
records that no event dispatch exists, so no token could be resolved. board:0030 has no renderer, so
no filter page could be shown.

## The choice

`FilterPageBuilder` accumulates `{ caption, table id, record, field, initial filter }` entries and hands
board:0030's renderer a CONSTRUCTED descriptor -- the second entry point named above.

**Token resolution happens BEFORE board:0509's parser sees the string**, as one event raise per filter
term, and the runtime names no token.

## Ordering

Behind board:0509's filter parser, board:0512's dispatch and board:0030's renderer. The constructed
descriptor is the piece that must exist in the renderer's design from the start rather than being
retrofitted.

## Gate, and its negative control

A filter page built from code shows one filter control per added table and applies what the user
enters; a filter string containing a token registered by a subscriber resolves to the subscriber's
value.

**The negative control is a filter string with a token AND other criteria** -- the documentation says to
preserve the rest, so `%mycustomers&>1000` must keep the second term. An implementation that lets the
subscriber overwrite the whole string passes any single-token gate and silently drops every other
criterion.
