# 0049 -- An option value resolves through any record variable, or refuses

`Rec.Status::Blank` is AL for "the member `Blank` of the option field `Status`". The generator
resolves the scope through a table of field enumerations, and where it cannot it emits the base
expression followed by `::`, which is not C++ at all:

    (*this).Status = (*this).Status::Blank;     // error: 'Status' is not a class

Counted over the generated tree, 2026-09-03:

| what the generator did | sites |
|---|---|
| resolved to `tables::<Table><Field>::<Member>` | 30 144 |
| refused loudly, as `RefusedOption("...")` | 1 463 |
| **emitted `Var.Field::Member` and did not compile** | **3 765** |

The third row is the item. It is not a missing mechanism -- 30 144 sites prove the mechanism -- it
is the lookup failing and then falling through to a shape that cannot work.

**The reference.** `devenv-options.md` and the `OptionMembers` property. The scope is the FIELD and
the value is one of its members; a field declared `Enum` scopes to the enum object instead.

**What is known.** Two holes are closed this round: a table's own scope and a page's source-table
scope never answered `FieldEnumeration` at all, so every `Rec.Field::Member` inside a table trigger
or a page fell through. That was 24 of 300 sampled bodies before and 10 after.

**What is left.** The remaining sites are a NAMED record variable in a codeunit --
`BankAccountLedgerEntry."Statement Status"::Closed`, where the variable is a procedure local, the
table is in scope, and `tables::BankAccountLedgerEntryStatementStatus` is emitted in the header. So
the enum exists and the lookup still misses. The candidates, in the order they are cheapest to rule
out: the subtype key carrying its AL quotes, the field key spelled as an identifier on one side and
an AL name on the other, and the table being indexed under a name the variable's subtype does not
match.

**The choice.** FIND THE KEY MISMATCH, and then make the fall-through IMPOSSIBLE: a scope the
generator cannot resolve emits `RefusedOption`, which compiles and throws with the AL text in it,
rather than `Var.Field::Member`, which does not compile and buries every other diagnostic in the
file behind it. The refusal already exists and is reached 1 463 times; it is the fall-through that
should never have had a third branch.

A gate belongs on the generator for this: an option scope through a table field, an enum field, a
named record variable and an unknown variable, with the fourth producing a refusal and not a
syntax error.
