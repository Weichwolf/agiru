Type:     task
Status:   open
Parent:   0055
Area:     gen
Source:   developer/properties/devenv-tooltipml-property.md, developer/properties/devenv-instructionaltextml-property.md, developer/properties/devenv-abouttextml-property.md, developer/properties/devenv-abouttitleml-property.md, developer/properties/devenv-additionalsearchtermsml-property.md, developer/properties/devenv-summaryml-property.md, developer/properties/devenv-entitycaptionml-property.md, developer/properties/devenv-entitysetcaptionml-property.md, developer/properties/devenv-optioncaptionml-property.md, developer/properties/devenv-promotedactioncategoriesml-property.md, developer/properties/devenv-profiledescriptionml-property.md, developer/properties/devenv-requestfilterheadingml-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# The `ML` spelling of every UI string is refused as one family

**Twelve pages, one item, because they are one decision measured twelve times.** Each is the
language-tagged twin of a plain string property -- `ToolTipML` beside `ToolTip`, `SummaryML` beside
`Summary` -- with the same applicability and the same syntax shape, `PROPERTY = ENU='...'; DEU='...'`.
Grouping them is not convenience: refusing eleven and accepting one would be the defect.

`CaptionML` is the thirteenth and has its own item (board:0383) because its plain twin is the
largest population in the sweep and the two have to be argued together.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

| property | declarations | its plain twin |
|---|---:|---:|
| `ToolTipML` | **0** | 159 993 |
| `InstructionalTextML` | **0** | 1 000 |
| `AboutTextML` | **0** | 2 119 |
| `AboutTitleML` | **0** | 1 859 |
| `AdditionalSearchTermsML` | **0** | 665 |
| `SummaryML` | **0** | 855 |
| `EntityCaptionML` | **0** | 287 |
| `EntitySetCaptionML` | **0** | 272 |
| `OptionCaptionML` | **1** | 3 905 |
| `PromotedActionCategoriesML` | measured with the theme | -- |
| `ProfileDescriptionML` | measured with the theme | -- |
| `RequestFilterHeadingML` | measured with the theme | -- |

**Eight of them are zero and the ninth is one.** The spelling is over: BC's translation lives in
`.xlf` files beside the app, and the `ML` properties are what it lived in before.

**A counter reporting 0 over N is an abort and not a pass**, so the zeros were checked rather than
assumed: `Caption` measures 288 491 with the same pattern on the same tree, so the pattern finds
properties.

## The IST-state

None of the twelve is among the nine properties the generator consumes (board:0067).

## The choice

**Refuse all twelve**, with one message naming the property and its plain twin. The refusal is one
list in the generator and not twelve checks, so adding the thirteenth (board:0383) costs a line.

**Multi-language is a separate question.** BC's mechanism is `.xlf`, and when agiru needs translation
it needs that; this item does not pre-empt it and must not be read as deciding it.

## Ordering

With board:0067's census and board:0383. No runtime work.

## Gate, and its negative control

An object declaring `ToolTipML` fails to transpile; the message names `ToolTip`.

**The negative control is the single `OptionCaptionML` declaration** -- the refusal must FIRE on it,
which is one known file. If the whole BaseApp transpiles with the refusal in place, one of the two
counts is wrong and the family was never measured.
