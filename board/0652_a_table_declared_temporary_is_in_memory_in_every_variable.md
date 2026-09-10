# A table declared `TableType = Temporary` is in memory in every variable

**Finding (2026-09-10).** `Duplicate Price Lines.Set` does `Rec.Copy(DuplicatePriceLine, true)` on
two plain record variables and was refused with "neither record is temporary" (8 UT cases of
`Suggest Price Lines UT`); `Price Asset` inserts collided with "already exists" across tests (11 of
`Asset List UT`). Both tables declare `TableType = Temporary`, which the generator wrote into the
`TableDef` and nothing read. 99 BaseApp tables declare it -- `VAT Amount Line`,
`Invoice Post. Buffer`, `Dimension Set Entry Buffer`, `Item Tracking Setup` among them -- and all of
them were database tables here, so a posting buffer's rows went through PostgreSQL and survived
into the next test.

**Reference.** `devenv-tabletype-property.md`: `Temporary` "specifies the table as an in-memory
table used to store temporary data". The predecessor never modelled the property (no board item;
its buffers were dictionaries and so temporary by accident).

**Choice.** The generated class of such a table constructs its own store the way `Temporary<T>`
does -- the constructor the generator already emits for `InitValue` also calls
`RuntimeMakeTemporary` when the declaration says so (`CarriesConstructor` in `TableWriter.cpp`,
gate case `ATableDeclaredTemporaryConstructsItsStore`). The declaration decides and never the
variable. Activation: buffers that were database tables become in-memory ones, so the milestone
is the measurement and a loss names the deeper root.

**Open.** The schema still creates the 99 tables in PostgreSQL; nothing writes to them any more.
`make schema` and `CreateTables` should skip a `TableType = Temporary` definition, which is a
report on how much of CRONUS the schema holds and not a correctness question.
