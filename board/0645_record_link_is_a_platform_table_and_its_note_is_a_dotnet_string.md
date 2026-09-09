# Record Link is a platform table and its note is a .NET string

**Finding (2026-09-09).** `RecordLinkImpl.CopyLinks` was not in the slice (85 UT cases, on every
posting path through `Sales-Post`/`Purch.-Post` copying links to the posted document) because the
source names `BinaryReader.BaseStream().Position()` on an absent .NET type, and the table it
walks, `Record Link` (2000000068), is a platform table this runtime did not declare -- generated
as `absent::RecordLink`. `NavTestExecution.IsInTestMode` blocked 53 more through
`EnvironmentInformation`.

**Reference.** `Record Link` is a system table (`devenv-table-object.md`, system tables); its
note is written by `System.IO.BinaryWriter.Write(string)` -- a 7-bit length prefix and UTF-8 --
and read by `BinaryReader.ReadString`, so a note this runtime writes is one BC reads.
`NavTestExecution.IsInTestMode` is true while a test runs, the same span `GuiAllowed` answers.

**Choice.** `include/platform/RecordLink.h` (the field numbers are the platform's, `URL2..4`
obsolete but kept), registered in the catalogue and the generator's platform index;
`include/dotnet/BinaryReader.h`, `BinaryWriter.h` over the AL streams' new raw byte access, with
the constructor spelled as a class-named member the way every absent stub already spells it, so
the generated `X := X.X(stream)` needs no generator rule; `include/dotnet/NavTestExecution.h`.
