Type:     task
Status:   open
Parent:   0054
Area:     rt
Source:   developer/attributes/devenv-strmenuhandler-attribute.md
Verdict:  fehlt
Class:    activation

# A `[StrMenuHandler]` picks an option and hands its NUMBER back

```al
[StrMenuHandler]
procedure H(Options: Text[1024]; var Choice: Integer; Instruction: Text[1024])
```

`StrMenu(Options, Default, Instruction)` hands a comma-separated option string and an instruction,
and reads the CHOICE back through `var Choice`. The choice is ONE-BASED, and **0 means the user
cancelled** -- which is a legal answer the code under test must handle, not an error.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**412 `[StrMenuHandler` declarations.** Over the milestone's 78 UT codeunits: 3 declarations in 3
codeunits -- the smallest of the four text handlers.

## The IST-state

`StrMenu` is a door refusal; the attribute parses and is dropped.

## The choice

The same table entry as 0194, kind `StrMenu`, no object id. The option string is passed through
verbatim -- **it is not split**, because the comma-separated form is what AL builds and what the
handler expects to parse, and splitting it here would lose an option whose caption contains a
comma (board:0053 names the same hazard for `OptionCaption`).

## Ordering

Needs 0199's table. Needs no page runtime.

## Gate, and its negative control

A `StrMenu` with three options whose handler writes 2: the code under test must take the second
branch. A handler writing 0 must make the caller take its cancel path.

**The negative control is the 0 case** -- a runtime that treats 0 as "no answer" and raises passes
the first case and breaks every cancel path in the BaseApp.
