# 0684 Two platform tables the BaseApp writes

**The finding.** `Record "Object Options"` (51 AL declarations) and `Record "OData Edm Type"` are
PLATFORM tables: no `.al` file declares them, so the transpiler classified both variables as .NET
types nothing rebuilds and every member refused --
`the .NET member ObjectOptions.SetFilter is named by AL and not rebuilt here`. 14 UT cases die
that way.

**The choice.** Both become platform tables beside `Record Link`, `Page Metadata` and the rest:
`include/platform/ObjectOptions.h` (2000000225, `Parameter Name`, `Company Name`, `Object Type`,
`Object ID`, `User Name`, `Public Visible`, `Option Data`, `Created By`) and
`include/platform/ODataEdmType.h` (2000000203, `Key`, `Description`, `Edm Xml`). Both are
WRITTEN by the BaseApp -- `Insert`, `Modify`, `DeleteAll`, `Validate` -- which is what `Record
Link` already showed a platform table may be, so nothing new is needed in the storage layer.

**The shape is read from the AL, because no `.al` declares it.** The field names come from the
BaseApp's own uses (`ObjectOptions."Public Visible"`, `ODataEdmType."Edm Xml"`) and the pages over
them (`OData EDM Definition Card` names `Rec.Key` and `Rec.Description`); the `Object Type` option
is `Report,Page`, the order the platform's own filters use. The numbers are the platform's.

**Four registration points, and each one is load-bearing:** the catalogue
(`RegisterTable<platform::X>`), the generator's table index (`PlatformTables()`, by name AND by
number), the option's spelling (`PlatformFieldEnums`, under both the spaced and the collapsed
field name, and under the table number), and the door's header table (`kElsewhere`, whose size
constant counts). Missing the last one leaves the generated file naming a type it does not include.

**Measured.** Chain 101, A/B against chain 100.

**Chain 100 named one more of the resolution kind, and it is in this round:** THE OBJECT'S OWN
PROCEDURE IS NEARER THAN THE RECORD'S. A report that declares `local procedure GetGLSetup()` over
a dataitem on `Item`, which declares one too, resolved to `Rec.GetGLSetup()` -- the table's,
private and of another arity. The same for `OnBeforeDeleteSalesLines` on `Sales Header`. A
dataitem's field or procedure is only reached when the object declares no procedure of that name.
