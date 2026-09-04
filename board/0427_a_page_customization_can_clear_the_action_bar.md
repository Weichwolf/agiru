Type:     task
Status:   open
Parent:   0033
Area:     gen
Source:   developer/properties/devenv-clearactions-property.md
Verdict:  fehlt
Class:    activation

# A page customization can clear the action bar

> **Version**: runtime 14.0. Applies to: **Page Customization.**
>
> **Clears all actions from the page's action bar.**

**One sentence, and it is a merge rule.** A `pagecustomization` normally ADDS to or MODIFIES a page;
this property REMOVES everything the base page declared, and then the customization's own actions are
what remains.

That makes it board:0033's territory rather than board:0030's: extensions are merged at translation
time, and this is the one property in the sweep that makes a merge SUBTRACTIVE.

**And it contradicts the direction rule elsewhere.** board:0379 records that a permission set
extension may include and may not exclude -- an extension may only widen. This property is the
opposite for a page customization, so "an extension only adds" is not a general rule of AL, and an
implementation built on that assumption has no place to put this.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ClearActions =`: **35 declarations.**

## The IST-state

Page customizations, like page extensions, are merged by board:0033's mechanism, and
`src/gen/PageWriter.cpp` consumes `SourceTable` alone -- so there are no actions to clear.

## The choice

The merge applies the property BEFORE the customization's own action declarations and AFTER the base
page's, which is the only order that makes the sentence true. Resolved entirely in the generator; the
runtime sees a page whose action list is what remained.

**An empty action list is a legal outcome** and must not be confused with an unparsed one -- which is
the same "empty result is an abort" trap CLAUDE.md names, arriving where the empty result is correct.

## Ordering

Inside board:0033's merge, with page customizations.

## Gate, and its negative control

A page customization declaring `ClearActions` over a page with three actions and declaring one of its
own produces a page with exactly one action.

**The negative control is the ORDER** -- an implementation that clears after applying the
customization's own actions produces a page with zero, which is also a plausible-looking result and
is wrong.
