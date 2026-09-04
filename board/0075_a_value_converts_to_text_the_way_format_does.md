Type: root
State: open
Area: net

# `ToText` is `Format(value, 0, 0)`, its invariant form is `Format(value, 0, 9)`, and eleven types owe both

Every AL value type documents the same pair, and the definition is the same sentence on each page:
**"Converts the value to a text. Equivalent to calling `Format(value, 0, 0)`"**, with an optional
`Invariant` argument meaning **`Format(value, 0, 9)`** instead.

Almost none of it exists, and where a name is missing the door is usually RIGHT to have withheld it.

## What is here, read 2026-09-04

| type | `ToText()` | `ToText([Invariant])` | in the door |
|---|---|---|---|
| Boolean | documented | documented | free `ToText(Boolean)` -> `"Yes"`/`"No"`; **no invariant form** |
| BigInteger | documented | -- | free `ToText(BigInteger)`, plain digits, correct for Integer format 0 |
| Byte | documented | documented | **absent** |
| Decimal | documented | documented | withheld on purpose -- `ToInvariantString` is named for what it is |
| Date, DateTime, Duration | documented | documented | withheld on purpose, each with the reason in the header |
| Integer, Char, Time, Guid | read as the sweep reaches them (board:0071) | | |

**The withholding is the right call and is written down where it was made.** `include/type/Date.h`
and `include/type/Duration.h` both say the name is not used *because* `ToText()` is defined as
`Format(value, 0, 0)` and format 0 is locale-dependent, which the value type cannot answer. That is
board:0007's argument applied consistently, and it means this item is blocked on board:0066 rather
than on eleven small pieces of work.

## THE BOOLEAN VALUE: TWO PLATFORM PAGES DISAGREE, AND THE SOURCE BREAKS THE TIE

This item was first filed claiming `"Yes"`/`"No"` was a defect. **It is retracted**, and the
reasoning is kept because the next reader will find the same two pages.

| source | says |
|---|---|
| `devenv-format-property.md`, "Standard boolean formats" | `<Text>` format 0 -> **True/False**; format 2 -> 1/0; format 9 -> **true/false** |
| `methods-auto/fieldtype/fieldtype-option.md` | "When formatted, a Boolean field is shown as **'Yes' or 'No'**" |

CLAUDE.md's rule is that where the documentation DESCRIBES and the source DECLARES, the source
declares. The source declares, in `System Application/App/Language/src/LanguageImpl.Codeunit.al:222`:

```al
ValueVariant.IsBoolean:
    begin
        DummyBoolean := ValueVariant;
        if DummyBoolean then Result := 'Yes' else Result := 'No';
    end;
else begin
    CurrentLanguage := GlobalLanguage();
    GlobalLanguage(GetDefaultApplicationLanguageId());
    Result := Format(ValueVariant);          // every other type goes through Format
    GlobalLanguage(CurrentLanguage);
end;
```

`ToDefaultLanguage` sends every other type through `Format` under the default language and
**special-cases the Boolean to `'Yes'`/`'No'`**, with a comment saying why: `Format` on a boolean
follows the CURRENT language and a report running in a local one would get the local words. Its own
test asserts the result: `Assert.AreEqual('Yes', Language.ToDefaultLanguage(DummyBoolean),
'Expected the boolean to be formatted in English.')`
(`System Application/Test/Language/src/LanguageTest.Codeunit.al:354`).

So `Format(Boolean)` in English is `Yes`/`No`, the format-property table's `True/False` row is the
outlier, and `src/net/Numeric.cpp:17` is right. **What the table does settle is the OTHER two
formats**, and those are missing: format 2 is `1`/`0` and format 9 is `true`/`false` -- the second is
what `Format(BooleanValue, 0, 9)` produces for XML, which
`Apps/W1/INTaxEngine/.../ScriptDataTypeMgmt.Codeunit.al:733` relies on.

## The choice

- **`ToText` is one call into `Format` and never a second formatter**, so the two cannot disagree.
  board:0066 owns `Format`; this item is the rule that `ToText` chooses format 0 or format 9 and
  does nothing else.
- **A type whose `ToText` cannot be answered without a locale keeps the name WITHHELD** until
  board:0066 lands, and the header says so. That is the state of Date, DateTime, Duration and
  Decimal today and it is correct: a `ToText` that quietly returned the invariant form would be the
  plausible wrong answer this tree refuses.
- **The `[Invariant]` overload arrives with board:0072's mechanism**, not as eleven hand-written
  overloads.
- **The call shape is the generator's** (board:0051): AL writes `B.ToText()` on a value whose C++
  spelling is `bool`, which cannot carry a member.

## Gate

Every documented `ToText` against the documentation's own tables, both forms. For Boolean: format 0
`Yes`, format 2 `1`, format 9 `true` -- three different answers from one value, which is what makes
this testable at all.

**Negative control**: make the invariant form return the format-0 text and require the XML case to
go red.
