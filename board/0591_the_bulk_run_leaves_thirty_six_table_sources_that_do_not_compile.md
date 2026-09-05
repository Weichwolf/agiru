Type:     task
Status:   open
Parent:   0033
Area:     gen
Verdict:  teilweise
Class:    compile root

# The bulk run leaves 36 table sources that do not compile, each a generic gap with a name

`bulk.sh` over the 388 table sources the slice includes but did not link found 78 that do not
compile (2026-09-06). One round of generic fixes (Duration, Enum, Dialog.Open, Validate on an
Enum, temporal literals, the shadow check, XRec, the platform spellings) took 42 of them; the
36 left fall into these classes, measured by their first diagnostic:

| class | count | the gap |
|---|---:|---|
| `Text<0> ConvertFiltersToParameters(dotnet::GenericDictionary2 ...)` | 2 | a .NET generic stub whose name carries the arity (`GenericDictionary2`) is not emitted into `absent/Types.h` |
| `ReadingWritingXMLport = xmlports::...` | 1 | board:0065, no XMLport generator |
| `absent::AllObjWithCaption` | 1 | board:0032, a platform table |
| `LineWithPrice = JobPlanningLinePrice` | 2 | an interface-typed variable assigned an implementation codeunit: `Implementation<I>` takes a codeunit that implements `I` |
| `Message(HasErrorsMsg)` | 1 | a label named like a builtin shadows the builtin in a record body; the call must reach the builtin (`::agiru::Message`) |
| `RowNo = RowNo == "" ? CopyStr(...) : RowNo` | 1 | a conditional whose branches are `Text<0>` and `Code<10>`: the emitted `?:` needs one type |
| `result of comparison of constant 3 with expression ... always` | 1 | an Option compared against an ordinal outside its members: `-Wtautological-constant-out-of-range` on an `Option<E>` against an int |
| `SystemId() = TempPurchLine.SystemId` | 3 | a bare system field in a record body was spelled as a call -- fixed the same day |

The settle loop (`settle.sh`) reaches them one at a time as the loader asks; this item holds
the list so each fix is a class and not a file.
