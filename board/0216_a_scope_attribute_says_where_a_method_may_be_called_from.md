Type:     task
Status:   open
Parent:   0190
Area:     gen
Source:   developer/attributes/devenv-scope-attribute.md
Verdict:  fehlt
Class:    activation

# A `[Scope]` attribute says where a method may be called from, and four of its six values are dead

`[Scope(Kind: Text)]` on a method. The value table is mostly a graveyard: `Solution`,
`Personalization`, `Extension` and `Internal` were all deprecated in runtime 4.0, each with its own
replacement -- `Solution` and `Internal` become **`OnPrem`**, `Personalization` and `Extension`
become **`Cloud`**.

So the live vocabulary is two values, and agiru is one of them: **`OnPrem`**. A method scoped
`Cloud` is callable here; a method scoped `OnPrem` is callable here; the distinction that BC uses to
keep cloud extensions off on-premises surface does not bind a runtime that is on-premises.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`grep -c "\[Scope"` reports **116 496**, but the token `Scope` is also an ordinary identifier in AL
(`DataScope`, `NotificationScope`, `TelemetryScope`, a variable named `Scope`), so that figure is an
upper bound and not the population. **The count that decides the ordering has to be re-measured with
an anchored pattern when this item is worked** -- and saying so is cheaper than carrying a number
that is wrong by an order of magnitude.

## The IST-state

The attribute parses into the raw list and is dropped.

## The choice

**Carry it as `constexpr` metadata and act on nothing.** agiru is on-premises, so every documented
value resolves to "callable", and the only thing the attribute can still do here is document a
method's intent to a reader.

**Why carry it at all.** board:0190's rule is that an attribute is acted on or REFUSED, and the
refusal has to distinguish "the generator does not know this attribute" from "this attribute has no
effect in an on-premises runtime". Recording it as known-and-inert is the second, and it is what
stops a future reader from filing this gap twice.

Its four deprecated values are a translation-time WARNING naming the replacement, because a `.al`
still carrying `Scope(Internal)` is source that predates runtime 4.0 and is worth seeing.

## Ordering

Low. Nothing depends on it and nothing breaks without it. It exists so the epic's counter can reach
41 of 41.

## Gate, and its negative control

A method with `[Scope('OnPrem')]` translates and is callable; a method with `[Scope('Internal')]`
translates, is callable, and produces the deprecation warning naming `OnPrem`.

**The negative control is the warning** -- remove it and the second case must go silent, which is
how you tell "known and inert" from "dropped".
