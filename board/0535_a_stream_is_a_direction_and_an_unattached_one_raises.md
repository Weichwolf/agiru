Type:     task
Status:   open
Parent:   0074
Area:     net, rt
Source:   developer/devenv-streams-overview.md, developer/devenv-write-read-methods-line-break-behavior.md, developer/devenv-file-handling-and-text-encoding.md
Verdict:  fehlt
Class:    activation

# A stream is a direction, and an unattached one raises

**Three pages, one item**: what a stream is, how `Write` and `Read` treat line breaks, and how text is
encoded on the way through. board:0074 is "a stream carries its encoding" and these three are its
specification.

## The direction is in the TYPE, not in the method

> "In C#, you only have ONE object called `Stream`. The direction is determined by using the
> Read/Write methods ... **In AL, the direction of the data flow is instead ENCODED IN THE TWO DATA
> TYPES `InStream` and `OutStream`.**"

**Two types, not one with two method sets** -- which is the shape C++ has anyway (`istream` /
`ostream`) and which makes the door's per-type files (board:0051) exactly right: `include/type/InStream.h`
and `include/type/OutStream.h`, with no operation that could write to an `InStream`.

**That is a case where AL and C++ agree and the predecessor's language did not**, and it is worth
recording as one of the places the port gets stricter for free.

## An unattached stream raises

> **"An instance of the `InStream` datatype MUST BE ATTACHED TO A DATA SOURCE to work, otherwise you
> get a runtime error stating *InStream variable not initialized*."**
>
> ```al
> var vInStr: InStream;
> Message(Format(vInStr.Length()));   // "this will trigger a runtime error"
> ```

**A declared-but-unattached stream is a runtime error with a specific message**, which board:0055's
wording owns. And it means the stream type is not default-constructible into a usable state -- a
`std::optional`-shaped invariant, checked on every operation.

## `CopyStream`'s parameter order is a historical accident, documented as one

> `System.CopyStream(OutStream, InStream [, BytesToRead])`
>
> **"The `CopyStream` method stems from the time of the C/AL programming language, which was inspired
> from Pascal. In Pascal it's common for procedures to follow the direction of assignments, for
> example `variable := value` (like `dest := source`). THIS IS THE REASON why parameters in
> `CopyStream` are ordered the way they are."**

**Destination first.** CLAUDE.md's rule is that AL names are law and the deviation must be visible
rather than clever -- so `CopyStream(out, in)` keeps AL's order even though every C++ reader expects
`(in, out)`, and the door's Doxygen carries this paragraph as the `\warning`.

**That is exactly the kind of thing a helpful implementation "fixes"** and thereby breaks every
transpiled call site silently -- the arguments are both streams, so swapping them compiles.

## Seeking is not universal

> "**Seek capability DEPENDS ON THE KIND OF BACKING STORE.** For example, **network streams have no
> unified concept of a current position, and therefore typically DON'T SUPPORT SEEKING.**"

So `InStream` has seek methods that are valid for some sources and not others -- a run-time capability,
not a type distinction. board:0017's BLOB stream seeks; an HTTP response stream does not.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`InStream`, `OutStream` and `CopyStream` are types and method calls; board:0028 owns the census.
**Stated rather than guessed** -- and CLAUDE.md already names the stakes: "`DotNet` and streams block
reports, imports and e-documents."

## The IST-state

board:0074 records the state. `include/type/` holds the AL types (board:0051); board:0448 measured the
encoding side -- `TextEncoding` defaults to **MSDOS**, which is where a stream's encoding first bites.

## The choice

Two types with no cross-assignment, each holding an attached source or nothing, raising board:0055's
declared message when used unattached. `CopyStream` keeps AL's argument order with the door's
`\warning` carrying this page's paragraph.

Seek is a run-time capability query on the source, not a second type.

**The encoding is the source's, not the stream's** -- board:0448's `TextEncoding` and `Encoding` are
XMLport properties that select it, and board:0074 owns the default.

## Ordering

board:0074's core. Behind board:0051's per-type door. Ahead of board:0063's reports and board:0065's
XMLports, which CLAUDE.md says streams block.

## Gate, and its negative control

An unattached `InStream` raises on `Length()`; `CopyStream(out, in)` copies from the second argument to
the first; a BLOB stream seeks and an HTTP stream refuses to.

**The negative control is `CopyStream`'s order** -- both arguments are streams, so a swapped
implementation compiles and copies the wrong way. The gate must assert the DESTINATION's content, not
that the call succeeded.
