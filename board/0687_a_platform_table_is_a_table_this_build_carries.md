# 0687 A platform table is a table this build carries

**The finding.** 21 UT cases die on
`the dataitem Integer of report Create Time Sheets is on a table this build does not carry`, and
the `Integer` virtual table has been in `include/platform/` all along. The index entry the
generator reads carried an EMPTY header and no fields, and an empty header is exactly what every
generator reads as "this build does not have the table" -- so every report over a platform table
refused, and so did every xmlport element over one.

**The choice, and it is four rules that all follow from the same one.**

- **The index entry carries the door's header and the door's fields.** `platform/<Name>.h` and
  `PlatformMembers(name)`, which the door already scans out of `include/platform/` -- so the list
  finds itself and no second list can go stale.
- **A dataitem named after its table is spelled with `_Var` when the name is a door type.**
  `dataitem(Number; Integer)` declared a member called `Integer`, which is the door's own type
  name, and the class stopped compiling. The numbering that already avoids a control's name now
  avoids a door type's as well.
- **A platform table is never FORWARD-DECLARED.** Its class is `X_Table` with `using X = X_Table;`,
  so `class X;` beside it declares the alias' name a second time. The header is included anyway,
  which is the whole point of a forward declaration -- three writers emitted one and all three
  stop.
- **A dataitem may name its table with the namespace**, `System.Utilities.Integer`, and the object
  index is keyed by the bare name. The namespace segments are stripped -- but ONLY segments that
  are plain identifiers followed by a non-space, because `Gen. Journal Line` is a table NAME with
  a period in it and stripping there turned 17 refusals into 120. That measurement is why the rule
  is written this way rather than as "the tail after the last dot".

**Measured.** Refusals over the generated tree: 17 -> 12 files (the rest are dataitems on tables
that really are out of scope). Chain 103, A/B against chain 97's 1 590, since chains 98-102 never
reached a milestone.

**And one thing it must NOT do, measured at 174 errors.** Putting the platform table's FIELDS into
the object index changed how every OTHER path spells them: a codeunit that had written
`AllProfile.ProfileID` started writing `ProfileId`, because a non-empty field map takes a
different route through the door's spelling table than `PlatformFieldSpelling` does. The index
carries the HEADER only, and the report path asks the door directly for a platform dataitem's
field. Chain 103 is what measured it -- 174 errors over the tree, all of that one shape.

**Chain 104 named the rest of the shape, and it is in this round:** a dataitem on a platform table
has FIELDS -- `HasField` and `MemberSpelling` ask the door for them, the way the report body path
does, so `CopyFilter("VAT Reporting Date", "Period Start")` inside `dataitem(..., Date)` is written
in the three-argument form the door declares. Beside it: EVERY textelement of an xmlport is a text
variable, children or not (AL reads `if PayeePartyTaxScheme = '' then` on an element that carries
whole elements), and the record form of `GetUrl` takes a record global by HANDLE.

**Chain 105, one error, same family:** `Page.FILTER.SETFILTER("Parameter Name", X)` names a FIELD
of the page's source record, and the generator wrote the page's control member -- which a page
that does not SHOW that field has not got. The filter pane takes the NAME: the generator writes
the literal and the door's `SetControlFilter` already falls back from control to field. `Report
Settings` is a page over `Object Options`, so this only became reachable once that table existed.

**The rename guard is about TYPE names and nothing else.** It first read `IsAlTypeName(plain) ||
DoorDeclares(plain)`, and `DoorDeclares` answers for every name the door spells -- so a text
element called `Note` became `Note_Var` and the xmlport generator gate went red. `IsAlTypeName`
alone covers the case that started this (`dataitem(Number; Integer)`), because `Integer` is an AL
type; a door METHOD's name is not a reason to rename anything.

**Chain 107 named the third and last variant of the rename guard.** `textelement(Id)` on the SEPA
xmlports declared a member `Id` that hid `XmlPort<Derived>::Id()`, so every `XMLPORT::"..."`
naming those ports stopped compiling. The guard is `IsAlTypeName(plain) ||
HiddenByABaseMember(plain)`: an AL type name, or a member `include/runtime/` declares on the base.
That is narrower than `DoorDeclares` -- which renamed a text element called `Note` -- and wider
than `IsAlTypeName` alone, which let `Id` through. Every textelement being a variable (board:0687
above) is what made the collision reachable at all.
