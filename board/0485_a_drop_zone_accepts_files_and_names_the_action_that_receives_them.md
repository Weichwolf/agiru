Type:     task
Status:   open
Parent:   0030
Area:     gen, rt
Source:   developer/properties/devenv-allowedfileextensions-property.md, developer/properties/devenv-allowmultiplefiles-property.md, developer/properties/devenv-fileuploadaction-property.md, developer/properties/devenv-fileuploadrowaction-property.md
Verdict:  fehlt
Class:    activation

# A drop zone accepts files and names the action that receives them

**Four pages, one item**: the drop zone's two constraints and the two properties that bind a control
to the action receiving the upload. Every one of them names `fileUploadAction`, and none works alone.

> **AllowedFileExtensions** (runtime 13.0, Page File Upload Action): **"which file types the user can
> drag to a drop zone. The property is a comma-separated list."**
> `AllowedFileExtensions = '.jpg','.jpeg','.png';`
>
> **AllowMultipleFiles** (runtime 13.0): whether the action accepts multiple files.
>
> **FileUploadAction** (runtime 14.0, Page Field, Page Group, Page Part): **"the `fileUploadAction`
> page action to be invoked when a file is uploaded."** A string naming the action.
>
> **FileUploadRowAction** (runtime 14.0, Page Group): the same, **for an upload onto a ROW.**

**Two upload targets and they are different interactions**: a control-level drop zone, and a row-level
one on a repeater. The row version hands the action the row it was dropped on, exactly as board:0362's
repeater-scoped action does.

**The extension list is a client-side filter and not a check.** A browser's file input accepts an
`accept` attribute, which is advisory -- so the SERVER must verify the extension too, and an
implementation that only emits the attribute accepts anything a scripted upload sends. That is a
boundary rule (CLAUDE.md: defensive at system boundaries) and it belongs in this item because the
documentation does not say it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`AllowMultipleFiles =` **12** · `AllowedFileExtensions =` **5** · `FileUploadAction =` **0** ·
`FileUploadRowAction =` **0**.

**Both binding properties measure zero**, so the BaseApp declares drop-zone constraints (17
declarations) and never binds a control to one -- which means uploads today go through the action's
own placement, and the two binding properties are runtime-14 additions nobody has adopted yet.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; there is no upload path and board:0031 records
that a media object has nowhere to live.

## The choice

A `constexpr` extension list and a bit on the file-upload action, an action reference on the control,
and **the extension check on the server side of the upload** -- the client attribute is emitted as a
convenience.

`FileUploadAction` and `FileUploadRowAction` are carried on their zero rather than refused: they are
the current syntax for a feature that exists, and refusing them would refuse the way new code binds an
upload.

## Ordering

Behind board:0031, which is where an uploaded file lands, and board:0030's action metadata.

## Gate, and its negative control

Dropping a `.png` on a zone declaring `'.jpg','.png'` invokes the action; dropping a `.exe` does not,
and neither does a scripted POST of a `.exe`.

**The negative control is the scripted POST** -- an implementation that only emits the client
attribute passes the drag gate and accepts the file, which is the whole reason the check is on the
server.
