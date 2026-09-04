Type:     task
Status:   open
Parent:   0190
Area:     gen
Source:   developer/attributes/devenv-caption-attribute.md
Verdict:  fehlt
Class:    activation

# A `[Caption]` attribute names an OData action, and it is the label grammar in attribute form

`[Caption(Text: Text [, Locked: Boolean] [, Comment: Text] [, MaxLength: Integer])]` on a METHOD --
"Specifies a caption for OData actions."

The four arguments are the label grammar (`devenv-using-labels.md`): the text, whether it may be
translated, translator guidance, and a length bound. So this attribute is a `Label` sitting on a
method rather than in a `var` block, and it should be carried the same way board:0055 carries the
others.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**38 `[Caption` declarations on methods** -- distinct from the 340 307 `Caption =` PROPERTY
declarations board:0067 counts, which sit on objects, fields and controls.

## The IST-state

The attribute parses into the raw list and is dropped. Method captions do not reach the generated
tree in any form.

## The choice

`constexpr` metadata beside the method: the text, the locked flag, the comment, the maximum length.
**`MaxLength` becomes a `static_assert`** on the text's own length, exactly as board:0055 specifies
for labels -- the bound and the value are both translation-time constants, so a caption longer than
its declared maximum is a build error and not a truncation nobody sees.

**What it is FOR does not exist here.** The caption labels an OData action, and the OData surface is
phase 3 and unclaimed. So the value is carried and nothing consumes it yet -- which is the honest
state and is why this item sits under board:0190 rather than under a web-services epic that has not
been filed.

## Ordering

Low. 38 sites, no consumer until OData exists. It is here so the epic's counter can reach 41 of 41
and so the `MaxLength` check is not invented twice.

## Gate, and its negative control

A method with `[Caption('Post', MaxLength = 10)]` translates and the caption appears in the
metadata. **The negative control is `MaxLength = 3` on the same text: the build must FAIL** -- if it
compiles, the bound was carried and not checked.
