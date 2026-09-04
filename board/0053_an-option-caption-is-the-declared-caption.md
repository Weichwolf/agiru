# 0053 -- An option's caption is its declared caption, not its member name

`FieldRef.OptionCaption()` returns the MEMBER NAMES, comma-separated. AL's `OptionCaption` property
is a separate, translated string beside `OptionMembers`, and BC falls back to the member names only
where a field declares none.

**The reference.** `properties/devenv-optioncaption-property.md` and
`devenv-optionmembers-property.md`. The two are declared apart because they ARE apart: `OptionMembers
= Open,"Bank Acc. Entry Applied",Closed` names the values a body writes, `OptionCaption =
'Open,Bank Acc. Entry Applied,Closed'` names what a user reads, and a translation replaces the
second and never the first.

**What the AL source does.** Under `Layers/W1`, a field that declares `OptionMembers` usually
declares `OptionCaption` beside it, and the two differ wherever a member name carries a dot or an
abbreviation the caption spells out.

**What is here now.** `meta/TableDef.h` gives each `EnumValueDef` a `caption` as well as a `name`,
and `OptionCaption()` reads the NAME. So a test that asserts on a caption gets the member name, and
where they agree -- which is most fields -- nothing shows.

**AND THE CAPTION LIST IS COMMA-JOINED, WHICH IS WHY A COMMA IN A CAPTION IS A DEFECT.**
`devenv-extensible-enums.md` (read 2026-09-04, board:0071):

> When creating captions for enums, it's important that the caption doesn't contain a comma. Having
> a comma in the caption, such as `Caption = 'Diamond Level, with bonus'`, can display over multiple
> lines in the UI. **This behavior also causes that the actual value selected by the user in the UI,
> doesn't correspond to the value, which is saved in the database.**

So `OptionCaption` is not a list of strings, it is ONE string split on commas -- and a caption
carrying a comma shifts every ordinal after it. That is the strongest possible argument for keeping
`EnumValueDef::caption` per VALUE in the metadata, as `meta/TableDef.h` already does, and for
building the comma-joined form only where AL asks for one. A runtime that stored the joined string
would inherit BC's own defect.

The same page settles two more: **the value ID IS the ordinal and must be unique** -- so an enum's
ordinals are sparse by declaration (board:0076) -- and values are displayed in DECLARATION order,
with an extension's values after the base's (board:0033).

**The choice.** Read `value.caption` and fall back to `value.name` when it is empty, which is BC's
own rule. What has to come first is the GENERATOR filling the caption: it emits the members from
`OptionMembers` and the captions from `OptionCaption`, and the second is where a translation would
one day land. A gate belongs on a field whose caption differs from its member name -- `Bank Account
Ledger Entry."Statement Status"` declares `OptionCaption = 'Open,Bank Acc. Entry Applied,Check Entry
Applied,Closed'` against members `Open,"Bank Acc. Entry Applied",...` -- so the difference is
visible rather than assumed.

## A CAPTION MAY NOT CONTAIN A COMMA, AND THE UI ORDER IS DECLARATION ORDER

`devenv-extensible-enums.md` (read 2026-09-04, board:0071) names a hazard that looks cosmetic and is
not:

> When creating captions for enums, it's important that **the caption doesn't contain a comma**.
> Having a comma in the caption, such as `Caption = 'Diamond Level, with bonus'`, can display over
> multiple lines in the UI. This behavior also causes that **the actual value selected by the user in
> the UI doesn't correspond to the value which is saved in the database.**

The reason is this item's own subject: an OptionString is a comma-separated list, and a caption
containing a comma splits it -- so the ordinals shift and a user picking the third entry stores the
second. **It is `static_assert`-able**: the generator already emits the caption table, and a caption
containing `,` is a translation error rather than a mis-saved record. Microsoft's own guard is an
AppSourceCop warning (AS0087) over the `.xlf` files, which is the same check one layer out.

**And the display ORDER is declaration order, across extensions**: "When the enum values are
displayed in the UI, they're sorted by the order of declaration. In addition, **if extension B
extends extension A, the enum values declared in extension A are displayed BEFORE the enum values
declared in extension B**." So the caption list is not sorted by ordinal and not sorted
alphabetically -- it is the merge order board:0033 produces, base first. One more reason extensions
merge in a DECLARED order.
