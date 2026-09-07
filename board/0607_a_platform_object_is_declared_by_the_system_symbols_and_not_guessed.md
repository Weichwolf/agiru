Type:     finding
Status:   open
Area:     rt, gen, doc
Source:   the hunt for `Company`'s field numbers, 2026-09-07
Class:    silent-wrong-data

# A platform object is declared by the system symbols, and never guessed

**`System.app` CARRIES THE AL SOURCE OF EVERY PLATFORM OBJECT, and this tree did not know it.**
362 `.al` files: 93 tenant database tables, 78 virtual tables, 43 application database tables, 29
system codeunits, 16 system enums, 10 pages, 9 temporary tables, 5 interfaces, 25 permission sets,
25 entitlements. `Company`, `User`, `Field`, `Date`, `Integer`, `Session`, `AllObj`,
`TableMetadata`, `Media`, `IsolatedStorage` -- all of them, with field numbers, lengths, option
members, keys, captions and `DataPerCompany`.

It is the symbol package the AL compiler compiles BCApps against, inside the on-prem PLATFORM
artefact on the same CDN the demo database comes from. `make symbols` reads three HTTP ranges out
of 1.37 GB -- the end-of-central-directory record, the central directory, the one 0.6 MB entry --
unwraps the NAVX container and unpacks it into `work/symbols/`. Nothing is vendored: it is a
reference tree, fetched like the database and pinned by `BC_VERSION`.

**IT IS REFERENCE #1½.** It outranks the AL source and the predecessor for anything a platform
object DECLARES, because it IS the declaration -- the same standing `~/Git/BCApps` has for a
BaseApp table. It says nothing about behaviour; the platform documentation keeps that.

## What it found, the first time it was read (measured 2026-09-07)

**Four of the six hand-written headers under `include/platform/` carry wrong field numbers.** Every
one of them came from `~/Git/openerp/openerp/runtime/base/system_tables.py`, which measured them
from a column order -- and a column order cannot see a gap.

| table | this tree said | the declaration says |
|---|---|---|
| `User` 2000000120 | `User Name` 3, `Full Name` 4, `State` 5, `Expiry Date` 6, `License Type` 9, `Authentication Email` 10, `Contact Email` 11, `Exchange Identifier` 12, `Application ID` 13 | 2, 3, 4, 5, 10, 11, 14, 15, **16** |
| `User Personalization` 2000000073 | `User SID` 1, `Profile ID` 3, `Language ID` 6, `Company` 7, `Scope` 8, `App ID` 9, `Locale ID` 10, `Time Zone` 11, `User ID` 12 | 3, 9, 12, 15, 11, 10, **27**, **30**, 6 |
| `Field` 2000000041 | `Enabled` 21, `RelationTableNo` 8, `RelationFieldNo` 9, `OptionString` 10, `ObsoleteState` 11, `ObsoleteReason` 12 | 8, **21**, **22**, **24**, **25**, **26** |
| `Date` 2000000007 | `Period Name` `Text[30]`, five fields | `Text[31]`, and a sixth: `Period Invariant Name` |
| `Integer` 2000000026 | `Number` 1 | 1 -- the one that was right |

`User."Exchange Identifier"` is `Text[250]` and this tree declared it `Text[80]`, which is the
`Full Name` length borrowed by name. Nine of `Field`'s twenty-four fields are missing here, and
`Enabled` sat on `RelationTableNo`'s number -- a `FieldRef` by number would have read the wrong
column and thrown nothing.

**AND THE GUESS THAT WAS ABOUT TO BE MADE WOULD HAVE BEEN WRONG TOO.** `Company`'s columns run
`Name`, `Evaluation Company`, `Display Name`, `Id`, `Business Profile Id` in the restored demo
database, and the derivation that produced `User."Application ID"` = 13 -- ascending numbers in
column order -- gives 1, 2, 3, 4, 5. The declaration says 1, 2, 3, **8000**, **8005**. Field 13 was
wrong the same way and is now 16.

## What is taken now, and what is left

**Now, mechanically:** `include/platform/Company.h` is added from the declaration and registered
with the generator, which is what `ICPartner` waits on (board:0601); the numbers, lengths, keys and
option vocabularies of `User`, `User Personalization`, `Field` and `Date` are corrected against it.
`User.State` and `User."License Type"` get the vocabulary board:0032 recorded as unknowable --
`Enabled,Disabled` and the ten license types, in the declared ORDER.

**118 OF THE 881 ABSENT AL OBJECTS ARE PLATFORM OBJECTS THE SYMBOLS DECLARE** (measured
2026-09-07 by matching `apps/absent/absent/Types.h` against the 277 objects the package names):
**110 tables**, 6 codeunits (`Base64Convert`, `IndexManagement`, `CompanyTriggers`,
`SystemActionTriggers`, `UIHelperTriggers`, `AgentUtilities`), one enum and one page.
`AccessControl`, `ActiveSession`, `AllObj`, `AllObjWithCaption`, `Media`, `MediaSet`,
`IsolatedStorage`, `Session`, `TableMetadata`, `PageMetadata`, `ScheduledTask` are among them. Every
one is a refusal today, and every one is a `.al` file this tree can already read. The transpiler's
own census says the same from the other side: `unknown 196 table(s) named by 406 declaration(s) are
declared outside this source root`, led by `AllObjWithCaption` 14, `All Profile` 10,
`Tenant Permission` 8, `Report Layout List` 7, `Aggregate Permission Set` 6.

**Left, and it is the generic fix:** a hand-written header IS an AL object inside the runtime, which
is the thing this tree forbids everywhere else. **The platform objects become a transpiled app.**
The transpiler already reads `.al`; these are `.al`. Two of the six stay in the door because the
runtime itself names them -- `RecordRef` includes `platform/Field.h` and `Session` includes
`platform/Tenant.h` -- and everything else moves out to `apps/platform/`. That closes 362 objects'
worth of absent stubs at once, and it is a separate round because it needs `scope.json`, an entry in
`apps.json` and a decision about the namespaces the symbols declare (`System.Environment`,
`System.Security.AccessControl`, `System.Utilities`).

## What the predecessor says, and it is about the QUESTION

**openerp WI-1041 is this item's other half, and it names the population.** 194 names / 1 719 call
sites in the generated tree resolve to neither an AL object nor a runtime type: they are platform
tables. Ranked: `all_profile` 125, `company` 91, `access_control` 71, `media_resources` 65,
`tenant_permission_set` 58, `user_personalization` 55, `aggregate_permission_set` 54,
`report_layout_list` 50, `code_coverage` 45. It built four families by hand and measured each
(All Profile +2, the permissions family +4 over 124 ids), and its own method note says the fields
were "aus der tatsächlichen Nutzung im generierten Code UND aus dem AL-Test ableiten, nicht
erfinden" -- derive from the usage, do not invent.

**THAT METHOD IS WHY THE NUMBERS ARE WRONG.** Usage names a field and never numbers it, so the
number came from the column order, and the column order cannot see field 8000. The predecessor was
right about the question -- these tables are missing and they cost tests -- and wrong about the
answer in the way CLAUDE.md predicts. WI-1040 records the neighbouring trap: it built a second
`Integer` table because it probed the generated tree rather than the started runtime.

## The open question this raises, and it is NOT closed here

**`Field.Type` IS NOT `FieldType`.** The virtual table declares its own option:

```al
field(5; Type; Option)
{
    OptionMembers = TableFilter,RecordID,OemText,Date,Time,DateFormula,Decimal,Media,MediaSet,Text,
                    Code,Binary,BLOB,Boolean,Integer,OemCode,Option,BigInteger,Duration,GUID,DateTime;
    OptionOrdinalValues = 4912, 4988, 11519, 11775, 11776, 11797, 12799, 26207, 26208, 31488,
                          31489, 33791, 33793, 34047, 34559, 35071, 35583, 36095, 36863, 37119, 37375;
}
```

so `Field.Type::Code` is **31489**, while `FieldType::Code` -- what `FieldRef.Type()` returns and
what `test/gate/PlatformFieldGate.cpp` pins -- is 33. This tree stores one option for both. The
BaseApp compares by MEMBER and never by number, so nothing has failed yet; a test that formats a
`Field.Type` or writes its ordinal would. It needs its own round: the two options are separated, or
the divergence is argued for with the call sites counted.

`ObsoleteState` is the same shape one size smaller: the virtual table declares three members
(`No,Pending,Removed`) and the AL PROPERTY has five. The first three agree ordinal for ordinal, so
the superset here is harmless -- but it is a superset and not the declaration.

## Found on the way, and red before this round started

**`test/gate/PlatformFieldGate.cpp` is 3 red at HEAD** (a36f0e9, measured by building only that
target from a clean checkout): `Temporary<Field>.Get(27, 1)` answers false and leaves the record on
the previously read row. `Field` declares its own `Get(TableNo, No)` which HIDES `Table<Field>::Get`
and reads the CATALOGUE -- so a TEMPORARY Field never reaches its own store. It is board:0052's
shape one level down, it is not caused by the renumbering here, and it is left standing rather than
folded into this item's diff.

**`test/door-reproduces.sh` is red at HEAD too**, and for the reason it was written: five builtins
had been written INTO the generated `src/rt/Builtins.cpp` -- `ClearCollectedErrors`,
`GetCollectedErrors`, `HasCollectedErrors`, `GuiAllowed`, `Hyperlink` and the handler helper
`AnsweredByHandler` -- where the next generator run takes them away. They are moved to
`src/rt/written/BuiltinsWritten.cpp` in this round, which is the place that exists for them.

**The gate's own check needed loosening to say that.** It refused ANY `RefuseDoor` in the written
file, and `GuiAllowed` and `Hyperlink` refuse CONDITIONALLY -- they answer from the handler table
first. The check now asks whether a refusal is the FIRST statement of a body, which is the failure
it was written for.

## What proves it

`make symbols` writes 362 declarations; `ldd -r build/agiru` names no undefined data symbol for
`kICPartnerTable`; and the numbers in `include/platform/` are diffable against
`work/symbols/src/**/<Table>.Table.al` line for line. The negative control is the old number: with
`User."Application ID"` at 13 the `AADApplication` source still compiles, because a wrong number is
exactly the defect that compiles.
