Type:     task
Status:   open
Parent:   0055
Area:     al, gen
Source:   developer/devenv-work-with-translation-files.md, developer/devenv-translations-overview.md, developer/methods-auto/label/label-data-type.md
Verdict:  teilweise
Class:    silent-wrong-data

# A label carries three attributes, and the parser keeps none

```AL
Caption = 'Developer translation for %1', Comment = '%1 is extension name', Locked = false, MaxLength = 999;
labels { LabelName = 'Label Text', Comment = 'Foo', MaxLength = 999, Locked = true; }
var a : Label 'Label Text', Comment = 'Foo', MaxLength = 999, Locked = true;
```

**Three optional attributes, in any order** -- *"the `comment`, `locked`, and `maxLength` attributes
are optional and the ORDER ISN'T ENFORCED"* -- on all three label-bearing forms.

**`src/al/Ast.h:24` keeps the name and the text:**

```cpp
struct LabelDecl { std::string name; std::string text; };
```

## What each attribute means, and only one of them is a value

| attribute | meaning |
|---|---|
| `Comment` | a note to the translator; never rendered |
| `Locked` | **"when Locked is set to `true`, the label SHOULD NOT BE TRANSLATED."** Default `false` |
| `MaxLength` | **"determines HOW MUCH OF THE LABEL IS USED. If no maximum length is specified, the string can be any length."** |

`Comment` is metadata for a human and dropping it costs nothing at run time. **`Locked` and
`MaxLength` are not.**

## The MaxLength ambiguity, and the measurement that makes it free

**Two readings and the documentation supports both.**

(a) `label-data-type.md` says *"determines how much of the label is USED"*, which reads as
TRUNCATION -- a label with `MaxLength = 10` over 30 characters of text yields ten.

(b) `devenv-work-with-translation-files.md` shows the XLIFF the compiler generates, and the attribute
lands there as `maxWidth="999" size-unit="char"` on the `<trans-unit>` -- a CONSTRAINT ON THE
TRANSLATION, addressed to whoever writes the target string, not to the runtime.

**Measured 2026-09-04 over `~/Git/BCApps/src`: 42 101 labels carry a `MaxLength`, and the text of
ZERO of them exceeds it.**

Counted by matching, on each line declaring a `Label`, the quoted source string and the attribute
tail, unescaping `''`, and comparing lengths. The `grep` count for the attribute is 42 282, so 181
declarations the line pattern does not reach -- a label wrapped over two lines, or a `MaxLength` on a
report-label block rather than a `Label` -- and that gap is stated rather than closed.

**Zero over 42 101 does not settle which reading is right, and that is the point.** It is consistent
with (a) -- BC keeps every text under its limit, so nothing ever truncates -- and equally with (b).
**What it does settle is that the two implementations are INDISTINGUISHABLE over the entire BaseApp**:
truncating and not truncating produce the same string 42 101 times out of 42 101.

**So the decision is deferred with a number instead of guessed**, and reading (b) is taken meanwhile,
because it is the one the compiler's own output demonstrates and the one that does nothing at run
time. If (a) is right, no BaseApp label changes; if (b) is right and (a) had been implemented, no
BaseApp label changes either. **The only thing that could tell them apart is a label somebody writes
later that violates its own limit**, and that is what the gate below is for.

## `Locked` is the one with a consequence

**22 202 `Locked` attributes: 22 196 `true`, SIX `false`.** So `Locked` is written to say "do not
translate" and essentially never to say the default out loud.

A locked label must come back byte for byte in every language. **Dropping the attribute means a
translation layer would translate 22 196 strings that must not be translated** -- among them the ones
that are locked precisely because they are format strings, codes or protocol tokens.

**It costs nothing TODAY** and that is worth saying plainly: agiru has no translation layer, so every
label is already its source text and locked and unlocked behave identically. **It becomes a defect the
moment `GlobalLanguage` does anything**, which is why this is filed now with the number rather than
discovered then.

## The ML properties are the OLD mechanism, and that closes an open question

board:0561 recorded that `CaptionML` is declared **3 times** against `Caption` at **288 491**, and
that the structural-group rule in `devenv-arranging-fields-on-fasttab.md` is written against
`CaptionML` -- concluding that "the documentation names a property the source has abandoned" and
leaving why unresolved.

**This page says why, in an `IMPORTANT` box.** The `ML` versions of eight properties are **NOT
INCLUDED IN THE XLIFF FILE**:

`AboutTitleML`, `AboutTextML`, **`CaptionML`**, `InstructionalTextML`, `OptionCaptionML`,
`PromotedActionCategoriesML`, `RequestFilterHeadingML`, `ToolTipML` -- **and the `TextConst` data type
is not either.**

So `CaptionML` carries its translations INLINE, the XLIFF route carries them in a file, and an app
using XLIFF cannot use the `ML` form for anything it wants translated. **The source abandoned
`CaptionML` because it adopted XLIFF**, and `TextConst` went with it: **10 declarations in 2.56
million lines**, against 212 109 `Label`s.

**That is a fact worth carrying into `Type` decisions**: `TextConst` is the predecessor of `Label`,
it is excluded from the modern translation route, and it has three orders of magnitude fewer call
sites. It is a compatibility surface, not a mechanism.

## The caption default, and a number for board:0382

`GenerateCaptions` in `app.json` *"generates captions based on the OBJECT NAME for pages, tables,
reports, XMLports, and request pages. **If the object already has a `Caption` property set, that value
is used.** For the table fields, the `OptionCaption` is used."*

**That is exactly what `src/gen/TableWriter.cpp:37` does for a FIELD** -- `Find(field.properties,
"Caption")` with a fall-back to the field name -- **and exactly what `TableWriter.cpp:551` does NOT do
for the TABLE**, which writes `.caption = <table>::kName` unconditionally.

board:0382 already owns that defect and names the line. **What this pass adds is its size: 3 765
tables declare a `Caption`, across 4 564 `.Table.al` files** -- so roughly four table captions in five
are declared and discarded.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

| | count |
|---|---:|
| `: Label '` declarations | **212 109** |
| `, MaxLength =` | 42 282 |
| `, Comment =` | 25 982 |
| `, Locked =` | 22 202 -- **`true` 22 196, `false` 6** |
| table `Caption =` | 3 765, over 4 564 `.Table.al` files |
| `: TextConst ` | **10** |

## The choice

**`LabelDecl` gains the three attributes, and each is `constexpr` where it lands.**

```cpp
struct LabelDecl {
  std::string name;
  std::string text;
  std::string comment;      // parsed, emitted nowhere
  bool locked = false;
  int maxLength = 0;        // 0 == unspecified
};
```

**Why parse `Comment` at all, when nothing renders it:** because the alternative is a parser that
accepts an attribute list and silently discards part of it, which is the failure this item is about.
Parsing all three and emitting one is a decision; parsing one and skipping two is an accident. It also
costs nothing -- the tokens are already being consumed to find the semicolon.

**`locked` reaches the emitted `constexpr` label metadata**, so a future translation layer has the bit
it needs and does not have to re-derive it from the AL.

**`maxLength` reaches it too and is NOT applied**, per the reading above, with the reason recorded in
this item rather than in a comment -- `src/` carries none.

**The order is not enforced**, so the parser reads attributes by NAME in a loop until the semicolon.
An unknown attribute name must ABORT rather than be skipped: CLAUDE.md's rule, and the reason the
current state is a finding.

## Ordering

**Inside board:0055's caption and label work.** `Locked` first -- it is the one with a consequence and
it is 22 202 declarations. `MaxLength` second, as carried metadata. `Comment` last, as a parse that
emits nothing.

**Before any translation layer**, which is what makes `Locked` matter; and board:0382 owns the table
caption independently.

## Gate, and its negative control

1. `Label 'Text', Comment = 'c', MaxLength = 5, Locked = true` parses, and all three attributes reach
   the emitted metadata
2. the same three in a DIFFERENT order parse to the same thing
3. `Label 'Text', Bogus = 1` **fails to transpile** rather than being skipped
4. a label whose text is longer than its `MaxLength` is emitted WHOLE -- reading (b)
5. `Locked` defaults to `false` when the attribute is absent

**The negative control is case 3.** Skip an unrecognised attribute -- which is what the parser does
today for all three real ones -- and cases 1, 2, 4 and 5 all stay green. It is the case that proves
the loop is reading names rather than counting commas.

**Case 4 is a gate the BaseApp cannot supply**, and the item says so: 42 101 real labels all satisfy
their limits, so the case has to be synthetic. **A gate written only from BaseApp labels would be
green under both readings and prove nothing** -- which is the same trap as a green negative control,
reached from the data side instead of the code side.

## Class

`silent-wrong-data`, conditionally and honestly: nothing is wrong today, because there is no
translation layer and every label is its own source text. The classification is for what happens when
one arrives -- 22 196 strings translated that must not be -- and the item exists so the attribute is
already carried when that day comes rather than being rediscovered from 22 196 wrong strings.
