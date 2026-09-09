Type:     task
Status:   open
Parent:   0634
Area:     gen, build
Source:   scripts/unlinked.py (chain 48, 2026-09-09); include/runtime/Implementation.h
Verdict:  measured
Class:    activation

# An interface implementer's vtable lives in its definition unit

`SFTP Client Implementation` joined the slice and `Dotnet SFTP Client`, the codeunit it binds
to its interface, did not compile -- so the slice needed `vtable for DotnetSFTPClient_Codeunit`
and nothing defined it: the vtable is emitted where the key function is, and that is the body.
The linker guard refused, which is right (a missing vtable is a crash, not a refusal).

237 generated codeunits implement an interface (counted 2026-09-09), and each is this case the
day its body stops compiling while a caller's does.

## The choice

A codeunit that implements an interface gets a definition unit like a table's: the header
declares the virtual destructor first and out of line, `<Codeunit>.def.cpp` defines it and the
`CodeunitDef` with the registration. The vtable then exists whenever the header compiles, and
every virtual member it points at is an undefined FUNCTION symbol -- which `unlinked.py` stands
in for with a refusal naming the procedure. The alternative, all virtuals inline, would put the
bodies in the header against the rule that the source carries every body.

## Measured

Chain 48 stopped on one such symbol; the immediate case compiles once
`Implementation<I>` takes a codeunit by value (same round). This item is the general guard.
