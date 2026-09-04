Type:     task
Status:   open
Parent:   0067
Area:     gen
Source:   developer/properties/devenv-signdisplacement-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `SignDisplacement` is recorded and refused, rather than silently ignored

> Sets a value to shift negative values to the right **for display purposes only**. You can shift
> negative values in increments of **1/100 of a millimeter**.

A print-layout property from the C/SIDE era: negative amounts indented so the minus sign hangs
outside the column. It is measured in hundredths of a millimetre, which only means something on
paper.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`SignDisplacement =`: **0 declarations.** Not one field in the whole tree.

**A count of 0 is why this item exists at all.** The rule here is that a declaration accepted and
then ignored is worse than one refused, so a property nobody uses still needs a decision -- and the
decision is to REFUSE it in the generator, loudly, rather than to build a millimetre offset no page
would consume.

## The IST-state

The generator does not know the property. Whether it currently refuses an unknown field property or
drops it is board:0067's counter, and this item's answer depends on that one being right.

## The choice

Refuse. `SignDisplacement` on a field is a translation error naming the property and the field, and
that costs nothing until BCApps declares one -- at which point the error is the notification.

**Not the alternative**: mapping it into the format engine. There is no consumer, a paper offset in
hundredths of a millimetre has no meaning in an HTML fragment, and a mapping with no consumer is a
line of code that can only be wrong.

## Ordering

With board:0067, which is the counter that finds the properties nobody handled.

## Gate, and its negative control

A table declaring `SignDisplacement = 600` fails to transpile with a message naming the property.

**The negative control is the count** -- if the transpiler passes the whole BaseApp while the
property is refused, the refusal is proven unreachable, which is the population saying the same
thing twice.
