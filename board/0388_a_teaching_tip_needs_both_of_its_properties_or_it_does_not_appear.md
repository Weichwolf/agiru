Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-abouttitle-property.md, developer/properties/devenv-abouttext-property.md
Verdict:  fehlt
Class:    activation

# A teaching tip needs both of its properties, or it does not appear

**Two pages, one item**: each states the dependency in the same sentence.

> **When setting this property, you must also set the [other] property. Both About properties must be
> specified for the teaching tip to appear.**
>
> **AboutTitle** (runtime 10.0): the large-font title. **AboutText**: the body, and it **supports a
> rich text value such as `**bold**` and `*italic*`**.
>
> Applies to (both): Page, Request Page, Page Field, Page Group, Page Part, Page Action, Page Action
> Group, Page Custom Action, Page File Upload Action, **Query**.

Three rules beyond the pairing, and each is checkable at translation time:

1. **The tip is not displayed if the page's `PageType` is `RoleCenter`, `NavigatePage`,
   `ConfirmationDialog`, `StandardDialog` or `HeadlinePart`** -- and the documentation names the
   analyzer that catches it, **UICop AW0012**. So AL's own toolchain treats this as a compile-time
   error, which makes it a `static_assert` here rather than a render-time skip.
2. **Ignored if the client is not the Web client.** agiru has only a web client, so the condition is
   always true -- a documented non-condition, recorded rather than implemented.
3. **In lookup mode the tip is not shown automatically** but is still reachable from the page caption.

`AboutText`'s rich text is the one piece of real work: `**bold**` and `*italic*` are a subset of
Markdown, rendered rather than escaped, and that is the only place in the page metadata where a
declared string is not emitted verbatim.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`AboutTitle =` **1 859** · `AboutText =` **2 119**.

**They do not match, and the difference is the finding**: 260 elements declare a body with no title,
and by the documentation's own rule none of those tips appears. Whether that is a BaseApp defect or a
measurement artefact -- the two properties applying to slightly different kinds -- is settled by
listing the 260 when the item is pulled.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

Two `string_view`s on the element descriptor with board:0382's caption, and a `static_assert` for the
pairing and for the five forbidden page types. The renderer emits the tip; the Markdown subset is a
small renderer over two constructs and not a Markdown library.

## Ordering

With board:0030's control metadata.

## Gate, and its negative control

An element declaring both renders a tip with the title in large font and `**bold**` as bold; an
element declaring only one fails to transpile.

**The negative control is the single-property element** -- an implementation that renders a tip with
an empty title passes the positive gate and shows a tip BC does not.
