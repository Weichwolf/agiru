Type:     task
Status:   open
Parent:   0063
Area:     gen, rt
Source:   developer/properties/devenv-includecaption-property.md
Verdict:  fehlt
Class:    activation

# `IncludeCaption` puts a column's caption into the report dataset

> Applies to: **Report Column.** **True** includes the caption in the report dataset; otherwise
> false. **The default value is false.**
>
> If set to true, **a corresponding label called `{column name}Caption` is included in the dataset**.
> The structure of where labels are available depends on the layout type (Excel, Word, or RDLC).
>
> **If you want to use the Caption or CaptionML values of a field as a label on a report layout, then
> you must include the caption in the dataset.** If you do not, then in a multilanguage application,
> you cannot use the multilanguage captions as labels.

**The generated NAME is the mechanism**: `column(No_Item; "No.")` with `IncludeCaption = true`
produces a dataset entry `No_ItemCaption` beside `No_Item`. So the property does not add a value, it
adds a SECOND column whose name is derived -- and a layout refers to that derived name.

That makes it a naming contract between the transpiler and the layout file, and the derivation is
one concatenation that must match BC's exactly. A layout referring to `No_ItemCaption` finds nothing
if the suffix is spelled differently.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`IncludeCaption =`: **2 649 declarations**, all necessarily `true` since `false` is the default.

Against CLAUDE.md's 668 reports in scope, that is roughly four caption columns per report.

## The IST-state

Reports have no generator (board:0063, board:0034), so there is no dataset to add a column to.

## The choice

The generator emits the extra dataset entry, named `<column name>Caption`, whose value is the
underlying field's caption (board:0382) -- **resolved at translation time**, since both the column
name and the field's caption are declarations.

**Not a run-time dataset transformation.** The dataset's shape is known when the report is
translated, and a layout is written against that shape.

## Ordering

Inside board:0063's report generator, with the dataset. Behind board:0382, which is where a field's
caption comes from.

## Gate, and its negative control

A column declaring `IncludeCaption = true` produces a dataset with both `X` and `XCaption`, and
`XCaption` holds the field's declared caption.

**The negative control is the caption's VALUE** -- an implementation that emits the column name into
`XCaption` rather than the field's caption produces a dataset of the right shape with the wrong
words, and a layout renders it without complaint.
