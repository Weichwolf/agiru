# 0051 -- The door's spelling and calling authority is per TYPE, not one global set

`Customer.SystemId` was emitted as `Customer.SystemId()` because
`include/type/ErrorInfo.h` declares `ErrorInfo.SystemId(Guid)`. One method on one AL type decided
the shape of a member access on an unrelated one.

The mechanism reads every header under `include/` and keeps two sets: names the door SPELLS (used to
collapse AL's case-insensitive casing -- `TESTFIELD` to `TestField`, 19 162 sites) and names the
door CALLS (used to put back the parentheses AL leaves off -- `Rec.IsEmpty` to `Rec.IsEmpty()`,
3 353 sites). Both sets are global across all 147 door headers.

**Why it has not bitten more.** AL's own vocabulary is consistent: a method called `Value` means the
same thing on a `TestField` and on a `FieldRef`, which is the naming invariant working in the
tree's favour. The collision needs a name that is a METHOD on one type and a FIELD on another, and
`SystemId` is the first found.

**The repair that was made instead, and why it is not enough.** A record's SYSTEM FIELDS were
missing from the field set the rule checks against -- they are added by the platform and appear in
no `field()` block -- so `SystemId` looked like "not a field of this table, therefore a call". They
are in the set now, taken from `agiru::kSystemFields`, which `meta/Declare.h` calls "the only place
in the tree that spells them". That closes THIS collision and leaves the shape.

**The choice.** The scan learns which CLASS each name belongs to -- it already walks every header
line by line, and a `class X {` line is as easy to see as a declaration -- and the generator asks
"does `TestPage` call `Value`" rather than "does the door call `Value`". The variable's declared AL
type is known at every call site that matters, and `Names::MemberSpelling` and `Names::MemberIsCall`
already carry it in `OfVariable`.

**The gate that belongs on it**: a name declared as a method on one door type and as a member on
another, with the generator asked about both. It goes red today.
