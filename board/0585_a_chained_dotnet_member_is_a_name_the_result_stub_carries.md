Type:     task
Status:   open
Parent:   0035
Area:     gen
Verdict:  fehlt
Class:    compile root

# A chained .NET member is a name the result stub carries, gathered from the tree

`WorksheetWriter.Worksheet().WorksheetPart().WorksheetCommentsPart()` in `OpenXMLManagement`:
the first link is a member of the absent type's generated stub, the second is a member of
`dotnet::RefusedResult`, and `RefusedResult` carries some 28 names written by hand. The third link
here is not among them and the codeunit does not compile -- `OpenXMLManagementUT` is the UT
codeunit behind it (loop19, 2026-09-05).

## The choice

The list stops being hand-written. `BodyWriter` records every member name it emits on a
`RefusedResult` receiver into `DotNetUse` beside the typed members it already gathers, and
`Main.cpp` emits `struct RefusedResult` into `absent/Types.h` with the union of those names --
each an `operator()` returning `RefusedResult`, the way the hand-written ones do. The hand-written
struct in `include/dotnet/Refused.h` keeps only the non-name surface (`ToText`, `Max`, iteration)
and derives from nothing the tree has to remember.

## Gate

A chain three members deep on an absent .NET type compiles; the emitted struct names exactly the
members the tree uses, so a name nobody chains is not there (the negative control).
