Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-instructionaltext-property.md
Verdict:  fehlt
Class:    activation

# `InstructionalText` stands at the top of a page or a group

`InstructionalText` is the paragraph a wizard page or a FastTab shows above its controls -- the
sentence that says what the user is being asked for, as against a `ToolTip`, which appears on hover
over one control.

It is a plain declared string like `Caption` and `ToolTip`, and it takes the same representation:
`constexpr` `string_view` on the element's descriptor.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`InstructionalText =` **1 000** · `InstructionalTextML =` **0** (board:0386).

A round thousand, which is what a BaseApp full of assisted-setup wizards looks like.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; no page element carries any string.

## The choice

One more `string_view` on the page-group and page descriptors, with board:0382's captions and
board:0385's tooltips. Three properties, one representation, one array -- which is why they should
land together rather than one per round.

The renderer emits it as a paragraph before the group's controls.

## Ordering

With board:0030's control metadata, board:0382 and board:0385.

## Gate, and its negative control

A group declaring `InstructionalText` renders the paragraph above its controls, in that order.

**The negative control is the order** -- an implementation that appends it after the controls renders
the same words and asks the question after the answer.

## The metadata half landed 2026-09-07 (board:0553)

The property this item is about now reaches `constexpr` metadata -- `ControlDef`/`PageDef` in
`include/meta/PageDef.h`, `FieldDef`/`TableDef` in `meta/TableDef.h`, `CodeunitDef` in
`meta/CodeunitDef.h` -- emitted per object into the `.cpp` and reached through the object's traits.
What is open here is what READS it, not what carries it: the property census over the whole BaseApp
went from 46 203 declarations dropped in silence to 813 in the same round.
