Type:     task
Status:   open
Parent:   0069
Area:     gen
Source:   developer/properties/devenv-obsoletereason-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# An `ObsoleteReason` is the text the diagnostic prints

> Specifies **why** the object has been marked as Pending in the `ObsoleteState` property.
>
> Applies to: 37 element kinds -- table, table field, table key, codeunit, enum type, enum value,
> interface, page and each of its parts, query and its columns, report and its data items, xmlport,
> profile, control add-in, permission set, report layout, page view.

board:0069 owns what `ObsoleteState` does to a reference: a warning for `Pending`, an error for
`Removed`. **This property is the message body of that diagnostic**, and without it the diagnostic is
"this is obsolete" with no way for the reader to find out what to use instead.

That makes it a `[SET]` string that travels from AL into a compiler message, which is CLAUDE.md's
"a diagnostic is a declared label, never a free literal" applied one level up: the label is declared
in the AL SOURCE and the transpiler must carry it rather than compose its own.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`ObsoleteReason =`: **4 538 declarations**, against 4 926 `ObsoleteState`. So **388 obsolete elements
carry no reason** -- 7.9 % of them, and each one is a diagnostic that can say nothing useful.

## The IST-state

The generator consumes nine properties (board:0067's census) and this is not one; the AST holds it at
`src/al/Ast.h:67` like every other.

## The choice

The reason travels to wherever board:0069's diagnostic is emitted, and nowhere else. It is not
runtime data: a message about a REFERENCE is produced when the reference is translated, so the string
never reaches `.rodata` and never costs a byte per session.

**The 388 without a reason are not an error.** AL does not require the property, so the transpiler
must not either -- the diagnostic simply carries less.

## Ordering

With board:0069. It has no meaning before there is a diagnostic to attach it to.

## Gate, and its negative control

A reference to a `Pending` field whose declaration carries a reason produces a warning containing
that reason, verbatim.

**The negative control is a `Pending` field with no reason** -- it must still warn. An implementation
that requires the reason silently drops the warning on 388 elements.
