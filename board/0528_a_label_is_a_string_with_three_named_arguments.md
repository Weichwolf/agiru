Type:     task
Status:   open
Parent:   0055
Area:     al, gen
Source:   developer/devenv-using-labels.md
Verdict:  teilweise
Class:    silent-wrong-data

# A label is a string with three named arguments, in four places

board:0389 recorded from the `AdditionalSearchTerms` page that a string property carries named
arguments and that a parser reading to the semicolon takes them as content. **This page is that rule's
home**, and it lists where it applies.

> Labels have "a text constant followed by **three optional parameters. They must be COMMA-SEPARATED,
> but THE ORDER OF THE PARAMETERS ISN'T ENFORCED.**"
>
> | parameter | type | |
> |---|---|---|
> | `Comment` | Text | general comments, **specifically about the placeholders** |
> | `Locked` | Boolean | **"the label shouldn't be translated"**, default `false` |
> | `MaxLength` | Integer | **"if no maximum length is specified, the string can be ANY LENGTH"** |

**Order is not enforced**, so a parser cannot read them positionally, and all three may be absent.

## Four places a label appears

> 1. **the property value** of seven named properties;
> 2. the **`Label` data type** variable;
> 3. a **report** label;
> 4. a **page** label.

The seven properties: **`Caption`** (board:0382, **288 491**), **`ToolTip`** (board:0385, **159 993**),
`OptionCaption` (board:0053), `AdditionalSearchTerms` (board:0389), `InstructionalText` (board:0387),
`PromotedActionCategories` (board:0477), `RequestFilterHeading` (board:0454).

**So the label grammar governs the two largest populations in the whole sweep.** A parser that mistook
`, Comment = '...'` for part of a caption would corrupt 288 491 captions -- invisibly, because the
caption would merely be longer.

```AL
Caption = 'Developer translation for %1', Comment = '%1 is extension name', locked = false, MaxLength=999;

var a: Label 'Label Text', Comment='...', MaxLength=999, Locked=true;
```

**Note the casing**: `locked` lowercase in one example and `Locked` in the other, on one page. AL is
case-insensitive and board:0349 is the bug that came from forgetting it -- so the argument names are
matched case-insensitively, and this page is the second citation for that rule.

## The `Label` data type is a variable, not a string

> "The `Label` data type denotes a string variable used to define **error messages, questions,
> captions, tokens, or other text constants displayed to the user.**"

**That makes `Label` the type CLAUDE.md's diagnostic rule is about**: "a diagnostic is a DECLARED
LABEL, never a free literal -- AL error texts are intended behaviour and tests compare them."

board:0055 owns the wording; this page names the mechanism that carries it, and it is a distinct AL
type with a door file of its own (board:0051).

**`MaxLength` on a label is checkable at translation time** -- the literal's length against the declared
maximum -- and it is one `static_assert` per label.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0382: `Caption` **288 491**. board:0385: `ToolTip` **159 993**. **The `Label` variable declaration
count is not a property and this sweep's pattern does not reach it** -- stated rather than guessed, and
it is the number that sizes the diagnostic surface board:0055 needs.

## The IST-state, and it is why this is `teilweise`

`src/al/Ast.h:24` -- `struct LabelDecl { std::string name; std::string text; }`.
`src/gen/CodeunitWriter.cpp:727` -- the generator looks up `unit_.labels` by lowered name. **So labels
are parsed and carried, with a name and a text.**

**`Comment`, `Locked` and `MaxLength` are not in `LabelDecl`** -- so the parser either drops them or
folds them into `text`, and which is this item's first check. If they are in `text`, every label
carrying arguments has a corrupted value.

## The choice

`LabelDecl` gains the three optional arguments, parsed by name, case-insensitively, in any order.
`MaxLength` becomes a `static_assert`; `Locked` and `Comment` are translation metadata and are dropped
by the generator -- board:0389 takes the same decision from the property side.

**One grammar, seven properties and four places**, which is what board:0389 asked for.

## Ordering

Ahead of board:0382 and board:0385, whose 448 484 declarations it parses. With board:0055's diagnostic
labels.

## Gate, and its negative control

`Caption = 'x %1', Comment = 'y', Locked = true, MaxLength = 4` yields the caption `x %1` and nothing
else; the same arguments in a different order yield the same; `MaxLength = 3` on that caption fails to
transpile.

**The negative control is the caption's VALUE** -- an implementation that keeps the arguments renders
`x %1, Comment = 'y', ...`, which is visibly wrong on a page and invisible to a test that only checks
the caption is non-empty.
