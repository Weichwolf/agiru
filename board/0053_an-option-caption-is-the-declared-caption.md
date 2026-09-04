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

**The choice.** Read `value.caption` and fall back to `value.name` when it is empty, which is BC's
own rule. What has to come first is the GENERATOR filling the caption: it emits the members from
`OptionMembers` and the captions from `OptionCaption`, and the second is where a translation would
one day land. A gate belongs on a field whose caption differs from its member name -- `Bank Account
Ledger Entry."Statement Status"` declares `OptionCaption = 'Open,Bank Acc. Entry Applied,Check Entry
Applied,Closed'` against members `Open,"Bank Acc. Entry Applied",...` -- so the difference is
visible rather than assumed.
