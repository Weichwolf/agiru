# 0681 A new row takes the key its filters fix

**The finding.** 33 UT cases fail with `Document No. must have a value in Sales Line: Document
Type='', Document No.='', Line No.='0'` and the Purchase Line equivalent. Every one of them drives
a document card through a `TestPage`, whose lines part narrows the sub-record by `SubPageLink`
(`src/rt/SubPageLink.cpp` sets filters), and then inserts a line through the part. The filters were
predicates and never values, so the inserted row carried a blank primary key.

**The platform documentation.** `properties/devenv-populateallfields-property.md`: "Values are
inserted in those fields where a currently active filter expression evaluates to exactly one
value", and -- the half that decides this case -- "Key fields are always populated", whatever the
property says. `methods-auto/record/record-init-method.md` names no filter at all and says the
primary key is not initialised, so this is a PAGE mechanism and not `Init`'s: a bare
`SetRange` plus `Init()` in a codeunit fills nothing.

**The AL source.** `PopulateAllFields = true` on 137 pages and `= false` on 6; the other ~2 700
inherit the documented default of false. So the property gates the non-key fields and the key is
seeded on every page.

**The predecessor.** openerp WI-1329 quotes the same page and records as an OPEN question that its
`_apply_filter_constants_to_rec` applied the rule on EVERY page, ungated, and never separated the
key fields from the rest -- the half that is unconditional from the half that is not. WI-1201
(done, 2236 -> 2239) is the neighbouring defect: a part reached through `CurrPage` never applied
its `SubPageLink` at all, and its lesson is that the document TYPE travels with the number because
lines key on both. The whole `... must have a value in ... Line` family there is some sixteen
items, which is why this is one generic mechanism and not a table's fix.

**The choice.** `detail::SeedFromFilters(record, table, populateAllFields)` in `src/rt/Filter.cpp`,
called from the page's new-row path (`TestPage::New`) after `Init` and before `OnNewRecord`. It
seeds a field when every active filter on it is the SAME single equality -- never a range, a set,
a wildcard or a negation (`SingleFilterValue`) -- and it seeds the PRIMARY KEY whatever the page
says, everything else only under `PopulateAllFields`. It ASSIGNS and does not `Validate`: the
property page names no trigger, and `Init`, the other place the platform writes a field nobody
asked it to, assigns as well. Where that turns out wrong a case will say so.

**Classification: activation.** A dead path starts writing key fields on every new row under a
filter, so the A/B is over the whole suite and a loss is taken back with the measurement here.

**The proof.** `test/gate/FilterGate.cpp` `ANewRowTakesTheKeyItsFiltersFix`: the key field takes
its filter's value with `PopulateAllFields` false, a non-key field does not, both do when it is
true, and four wider filters -- a range, a set, a wildcard, a negation -- seed nothing. The
negative control was run: with the seeding neutered, 2 of its checks go red.

**Measured.** Chain 99, A/B against chain 98.
