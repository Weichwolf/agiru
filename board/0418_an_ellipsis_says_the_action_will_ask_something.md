Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-ellipsis-property.md
Verdict:  fehlt
Class:    activation

# An ellipsis says the action will ask something

> Sets a value that specifies whether an ellipsis (`...`) is appended to the caption on a command
> button or menu item. **An ellipsis tells the user that other choices will appear if the command
> button or menu item are selected.** **The default is false.**
>
> Applies to: **Page Action, Page Custom Action.**

**The caption itself is unchanged** and the ellipsis is appended at render time -- which is the whole
implementation and also the reason it is not free: `FieldCaption`-style code that reads the action's
caption must get the declared string without the dots, and only the rendered label carries them.

That is board:0395's `ShowCaption` distinction again from the other side: one property hides the
label without touching the caption, this one decorates the label without touching the caption.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Ellipsis =`: **4 003 declarations**, all necessarily `true` since `false` is the default.

Four thousand actions in the BaseApp open a dialog, which is a useful number on its own: it is a lower
bound on how many actions board:0054's handler mechanism has to be able to stand in for.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone.

## The choice

One bit on the action descriptor; the renderer appends `...` to the rendered label and to nothing
else.

**Appended, not stored.** Baking the dots into the caption at translation time would put them into
every place the caption is read, including an error message that quotes the action.

## Ordering

With board:0030's action metadata and board:0382's captions.

## Gate, and its negative control

An action declaring `Ellipsis = true` renders `Post...` and its caption reads `Post`.

**The negative control is the caption** -- an implementation that appends at translation time renders
identically and returns `Post...` wherever the caption is read.
