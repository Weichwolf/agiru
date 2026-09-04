Type:     task
Status:   open
Parent:   0043
Area:     gen
Source:   developer/properties/devenv-testtablerelation-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `TestTableRelation` is refused, and every occurrence of it is commented out

> **Version**: Available or changed with runtime version 1.0 **until version 1.0 where it was
> deprecated.**
>
> Sets whether to include this field when evaluating field relations between primary and secondary
> indexes. **The default is true.**
>
> **This test is available from the Database option on the File menu.**

The last line dates the property: it gates a C/SIDE consistency tool with no successor in the modern
client, and the header says it was deprecated in the same runtime version that introduced it.

The page still states one rule that belongs to its live twin (board:0332):

> If you set the `ValidateTableRelation` property to false, then you should also set the
> `TestTableRelation` property to false. Otherwise, a database test on the field relations in a
> database may fail.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`TestTableRelation =`: **0 declarations.**

**The first measurement said 15 and every one of them is commented out** -- `//TestTableRelation =
false;`, in `PaymentStepLedgerFR.Table.al` and four places in `VendorSubscriptionContract.Table.al`.
The same re-measurement removed 61 of `ValidateTableRelation`'s apparent declarations for the same
reason. A pattern anchored at a statement boundary excludes a comment; one that is not, does not.

That number changes this item's decision. **The first version of it argued that 15 declarations made
a refusal too expensive and recorded the property as known-and-ignored instead.** With 0, that
argument is void and the property joins the group that is refused.

## The IST-state

The generator does not know the property; whether it refuses or drops an unknown field property is
board:0067's counter.

## The choice

**Refuse in the generator**, as board:0327, board:0346 and board:0347 refuse `SignDisplacement`,
`SqlIndex` and `ColumnStoreIndex`. The arithmetic is the same at every one of them: at 0 declarations
a refusal costs nothing today, and it is the notification if one ever appears. There is no
database-consistency tool here to gate, so accepting it would mean accepting a declaration and doing
nothing with it, which this tree treats as worse than refusing.

## Ordering

With board:0067's census. No runtime work.

## Gate, and its negative control

A table declaring `TestTableRelation` fails to transpile with a message naming the property.

**The negative control is the whole BaseApp transpiling with the refusal in place.** A count of 0 is
exactly the number that has to be proven rather than trusted -- this item's own first number was
wrong in that direction.
