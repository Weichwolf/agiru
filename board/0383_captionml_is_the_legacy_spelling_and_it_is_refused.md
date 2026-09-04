Type:     task
Status:   open
Parent:   0055
Area:     gen
Source:   developer/properties/devenv-captionml-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# `CaptionML` is the legacy spelling, and three declarations are all that remain

> Sets the text string that displays with the object, control, or other element in the user interface
> **for the current language**. Applies to: the same fourteen kinds as `Caption`.

The `ML` form carries a language-tagged list -- `CaptionML = ENU='Customer'; DEU='Debitor'` -- and it
is what BC used before translation moved into `.xlf` files beside the app. It is not deprecated on the
page, and the population says it is over anyway.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`CaptionML =` **3**, against `Caption =` **288 491**.

**Three declarations in 2.56 million lines.** And the same collapse holds across the whole `ML`
family measured together: `ToolTipML` **0**, `InstructionalTextML` **0**, `AboutTextML` **0**,
`AboutTitleML` **0**, `AdditionalSearchTermsML` **0**, `SummaryML` **0**, `EntityCaptionML` **0**,
`EntitySetCaptionML` **0**, `OptionCaptionML` **1**, `PromotedActionCategoriesML`, `SummaryML`,
`ProfileDescriptionML`, `RequestFilterHeadingML` -- the whole spelling is dead in this tree.

## The IST-state

Not among the nine properties the generator consumes (board:0067).

## The choice

**Refuse it**, and record the measurement as the reason. Three declarations do not justify a
language-tagged caption path beside the 288 491-declaration one, and a second caption source that
silently won or lost against `Caption` would be a caption defect nobody could locate.

**Not the alternative** -- parsing the language list and taking the ENU value. That builds half a
translation mechanism for three call sites and leaves the question of what happens when both
properties are declared.

**Multi-language is a separate question and not this one.** BC's answer is `.xlf` translation files,
not this property; when agiru needs translation it needs that, and this item does not pre-empt it.

## Ordering

With board:0067's census. No runtime work.

## Gate, and its negative control

An object declaring `CaptionML` fails to transpile with a message naming the property and pointing at
`Caption`.

**The negative control is the three declarations** -- the refusal must FIRE on them, which means the
transpiler stops on three known files. If it does not, the property is spelled differently than
measured and the count is wrong.
