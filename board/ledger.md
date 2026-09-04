# The documentation ledger

Bookkeeping only: which `.md` is COMPLETELY read, and which work items came out of it. No findings
here -- a finding belongs in its item, or the ledger absorbs the backlog.

`-` in the WI column means the page produces no task; the reason is in the row.

## The work item header, for the items this sweep files

```
Type:     task
Status:   open
Parent:   0057
Area:     gen, rt
Source:   developer/attributes/devenv-eventsubscriber-attribute.md
Verdict:  fehlt
Class:    activation
```

| field | required | values |
|---|---|---|
| `Type` | yes | `task` work to be done · `bug` the tree does something and it is WRONG · `arc` an architecture decision · `epic` a subject that contains other items |
| `Status` | yes | `open` · `active` (said in the item's own commit BEFORE the work -- the only ownership mark) · `blocked` |
| `Parent` | on a child | the id it sits under; absent on a top-level item |
| `Area` | yes | `al` `net` `db` `gen` `rt` `cli` `build` |
| `Source` | on a documentation item | the `.md` it was written from, relative to its reference tree |
| `Verdict` | on a documentation item | `implementiert` · `teilweise` · `deklariert` · `ungeprueft` · `fehlt` -- the state of the tree when it was written. `teilweise` is the honest one: some of the page's statements have a body and some do not, and the item names which at which `file:line` |
| `Class` | on a fix | `silent-wrong-data` or `activation`; the second always carries an A/B |
| `Blocked-by` | on a blocked item | the ids that must close first |
| `Tags` | no | free |

**There is no `done`.** A finished item is DELETED and what it said is in the commit.

The 78 items that predate this sweep use the older `State:` field and `Type: root|leaf|arc|bug`;
they are left as they are.

## How the two halves of "compare against `src/` and `apps/`" are read

**`apps/` is not generated.** `find apps -type f` returns two `reaches` files and nothing else
(2026-09-04), so no AL object is translated yet and nothing can be implemented there. Therefore:

- **the IST-state comes from `include/` and `src/`** -- does the signature exist on THIS type, does it
  have a body, does the body do what the page says, and at which `file:line` that is provable;
- **the POPULATION is measured over `~/Git/BCApps/src`**, which is what `apps/` will contain once
  `make transpile` runs. Every count in an item names that tree and its date.

### How a population is measured, and what it took to get there

**A property DECLARATION begins at a statement boundary**: the start of a line, an opening brace, or
a semicolon. That is the pattern, and it is the same one every time:

```
/usr/bin/grep -rhoiE "(^|[{;])[[:space:]]*<Property>[[:space:]]*=" --include=*.al <tree> | wc -l
```

Five things in it were each paid for by a wrong number, not reasoned out in advance:

| clause | without it | example |
|---|---|---|
| `-i` | the page's spelling is not AL's | `ExtendedDataType` measured **4** instead of 2 745 (board:0329) |
| a boundary | a longer identifier ends in the name | `NotBlank` measured **2 983**; 535 were `ExportIfNotBlank` (board:0319) |
| `(^\|[{;])` rather than `[^A-Za-z]` | an AL expression is not a declaration | `TestTableRelation` measured **15**; all 15 are `//TestTableRelation = false;` (board:0333) |
| the same | an XML attribute inside a report is not a property | `BlankZero` measured **5 918**; 814 were `BlankZero="false"` in embedded RDLC (board:0324) |
| no `;` required | a declaration may span lines | `CalcFormula` measures 2 737 with `;` on the same line and 8 761 without (board:0340) |
| `/usr/bin/grep` | the interactive shell wraps `grep` in a different engine | the two disagree by ~0.5 % on this tree; `Clustered` is 4 565 against 4 586 |

**Every number in this sweep is re-measured with that pattern.** The first two themes were filed with
earlier ones and corrected in place; the corrections moved 19 numbers, one of them by 15 % and one
from 15 to 0 -- and the one that went to 0 REVERSED board:0333's decision, from "record it as
ignored, refusing 15 declarations is too expensive" to "refuse it, nobody declares it".

A population is the number that orders the work. Measured differently in two items, it orders it
wrongly and nothing says so.

## Counters

| family | pages | read | items filed |
|---|---:|---:|---:|
| `developer/attributes/` | 41 | 41 | **38** |
| `developer/methods-auto/` | 1 876 | **1 876** | **3** |
| `developer/properties/` | 349 | **349** | **173** |
| `developer/triggers-auto/` | 152 | **152** | **85** |
| `developer/` root | 470 | **470** | **87** |
| `dev-itpro/security/` | 20 | **20** | **4** |
| | **2 908** | **2 908** | **390** |

**Four families are OUT OF SCOPE, by the user's instruction of 2026-09-04**, and the denominator is
cut accordingly rather than left standing at 0 read:

| family | pages | why it is out |
|---|---:|---|
| `developer/diagnostics/` | 907 | compiler messages -- what the AL COMPILER refuses, and agiru is not it |
| `developer/analyzers/` | 279 | CodeCop / AppSourceCop / UICop rules -- advice to an AL author |
| `dev-itpro/administration/` | 394 | operating a BC tenant, not translating one |
| `dynamics365smb-docs/` | 2 642 | user documentation |

**4 222 pages struck.** The two families that remained -- `developer/` root and `methods-auto/` --
are both finished: **2 908 of 2 908 pages read, 390 items filed.**

A counter reporting 0 over N pages is an abort, not a pass -- which is why a family that will never be
read leaves the table instead of sitting at 0.

---

## `developer/attributes/` -- 41 pages

**What the tree does with attributes, established once, 2026-09-04.** `src/al/Parser.cpp:545`
reads EVERY attribute into `ProcedureDecl::attributes` as a raw string, and
`al::HasAttribute` (`Parser.cpp:926`) matches one by name. The GENERATOR acts on exactly four:
`IntegrationEvent`, `BusinessEvent` and `InternalEvent` through `IsPublisher`
(`src/gen/CodeunitWriter.cpp:29`), `Test` through `IsTest` (`:77`), and `TransactionModel` through
`TransactionModelOf` (`:65`). Every other attribute is parsed and dropped -- which CLAUDE.md names as
worse than refusing it.

| page | WI | note |
|---|---|---|
| `devenv-businessevent-attribute.md` | 0191 | |
| `devenv-caption-attribute.md` | 0192 | |
| `devenv-commitbehavior-attribute.md` | 0193 | |
| `devenv-confirmhandler-attribute.md` | 0194 | |
| `devenv-errorbehavior-attribute.md` | 0195 | |
| `devenv-eventsubscriber-attribute.md` | 0196 | |
| `devenv-externalbusinessevent-attribute.md` | 0197 | |
| `devenv-filterpagehandler-attribute.md` | 0198 | |
| `devenv-handlerfunctions-attribute.md` | 0199 | |
| `devenv-httpclienthandler-attribute.md` | 0200 | |
| `devenv-hyperlinkhandler-attribute.md` | 0201 | |
| `devenv-indataset-attribute.md` | - | deprecated in runtime 11.0 with the reason "The InDataset attribute is unused"; there is nothing to build |
| `devenv-inherentpermissions-attribute.md` | 0202 | |
| `devenv-integrationevent-attribute.md` | 0203 | |
| `devenv-internalevent-attribute.md` | 0204 | |
| `devenv-messagehandler-attribute.md` | 0205 | |
| `devenv-method-attributes.md` | 0190 | the index over the other 40; it is the epic's source because it is where the SET is defined |
| `devenv-modalpagehandler-attribute.md` | 0206 | |
| `devenv-native-attribute.md` | - | "Restricted to Microsoft only", and 0 occurrences in the read roots |
| `devenv-nondebuggable-attribute.md` | 0207 | |
| `devenv-none-attribute.md` | - | "used implicitly when a method does not have an attribute set" -- the absence of an attribute, not one |
| `devenv-normal-attribute.md` | 0208 | |
| `devenv-obsolete-attribute.md` | 0209 | |
| `devenv-pagehandler-attribute.md` | 0210 | |
| `devenv-recallnotificationhandler-attribute.md` | 0211 | |
| `devenv-reporthandler-attribute.md` | 0212 | |
| `devenv-requestpagehandler-attribute.md` | 0213 | |
| `devenv-requiredpermissions-attribute.md` | 0214 | |
| `devenv-runonclient-attribute.md` | 0215 | |
| `devenv-scope-attribute.md` | 0216 | |
| `devenv-securityfiltering-attribute.md` | 0217 | |
| `devenv-sendnotificationhandler-attribute.md` | 0218 | |
| `devenv-serviceenabled-attribute.md` | 0219 | |
| `devenv-sessionsettingshandler-attribute.md` | 0220 | |
| `devenv-strmenuhandler-attribute.md` | 0221 | |
| `devenv-suppressdispose-attribute.md` | 0222 | |
| `devenv-test-attribute.md` | 0223 | |
| `devenv-testpermissions-attribute.md` | 0224 | |
| `devenv-transactionmodel-attribute.md` | 0225 | |
| `devenv-tryfunction-attribute.md` | 0226 | |
| `devenv-withevents-attribute.md` | 0227 | |

---

## `developer/triggers-auto/` -- 152 pages

**What the tree does with triggers, established once, 2026-09-04.** `include/runtime/Table.h` fires
the base table's `OnInsert` (`:353`), `OnModify` (`:381`) and `OnDelete` (`:406`) under
`Insert/Modify/Delete(RunTrigger)`, each guarded by `if constexpr (requires ...)`; `Rename` is a
variadic refusal (`:1141`) so `OnRename` has no call site. `src/gen/TableWriter.cpp:590` emits a
`constexpr kOnValidate` map per table and `Table.h:1373` runs it AFTER `CheckRelation` and restores
the record on any failure. **`TableWriter.cpp:592` keeps only the trigger named `onvalidate`** from
each field's list, so `OnLookup`, `OnBeforeValidate` and `OnAfterValidate` are discarded by a filter
that names their sibling.

| page | WI | note |
|---|---|---|
| `devenv-triggers.md` | - | the index over the family; board:0029 is the epic |
| `table/devenv-oninsert-table-trigger.md` + `tableextension/devenv-oninsert-...` | 0228 | one trigger, two declaration sites |
| `table/devenv-onmodify-table-trigger.md` + `tableextension/devenv-onmodify-...` | 0229 | |
| `table/devenv-ondelete-table-trigger.md` + `tableextension/devenv-ondelete-...` | 0230 | |
| `table/devenv-onrename-table-trigger.md` + `tableextension/devenv-onrename-...` | 0231 | |
| `field/devenv-onvalidate-field-trigger.md` | 0232 | |
| `field/devenv-onlookup-field-trigger.md` | 0233 | |
| `tableextension/devenv-onbeforeinsert-tableextension-trigger.md` | 0234 | |
| `tableextension/devenv-onafterinsert-tableextension-trigger.md` | 0235 | |
| `tableextension/devenv-onbeforemodify-tableextension-trigger.md` | 0236 | |
| `tableextension/devenv-onaftermodify-tableextension-trigger.md` | 0237 | |
| `tableextension/devenv-onbeforedelete-tableextension-trigger.md` | 0238 | |
| `tableextension/devenv-onafterdelete-tableextension-trigger.md` | 0239 | |
| `tableextension/devenv-onbeforerename-tableextension-trigger.md` | 0240 | |
| `tableextension/devenv-onafterrename-tableextension-trigger.md` | 0241 | |
| `fieldextension/devenv-onbeforevalidate-fieldextension-trigger.md` | 0242 | |
| `fieldextension/devenv-onaftervalidate-fieldextension-trigger.md` | 0243 | |
| `events/table/devenv-onbeforeinsertevent-table-trigger.md` | 0244 | |
| `events/table/devenv-onafterinsertevent-table-trigger.md` | 0245 | |
| `events/table/devenv-onbeforemodifyevent-table-trigger.md` | 0246 | |
| `events/table/devenv-onaftermodifyevent-table-trigger.md` | 0247 | |
| `events/table/devenv-onbeforedeleteevent-table-trigger.md` | 0248 | |
| `events/table/devenv-onafterdeleteevent-table-trigger.md` | 0249 | |
| `events/table/devenv-onbeforerenameevent-table-trigger.md` | 0250 | |
| `events/table/devenv-onafterrenameevent-table-trigger.md` | 0251 | |
| `events/table/devenv-onbeforevalidateevent-table-trigger.md` | 0252 | |
| `events/table/devenv-onaftervalidateevent-table-trigger.md` | 0253 | |
| `events/page/devenv-onopenpageevent-page-trigger.md` | 0254 | |
| `events/page/devenv-onqueryclosepageevent-page-trigger.md` | 0255 | |
| `events/page/devenv-onclosepageevent-page-trigger.md` | 0256 | |
| `events/page/devenv-onnewrecordevent-page-trigger.md` | 0257 | |
| `events/page/devenv-oninsertrecordevent-page-trigger.md` | 0258 | |
| `events/page/devenv-onmodifyrecordevent-page-trigger.md` | 0259 | |
| `events/page/devenv-ondeleterecordevent-page-trigger.md` | 0260 | |
| `events/page/devenv-onaftergetrecordevent-page-trigger.md` | 0261 | |
| `events/page/devenv-onaftergetcurrrecordevent-page-trigger.md` | 0262 | |
| `events/page/devenv-onbeforeactionevent-page-trigger.md` | 0263 | |
| `events/page/devenv-onafteractionevent-page-trigger.md` | 0264 | |
| `events/page/devenv-onbeforevalidateevent-page-trigger.md` | 0265 | |
| `events/page/devenv-onaftervalidateevent-page-trigger.md` | 0266 | |
| `codeunit/devenv-onrun-codeunit-trigger.md` | 0267 | filed as a **bug**: the trigger runs, the boundary around it does not commit or raise |
| `codeunit/devenv-onbeforetestrun-codeunit-trigger.md` | 0268 | |
| `codeunit/devenv-onaftertestrun-codeunit-trigger.md` | 0269 | |
| `codeunit/devenv-oninstallapppercompany-codeunit-trigger.md` | 0270 | |
| `codeunit/devenv-oninstallappperdatabase-codeunit-trigger.md` | 0271 | |
| `codeunit/devenv-onupgradepercompany-codeunit-trigger.md` | 0272 | |
| `codeunit/devenv-onupgradeperdatabase-codeunit-trigger.md` | 0273 | |
| `codeunit/devenv-oncheckpreconditionspercompany-codeunit-trigger.md` | 0274 | |
| `codeunit/devenv-oncheckpreconditionsperdatabase-codeunit-trigger.md` | 0275 | |
| `codeunit/devenv-onvalidateupgradepercompany-codeunit-trigger.md` | 0276 | |
| `codeunit/devenv-onvalidateupgradeperdatabase-codeunit-trigger.md` | 0277 | |
| `page/devenv-oninit-page-trigger.md` + `pageextension/...` | 0278 | |
| `page/devenv-onopenpage-page-trigger.md` + `pageextension/...` | 0279 | |
| `page/devenv-onqueryclosepage-page-trigger.md` + `pageextension/...` | 0280 | |
| `page/devenv-onclosepage-page-trigger.md` + `pageextension/...` | 0281 | |
| `page/devenv-onfindrecord-page-trigger.md` + `pageextension/...` | 0282 | |
| `page/devenv-onnextrecord-page-trigger.md` + `pageextension/...` | 0283 | |
| `page/devenv-onaftergetrecord-page-trigger.md` + `pageextension/...` | 0284 | |
| `page/devenv-onaftergetcurrrecord-page-trigger.md` + `pageextension/...` | 0285 | |
| `page/devenv-onnewrecord-page-trigger.md` + `pageextension/...` | 0286 | |
| `page/devenv-oninsertrecord-page-trigger.md` + `pageextension/...` | 0287 | |
| `page/devenv-onmodifyrecord-page-trigger.md` + `pageextension/...` | 0288 | |
| `page/devenv-ondeleterecord-page-trigger.md` + `pageextension/...` | 0289 | |
| `page/devenv-onpagebackgroundtaskcompleted-page-trigger.md` + `pageextension/...` | 0290 | |
| `page/devenv-onpagebackgroundtaskerror-page-trigger.md` + `pageextension/...` | 0291 | |
| `pagefield/devenv-onvalidate-pagefield-trigger.md` + `pagefieldextension/...` | 0292 | |
| `pagefield/devenv-onlookup-pagefield-trigger.md` + `pagefieldextension/...` | 0293 | |
| `pagefield/devenv-onafterlookup-pagefield-trigger.md` + `pagefieldextension/...` | 0294 | |
| `pagefield/devenv-ondrilldown-pagefield-trigger.md` + `pagefieldextension/...` | 0295 | |
| `pagefield/devenv-onassistedit-pagefield-trigger.md` + `pagefieldextension/...` | 0296 | |
| `pagefield/devenv-oncontroladdin-pagefield-trigger.md` + `pagefieldextension/...` | 0297 | |
| `action/devenv-onaction-action-trigger.md` + `actionextension/` (2) | 0298 | one call site, the extension's two brackets around it |
| `query/devenv-onbeforeopen-query-trigger.md` | 0299 | |
| `fileuploadaction/devenv-onaction-fileuploadaction-trigger.md` | 0300 | |
| `requestpage/` (12) + `requestpageextension/` (9) | 0301 | one task: the page trigger set on a request page, plus the attachment |
| `report/devenv-oninitreport-report-trigger.md` | 0302 | |
| `report/devenv-onprereport-report-trigger.md` + `reportextension/...` | 0303 | |
| `report/devenv-onpostreport-report-trigger.md` + `reportextension/...` | 0304 | |
| `report/devenv-onprerendering-report-trigger.md` + `reportextension/...` | 0305 | |
| `reportdataitem/devenv-onpredataitem-reportdataitem-trigger.md` | 0306 | |
| `reportdataitem/devenv-onaftergetrecord-reportdataitem-trigger.md` | 0307 | |
| `reportdataitem/devenv-onpostdataitem-reportdataitem-trigger.md` | 0308 | |
| `reportextensiondatasetmodify/` (6) | 0309 | one task: three brackets around 0306, 0307 and 0308 |
| `xmlport/` (3) | 0310 | one task: three points in one driver loop |
| `xmlporttableelement/` (7) | 0311 | one task: seven points in one element loop, selected by `Direction` |
| `xmlportfieldelement/` (2) + `xmlportfieldattribute/` (2) + `xmlporttextelement/` (2) + `xmlporttextattribute/` (2) | 0312 | one task: four node kinds, two call points |

**Done: 152 of 152 pages, 85 items.** Four of them fold several pages that are one loop and one
implementation -- 0301, 0309, 0311, 0312 -- and each says so in its own text.

---

## `dev-itpro/security/` -- 20 pages

Read in full 2026-09-04. This family is outside the goal's declared population -- it is a sibling of
`developer/` -- and is read because `ui-define-granular-permissions.md` cites it as the specification
for record-level security.

| page | WI | note |
|---|---|---|
| `Data-Security.md` | 0313 | |
| `Security-Filters.md` | 0314 | |
| `Security-Considerations.md` | 0315 | |
| `security-auditing.md` | 0316 | |
| `privacy-developers.md` | - | `DataClassification` is a field property (board:0067); the telemetry route it gates does not exist here |
| `security-developers.md` | - | an index over authentication, authorisation, auditing and encryption |
| `security-application.md` | - | the layered model, an index |
| `Security-and-Protection.md`, `security-faq.md`, `PrivacyFAQ.md` | - | overviews and FAQs |
| `security-online.md`, `security-onpremises.md`, `security-users.md`, `security-scenarios-guide.md`, `enhancing-server-instance-security.md`, `security-lock-down-server-communication.md`, `security-service-tags.md`, `multifactor-authentication.md`, `transparent-data-encryption.md`, `Setting-Database-Owner-and-Security-Administration-Permissions.md` | - | hosting, network, disk, MFA, Azure service tags, TDE, SQL Server logins -- agiru is one process and one PostgreSQL, so none of it is a runtime rule |

---

## `developer/properties/` -- 349 pages

Worked by THEME rather than alphabetically, because the pages cross-reference each other inside a
theme and reading one without its neighbours produces the wrong item. The first theme is the FIELD
CONSTRAINT and FIELD FORMAT properties, which is board:0068's and board:0066's subject.

**One rule decides the whole constraint half, and it is stated in three of the pages verbatim:**
"This setting is evaluated for controls and fields during validation. Validation occurs only if the
field or control value is updated **through the UI** ... If a field is updated through **application
code**, then the property is not validated." So `MinValue`, `MaxValue`, `NotBlank`, `Numeric`,
`CharAllowed` and `ValuesAllowed` do NOT fire on an AL assignment. An implementer's first instinct --
put the check in the field's setter -- refuses code the BaseApp writes, which is why every one of
those items is classified `activation` and carries the AL assignment as its negative control.

**The shared IST-state**: `include/meta/TableDef.h:67` -- `FieldDef` carries `no`, `name`, `caption`,
`type`, `length`, `offset`, `values` and `initValue`, and nothing else. Every property below except
`InitValue` is therefore `fehlt` at the METADATA and not merely at the check.

| page | WI | population in `~/Git/BCApps/src`, 2026-09-04 |
|---|---|---:|
| `devenv-minvalue-property.md` | 0317 | 3 398 |
| `devenv-maxvalue-property.md` | 0318 | 1 448 |
| `devenv-notblank-property.md` | 0319 | 2 448 |
| `devenv-numeric-property.md` | 0320 | 201 |
| `devenv-charallowed-property.md` | 0321 | 97 |
| `devenv-valuesallowed-property.md` | 0322 | 46 |
| `devenv-blanknumbers-property.md` | 0323 | 155 |
| `devenv-blankzero-property.md` | 0324 | 5 104 |
| `devenv-decimalplaces-property.md` | 0325 | 10 577 |
| `devenv-closingdates-property.md` | 0326 | 187 |
| `devenv-signdisplacement-property.md` | 0327 | **0** |
| `devenv-initvalue-property.md` | 0328 | 2 546 |
| `devenv-extendeddatatype-property.md` | 0329 | 2 745 |
| `devenv-masktype-property.md` | 0330 | 453 |

Three of these are not constraints and their parent says so: `BlankNumbers`, `BlankZero` and
`SignDisplacement` are FORMAT properties under board:0066, `ClosingDates` is board:0016's, and
`ExtendedDatatype` and `MaskType` are the client's under board:0030.

`SignDisplacement`'s population is **0** and it still gets an item. A property nobody declares still
needs a decision, and the decision -- refuse it in the generator -- is what stops it from being
silently dropped the day somebody declares one.

`InitValue` is the family's only `teilweise`: the metadata exists (`include/meta/TableDef.h:95`) and
`Record.Init()` honours it (`src/rt/Table.cpp:296`), while the page's other two entry points --
`Clear` and `ClearAll` -- refuse the door (`src/rt/Builtins.cpp:74`, `:79`).

---

### The generator's property census, measured 2026-09-04

The IST-state sentence for the whole `properties/` family is one measurement, and it is the same
shape the attributes family had: **the parser reads every property, the generator consumes nine.**

- `src/al/Ast.h:12` -- `struct Property`, and eight AST nodes each carry a
  `std::vector<Property> properties`: the object, the field, the key, the enum value, the page
  control and the rest. **Nothing is dropped at parse time.**
- `grep "Find(.*properties" src/gen/*.cpp` returns 16 call sites naming **9 distinct property
  names**:

| property | on | consumed in |
|---|---|---|
| `TableNo` | codeunit | `src/gen/CodeunitWriter.cpp` |
| `Subtype` | codeunit | `src/gen/CodeunitWriter.cpp` |
| `Caption` | enum value, table field | `src/gen/EnumWriter.cpp`, `src/gen/TableWriter.cpp` |
| `Implementation` | enum value | `src/gen/EnumWriter.cpp` |
| `OptionMembers` | table field | `src/gen/TableWriter.cpp`, `src/gen/BodyWriter.cpp` |
| `OptionCaption` | table field | `src/gen/TableWriter.cpp` |
| `InitValue` | table field | `src/gen/TableWriter.cpp` |
| `SourceTable` | page | `src/gen/PageWriter.cpp` |
| `Clustered` | key | `src/gen/TableWriter.cpp` |

**9 of 349 pages have a consumer.** That is board:0067's counter with a number on it, and it means an
item in this family is `fehlt` at the metadata unless it names one of those nine.

---

## `developer/properties/`, second theme -- the relation and the lookup

board:0043's subject and board:0030's. The theme is held together by one fact: `TableRelation` is not
a table name but a GRAMMAR, and every other property here modifies what that grammar does.

**The IST-state that covers the theme**: `include/runtime/Table.h:116` declares `CheckRelation`,
`include/runtime/Table.h:1380` calls it from `Validate` at the documented point -- before the trigger,
which is the order the predecessor paid four rounds for -- and `src/rt/Table.cpp:350` is an empty
body with three discards. **The call site is right and there is nothing behind it**, which is why
board:0331 is `deklariert` and not `fehlt`.

| page | WI | population in `~/Git/BCApps/src`, 2026-09-04 |
|---|---|---:|
| `devenv-tablerelation-property.md` | 0331 | 40 221 |
| `devenv-validatetablerelation-property.md` | 0332 | 2 240 |
| `devenv-testtablerelation-property.md` | 0333 | **0** -- the 15 apparent declarations are all commented out |
| `devenv-lookuppageid-property.md` | 0334 | 2 294 |
| `devenv-drilldownpageid-property.md` | 0335 | 1 822 |
| `devenv-lookup-property.md` | 0336 | 497 |
| `devenv-drilldown-property.md` | 0337 | 1 530 |
| `devenv-fieldvalidate-property.md`, `devenv-defaultfieldsvalidation-property.md` | 0338 | 70, 22 |

**0338 is the theme's one deliberate grouping.** `DefaultFieldsValidation` sets the value of
`FieldValidate` and `FieldValidate` overrides it; the pages define each other and neither can be
implemented alone, so they are one task.

Three findings the theme produced that are worth having outside their items:

1. **`TableRelation` at 40 221 is the largest population measured in this sweep**, four times
   `DecimalPlaces`.
2. **Two pages disagree about drill-down's scope.** `devenv-drilldown-property.md` calls it "a
   system-wide feature of FlowFields"; `devenv-drilldownpageid-property.md` says "of fields (normal
   fields and FlowFields)". Recorded in board:0337, to be settled from the AL source, which declares
   where the documentation describes.
3. **Three properties in one theme need a TRI-STATE and not a `bool`** -- `Lookup`, `DrillDown` and
   `FieldValidate`. Absent is a third instruction meaning "inherit", and `initValue`'s door already
   states the rule for a different property: empty is not absent.

---

## `developer/properties/`, third theme -- the field class, the FlowField and the key

board:0019's subject, board:0047's and board:0045's. Sixteen pages, and they are one theme because
`FieldClass` decides which of them applies to a field at all.

**The IST-state that covers the theme:**

- `include/platform/Field.h:27` -- the `FieldClass` enumeration exists and is right.
- `src/rt/written/PlatformField.cpp:21` -- `AsClass` discards its argument and returns `Normal`.
  **Every field in the system reports `Normal`, including 10 282 that are not.**
- `include/meta/TableDef.h:98` -- `KeyDef` carries `name`, `fields`, `clustered`. Of the eight key
  properties in this theme, one reaches the metadata.
- **The schema writer is `src/rt/Storage.cpp:94`, not anything under `src/db/`**, and it already
  emits `CREATE TABLE` with a `PRIMARY KEY` from `keys[0]` and one plain `CREATE INDEX` for every
  further key. That corrects a claim four of these items were first filed with: it is not that no key
  is an index, it is that **every** key is an index, unconditionally -- so the 158 keys declaring
  `MaintainSqlIndex = false` and the 789 declaring `IncludedFields` are wrong in the tree TODAY, and
  the primary key's uniqueness is already enforced while `Unique`'s 13 secondary keys are not.

| page | WI | population in `~/Git/BCApps/src`, 2026-09-04 |
|---|---|---:|
| `devenv-fieldclass-property.md` | 0339 | 10 337 |
| `devenv-calcformula-property.md` | 0340 | 8 761 |
| `devenv-autocalcfield-property.md` | 0341 | 238 |
| `devenv-calcfields-property.md` | 0342 | 115 |
| `devenv-sumindexfields-property.md` | 0343 | 762 |
| `devenv-maintainsiftindex-property.md` | 0344 | 70 |
| `devenv-maintainsqlindex-property.md` | 0345 | 158 |
| `devenv-sqlindex-property.md` | 0346 | **0** |
| `devenv-columnstoreindex-property.md` | 0347 | **0** |
| `devenv-clustered-property.md` | 0348 | 4 565 |
| -- | **0349** (bug) | 50 |
| `devenv-unique-property.md` | 0350 | 13 |
| `devenv-includedfields-property.md` | 0351 | 789 |
| `devenv-orderby-property.md` | 0352 | 113 |
| `devenv-autoincrement-property.md` | 0353 | 357 |
| `devenv-autosplitkey-property.md` | 0354 | 622 |

**board:0349 is the theme's finding and it has no page of its own.** Reading
`devenv-clustered-property.md` against `src/gen/TableWriter.cpp:546` showed the emitted value compared
case-sensitively -- `clustered->text == "true"` -- while AL is case-insensitive and BCApps writes
`Clustered = True` **50 times**. Those 50 keys would be emitted as not clustered, silently. The tree's
own `LowerKey` idiom is used at 20 other call sites in the same file, so it is a slip and not a
convention. Filed as `Type: bug` in the same round the page was read, per the board convention.

Four breakdowns the theme produced that decide ordering:

**`FieldClass`, 10 337 declarations:** `FlowField` 8 772, `FlowFilter` 1 510, `Normal` 55. Ten
thousand fields in the BaseApp are not columns, and `AsClass` says all of them are.

**`CalcFormula`, 8 761 declarations, by verb:** `Sum` 3 967, `Lookup` 2 125, `Count` 1 378, `Exist`
1 026, `Max` 143, `Min` 111, `Average` 9. **Four verbs are 97 %**; `Average` is nine call sites in the
whole BaseApp, so the build order is not a matter of taste.

**The key properties against 3 272 declared keys:** `IncludedFields` 24 %, `SumIndexFields` 23 %,
`MaintainSqlIndex` 4.8 %, `MaintainSiftIndex` 2.1 %, `Unique` 0.4 %. **The 158 `MaintainSqlIndex`
declarations are the named exception to CLAUDE.md's "every declared key is a real INDEX"** -- the rule
holds for 95.2 % of keys and this property is the rest.

**Two more populations of 0:** `SqlIndex` and `ColumnStoreIndex`, joining `SignDisplacement`
(board:0327). All three take the same decision -- refuse in the generator -- for the same reason: a
refusal costs nothing at zero declarations and is the notification if one ever appears. `SqlIndex` and
`ColumnStoreIndex` additionally have no PostgreSQL equivalent, so the divergence is named rather than
mapped, the way board:0012 named the missing dirty read.

---

## `developer/properties/`, fourth theme -- obsoletion, the move, and who may see an object

board:0069's subject and board:0033's. Eleven pages, nine items -- the first theme where a page
produces no WI because a pre-existing root already IS the task.

| page | WI | population in %TREE%, 2026-09-04 |
|---|---|---:|
| `devenv-obsoletestate-property.md` | **board:0069** | 4 926 |
| `devenv-obsoletereason-property.md` | 0355 | 4 538 |
| `devenv-obsoletetag-property.md` | 0356 | 4 922 |
| `devenv-movedfrom-property.md`, `devenv-movedto-property.md` | 0357 | 133, 143 |
| `devenv-replicatedata-property.md` | 0358 | 1 064 |
| `devenv-access-property.md` | 0359 | 3 738 |
| `devenv-extensible-property.md` | 0360 | 2 285 |
| `devenv-scope-property.md` | 0361 | 1 131 (all three uses) |
| `devenv-scope-action-property.md` | 0362 | 1 096 `Repeater`, 29 `Page` |
| `devenv-scope-table-property.md` | 0363 | 2 `Cloud`, 0 `OnPrem` |

**`devenv-obsoletestate-property.md` gets no new WI.** board:0069 already is that task, in more detail
than a new item could add: it names the generator's blind spot, quotes `devenv-obsolete-objects.md`
on why a `Removed` field keeps its column, and records that its own first title was wrong. Filing a
second item beside it would split one task across two files. Its Remarks DO carry something 0069 does
not cover -- the full five-value vocabulary, `Moved` and `PendingMove` included -- and those went to
0357 with the properties they exist for.

Three findings from the theme:

**0357 is a grouping and the population proves the pairing.** `MovedTo` measures 143 and
`ObsoleteState = Moved` measures 143. A table that declares where it went also declares that it went,
and the two properties are the two ends of one move.

**`ReplicateData` says two thirds of the schema is scratch.** 1 064 declarations, all necessarily
`false` since `true` is the default, against 1 609 tables. That is a fact board:0004's CRONUS load and
board:0087's insert buffering would both want before sizing anything, and it is why 0358 carries the
flag into the metadata rather than refusing a property with no consumer.

**`Access` is two thirds `Internal`, which is the level C++ does not express.** 3 738 declarations:
2 532 `Internal`, 1 206 `Public`. `Local` and `Protected` on a field map exactly onto `private` and
`protected`; `Internal` means "this library and not its clients", which a C++ class member cannot be.
0359 does the field half now and leaves the `Internal` half to board:0033, because the answer depends
on what a generated app looks like and none exists.

**One property name, three unrelated meanings.** `Scope` is `Page`/`Repeater` on a page action,
`Cloud`/`OnPrem` on a table, and `Cloud`/`OnPrem` on an enum or interface -- and 1 096 of the 1 131
declarations are the first. A generator that looked the property up by name would refuse every one of
them, so the kind dispatch is the item's content and not its framing.

---

## `developer/properties/`, fifth theme -- where a table's rows actually live

Seventeen pages, twelve items. The theme is what the schema writer would have to know that it does
not: whether a table is a table, which database it is in, what the index and the storage do, and what
the page puts beside its caption.

**The IST-state that covers it**: `src/rt/Storage.cpp:94` -- `CreateTable` emits one `NOT NULL` column
per field from `ColumnType`, a `PRIMARY KEY` from `keys[0]`, and one plain `CREATE INDEX` per further
key. That is the whole schema. No table type, no storage parameter, no text index, no external
connection.

| page | WI | population in %TREE%, 2026-09-04 |
|---|---|---:|
| `devenv-datapercompany-property.md` | **board:0060** | 214 |
| `devenv-tabletype-property.md` | 0364 | 386 |
| `devenv-externalname-property.md`, `devenv-externalschema-property.md` | 0365 | 3 900, 2 |
| `devenv-linkedobject-property.md`, `devenv-linkedintransaction-property.md` | 0366 | **0**, **0** |
| `devenv-linktable-property.md`, `devenv-linkfields-property.md`, `devenv-linktableforceinsert-property.md` | 0367 | 115, 113, 1 |
| `devenv-dataaccessintent-property.md` | 0368 | 367 |
| `devenv-changetrackingallowed-property.md` | 0369 | 63 |
| `devenv-optimizefortextsearch-property.md` | 0370 | 1 197 |
| `devenv-sqltimestamp-property.md` | 0371 | 2 |
| `devenv-compressed-property.md` | 0372 | 6 |
| `devenv-compressiontype-property.md` | 0373 | 1 |
| `devenv-datacaptionfields-property.md` | 0374 | 1 326 |
| `devenv-datacaptionexpression-property.md` | 0375 | 690 |

**`devenv-datapercompany-property.md` gets no new WI**, for the same reason `ObsoleteState` did not:
board:0060 already IS that task, down to the `src/rt/Storage.cpp` line and the refusing
`ChangeCompany`.

Five findings:

**298 tables are in-memory and the schema writer creates all of them.** `TableType = Temporary`
measures 298 of the property's 386 declarations, `CRM` 83, `Exchange` 4, `MicrosoftGraph` 1;
`ExternalSQL` and `CDS` are never declared. So board:0364 is not a missing feature, it is 298
`CREATE TABLE`s BC does not issue.

**`ExternalName` measures 3 900 against zero tables of the type the page says it belongs to.** The
page says the property "appears when you specify CDS or ExternalSQL in the TableType property" and
neither value occurs. board:0365 records the contradiction rather than resolving it by preference:
the source declares where the documentation describes, so the AL settles it before the item is sized.

**`Compressed` is the theme's one inverted population.** Six declarations, all opting OUT, because
BLOB compression is on by default -- so what has to be built is the default and the six declarations
are the easy part. For every other property in this sweep the declaration count is the work.

**Two unpopulated properties, two different decisions.** `LinkedObject` measures 0 and is REFUSED,
because the documentation says a linked object's writes are outside transaction control and
CLAUDE.md's first invariant is that a posting is all or nothing. `CompressionType` measures 1 and is
CARRIED and ignored, because it changes what a table costs and not what it contains. The distinction
is whether ignoring the property can produce a wrong answer.

**One property Microsoft has already turned off.** `ChangeTrackingAllowed`: "From Wave 1 2024 setting
this property has no effect, as delta links are no longer supported." 63 pages still declare it.
board:0369 accepts and ignores it -- a refusal would stop 63 pages over a property that does nothing
in BC either, which is the opposite arithmetic from board:0333's zero.

---

## `developer/properties/`, sixth theme -- who may do what

board:0062's subject. Eight pages, six items, and the theme is held together by ONE VALUE SYNTAX:
`<objectkind> <identifier> = <letters>`, read by three properties and one attribute. A second parser
for any of them would be the mistake.

**The IST-state**: board:0062 already measured it. `RecordRef.ReadPermission()`,
`WritePermission()` and `SetPermissionFilter()` throw at `include/runtime/RecordRef.h:966`, `:1155`
and `:1048`; `Record.ReadPermission` at `include/runtime/Table.h:1099` is a variadic refusal. There
is no permission check anywhere, so every one of these properties currently grants everything.

| page | WI | population in %TREE%, 2026-09-04 |
|---|---|---:|
| `devenv-permissions-property.md` | 0376 | 4 020 |
| `devenv-accessbypermission-property.md` | 0377 | **10 211** |
| `devenv-inherentpermissions-property.md`, `devenv-inherententitlements-property.md` | 0378 | 3 199, 3 170 |
| `devenv-includedpermissionsets-property.md`, `devenv-excludedpermissionsets-property.md` | 0379 | 968, **1** |
| `devenv-assignable-property.md` | 0380 | 1 116 |
| `devenv-objectentitlements-property.md` | 0381 | 203 |

Four findings:

**`AccessByPermission` at 10 211 is the second-largest property population in this sweep**, behind
`TableRelation`'s 40 221 and ahead of `DecimalPlaces`. Ten thousand UI elements in the BaseApp are
conditional on a permission, and a renderer that ignores the property shows every one of them to
every user.

**The case of the letter is the semantics.** `R` is direct read and `r` is INDIRECT read, and a user
with indirect read cannot open a page showing the table -- only reach it through another object. A
case-insensitive read of the value collapses the two, which is board:0349's defect class arriving
where the collapse would GRANT rather than withhold.

**`InherentPermissions` and `InherentEntitlements` track each other within 1 %** -- 3 199 against
3 170. They open BC's two different gates, the permission set and the licence, and an object that
passes one alone is unreachable, so declaring one means declaring the other. That is why they are one
item.

**Inclusion is the mechanism and exclusion is used once.** `IncludedPermissionSets` measures 968 and
`ExcludedPermissionSets` measures **1** in the entire BaseApp. And a permission set EXTENSION may
include and may not exclude -- an extension can only widen, which is board:0033's direction rule
appearing in a second place.

---

## `developer/properties/`, seventh theme -- every string a user reads

board:0055's subject, board:0030's and board:0053's. **Thirty-three pages, seventeen items** -- the
largest theme so far and the one with the largest populations in the whole sweep.

**The IST-state**: `src/gen/TableWriter.cpp:37` and `src/gen/EnumWriter.cpp` consume `Caption` on a
table field and on an enum value. That is the entire string surface. `src/gen/TableWriter.cpp:551`
writes `.caption = <table>::kName`, so a TABLE's own caption is hardcoded to its name; every page
element carries no string at all.

| page | WI | population in %TREE%, 2026-09-04 |
|---|---|---:|
| `devenv-caption-property.md` | 0382 | **288 491** |
| `devenv-captionml-property.md` | 0383 | 3 |
| `devenv-captionclass-property.md` | 0384 | 9 524 |
| `devenv-tooltip-property.md` | 0385 | **159 993** |
| `devenv-tooltipml-property.md`, `devenv-instructionaltextml-property.md`, `devenv-abouttextml-property.md`, `devenv-abouttitleml-property.md`, `devenv-additionalsearchtermsml-property.md`, `devenv-summaryml-property.md`, `devenv-entitycaptionml-property.md`, `devenv-entitysetcaptionml-property.md`, `devenv-optioncaptionml-property.md`, `devenv-promotedactioncategoriesml-property.md`, `devenv-profiledescriptionml-property.md`, `devenv-requestfilterheadingml-property.md` | 0386 | 0 except `OptionCaptionML` 1 |
| `devenv-instructionaltext-property.md` | 0387 | 1 000 |
| `devenv-abouttitle-property.md`, `devenv-abouttext-property.md` | 0388 | 1 859, 2 119 |
| `devenv-additionalsearchterms-property.md` | 0389 | 665 |
| `devenv-entityname-property.md`, `devenv-entitysetname-property.md`, `devenv-entitycaption-property.md`, `devenv-entitysetcaption-property.md` | 0390 | 854, 854, 287, 272 |
| `devenv-summary-property.md` | 0391 | 855 |
| `devenv-description-property.md` | 0392 | 3 995 |
| `devenv-helplink-property.md`, `devenv-contextsensitivehelppage-property.md` | 0393 | 10, 53 |
| `devenv-title-property.md`, `devenv-flowcaption-property.md` | 0394 | **0**, **0** |
| `devenv-showcaption-property.md` | 0395 | 8 636 |
| `devenv-includecaption-property.md` | 0396 | 2 649 |
| `devenv-optioncaption-property.md`, `devenv-optionmembers-property.md`, `devenv-optionmembers-field-property.md` | **board:0053** | 3 905, 3 790 |
| `devenv-optionmembers-report-property.md` | 0397 | part of 3 790, not separable by grep |
| `devenv-optionordinalvalues-property.md` | 0398 | 310 |

The `*ML` pages grouped into 0386 are `tooltipml`, `instructionaltextml`, `abouttextml`,
`abouttitleml`, `additionalsearchtermsml`, `summaryml`, `entitycaptionml`, `entitysetcaptionml`,
`optioncaptionml`, `promotedactioncategoriesml`, `profiledescriptionml`, `requestfilterheadingml`.
The last three have plain twins that belong to later themes; the ML decision is the same for all
twelve and is taken once.

Five findings:

**`Caption` at 288 491 and `ToolTip` at 159 993 are the two largest populations in the sweep** --
together 448 484, seven and four times `TableRelation`'s 40 221. That decides the representation
before anything else: `constexpr string_view` in `.rodata`, nothing built at startup, nothing per
session.

**A table's own caption is hardcoded to its name.** `src/gen/TableWriter.cpp:551` writes
`.caption = <table>::kName`, so `Caption` works on a field and on an enum value and silently does
nothing on the table itself. One line, and a gate on a table whose caption differs from its name is
the only thing that shows it.

**The `ML` spelling is over.** Eight of the twelve measure 0 and the ninth measures 1. Checked rather
than assumed: the same pattern measures `Caption` at 288 491 on the same tree.

**`AboutTitle` and `AboutText` do not match -- 1 859 against 2 119.** The documentation says both must
be set for the teaching tip to appear, so by its own rule 260 tips do not appear. board:0388 lists
them when it is pulled rather than assuming a measurement artefact.

**A string property carries named arguments.** `devenv-additionalsearchterms-property.md` documents
`Locked`, `Comment` and `MaxLength` as parameters of the value, and the same three appear on `Caption`
and its relatives. A parser that read to the semicolon would take them as content -- recorded in
board:0389, where the documentation states it explicitly.

**One more contradiction of the `CDS` kind.** `OptionOrdinalValues` measures 310 against zero tables
of `TableType = CDS`, exactly as `ExternalName` measures 3 900 against the same zero. board:0398
waits on board:0365's resolution rather than guessing the same thing twice.

---

## `developer/properties/`, eighth theme -- what a user may do on a page

board:0030's subject, with two XMLport pairs that belong to board:0065. Twenty-two pages, sixteen
items.

**The IST-state for the whole theme is one line**: `src/gen/PageWriter.cpp` consumes `SourceTable` and
nothing else. There is no control list, no control descriptor and no page lifecycle, so every
property here is `fehlt` at the metadata.

| page | WI | population in %TREE%, 2026-09-04 |
|---|---|---:|
| `devenv-applicationarea-property.md` | 0399 | **186 502** |
| `devenv-editable-property.md` | 0400 | 51 886 |
| `devenv-visible-property.md` | 0401 | 48 225 |
| `devenv-enabled-property.md` | 0402 | 9 514 |
| `devenv-insertallowed-property.md`, `devenv-modifyallowed-property.md`, `devenv-deleteallowed-property.md` | 0403 | 2 101, 1 020, 1 863 |
| `devenv-delayedinsert-property.md` | 0404 | 1 048 |
| `devenv-linksallowed-property.md` | 0405 | 927 |
| `devenv-quickentry-property.md` | 0406 | 2 957 |
| `devenv-showmandatory-property.md` | 0407 | 2 165 |
| `devenv-multiline-property.md` | 0408 | 717 |
| `devenv-hidevalue-property.md` | 0409 | 184 |
| `devenv-unbound-property.md` | 0410 | 6 |
| `devenv-importance-property.md` | 0411 | 16 781 |
| `devenv-autosave-property.md`, `devenv-autoreplace-property.md`, `devenv-autoupdate-property.md` | 0412 | 209, 2, 68 |
| `devenv-savevalues-property.md` | 0413 | 1 690 |
| `devenv-refreshonactivate-property.md` | 0414 | 771 |

Five findings:

**`ApplicationArea` at 186 502 is the third-largest population in the sweep** -- behind `Caption` and
`ToolTip`, ahead of everything else. Its tag set is OPEN: `All`, `Basic`, `Suite`, `Advanced` are
"standard values", not the values, so it is a `string_view` list and not an enum, and `All` is a
wildcard rather than a tag.

**Three properties hide three different things and an implementation that shares one path breaks two
of them.** `Visible` removes the control from the fragment; `Enabled` keeps it and refuses input;
`HideValue` keeps the control and blanks the value, so the column keeps its width. Each item's
negative control is what the other two would break.

**`Enabled` applies to a TABLE KEY as well as to a control** -- a fact hidden in a list of fourteen UI
kinds. Against board:0345's finding that `src/rt/Storage.cpp:112` indexes every key unconditionally,
a disabled key is another index BC does not create, on top of the 158 `MaintainSqlIndex = false`
ones. And `include/Builtins.h:551` already records the neighbouring rule without naming this property.

**`AutoReplace` and `AutoUpdate` differ only in what happens to the fields the file does not
mention** -- reset to `InitValue`, or left alone -- and each page claims precedence over the other,
which cannot both be true. Recorded in board:0412 rather than resolved by preference.

**One property makes agiru's own caching rule visible.** `RefreshOnActivate` is BC's per-page opt-in
to not showing stale data, and CLAUDE.md says anything cached across a transaction without the
rowversion is stale by design. board:0414 asks whether agiru refreshes always and treats the property
as a no-op -- and says the deviation must be taken deliberately, with its own gate, because a
refresh-everything renderer makes BC's negative control pass for the wrong reason.

---

## `developer/properties/`, ninth theme -- how a page is laid out

board:0030's subject again, with one item under board:0033 and one under board:0034. Twenty-nine
pages, fourteen items. The same one-line IST-state covers all of them: `src/gen/PageWriter.cpp`
consumes `SourceTable` and nothing else.

| page | WI | population in %TREE%, 2026-09-04 |
|---|---|---:|
| `devenv-style-property.md`, `devenv-styleexpr-property.md` | 0415 | 1 579, 3 076 |
| `devenv-image-property.md`, `devenv-images-property.md` | 0416 | **46 008**, 6 |
| `devenv-shortcutkey-property.md` | 0417 | 5 070 |
| `devenv-ellipsis-property.md` | 0418 | 4 003 |
| `devenv-indentationcolumn-property.md`, `devenv-indentationcontrols-property.md`, `devenv-showastree-property.md`, `devenv-treeinitialstate-property.md` | 0419 | 181, 168, 46, 10 |
| `devenv-freezecolumn-property.md` | 0420 | 37 |
| `devenv-showfilter-property.md` | 0421 | 1 017 |
| `devenv-gridlayout-property.md`, `devenv-columnspan-property.md`, `devenv-rowspan-property.md` | 0422 | 32, 7, 6 |
| `devenv-width-property.md` | 0423 | 248 |
| `devenv-horizontalshrink-property.md`, `devenv-horizontalstretch-property.md`, `devenv-verticalshrink-property.md`, `devenv-verticalstretch-property.md`, `devenv-minimumwidth-property.md`, `devenv-maximumwidth-property.md`, `devenv-minimumheight-property.md`, `devenv-maximumheight-property.md`, `devenv-requestedwidth-property.md`, `devenv-requestedheight-property.md` | 0424 | 91 across ten, two of them **0** |
| `devenv-infooterbar-property.md` | 0425 | 492 |
| `devenv-gesture-property.md` | 0426 | 32 |
| `devenv-clearactions-property.md` | 0427 | 35 |
| `devenv-cuegrouplayout-property.md` | 0428 | 10 |

0424 groups `horizontalshrink`, `horizontalstretch`, `verticalshrink`, `verticalstretch`,
`minimumwidth`, `maximumwidth`, `minimumheight`, `maximumheight`, `requestedwidth`,
`requestedheight` -- one sizing model over two axes, on one object kind, where each page names the
others as its dependency.

Five findings:

**`Image` at 46 008 is the fourth-largest population in the sweep.** The name comes from a closed
platform icon list, so an unknown name is a translation error -- but the list is not on the page,
which links to an external reference instead. Where the list comes from is board:0416's first task,
and 46 008 declarations give a used set that is a lower bound and not the list.

**`StyleExpr` outnumbers `Style` two to one**, because its live form is a global page VARIABLE holding
the style NAME, set in `OnAfterGetRecord`. The documentation warns to "cover all cases in else
branches to avoid incorrect styles" -- the variable keeps the previous row's value, so a page that
colours row 3 red colours row 4 red too. Reproducing that is reproducing a documented footgun, and
board:0415's negative control is exactly that row.

**Four properties place an action and none of them knows about the others**: `Scope = Repeater`
(0362), `Importance = Promoted` (0411), `InFooterBar` (0425), and the default action bar. board:0425
resolves all of them into one placement enumerator in the generator so the renderer has no
combination left to get wrong.

**`ClearActions` breaks a rule the sweep had already recorded.** board:0379 measured that a permission
set EXTENSION may include and may not exclude -- an extension may only widen. A page customization
declaring `ClearActions` removes everything the base page declared. So "an extension only adds" is not
a general rule of AL, and the merge has to have a place for a subtractive step.

**Two documented no-ops and one documented inversion.** `RowSpan` is "not supported by the web
client... the property is ignored", so it is parsed and deliberately dropped -- and that has to be
recorded, because "we did not implement it" and "the web client ignores it" look identical in the
output. `MaximumWidth` and `MaximumHeight` measure 0, so every stretching add-in in the BaseApp can
stretch indefinitely by the documentation's own rule. And `Gesture`'s `LeftSwipe` is documented as a
swipe from the RIGHT edge -- an inversion a later reader will not re-check, so board:0426's gate
asserts it.

---

## `developer/properties/`, tenth theme -- what a page is and what it runs

board:0030's subject, with two items under board:0018 and board:0063. Twenty pages, eight items.

| page | WI | population in %TREE%, 2026-09-04 |
|---|---|---:|
| `devenv-pagetype-property.md` | 0429 | 6 891 |
| `devenv-subpagelink-property.md`, `devenv-subpageview-property.md` | 0430 | 3 487, 96 |
| `devenv-sourcetable-property.md`, `devenv-sourcetabletemporary-property.md` | 0431 | 6 295, 680 |
| `devenv-sourcetableview-property.md`, `devenv-sourcetableview-pages-property.md`, `devenv-sourcetableview-xmlports-property.md` | 0432 | 1 275 |
| `devenv-runobject-property.md`, `devenv-runpagelink-property.md`, `devenv-runpageview-property.md`, `devenv-runpagemode-property.md`, `devenv-runpageonrec-property.md` | 0433 | **33 486**, 6 294, 3 193, 1 232, 387 |
| `devenv-cardpageid-property.md` | 0434 | 599 |
| `devenv-navigationpageid-property.md` | 0435 | **0** |
| `devenv-previewmode-property.md`, `devenv-promptmode-property.md`, `devenv-userequestpage-property.md`, `devenv-processingonly-property.md` | 0436 | 223, 2, 140, 767 |

Six findings:

**`PageType` is nineteen values and four of them are 71 % of the BaseApp.** 6 891 declarations: `List`
2 740, `Card` 923, `ListPart` 817, `Document` 429 -- 4 909 together. `PromptDialog`,
`ConfigurationDialog`, `ReportPreview`, `ReportProcessingOnly` and `XmlPort` do not appear at all.
Four renderers cover seven pages in ten; five values are refused with their zero measured.

**`RunObject` at 33 486 is the fifth-largest population in the sweep** -- most actions in the BaseApp
run an object rather than calling code.

**`RunPageLink` and `RunPageView` differ only in whether the USER can lift the filter.** The pages say
it in opposite sentences: the link's filters are "visible in the UI and can be modified by
end-users", the view's are "not visible and cannot be modified". Merging them lets a user widen a
filter BC hides, which every "the right rows appear" gate passes.

**Half the links violate the documentation's own performance rule.** It says the `RunPageView` sort
"must contain the fields listed in `RunPageLink` or else the performance is decreased", and there are
6 294 links against 3 193 views. Both are declarations, so the relation is decidable at translation
time -- board:0045's index rule as a property relation.

**A list page's New, Delete and Edit come from its CARD page, not from itself.** board:0403 files
those three as page properties; `devenv-cardpageid-property.md` says that on a list with a
`CardPageId` the runtime reads them from the linked card. 599 list pages, and the failure is an action
that appears or disappears rather than an error.

**The common report shape needs no renderer.** `ProcessingOnly` measures 767 -- more than CLAUDE.md's
668 in-scope reports, which is itself a thing board:0436 checks -- against 223 `PreviewMode`. A
processing-only report runs its data items and produces nothing, so board:0063's first deliverable is
that path and not the XSL-FO one.

**One more zero.** `NavigationPageId` measures 0 against `LookupPageId`'s 2 294 and
`DrillDownPageId`'s 1 822 -- three page-id properties on a field for three different interactions, and
nobody overrides the navigation default.

---

## `developer/properties/`, eleventh theme -- how a value is formatted

board:0066's subject, with three items under board:0080, board:0082 and board:0064. Nine pages, five
items.

| page | WI | population in %TREE%, 2026-09-04 |
|---|---|---:|
| `devenv-autoformattype-property.md`, `devenv-autoformatexpression-property.md` | 0437 | **40 808**, 23 126 |
| `devenv-formatregion-property.md`, `devenv-culture-property.md` | 0438 | **0**, 47 |
| `devenv-sqldatatype-property.md` | 0439 | **0** |
| `devenv-dateformula-property.md` | 0440 | 5, ambiguous |
| `devenv-reversesign-property.md` | 0441 | 11 |

**`AutoFormatType` at 40 808 is the sixth-largest population in the sweep**, above `TableRelation`'s
40 221 -- every Decimal field in an ERP declares how it is formatted. And the resolution is AL:
board:0066 already measured the `Auto Format` codeunit's `OnResolveAutoFormat`, so this is
board:0384's `CaptionClass` shape and a switch on the type value in `src/` would break the invariant
that the runtime knows no AL object. The page documents three of the six type values; `3`, `10` and
`11` are explained only in the root page `devenv-format-field-data.md`, which is read separately.

**`SqlDataType` measures 0 and that removes a question from board:0080** rather than adding work: no
Code field in the BaseApp declares a non-`Varchar` SQL type, so text ordering is the only case and a
numeric-ordering path is not needed.

**`DateFormula` is the sweep's one ambiguous count.** The same spelling declares the deprecated
PROPERTY and declares a field of the TYPE, and no statement-boundary pattern separates them. The
property's population is between 0 and 5, and board:0440's decision depends on which -- so the item's
first task is the count, and it does not pre-decide it.

---

## `developer/properties/`, twelfth theme -- the XMLport

board:0065's subject, and the first theme in this sweep whose whole object kind is missing rather
than partly built. Twenty pages, eight items.

| page | WI | population in %TREE%, 2026-09-04 |
|---|---|---:|
| `devenv-format-property.md` | 0442 | 272 |
| `devenv-fieldseparator-property.md`, `devenv-fielddelimiter-property.md`, `devenv-recordseparator-property.md`, `devenv-tableseparator-property.md` | 0443 | 189, 47, 140, 10 |
| `devenv-minoccurs-property.md`, `devenv-maxoccurs-property.md`, `devenv-occurrence-property.md` | 0444 | 887, 227, 502 |
| `devenv-namespaces-property.md`, `devenv-namespaceprefix-property.md`, `devenv-defaultnamespace-property.md`, `devenv-usedefaultnamespace-property.md` | 0445 | 15, **2 473**, 29, 34 |
| `devenv-direction-property.md` | 0446 | 303 |
| `devenv-inlineschema-property.md`, `devenv-uselax-property.md`, `devenv-preservewhitespace-property.md`, `devenv-xmlversionno-property.md` | 0447 | 2, 2, 11, **0** |
| `devenv-xmlname-property.md`, `devenv-texttype-property.md`, `devenv-encoding-property.md`, `devenv-textencoding-property.md`, `devenv-width-xmlport-property.md`, `devenv-filename-property.md` | 0448 | 3 587, 47, 80, 39, 248, 6 |
| `devenv-usetemporary-property.md`, `devenv-usetemporary-report-property.md`, `devenv-usetemporary-xmlport-property.md` | 0449 | 336 |

Five findings:

**`Format` is a discriminator over half the XMLport property list**, and the other pages say so
themselves: `TextEncoding` is "only available when Format is Fixed Text or Variable Text",
`FieldDelimiter` "only used if Format is Variable Text. Otherwise the setting is ignored",
`RecordSeparator` likewise. Every one of those conditions is decidable at translation time.

**Two encodings, two defaults, and one is not Unicode.** `Encoding` (XML) defaults to UTF-8 with BOM;
`TextEncoding` (text files) defaults to **MSDOS**. So a text XMLport that declares nothing writes a
lossy single-byte file, and board:0074 has to be honest about that rather than defaulting to UTF-8
because it is nicer.

**`<NewLine>` is a SET and not a string** -- "any combination of CR and LF" -- so reading it as `\n`
splits a CRLF file into empty rows, and the separator values compose (`'<CR><LF>x'` is legal). Those
properties are also ASSIGNABLE at run time, which makes them the sweep's first mutable object state
rather than `constexpr` metadata.

**Two defaults that disagree.** `Direction` defaults to `Both`; an XMLport with no request page
defaults to `Import`. The documentation states both, and an implementation with one default gets one
of the two cases backwards.

**`UseTemporary` is the fourth way to declare the same thing.** With `TableType = Temporary` (0364),
`SourceTableTemporary` (0431) and this property on two object kinds, there are **1 314 declarations of
temporariness across four properties** -- which is what sizes board:0032, and why all four must reach
one mechanism.

---

## `developer/properties/`, thirteenth theme -- the report

board:0063's subject, with one item under board:0064 and one under board:0012. Thirty-one pages,
eleven items.

| page | WI | population in %TREE%, 2026-09-04 |
|---|---|---:|
| `devenv-dataitemlink-property.md`, `devenv-dataitemlink-reports-property.md`, `devenv-dataitemlinkreference-property.md` | 0450 | 2 023, 896 |
| `devenv-dataitemtableview-property.md` | 0451 | **7 710** |
| `devenv-rdlclayout-property.md`, `devenv-wordlayout-property.md`, `devenv-excellayout-property.md`, `devenv-layoutfile-property.md`, `devenv-defaultlayout-property.md`, `devenv-defaultrenderinglayout-property.md`, `devenv-sharedlayout-property.md`, `devenv-clearlayout-property.md` | 0452 | 768, 2, 1, 884, 730, 646, **0**, 33 |
| `devenv-dataitemtablefilter-property.md`, `devenv-columnfilter-property.md`, `devenv-dataitemlink-query-property.md` | 0453 | 230 |
| `devenv-requestfilterfields-property.md`, `devenv-requestfilterheading-property.md` | 0454 | 1 944, 425 |
| `devenv-allowscheduling-property.md`, `devenv-showprintstatus-property.md`, `devenv-usesystemprinter-property.md`, `devenv-maximumdatasetsize-property.md`, `devenv-maximumdocumentcount-property.md`, `devenv-topnumberofrows-property.md` | 0455 | 25, 2, 2, 12, 1, 4 |
| `devenv-papersourcedefaultpage-property.md`, `devenv-papersourcefirstpage-property.md`, `devenv-papersourcelastpage-property.md`, `devenv-pdffontembedding-property.md` | 0456 | **0**, **0**, **0**, **0** |
| `devenv-wordmergedataitem-property.md`, `devenv-excellayoutmultipledatasheets-property.md` | 0457 | 299, 17 |
| `devenv-transactiontype-property.md` | 0458 | 5 |
| `devenv-executiontimeout-property.md` | 0459 | 1 |
| `devenv-analysismodeenabled-property.md`, `devenv-clearviews-property.md` | 0460 | 58, 8 |

Six findings:

**The BaseApp is RDL.** 768 `RDLCLayout` declarations against **2** Word layouts and **1** Excel, so
CLAUDE.md's XSL-FO-through-FOP route covers 768 of 771 and the other two formats are three files.

**Both layout generations are live at once**: 768 old-form `RDLCLayout` against 884 new-form
`LayoutFile`, so a report generator must read both and board:0452 reconciles them into one list.

**A report link IS a `SetRange`, and the documentation says so.** `DataItemLink`'s page gives the
equivalent `OnPreDataItem` code, so a report's nesting is a nested loop with a filter and never a
join. And the reference may point at a GRANDPARENT, which is board:0450's negative control.

**A query's three filter sources have two combination rules.** `DataItemTableFilter` ANDs with
everything and cannot be removed; `ColumnFilter` ANDs with it but is OVERWRITTEN by an AL `SetFilter`
on the same field. One filter list per field gets the second wrong, and the failure is a wider result
set that looks plausible.

**Two limits with different authority, twice.** `MaximumDatasetSize` overrides one server row limit
and cannot override another; `ExecutionTimeout` does the same for time. Both are three-way
precedences, and an implementation that keeps one number gets the hard limit wrong. Neither server
setting exists here, so **agiru currently has no report limit of any kind** -- board:0459's finding.

**`WordMergeDataItem` measures 299 against 2 Word layouts**, which cannot both describe the same
reports. board:0457 makes resolving that its first task rather than assuming a measurement artefact:
the likely explanation is that layouts are added by users and extensions rather than declared, and
that says something about how BC reports are used.

---

## `developer/properties/`, fourteenth theme -- the query, the API and the enum's interface

board:0064's subject, with items under board:0012, board:0027, board:0083 and board:0084.
Twenty-five pages, nine items.

| page | WI | population in %TREE%, 2026-09-04 |
|---|---|---:|
| `devenv-sqljointype-property.md` | 0461 | 216 |
| `devenv-method-property.md` | 0462 | 268 |
| `devenv-readstate-property.md` | 0463 | 8 |
| `devenv-querytype-property.md`, `devenv-querycategory-property.md`, `devenv-columnfilter-property.md`, `devenv-orderby-property.md` | 0464 | 346, 136, 214, 113 |
| `devenv-apipublisher-property.md`, `devenv-apipublisher-page-property.md`, `devenv-apipublisher-query-property.md`, `devenv-apigroup-property.md`, `devenv-apigroup-page-property.md`, `devenv-apigroup-query-property.md`, `devenv-apiversion-property.md`, `devenv-apiversion-page-property.md`, `devenv-apiversion-query-property.md`, `devenv-odatakeyfields-property.md`, `devenv-odataedmtype-property.md` | 0465 | 481, 295, 295, 349, 34 |
| `devenv-usagecategory-property.md` | 0466 | **3 378** |
| `devenv-implementation-property.md`, `devenv-defaultimplementation-property.md`, `devenv-unknownvalueimplementation-property.md` | 0467 | 571, 40, 9 |
| `devenv-assignmentcompatibility-property.md`, `devenv-assignmentcompatibilityreason-property.md` | 0468 | 573, **0** |
| `devenv-multiplicity-property.md` | 0469 | 83 |

0465 groups `apipublisher`, `apipublisher-page`, `apipublisher-query`, `apigroup`, `apigroup-page`,
`apigroup-query`, `apiversion`, `apiversion-page`, `apiversion-query`, `odatakeyfields` and
`odataedmtype`.

Five findings:

**`DataItemLink` means two different things on two object kinds.** board:0450 records that a REPORT's
link is a `SetRange` and a nested loop, with the documentation's own equivalent code; a QUERY's is a
real SQL join and `SqlJoinType` names which of five. Same property name, two executions -- the
`Scope` situation again, with the divergence in the semantics rather than the values.

**`Average` over an integer column TRUNCATES.** "5÷2=2 instead of 2.5", by the documentation. A direct
translation to PostgreSQL's `avg(integer)` returns numeric and gives 2.5, which is the silent numeric
difference CLAUDE.md's determinism invariant exists for. And board:0462's negative control is a
DECIMAL column, which must not truncate.

**`ReadState` is an exception to board:0012's isolation state machine and has to be built as one.**
Queries ignore `CurrentTransactionType`; two queries in one transaction may read at different levels;
the strictest lock nevertheless persists. None of that is derivable from board:0012's own reference.

**Less than half the BaseApp's UI objects are findable.** `UsageCategory` measures 3 378 declarations
against roughly 7 900 pages, reports and queries -- and an object that declares nothing is not in the
search catalogue at all. That is board:0083's population, and a search over the whole catalogue would
return twice what BC returns.

**573 enums are assignable-from and not one says why.** `AssignmentCompatibility` measures 573,
`AssignmentCompatibilityReason` measures **0** -- so the warning text exists in AL and Microsoft never
declares one, which licenses the transpiler to compose the warning itself. That is a deviation from
"a diagnostic is a declared label", taken deliberately because no label is ever declared.

---

## `developer/properties/`, fifteenth theme -- the codeunit, the test and the rest of the page

Items under board:0039, board:0062, board:0037, board:0030, board:0034 and board:0067. Fifty-three
pages, seventeen items -- the sweep's largest theme by page count, because it collects every remaining
object kind's own declarations.

| page(s) | WI | population in %TREE%, 2026-09-04 |
|---|---|---:|
| `devenv-testisolation-property.md`, `devenv-requiredtestisolation-property.md`, `devenv-testtype-property.md` | 0470 | 7, 64, 564 |
| `devenv-singleinstance-property.md`, `devenv-eventsubscriberinstance-property.md` | 0471 | 866, 1 380 |
| `devenv-subtype-property.md`, `devenv-subtype-codeunit-property.md`, `devenv-subtype-blob-property.md` | 0472 | 4 589 |
| `devenv-testpermissions-property.md` | 0473 | 3 745 |
| `devenv-assistedit-property.md`, `devenv-updatepropagation-property.md`, `devenv-populateallfields-property.md`, `devenv-pasteIsvalid-property.md` | 0474 | 221, 607, 143, 31 |
| `devenv-filters-property.md` | 0475 | 179 |
| `devenv-customactiontype-property.md`, `devenv-flowid-property.md`, `devenv-flowtemplateid-property.md`, `devenv-flowtemplatecategoryname-property.md`, `devenv-flowenvironmentid-property.md` | 0476 | 150 |
| `devenv-promoted-property.md`, `devenv-promoted-action-property.md`, `devenv-promotedcategory-property.md`, `devenv-promotedisbig-property.md`, `devenv-promotedonly-property.md`, `devenv-promotedactioncategories-property.md` | 0477 | 1 228, 983, 488, 572, 64 |
| `devenv-isheader-property.md`, `devenv-groupname-property.md` | 0478 | 272, 16 |
| `devenv-scripts-property.md`, `devenv-startupscript-property.md`, `devenv-recreatescript-property.md`, `devenv-refreshscript-property.md`, `devenv-stylesheets-property.md`, `devenv-definitionfile-property.md` | 0479 | not taken -- the item says so |
| `devenv-allowincustomizations-property.md` | 0480 | 217 |
| `devenv-externaltype-property.md`, `devenv-externalaccess-property.md`, `devenv-provider-property.md`, `devenv-publickeytoken-property.md`, `devenv-iscontroladdin-property.md`, `devenv-enableexternalassemblies-property.md` | 0481 | 3 789, 1 752, 345, 45, 1 |
| `devenv-rolecenter-property.md`, `devenv-profiledescription-property.md`, `devenv-customizations-property.md`, `devenv-enabled-profile-property.md`, `devenv-promoted-profile-property.md`, `devenv-profile-properties.md` | 0482 | 59, 55, 4 |
| `devenv-type-property.md`, `devenv-type-entitlement-property.md`, `devenv-roletype-property.md`, `devenv-type-report-property.md` | 0483 | 56; `Type` not separable |
| `devenv-properties.md`, `devenv-table-property-overview.md`, `devenv-page-property-overview.md`, `devenv-report-property-overview.md`, `devenv-codeunit-properties.md`, `devenv-enum-properties.md`, `devenv-key-properties.md`, `devenv-query-properties.md`, `devenv-xmlport-properties.md`, `devenv-view-properties.md`, `devenv-control-addin-properties.md`, `devenv-profile-properties.md`, `devenv-report-properties.md`, `devenv-demolicense-properties.md` | 0484 | 349 pages, 9 consumed |
| `devenv-allowedfileextensions-property.md`, `devenv-allowmultiplefiles-property.md`, `devenv-fileuploadaction-property.md`, `devenv-fileuploadrowaction-property.md` | 0485 | 5, 12, **0**, **0** |
| `devenv-mimetype-property.md`, `devenv-version-property.md`, `devenv-id-property.md` | 0486 | 203, 1 |

0484 groups `devenv-properties.md`, the three `*-property-overview.md` pages and the nine
`*-properties.md` per-kind pages, plus `devenv-demolicense-properties.md`.

Six findings:

**`TestIsolation` rolls back a `Commit`.** "ALL database changes are rolled back, INCLUDING changes
that were explicitly committed during the test." That is a documented exception to CLAUDE.md's first
invariant, it is compulsory for the 2 291, and it needs a mode reachable only from the test runner --
because the same path in a posting would be the invariant broken.

**866 single-instance codeunits are per SESSION, not per process.** "The codeunit remains instantiated
until you close the company" is the lifetime, so a single-instance codeunit is the one AL construct
that looks like a global and must not be one. board:0471's negative control is a second session, where
a process-wide instance is a cross-tenant data leak.

**The generator acts on one of `SubType`'s five codeunit values.** `src/gen/CodeunitWriter.cpp:62`
compares `LowerKey(subtype->text) == "test"`; `TestRunner`, `Upgrade` and `Install` fall through to
`Normal` silently, and so would a typo. An enumerator makes the five exhaustive.

**`TestPermissions` is applied by AL, not by the platform** -- the value is passed to the runner's
`OnBeforeTestRun` and `Library - Lower Permissions` does the work. Third property in this sweep with
that shape, after `CaptionClass` and `AutoFormatType`.

**9 751 declarations point at a table type that measures zero.** `ExternalName` 3 900, `ExternalType`
3 789, `ExternalAccess` 1 752, `OptionOrdinalValues` 310, against zero `TableType = CDS` or
`ExternalSQL`. board:0365 owns resolving it and board:0481 waits rather than guessing a fourth time.

**Fourteen index pages become one item rather than fourteen ledger rows**, because the three overview
pages are not link lists: each is a table of property, **Extensible**, and every element kind it
applies to. That is board:0067's census written by Microsoft, in a form a script can read, and this
sweep's hand-built ledger is its negative control.

---

## `developer/properties/`, sixteenth theme -- the remainder, and the family closed

Three items over ten pages, and with them **all 349 pages of `developer/properties/` are read.**

| page(s) | WI | population in %TREE%, 2026-09-04 |
|---|---|---:|
| `devenv-showas-property.md` | 0487 | 882 |
| `devenv-maxIteration-property.md`, `devenv-printonlyifdetail-property.md`, `devenv-multiplenewlines-property.md`, `devenv-ispreview-property.md` | 0488 | 196, 489, 247, 10 |
| `devenv-enableexternalimages-property.md`, `devenv-enablehyperlinks-property.md`, `devenv-testhttprequestpolicy-property.md`, `devenv-tableno-property.md`, `devenv-formatevaluate-property.md` | 0489 | 2, 23, 45, 1 341, 144 |

**A split button's default action is the first VISIBLE AND ENABLED one, not the first declared** -- so
it is resolved at render time from board:0401's and board:0402's bits, and a page extension reordering
the group silently changes which action a one-click press runs. The documentation flags that as a
hazard rather than a feature. 882 declarations.

**Three properties ask one boundary question and their defaults disagree.** A report defaults to
REFUSING external images and hyperlinks; a test codeunit defaults to ALLOWING all outbound HTTP.
So an agiru test suite would reach the network by default -- a hermeticity hole in the 2 291 --
and board:0489 records that `agiru run-tests` should consider blocking as its own default, which is a
deliberate deviation from BC.

---

# `developer/properties/` -- closed

**349 pages read. 173 work items filed (0317-0489, minus the numbers used elsewhere), plus four pages
whose task a pre-existing root already is.**

| | |
|---|---:|
| pages | **349** |
| items filed by this sweep | **173** |
| pages routed to a pre-existing root | 4 -- `ObsoleteState` (0069), `DataPerCompany` (0060), `OptionCaption`/`OptionMembers`/`OptionMembers-field` (0053) |
| properties the generator consumes today | **9** |

**The single most repeated finding**: `src/gen/PageWriter.cpp` consumes `SourceTable` and nothing
else, so of the 349 properties, every one that lives on a page element is `fehlt` at the metadata.
board:0030 is the root under which most of this family sits.

**The five largest populations in the family**, and each one decided a design in its item:

| property | declarations | why it mattered |
|---|---:|---|
| `Caption` | 288 491 | `constexpr` `.rodata`, never built at startup |
| `ApplicationArea` | 186 502 | an OPEN tag set, so `string_view` and not an enum |
| `ToolTip` | 159 993 | one member on a descriptor that has to exist anyway |
| `AutoFormatType` | 40 808 | resolved by AL codeunit 42, never by `src/` |
| `TableRelation` | 40 221 | a grammar parsed by the generator, never at run time |

**Fourteen properties measure zero** and are refused on the same arithmetic each time:
`SignDisplacement`, `TestTableRelation`, `SqlIndex`, `ColumnStoreIndex`, `Scope`(enum/interface),
`LinkedObject`, `LinkedInTransaction`, `Title`, `FlowCaption`, `NavigationPageId`, `SqlDataType`,
`XmlVersionNo`, `SharedLayout`, `AssignmentCompatibilityReason`, and the four printer properties.

**Nine properties are `teilweise` or `deklariert`** -- the tree already does part of what the page
says: `InitValue`, `Clustered`, `SourceTable`, `Caption`, `TableRelation`, `FieldClass`, `SubType`,
`Implementation`, `TableNo`, `TestPermissions`.

---

# `developer/` root -- 470 pages

**The concept pages, and CLAUDE.md's reference #1.** They say what the PLATFORM guarantees -- validate
order, trigger lifecycle, transaction behaviour, system fields -- and that is not in the AL source.
The `properties/` family is read against them, not the other way round.

## First pass: the sixteen pages the properties sweep deferred to by name

`properties/` produced 173 items and sixteen of them parked an open question on a named root page.
Those sixteen are read first, and each one closed a question rather than opening one.

| page | WI | what it settled |
|---|---|---|
| `devenv-tri-state-locking.md` | 0490 | the three isolation states, and that the legacy scheme is not a target |
| `devenv-format-field-data.md` | 0491 | board:0437's gap -- the five `AutoFormatType` values and the expression grammar |
| `devenv-permissionset-composing.md`, `devenv-permissionset-object.md` | 0492 | board:0379's open question -- the composition truth table |
| `devenv-test-codeunits-and-test-methods.md` | 0493 | board:0470's -- a transaction per test METHOD is the default |
| `devenv-httpclient-mock-outbound-calls.md` | 0494 | board:0200's -- a handler intercepts by default |
| `devenv-testing-with-permission-sets.md` | 0495 | board:0473's -- and that security filters are not tested at all |
| `devenv-progress-windows-message-error-and-confirm-methods.md` | 0496 | `Message` is asynchronous |
| `devenv-secret-text.md` | 0497 | the compiler's literal prohibition |
| `devenv-page-background-tasks.md` | 0498 | the read-only child session and its thirteen rules |
| `devenv-inherent-permissions.md` | 0499 | the grant's lifetime -- push on entry, pop on exit |
| `devenv-extension-install-code.md`, `devenv-upgrading-extensions.md` | 0500 | board:0270-0277's -- a session per trigger per company |
| `devenv-request-pages.md` | 0501 | board:0454's -- one request page, two object kinds |
| `devenv-dotnet-subscribe-to-events.md` | 0502 | a trigger syntax the parser must accept even to refuse |
| `devenv-format-property.md` | 0503 | board:0066's core -- the `Format` grammar |

Seven findings from the sixteen:

**`TestIsolation` and the per-method transaction are two mechanisms, not one.** Every test method
already runs in its own transaction by default; isolation decides whether a `Commit` inside it
survives. Getting that wrong in either direction breaks the 2 291 -- no per-method transaction and
every test sees the last one's rows; rollback where none is declared and a test loses its fixture.

**The permission composition is a lattice and its truth table turns on ONE LETTER'S CASE.** Excluding
an indirect `i` leaves a direct `I` standing; excluding a direct `I` removes the indirect `i` too.
Rows 3 and 4 of the documentation's own table differ only in case, so any implementation that
normalises case gives the same answer for both -- board:0376 warned about this from the property side
and here is the table that proves it.

**An `HttpClientHandler` returns `false` by default, and `TestHttpRequestPolicy` allows by default.**
The two defaults pull in opposite directions: a test with a handler intercepts everything, a test
without one may reach the network. Both are documented.

**Security filters are not tested by BC's own suite** -- Microsoft says so in one line. So board:0062's
filtering work cannot be proven by a green 2 291 and needs a hand-written gate.

**`Message` is asynchronous.** It does not run until the calling method ends or something requests
input, so a procedure that reports progress and then raises shows nothing. `[MessageHandler]` reads
that queue, not the call.

**An upgrade runs each `PerCompany` trigger in its OWN system session, and `PerDatabase` in a session
with NO company open.** Neither is on the trigger pages. And BC explicitly does not guarantee an order
between codeunits -- agiru must declare one, which makes it more deterministic than BC and is recorded
as a deviation.

**Format 2 and format 9 are the culture-free renderings**, and the standard-format tables show how:
format 2 is format 1 with an explicit `<Comma,.>`. Date format 2 is also the AL code-constant form,
which is what `Evaluate` reads back -- so format 2 across types is the round-trip format board:0066's
`Evaluate` half depends on.

---

## `developer/` root, second pass -- the record primitives

The pages that describe what a `Record` DOES. These are the most load-bearing concept pages in the
tree: board:0018, board:0044, board:0047, board:0055 and board:0056 are all written against them.

| page | WI |
|---|---|
| `devenv-get-find-and-next-methods.md` | 0504 |
| `devenv-insert-modify-modifyall-delete-and-deleteall-methods.md` | 0505 |
| `devenv-calcfields-calcsums-fielderror-fieldname-init-testfield-and-validate-methods.md` | **0506 and 0507** |
| `devenv-setcurrentkey-setrange-setfilter-getrangemin-and-getrangemax-methods.md` | 0508 |
| `devenv-entering-criteria-in-filters.md` | 0509 |
| `devenv-flowfields.md`, `devenv-flowfilter-overview.md`, `calculate-only-visible-flowfields-feature-key.md` | 0510 |
| `devenv-table-system-fields.md` | 0511 |

**One page is deliberately SPLIT into two items** -- the field-methods page covers seven methods in
two unrelated subjects, error wording (board:0055) and aggregate calculation (board:0047), and one WI
is one theme. That is the first split in this sweep and it is recorded here rather than left implicit.

Six findings:

**The value-context rule is a PLATFORM rule, stated twice.** `Get` "produces a runtime error if it
fails and the return value isn't checked"; the write methods say the same for the whole family. So
CLAUDE.md's inherited "value context" failure mode has its documentation, and the contexts it names --
assignment, `if`/`while`, `exit`, argument, `case` selector -- are the guard.

**`Get` ignores the record's filters and `Find` uses them.** The natural implementation of `Get` --
apply the filters and add the primary key -- is wrong, silently, and only for filtered variables.

**`DeleteAll(true)` runs `OnDelete` against a COPY of the variable with its initial values, and the
documentation says there is NO performance difference from `Delete(true)` in a loop.** So the
simplification has nothing to trade: an implementation that loops lets the trigger see caller state.

**`Truncate` has seven documented refusals and each names another board item** -- temporary and
non-Normal tables, try functions, security filters, FlowField filters, marked records, delete-event
subscribers, media fields. It is a good measure of how much runtime exists, because it cannot be
implemented before those seven conditions can be ASKED.

**`GetRangeMin` raises when the current filter is not a range.** With board:0474's
`PopulateAllFields` -- "evaluates to exactly one value" -- that is twice the runtime must interrogate a
filter's SHAPE. It settles a design point for board:0018: the parsed filter is a structure and the SQL
is derived from it, not the reverse.

**The audit fields are stamped between the triggers** -- after `OnInsert`, before `OnAfterInsert` -- so
a table's own trigger reads blanks and a subscriber reads values. Not derivable from the trigger
pages, and it constrains board:0057's dispatch.

And one deviation taken deliberately: **BC calculates FlowFields for controls that are not visible**,
says so, and ships the fix behind a feature flag. board:0510 implements the FEATURE rather than the
default, because the default is documented as a performance defect and no test asserts it.

---

## `developer/` root, third pass -- events

board:0057's subject. Ten pages, five items. The event mechanism is 11 142 subscriber declarations and
the single largest extensibility surface in the BaseApp.

| page(s) | WI |
|---|---|
| `devenv-events-in-al.md`, `devenv-event-types.md` | 0512 |
| `devenv-subscribing-to-events.md` | 0513 |
| `devenv-events-isolated.md` | 0514 |
| `devenv-publishing-events.md`, `devenv-raising-events.md` | 0515 |
| `types-of-events-for-extensibility.md`, `devenv-use-ishandled-pattern.md`, `devenv-use-ishandled-min-req.md` | 0516 |
| `devenv-events-discoverability.md` | - | the Event Recorder, a VS Code and client tool for finding events to subscribe to; no runtime behaviour |
| `business-events-overview.md`, `devenv-deprecate-external-business-events.md` | - | external business events through Dataverse and Power Automate -- a cloud bridge, which `scope.json` excludes |

Six findings:

**A "global event" is an ordinary integration event in a named BaseApp codeunit.** The page lists them
with codeunit ids -- 9170 Conf./Personalization Mgt., 42 TextManagement, 42 Caption Class, 44
ReportManagement. So the runtime must NOT know these names; it raises by publisher identity from the
generated catalogue. That is the strongest confirmation in this sweep of CLAUDE.md's invariant, and it
settles board:0384's and board:0437's shape: the runtime supplies the RAISE POINT, the transpiled
BaseApp supplies the handler.

**There are FIVE database-trigger-event signatures, not one.** Insert and delete carry no `xRec`;
modify and rename do; the two validate events take `CurrFieldNo` where the others take `RunTrigger`.
A dispatcher with one signature passes a Boolean where an Integer belongs.

**BC declines to order subscribers and agiru must order them anyway.** "You can't specify the order"
against "anything assembled from concurrent work is combined in a DECLARED order". 11 142 subscribers,
so the choice is between non-determinism at 11 142 points and a deviation: object id, then procedure
name, sorted by the generator.

**A raise with no subscribers is not executed AT ALL** -- and neither are its arguments. That is
semantics, not an optimisation: an argument with a side effect does not happen. And because agiru
merges extensions at translation time it knows the whole subscriber set, so a statically empty raise
can be emitted as nothing.

**An isolated event is conditional on there being no pending write.** Raised inside an uncommitted
write transaction it degrades silently to a normal event -- which is what keeps the posting invariant
intact, and is a rule an implementation would never invent. Only `TableType: Normal` changes roll back;
`var` parameters, single-instance members and HTTP calls do not.

**The eight event patterns are AL conventions over one mechanism, and the runtime owes none of them.**
Recorded as a decision rather than an omission -- what they need is `var` by reference, ordered
dispatch, manual binding and isolation, all of which are already items. The handled pattern is where a
copied `var` fails silently: every subscriber sets a copy, the publisher reads `false`, and the default
code runs IN ADDITION to the substituted behaviour. A duplicate posting, with nothing raised.

---

## `developer/` root, fourth pass -- errors

board:0055's subject and board:0061's. Seven pages, three items.

| page(s) | WI |
|---|---|
| `devenv-handling-errors-using-try-methods.md` | 0517 |
| `devenv-error-collection.md`, `devenv-error-collection-api.md` | 0518 |
| `devenv-error-dialog.md`, `devenv-actionable-errors.md`, `devenv-error-handling-guidelines.md` | 0519 |
| `devenv-al-error-handling.md` | - | an index over the four error features, each of which has its own page and item |
| `devenv-extension-errors-recommendations.md` | - | AppSource submission advice on error wording; no runtime obligation |

Five findings:

**A `[TryFunction]` catches only when its return value is used, so the SAME procedure is a try function
or not depending on the CALL SITE.** The attribute does not make it one. That is not a C++ shape --
a function either has a `try` in it or does not -- so the `try` is emitted at the CALL SITE, and the
generator must resolve the value context of every call. CLAUDE.md already names that guard.

**And the arguments are evaluated INSIDE the try function**, which is the opposite of C++'s order. So
the emitted `try` wraps argument evaluation and the call together, not just the call.

**On-premises, a database write inside a try function raises by default.** agiru is on-premises, so
that is the behaviour -- and the reason is stated: changes made inside a try method are NOT rolled
back, which under the posting invariant is unacceptable. board:0514's isolated events reach the same
place and answer it with savepoints; BC's answer here is to forbid the write.

**Clearing collected errors does not roll anything back.** A procedure can collect five errors, clear
them, and keep every write the failing paths performed. BC's own advice is to wrap it in
`Codeunit.Run` -- which is board:0077's boundary. So collecting changes error PROPAGATION and never
transaction scope.

**`ErrorInfo` carries `TableId`, `FieldNo`, `RecordId`, `SystemId` and `PageNo`** -- an error is
addressable to a record and a field. That is what board:0506's `TestField` navigation and board:0519's
Fix-it actions both consume, and it makes `ErrorInfo` the structured error type board:0055 needs.

One requirement named without an item, because it belongs to no page: **`ErrorInfo.Callstack()` and the
Copy Details section specify an AL call stack with LINE NUMBERS** -- `Report1(Report 50101).OnPostReport(Trigger) line 2`.
Nothing in this tree emits AL line numbers, and doing so is a generator decision with a cost across
7 885 translation units. board:0519 records it as its largest open question rather than answering it.

---

## `developer/` root, fifth pass -- keys, SIFT, and the two tables that are not tables

board:0045's subject, board:0343's and board:0032's. Twelve pages, four items.

| page(s) | WI |
|---|---|
| `devenv-table-keys.md` | 0520 |
| `devenv-sift-technology.md`, `devenv-sift-and-sql-server.md`, `devenv-sift-performance.md`, `devenv-sift-tuning-and-tracing.md`, `devenv-migrating-from-sift-to-ncci.md` | 0521 |
| `devenv-temporary-tables.md` | 0522 |
| `devenv-virtual-tables.md`, `devenv-integer-virtual-table.md`, `devenv-date-virtual-table.md`, `devenv-extend-pages-based-on-date-virtual-table.md` | 0523 |
| `devenv-table-object.md`, `devenv-tables-overview.md` | - | object-syntax overviews; every element they describe has its own property or trigger page and item |

Six findings:

**A primary key of 17 to 20 fields COMPILES in AL and silently uses 16.** So two records differing only
in fields 17 and 18 collide. board:0520 asserts at 16 anyway -- refusing what BC truncates is the only
way the truncation cannot happen, and that is a deliberate deviation.

**SIFT is an indexed view per key, at the FINEST granularity, and a read is still a SUM.** The
documentation's own example sums up to 365 rows for one account-year. So SIFT does not turn an
aggregate into a lookup; it turns a sum over N rows into a sum over the distinct key combinations --
which changes board:0343's arithmetic completely.

**Microsoft says to stop using SIFT.** "The nonclustered columnstore index is envisioned to be the
successor"; the cost/benefit table is two costs -- index updates and **potential locking conflicts** --
against one benefit. board:0347 already refuses `ColumnStoreIndex` on its zero population, so agiru
would have neither, and board:0521 makes that a measured decision rather than an omission: compute from
the base table, and record the ratio against the same `SUM` from `psql`.

**A temporary record needs NO PERMISSION on its table.** "The permission system doesn't apply ... a
user can create, read, modify and delete even if they have no permissions defined for that table." So
board:0062's check must take the RECORD and not the table id -- the same table is checked through a
normal variable and not through a temporary one, possibly in the same procedure.

**Two pages on temporary system fields look contradictory and are not.** The temporary-tables page says
temporary tables RETAIN system fields; the system-fields page says their values AREN'T CHANGED by
insert or modify. Both true: the fields exist and are not stamped. Recorded as a resolved apparent
contradiction, because a reader meeting only one of them would guess wrong.

**The `Integer` virtual table is how AL writes a `for` loop**, and CLAUDE.md's canonical documentation
contradiction lives on its page: the table lists the field as `Integer`, the field is called `Number`,
the source says so 33 times and contradicts it 0 times. board:0523 records it as a SETTLED case so the
next reader does not rediscover it. And the `Date` table's `Period End` returns CLOSING DATES, which is
board:0016's subject arriving from a source nobody would look for.

---

## `developer/` root, sixth pass -- the AL language itself

board:0028's subject. Six pages, two items. These are the pages `src/al/Parser.cpp` and
`src/gen/BodyWriter.cpp` are written against, so both items are `teilweise`: the constructs are
translated today and WHICH of their documented rules are honoured is unmeasured.

| page(s) | WI |
|---|---|
| `devenv-al-operators.md`, `devenv-al-arithmetic-operators.md`, `devenv-al-relational-operators.md`, `devenv-al-boolean-operators.md` | 0524 |
| `devenv-al-control-statements.md`, `devenv-al-simple-statements.md` | 0525 |

Six findings:

**The operator tables are a 40-cell type matrix, and every EMPTY cell is a `static_assert`.** An
operator exists exactly where the table has a cell, so `Boolean > Boolean` fails to compile because
there is no such overload rather than because something checked it. That is CLAUDE.md's "every
construct the type system can carry, it carries" applied to a table, and it moves the whole class of
defect to build time for free.

**Four footnotes, three of them silent in C++.** `Date + Decimal` is undefined **if the decimal has a
fractional part**; the undefined Date `0D` and Time `0T` RAISE rather than behaving as zero; overflow
is possible on most numeric cells. A `Date` implemented as a day count makes `0D + 1` silently the
second day of the epoch.

**String comparison is by "the built-in character comparison table of the system, NOT by comparing
'true' ASCII characters".** So `<` on two `Text` values is not `std::string`'s, and every sort, range
and filter inherits it -- board:0080's collation and board:0509's "you must know the sorting rules for
the field" are the same fact from two directions.

**The Boolean-operator page does not mention short-circuiting at all**, while board:0089 records that
AL does not short-circuit and both operands run. Recorded so the next reader does not take the page as
complete.

**`case` is not a `switch`**: value sets may be expressions and ranges, so it is a comparison chain.
And there is exactly one type-conversion exception -- **if the selector is a `Code`, the value sets are
NOT converted**, so `case CodeVar of 'abc':` never matches a `Code` holding `ABC`. A helpful
implementation normalises that away and the documentation calls it out as the only exception.

**`for` converts its bounds to the control variable's type and that conversion RAISES** -- the page's
own example, `for Count := 1000 to 100000000000000` with an Integer counter. And the control variable
is undefined outside the loop and unpredictable if changed inside it.

---

## `developer/` root, seventh pass -- variables, labels, dates, field groups, relations

Seven pages, five items. Four different roots -- board:0042, board:0055, board:0016, board:0331 -- and
each page settles a question one of them had left open.

| page(s) | WI |
|---|---|
| `devenv-system-defined-variables.md` | 0526 |
| `devenv-about-dates.md` | 0527 |
| `devenv-using-labels.md` | 0528 |
| `devenv-field-groups.md`, `devenv-lists-as-tiles.md` | 0529 |
| `devenv-set-relationships-between-tables.md` | 0530 |
| `devenv-al-variables.md`, `devenv-al-complextypes.md` | - | variable and type syntax overviews; every type has its own `methods-auto/` page and board:0051's per-type door item |

Six findings:

**`xRec` is NOT a copy in BC, and the documentation says so as a caveat**: "the record MIGHT SHARE SOME
OF THE UNDERLYING STATE with the `Rec` variable ... changes can UNEXPECTEDLY PROPAGATE." That is why
`xRec` cost the predecessor four rounds -- the semantics are underspecified in the platform itself.
board:0526 takes the copy, records it as a deviation, and notes it is the one place where being
stricter than BC removes a hazard instead of adding one.

**`CurrFieldNo` is documented as "retained for compatibility reasons"**, so nothing new is expected of
it -- and `include/runtime/Table.h:1375` already implements it.

**A `Date` is not a `DateTime` at midnight.** BC stores all `DateTime` as UTC and renders per user;
`Date` fields "are NEVER CONVERTED per time zone; a date value stays as it was entered", because they
"don't represent a timestamp" -- they are financial-reporting dates. `date` and `timestamptz`, and no
implicit conversion between them.

**And Microsoft documents a defect in its own product**: defaulting a posting date from `Today` or
converting `DateTime` to `Date` uses UTC, "which for businesses in the US and Australia WILL SURFACE
IMMEDIATELY." board:0527 records that the decision exists and what BC's answer is, rather than choosing.

**The label grammar governs the two largest populations in the sweep.** Three named arguments --
`Comment`, `Locked`, `MaxLength` -- in any order, on seven properties including `Caption` (288 491) and
`ToolTip` (159 993). `src/al/Ast.h:24`'s `LabelDecl` carries only `name` and `text`, so either the
arguments are dropped or they are inside the text, and if the latter then every label carrying
arguments has a corrupted value.

**A field group is where 37 927 dropdowns get their columns.** board:0334 measured 2 294 `LookupPageId`
declarations against board:0331's 40 221 relations, so for the rest the dropdown's columns come from
`fieldgroup(DropDown; ...)` and nowhere else. And the page says the spelling `DropDown` is
case-SENSITIVE, which contradicts AL's own case-insensitivity -- recorded, not resolved.

**A relation is what makes `Rename` possible.** "If you change one of the currency codes, the change is
AUTOMATICALLY PROPAGATED to all tables that refer to this code." `include/runtime/Table.h:1141` refuses
`Rename`; what is behind it is a reverse index over 40 221 relations, `constexpr`, built by the
generator -- and it is the only way a rename cascade can work without the runtime knowing an AL object.

---

## `developer/` root, eighth pass -- interfaces, media, isolated storage

Nine pages, three items.

| page(s) | WI |
|---|---|
| `devenv-interfaces-in-al.md`, `devenv-interfaces-in-al-extend.md`, `devenv-interfaces-in-al-operators.md` | 0531 |
| `devenv-working-with-media-on-records.md`, `devenv-lists-as-tiles.md` | 0532 |
| `devenv-isolated-storage.md`, `devenv-encrypting-data.md`, `devenv-app-key-vault-overview.md`, `devenv-app-key-vault.md` | 0533 |

Five findings:

**AL forbids the diamond that C++ would resolve.** An interface may `extends` several others, and
analyzer rules AL0587 and AL0675 refuse duplication across multiple implemented interfaces. So
non-virtual multiple inheritance is the right shape and the ambiguity is a translation error before it
is a C++ problem.

**`is` and `as` are runtime type tests over interfaces AND over `Variant`** -- four forms. That is AL
deliberately deferring a check to run time, in a tree whose reason for leaving Python was to move
checks the other way. board:0531 answers with a `constexpr` interface-id set per implementing object
rather than RTTI: `static_assert`-checkable, zero cost when unused, and identical for the `Variant`
forms.

**Media lives in two named system tables** -- 2000000184 Tenant Media and 2000000183 Tenant Media Set --
and the field holds an ID, which `src/rt/Storage.cpp:88` already gets right by mapping `Media` and
`MediaSet` to `uuid`. The id is also the client's CACHE KEY, which is the documented reason `Media`
beats `Blob`, and it falls straight out of an htmx renderer.

**A `MediaSet` index is a POSITION, not a key**: one-based, insertion-ordered, and **reindexed when an
object is removed**. So index 2 refers to a different object after index 1 goes. An implementation with
stable sparse indices is more useful and is not BC.

**Two encryption methods disagree by design.** `EncryptionKeyEnabled` reads the tenant table and keeps
returning true after the key FILE is deleted; `EncryptionKeyExists` checks the file system. An
implementation with one method behind both is wrong in exactly the failure case they exist to
distinguish. And BC's keys come from the .NET Data Protection API, so **encrypted values are not
portable between BC and agiru** -- board:0004's CRONUS load could not decrypt them.

One deviation recorded rather than implemented: BC **strips VBA macros from imported `.docx` files**.
That is a security transformation on write, invisible in the AL, and doing it wrong is worse than not
claiming it -- so agiru stores the file unchanged and says so.

---

## `developer/` root, ninth pass -- bulk transfer, streams, background work

Eight pages, three items.

| page(s) | WI |
|---|---|
| `devenv-data-transfer.md` | 0534 |
| `devenv-streams-overview.md`, `devenv-write-read-methods-line-break-behavior.md`, `devenv-file-handling-and-text-encoding.md` | 0535 |
| `devenv-task-scheduler.md`, `devenv-job-queue.md`, `devenv-async-overview.md` | 0536 |
| `devenv-excel-buffer.md`, `devenv-extract-data`, `devenv-export-data-for-extension.md` | - | BaseApp modules and admin export tooling, not platform behaviour |

Five findings:

**`DataTransfer` is the one place AL exposes a SET OPERATION**, and it exists because the row loop is
too slow for an upgrade. `CopyRows` is an `INSERT ... SELECT`; translating it back into a loop defeats
the only reason the type exists, and board:0534's negative control is therefore the STATEMENT COUNT,
not the rows.

**It may only be used in upgrade code, checked at RUN time**, and in install code only inside the
install scope. So board:0500's drivers must raise a session flag -- **the same flag board:0514 needs to
switch isolated events off during install and upgrade. One flag, two consumers.**

**`CopyStream`'s parameter order is destination-first, and the documentation explains why**: it comes
from Pascal, where procedures follow the direction of assignment. Both arguments are streams, so a
"fixed" implementation compiles and copies backwards. CLAUDE.md's rule -- the deviation is visible
rather than clever -- means AL's order is kept and the door's `\warning` carries the paragraph.

**A scheduled task's exception path rolls back FIRST, then retries in the same session** -- or, for a
non-retriable error with a failure codeunit, **terminates the session and starts a new one**. So a
failure codeunit cannot see the failed run's in-memory state, and board:0536's negative control is the
session identity rather than whether the codeunit ran.

**"Retriable" is a classification of the exception that the runtime must carry**, and BC's page does
not say what the default is for an AL `Error()`. Recorded as an open question; the flag lands on
board:0518's `ErrorInfo`, which already carries five addressing fields.

And one place where AL and C++ agree for free: **the stream direction is in the TYPE**, `InStream` and
`OutStream`, not in the method as in C#. The door's per-type files make an `InStream` that cannot be
written to, which is the invariant AL states in prose.

---

## `developer/` root, tenth pass -- the page renderer

board:0030's subject. Twenty pages, three items -- the largest ratio of pages to items so far, because
the design pages describe one renderer from many angles.

| page(s) | WI |
|---|---|
| `devenv-designing-card-pages.md`, `devenv-designing-list-pages.md`, `devenv-designing-navigate-pages.md`, `devenv-designing-parts.md`, `devenv-designing-cardparts.md`, `devenv-designing-listparts.md`, `devenv-designing-multilist-pages.md`, `devenv-simple-card-page-example.md`, `devenv-simple-list-page-example.md` | 0537 |
| `devenv-designing-role-centers.md`, `devenv-cues-action-tiles.md`, `devenv-create-role-center-headline.md`, `devenv-role-center-behaviors.md`, `devenv-simple-role-center-example.md` | 0538 |
| `devenv-actions-overview.md`, `devenv-adding-actions-to-a-page.md`, `devenv-defining-action-scope-for-pages.md`, `devenv-common-promoted-action-groups.md`, `devenv-actions-user-interface.md`, `devenv-action-bar-improvements.md` | 0539 |
| `devenv-designing-user-interfaces.md`, `devenv-designing-different-screen-sizes-tablet-and-phone.md` | - | design guidance for a client agiru does not have; no runtime obligation |

Five findings:

**A page bound to a real table WRITES ON FOCUS CHANGE.** The `NavigatePage` page explains why wizards
use a temporary source: "Business Central AUTOMATICALLY STORES ALL MODIFICATIONS TO DATABASE TABLES AS
SOON AS USERS MOVE FOCUS to another field or close the page." That is the default for every page in the
system, and board:0404's `DelayedInsert` is the same fact for the insert half. A renderer that batched
until OK would be safer, more familiar, and would change when every `OnModify` in the BaseApp fires.

**Three system actions exist on every page and AL cannot declare or remove them** -- edit, new, delete,
enabled from `Editable`, which board:0434 says is read from the CARD page when a list declares a
`CardPageId`. Three items meet before the renderer draws one button.

**A cue is an ordinary bound field on a table**, restricted to Integer and Decimal, whose value is
usually a FlowField. Nothing about a cue is special except its rendering -- which is what makes role
centres implementable before board:0498's background tasks, and why board:0510's FlowField work is
their real dependency.

**Actions may be attached to a PAGE or a GROUP and to nothing else** -- not to fields, not to parts.
Structural rather than a check: a field's descriptor simply has no action list.

**Anchors remove a deviation before it was taken.** board:0538 was about to declare an extension-merge
order for actions, as board:0513 does for subscribers -- but `addfirst`/`addlast`/`addbefore`/
`addafter` mean AL already declares it. So for actions the order is in the source, and the generator
resolves it during board:0033's merge.

And a fourth declared-but-ignored value, after board:0374, board:0422 and board:0487: **a `cuegroup`'s
`Caption` is ignored when its layout is wide.**

---

## `developer/` root, eleventh pass -- the test surface

board:0054's subject and board:0030's. Six pages, two items. CLAUDE.md's phase 2 is built on these.

| page(s) | WI |
|---|---|
| `devenv-creating-handler-methods.md` | 0540 |
| `devenv-testing-pages.md`, `devenv-testing-application.md`, `devenv-testrunner-codeunits.md` | 0541 |
| `devenv-test-application-example-purchase-invoice-discounts.md`, `devenv-extension-advanced-example-test.md` | - | worked examples of tests already covered by 0540 and 0541 |
| `devenv-test-explorer-vscode.md` | - | a VS Code tool |

Four findings:

**A declared handler that is never called FAILS the test.** "Every handler method that you enter in the
`HandlerFunctions` attribute must be called at least one time ... if a handler is listed that isn't
called, then the test fails." With board:0199's **47 994** declarations, that is the assertion which
makes a handler-driven test prove the UI interaction happened at all -- and an implementation that
registers handlers without counting invocations passes every test whose handler does fire, turning
"the dialog never appeared" into a pass.

**Eleven handler kinds with eleven signatures**, not one: four return `Boolean`, three take a `var`
out-parameter, and the two page handlers differ by one parameter. `ModalPageHandler` returns the user's
`Action` through a `var`, which is board:0516's `IsHandled` shape again -- a copied `var` makes every
modal dialog read Cancel.

**`ReportHandler` suppresses `RequestPageHandler`**, so declaring both means the request-page handler
is never called -- which by the rule above fails the test. Two rules interacting, and the combination
is decidable from the declarations.

**A `TestPage` is a generated type per page, with a member per control**: `CustomerCard.Name.Value`,
`CustomerCard."Sales Hist. Sell-to FactBox"."No.".Value`. That is `TableWriter`'s decision for records,
applied to pages -- typed members and a compile error rather than a string lookup and a run-time miss.
And it brings the same identifier-collision problem `src/gen/TableWriter.cpp:96` already solves.

Two door files the page requires that CLAUDE.md's `runtime/test/` list does not name: **`TestPart` and
`TestFilter`**. With `TestPage`, `TestField`, `TestAction` and `TestPermissions` that makes six.

---

# `developer/` root -- the per-page index

One row per `.md` read in this family. `-` in the WI column means the page produced no work item, with
the reason beside it; a `board:NNNN` means the page's task is a pre-existing root and was routed there.

The family's findings are in the per-pass sections above and in the items themselves; this table is
bookkeeping only.

| page | read | WI |
|---|---|---|
| `calculate-only-visible-flowfields-feature-key.md` | yes | 0510 |
| `devenv-about-dates.md` | yes | 0527 |
| `devenv-action-bar-improvements.md` | yes | 0539 |
| `devenv-actionable-errors.md` | yes | 0519 |
| `devenv-actions-overview.md` | yes | 0539 |
| `devenv-actions-user-interface.md` | yes | 0539 |
| `devenv-adding-actions-to-a-page.md` | yes | 0539 |
| `devenv-al-arithmetic-operators.md` | yes | 0524 |
| `devenv-al-boolean-operators.md` | yes | 0524 |
| `devenv-al-control-statements.md` | yes | 0525 |
| `devenv-al-operators.md` | yes | 0524 |
| `devenv-al-relational-operators.md` | yes | 0524 |
| `devenv-al-simple-statements.md` | yes | 0525 |
| `devenv-app-key-vault-overview.md` | yes | 0533 |
| `devenv-app-key-vault.md` | yes | 0533 |
| `devenv-async-overview.md` | yes | 0536 |
| `devenv-calcfields-calcsums-fielderror-fieldname-init-testfield-and-validate-methods.md` | yes | 0506, 0507 |
| `devenv-common-promoted-action-groups.md` | yes | 0539 |
| `devenv-create-role-center-headline.md` | yes | 0538 |
| `devenv-creating-handler-methods.md` | yes | 0540 |
| `devenv-cues-action-tiles.md` | yes | 0538 |
| `devenv-data-transfer.md` | yes | 0534 |
| `devenv-date-virtual-table.md` | yes | 0523 |
| `devenv-defining-action-scope-for-pages.md` | yes | 0539 |
| `devenv-designing-card-pages.md` | yes | 0537 |
| `devenv-designing-cardparts.md` | yes | 0537 |
| `devenv-designing-list-pages.md` | yes | 0537 |
| `devenv-designing-listparts.md` | yes | 0537 |
| `devenv-designing-multilist-pages.md` | yes | 0537 |
| `devenv-designing-navigate-pages.md` | yes | 0537 |
| `devenv-designing-parts.md` | yes | 0537 |
| `devenv-designing-role-centers.md` | yes | 0538 |
| `devenv-dotnet-subscribe-to-events.md` | yes | 0502 |
| `devenv-encrypting-data.md` | yes | 0533 |
| `devenv-entering-criteria-in-filters.md` | yes | 0509 |
| `devenv-error-collection-api.md` | yes | 0518 |
| `devenv-error-collection.md` | yes | 0518 |
| `devenv-error-dialog.md` | yes | 0519 |
| `devenv-error-handling-guidelines.md` | yes | 0519 |
| `devenv-event-types.md` | yes | 0512 |
| `devenv-events-in-al.md` | yes | 0512 |
| `devenv-events-isolated.md` | yes | 0514 |
| `devenv-extend-pages-based-on-date-virtual-table.md` | yes | 0523 |
| `devenv-extension-install-code.md` | yes | 0500 |
| `devenv-field-groups.md` | yes | 0529 |
| `devenv-file-handling-and-text-encoding.md` | yes | 0535 |
| `devenv-flowfields.md` | yes | 0510 |
| `devenv-flowfilter-overview.md` | yes | 0510 |
| `devenv-format-field-data.md` | yes | 0491 |
| `devenv-format-property.md` | yes | 0503 |
| `devenv-get-find-and-next-methods.md` | yes | 0504 |
| `devenv-handling-errors-using-try-methods.md` | yes | 0517 |
| `devenv-httpclient-mock-outbound-calls.md` | yes | 0494 |
| `devenv-inherent-permissions.md` | yes | 0499 |
| `devenv-insert-modify-modifyall-delete-and-deleteall-methods.md` | yes | 0505 |
| `devenv-integer-virtual-table.md` | yes | 0523 |
| `devenv-interfaces-in-al-extend.md` | yes | 0531 |
| `devenv-interfaces-in-al-operators.md` | yes | 0531 |
| `devenv-interfaces-in-al.md` | yes | 0531 |
| `devenv-isolated-storage.md` | yes | 0533 |
| `devenv-job-queue.md` | yes | 0536 |
| `devenv-lists-as-tiles.md` | yes | 0529, 0532 |
| `devenv-migrating-from-sift-to-ncci.md` | yes | 0521 |
| `devenv-page-background-tasks.md` | yes | 0498 |
| `devenv-permissionset-composing.md` | yes | 0492 |
| `devenv-permissionset-object.md` | yes | 0492 |
| `devenv-progress-windows-message-error-and-confirm-methods.md` | yes | 0496 |
| `devenv-publishing-events.md` | yes | 0515 |
| `devenv-raising-events.md` | yes | 0515 |
| `devenv-request-pages.md` | yes | 0501 |
| `devenv-role-center-behaviors.md` | yes | 0538 |
| `devenv-secret-text.md` | yes | 0497 |
| `devenv-set-relationships-between-tables.md` | yes | 0530 |
| `devenv-setcurrentkey-setrange-setfilter-getrangemin-and-getrangemax-methods.md` | yes | 0508 |
| `devenv-sift-and-sql-server.md` | yes | 0521 |
| `devenv-sift-performance.md` | yes | 0521 |
| `devenv-sift-technology.md` | yes | 0521 |
| `devenv-sift-tuning-and-tracing.md` | yes | 0521 |
| `devenv-simple-card-page-example.md` | yes | 0537 |
| `devenv-simple-list-page-example.md` | yes | 0537 |
| `devenv-simple-role-center-example.md` | yes | 0538 |
| `devenv-streams-overview.md` | yes | 0535 |
| `devenv-subscribing-to-events.md` | yes | 0513 |
| `devenv-system-defined-variables.md` | yes | 0526 |
| `devenv-table-keys.md` | yes | 0520 |
| `devenv-table-system-fields.md` | yes | 0511 |
| `devenv-task-scheduler.md` | yes | 0536 |
| `devenv-temporary-tables.md` | yes | 0522 |
| `devenv-test-codeunits-and-test-methods.md` | yes | 0493 |
| `devenv-testing-application.md` | yes | 0541 |
| `devenv-testing-pages.md` | yes | 0541 |
| `devenv-testing-with-permission-sets.md` | yes | 0495 |
| `devenv-testrunner-codeunits.md` | yes | 0541 |
| `devenv-tri-state-locking.md` | yes | 0490 |
| `devenv-upgrading-extensions.md` | yes | 0500 |
| `devenv-use-ishandled-min-req.md` | yes | 0516 |
| `devenv-use-ishandled-pattern.md` | yes | 0516 |
| `devenv-using-labels.md` | yes | 0528 |
| `devenv-virtual-tables.md` | yes | 0523 |
| `devenv-working-with-media-on-records.md` | yes | 0532 |
| `devenv-write-read-methods-line-break-behavior.md` | yes | 0535 |
| `types-of-events-for-extensibility.md` | yes | 0516 |
| `devenv-al-error-handling.md` | yes | - -- an index over the four error features; each has its own page and item |
| `devenv-extension-errors-recommendations.md` | yes | - -- AppSource submission advice on error wording; no runtime obligation |
| `devenv-events-discoverability.md` | yes | - -- the Event Recorder, a client and VS Code tool; no runtime behaviour |
| `business-events-overview.md` | yes | - -- external business events through Dataverse; a cloud bridge, excluded by scope.json |
| `devenv-deprecate-external-business-events.md` | yes | - -- as above |
| `devenv-table-object.md` | yes | - -- object-syntax overview; every element has its own property or trigger page |
| `devenv-tables-overview.md` | yes | - -- as above |
| `devenv-al-variables.md` | yes | - -- variable syntax overview; every type has its methods-auto page and board:0051 |
| `devenv-al-complextypes.md` | yes | - -- as above |
| `devenv-excel-buffer.md` | yes | - -- a BaseApp module, not platform behaviour |
| `devenv-extract-data.md` | yes | - -- admin export tooling |
| `devenv-export-data-for-extension.md` | yes | - -- admin export tooling |
| `devenv-designing-user-interfaces.md` | yes | - -- design guidance for a client agiru does not have |
| `devenv-designing-different-screen-sizes-tablet-and-phone.md` | yes | - -- as above |
| `devenv-test-application-example-purchase-invoice-discounts.md` | yes | - -- a worked example of tests covered by board:0540 and board:0541 |
| `devenv-extension-advanced-example-test.md` | yes | - -- as above |
| `devenv-test-explorer-vscode.md` | yes | - -- a VS Code tool |

| `devenv-views.md` | yes | 0542 |
| `devenv-views-legacy.md` | yes | 0542 |
| `devenv-view-table-data.md` | yes | 0542 |
| `devenv-filter-pages-for-filtering-tables.md` | yes | 0543 |
| `devenv-adding-filter-tokens.md` | yes | 0543 |
| `devenv-table-field-text-search.md` | yes | 0543 |

| `devenv-namespaces-overview.md` | yes | 0544 |
| `devenv-namespaces-structure.md` | yes | 0544 |
| `devenv-using-access-modifiers.md` | yes | 0544 |
| `devenv-compilation-scope-overview.md` | yes | 0544 |
| `devenv-extension-object-overview.md` | yes | 0545 |
| `devenv-table-ext-object.md` | yes | 0545 |
| `devenv-page-ext-object.md` | yes | 0545 |
| `devenv-report-ext-object.md` | yes | 0545 |
| `devenv-permissionset-ext-object.md` | yes | 0545 |
| `devenv-object-ranges.md` | yes | 0545 |

| `devenv-substituting-reports.md` | yes | 0546 |
| `devenv-howto-report-layout.md` | yes | 0547 |
| `devenv-howto-rdl-report-layout.md` | yes | 0547 |
| `devenv-howto-excel-report-layout.md` | yes | 0547 |
| `devenv-using-word-to-author-your-report-layout.md` | yes | 0547 |
| `devenv-hyperlinks-in-word-report-layouts.md` | yes | 0547 |
| `word-layout-add-in.md` | yes | 0547 |
| `devenv-format-report-field-data.md` | yes | 0547 |
| `devenv-xmlport-schema.md` | yes | 0548 |
| `devenv-xmlport-overview.md` | yes | 0548 |
| `devenv-xmlport-object.md` | yes | 0548 |
| `devenv-using-namespaces-with-xmlports.md` | yes | 0548 |

| `devenv-report-object.md` | yes | 0549 |
| `devenv-report-dataset.md` | yes | 0549 |
| `devenv-walktrough-designing-reports-multiple-tables.md` | yes | 0549 |
| `devenv-get-report-parameters-with-virtual-tables.md` | yes | 0549 |
| `devenv-testing-reports.md` | yes | 0549 |
| `devenv-query-object.md` | yes | 0550 |
| `devenv-query-links-joins.md` | yes | 0550 |
| `devenv-api-querytype.md` | yes | 0550 |

| `devenv-profile-object.md` | yes | 0551 |
| `devenv-page-customization-object.md` | yes | 0551 |
| `devenv-profile-ext-object.md` | yes | 0551 |
| `devenv-design-profiles.md` | yes | 0551 |
| `devenv-role-customization.md` | yes | 0551 |
| `devenv-assign-user-profile.md` | yes | 0551 |
| `devenv-obsolete-objects.md` | yes | 0552 |
| `devenv-deprecation-guidelines.md` | yes | 0552 |
| `devenv-deprecation-timeline.md` | yes | 0552 |
| `devenv-deprecating-with-statements-overview.md` | yes | 0552 |
| `devenv-app-discontinue.md` | yes | 0552 |

| `devenv-page-types-and-layouts.md` | yes | 0553 |
| `devenv-adding-a-factbox-to-page.md` | yes | 0554 |
| `devenv-promoted-actions.md` | yes | 0555 |
| `devenv-promoted-actions-behavioral-changes.md` | yes | 0555 |
| `devenv-organizing-promoted-actions.md` | yes | 0555 |

| `devenv-query-totals-grouping.md` | yes | 0556 |
| `devenv-query-filters.md` | yes | 0556 |
| `devenv-query-retrieve-date-data.md` | yes | 0556 |
| `devenv-query-accessing-columns.md` | yes | 0556 |
| `devenv-query-using-instead-record-variables.md` | yes | 0556 |

| `devenv-report-triggers.md` | yes | 0557, 0558 |
| `devenv-request-pages-for-reports.md` | yes | 0557 |

| `devenv-entitlements-and-permissionsets-overview.md` | yes | 0559 |
| `devenv-entitlement-object.md` | yes | 0559 |

| `devenv-read-isolation.md` | yes | routed to root 0012 |
| `devenv-partial-records.md` | yes | routed to root 0048 |
| `devenv-partial-records-faq.md` | yes | routed to root 0048 |
| `devenv-permissions-on-database-objects.md` | yes | routed to root 0062 |
| `devenv-number-sequences.md` | yes | routed to root 0028 |
| `devenv-object-specifications-limitations.md` | yes | routed to root 0081 |
| `devenv-al-this-keyword.md` | yes | routed to root 0026 |
| `devenv-extensible-enums.md` | yes | routed to root 0053 |
| `devenv-extending-application-areas.md` | yes | routed to root 0030 |
| `devenv-al-menusuite-functionality.md` | yes | routed to root 0083 |
| `devenv-oncompanyopencompleted.md` | yes | routed to root 0057 |
| `devenv-ncci-overview.md` | yes | routed to root 0019 |
| `devenv-debug-upgrade-install-code.md` | yes | routed to root 0070 |
| `devenv-methodtype-property-upgrade-codeunits.md` | yes | routed to root 0070 |
| `devenv-report-custom-render.md` | yes | routed to root 0063 |

| `devenv-repeater-controls.md` | yes | 0560 |
| `devenv-indented-hierarchy-lists.md` | yes | 0560 |
| `devenv-creating-flowfields-and-flowfilters.md` | yes | 0560 -- an example page; its substance is board:0510's and board:0340's, and the one thing it adds is that a FlowField's `where` reads a FlowFilter field of the SAME record |

| `devenv-arranging-fields-on-fasttab.md` | yes | 0561 |
| `devenv-arranging-fields-using-grid-and-fixed-controls.md` | yes | 0561 |
| `devenv-arrange-fields-in-rows-and-columns-using-gridlayout-control.md` | yes | 0561 |
| `devenv-arrange-fields-in-rows-and-columns-using-fixedlayout-control.md` | yes | 0561 |

| `devenv-notifications-developing.md` | yes | 0562 |

| `devenv-httpclient.md` | yes | 0563 |

| `devenv-al-type-conversion-expressions.md` | yes | routed to 0524 |
| `devenv-protected-variables.md` | yes | routed to 0359 |

| `devenv-al-methods.md` | yes | 0564 |

| `devenv-control-addin-object.md` | yes | 0565 |

| `devenv-work-with-translation-files.md` | yes | 0566 |
| `devenv-translations-overview.md` | yes | 0566 |

| `devenv-api-pagetype.md` | yes | 0567 |
| `devenv-creating-and-interacting-with-odatav4-bound-action.md` | yes | 0567 |
| `devenv-creating-and-interacting-with-odatav4-unbound-action.md` | yes | 0567 |

| `TOC.md` | yes | no task -- the family's table of contents; it carries no statement about behaviour |
| `ai-build-capability-in-al.md` | yes | no task -- integrating Azure OpenAI through the BaseApp's AI module; an external service, not a platform primitive |
| `ai-build-experience-overview.md` | yes | no task -- the Copilot build overview; index to the pages below it |
| `ai-dev-tools-get-started.md` | yes | no task -- provisioning an Azure OpenAI resource |
| `ai-dev-tools-resources.md` | yes | no task -- model availability, billing and migration for Microsoft-hosted AI resources |
| `ai-extend-copilot-overview.md` | yes | no task -- guidance on whether a feature qualifies as a Copilot extension |
| `ai-prepare-app-help-copilot.md` | yes | no task -- writing help content that Copilot can ground on |
| `ai-system-app-function-calling.md` | yes | no task -- use of the `AOAI Chat Messages` BaseApp codeunit; an AL object, not a runtime primitive |
| `ai-system-app-token-counting.md` | yes | no task -- use of the `AOAI Token` BaseApp codeunit |
| `ai-test-copilot.md` | yes | no task -- testing strategy for LLM output |
| `ai-test-copilot-agent-tests.md` | yes | no task -- agent tests over the `Library - Agent` BaseApp helpers |
| `ai-test-copilot-ai-tests.md` | yes | no task -- AI tests over BaseApp helpers and external evaluation |
| `ai-test-copilot-bestpractices.md` | yes | no task -- advice on testing non-deterministic output |
| `ai-test-copilot-datasets.md` | yes | no task -- dataset format for the AI test tool |
| `ai-test-copilot-testtool.md` | yes | no task -- operating the Evaluation tool |
| `app-faq-dependencies-libraries.md` | yes | no task -- Marketplace library and dependency apps; a publishing question |
| `app-faq-offer.md` | yes | no task -- Partner Center offer management |
| `app-faq-test.md` | yes | no task -- advice on testing before Marketplace validation |
| `app-faq-update.md` | yes | no task -- Marketplace update process |
| `app-maintain.md` | yes | no task -- keeping a published app compatible; a publishing process |
| `copilot-and-agents-influence-without-extending.md` | yes | no task -- guidance on influencing Copilot output without AL |
| `developer-tools-for-copilot-overview.md` | yes | no task -- overview of the Copilot toolkit |
| `create-extensibility-request.md` | yes | no task -- how to ask Microsoft for an extension point |
| `dataverse-integration-overview.md` | yes | no task -- synchronising with an external service |
| `devenv-aad-auth-onprem.md` | yes | no task -- Microsoft Entra authentication for a BC server; agiru has no Entra |
| `devenv-al-code-navigation.md` | yes | no task -- Go To Definition in Visual Studio Code |

| `devenv-page-type-promptdialog.md` | yes | 0568 |
| `devenv-page-promptguide.md` | yes | 0568 |
| `devenv-page-prompt-error-handling.md` | yes | 0568 |
| `devenv-page-prompting-floating-actionbar.md` | yes | 0568 |
| `copilot-create-promptdialog.md` | yes | 0568 |
| `copilot-customize-generate-mode.md` | yes | 0568 |
| `copilot-design-prompt-mode.md` | yes | 0568 |
| `copilot-design-content-mode.md` | yes | 0568 |
| `ai-build-experience.md` | yes | 0568 |

| `devenv-al-explorer.md` | yes | no task -- the VS Code object browser |
| `devenv-al-extension-configuration.md` | yes | no task -- Visual Studio Code settings for the AL extension |
| `devenv-al-formatter.md` | yes | no task -- whitespace formatting in the editor |
| `devenv-al-home.md` | yes | no task -- the AL Home view in Visual Studio Code |
| `devenv-al-outline-view.md` | yes | no task -- the outline view in Visual Studio Code |
| `devenv-al-profiler-overview.md` | yes | no task -- snapshot and sampling profiling against a BC server |
| `devenv-al-reference-guide.md` | yes | no task -- an index to the reference families |
| `devenv-al-table-proxy-generator.md` | yes | no task -- generating Dataverse proxy tables |
| `devenv-al-tool.md` | yes | no task -- the ALTool command line: compile, package, CI |
| `devenv-al-tool-package.md` | yes | no task -- installing the ALTool package |
| `devenv-app-identity.md` | yes | no task -- when an app's id, name, publisher or version may change; a publishing rule |
| `devenv-app-life-cycle.md` | yes | no task -- the service-update and app-update process |
| `devenv-application-insights-for-extensions.md` | yes | no task -- configuring Azure Application Insights |
| `devenv-application-insights-for-extensions-data.md` | yes | no task -- what telemetry is sent to Azure |
| `devenv-attach-debug-next.md` | yes | no task -- attaching the debugger to a running session |
| `devenv-business-central-manage-selfservice-signups.md` | yes | no task -- tenant administration in Microsoft Entra |
| `devenv-checklist-submission.md` | yes | no task -- the Marketplace validation checklist |
| `devenv-checklist-submission-app-identity.md` | yes | no task -- Marketplace app identity questions |
| `devenv-checklist-submission-app-insights.md` | yes | no task -- Application Insights during Marketplace submission |
| `devenv-checklist-submission-app-preview.md` | yes | no task -- Marketplace preview versions |
| `devenv-checklist-submission-channels.md` | yes | no task -- where to ask Marketplace questions |
| `devenv-checklist-submission-code-sign.md` | yes | no task -- code-signing certificates for Marketplace |
| `devenv-checklist-submission-develop-maintain.md` | yes | no task -- one FAQ about dependency id ranges in a container |
| `devenv-checklist-submission-faq.md` | yes | no task -- the collected Marketplace validation FAQ |
| `devenv-checklist-submission-name-affix-range.md` | yes | no task -- registering affixes and id ranges with Microsoft |
| `devenv-checklist-submission-offer.md` | yes | no task -- Marketplace offer types |
| `devenv-checklist-submission-validation-process.md` | yes | no task -- how and against what Microsoft validates a submission |
| `devenv-choosing-runtime.md` | yes | no task -- choosing a runtime version in `app.json`; agiru transpiles BCApps as it stands |
| `devenv-code-actions.md` | yes | no task -- Visual Studio Code quick fixes |
| `devenv-code-analysis-performance-configuration.md` | yes | no task -- tuning the analyzers, and the analyzer family is struck |
| `devenv-code-spaces-al.md` | yes | no task -- GitHub Codespaces for AL development |
| `devenv-connect-apps-filtering.md` | yes | no task -- OData filter syntax in a URL; the endpoint half board:0567 puts out of scope |
| `devenv-contribute-extensibility.md` | yes | no task -- how to contribute a change to Microsoft's codebase |
| `devenv-control-addin-bestpractices.md` | yes | no task -- performance advice for JavaScript inside an add-in |
| `devenv-control-addin-style.md` | yes | no task -- colours and typography for add-in authors |
| `devenv-creating-runtime-packages.md` | yes | no task -- distributing an extension as a runtime package |
| `devenv-customization-update-lifecycle.md` | yes | no task -- automated validation and removal of tenant customizations |
| `devenv-data-archive-extension.md` | yes | no task -- the `Data Archive` BaseApp codeunit; an AL object the transpiler translates like any other |
| `devenv-debug-mcp-server.md` | yes | no task -- an MCP server for inspecting a debug session |
| `devenv-debugging.md` | yes | no task -- the Visual Studio Code debugger |
| `devenv-debugging-conditional-breakpoints.md` | yes | no task -- breakpoint conditions in the debugger |
| `devenv-deciding-on-tablet-and-phone-strategy.md` | yes | no task -- advice on mobile form factors |
| `devenv-deploy-tenant-customization.md` | yes | no task -- uploading an `.app` through the admin centre |
| `devenv-dev-faq.md` | yes | no task -- getting-started FAQ, every answer a pointer elsewhere |
| `devenv-dev-faq-teams.md` | yes | no task -- the Microsoft Teams integration |

| `devenv-adding-tooltips.md` | yes | routed to 0385 -- the table-field / page-field fallback, sized: `ToolTip` 50 325 in `*.Table.al` against 92 903 in `*.Page.al` |
| `devenv-adding-help-links-from-pages-tables-xmlports.md` | yes | routed to 0393 -- the property sits on a page, and on a report's and an XMLport's REQUEST PAGE |
| `devenv-adding-menus-to-navigation-pane.md` | yes | routed to 0538 -- `area(Sections)` is the role centre's navigation menu, 204 files, all `PageType = RoleCenter` |

| `devenv-app-resources.md` | yes | 0572 |
| `devenv-create-a-wrapper-module.md` | yes | routed to 0035 -- the BaseApp WRAPS a .NET class in a facade plus an `Internal` impl, so what agiru owes is the `DotNet` type and not the module |
| `devenv-blueprint.md` | yes | routed to 0033 -- one module one app.json, dependencies point down only, a facade with no logic, `Target` Cloud by default |
| `devenv-analysis-view-package.md` | yes | routed to 0553 -- `analysisviews` is a third page section beside `layout` and `actions`, and it is allowed on a page customization |
| `devenv-api.md` | yes | routed to 0567 -- API page is one table with CRUD, API query is a join with none, neither extensible |
| `devenv-connect-apps-tips.md` | yes | routed to 0567 -- the client side of the endpoint this sweep puts out of phase 1-3 scope |
| `devenv-control-addin-asynchronous-considerations.md` | yes | routed to 0565 -- every add-in method MUST be `void`, because the boundary is asynchronous |
| `devenv-codeunit-object.md` | yes | routed to 0267 -- `TableNo` is what puts a `Rec` on a codeunit, and the two entry points differ in transaction behaviour |
| `devenv-design-profiles-using-client.md` | yes | routed to 0551 -- a profile round-trips through the client, which is why a customization can hold no code |
| `devenv-camera-options.md` | yes | no task -- `CameraOptions` is a client-side capability class in a .NET client DLL, not an AL type |

| `devenv-extend-edocuments.md` | yes | no task -- extending the E-Documents BaseApp feature through its events and interfaces; AL objects the transpiler translates like any other |
| `devenv-extend-exchange-rates.md` | yes | no task -- extending the exchange-rate adjustment BaseApp feature |
| `devenv-extending-best-price-calculations.md` | yes | no task -- extending the price-calculation BaseApp feature through its interface objects |
| `devenv-extending-document-sharing-onedrive.md` | yes | no task -- extending document sharing over OneDrive; a BaseApp feature and an external service |
| `devenv-extending-email.md` | yes | no task -- extending the Email BaseApp module, its connectors and view policies |
| `devenv-extending-item-charges.md` | yes | no task -- adding an item-charge distribution method to a BaseApp enum and interface |
| `devenv-extending-shopify.md` | yes | no task -- extending the Shopify connector |
| `devenv-extending-templates.md` | yes | no task -- extending the customer, vendor and item template BaseApp feature |
| `devenv-extensibility-overview.md` | yes | no task -- an overview of the extensibility mechanisms, each owned elsewhere |
| `devenv-dev-overview.md` | yes | no task -- the development-experience overview; an index |
| `devenv-dev-productivity-tips.md` | yes | no task -- Visual Studio Code productivity tips |
| `devenv-ext-dev-lifecycle-overview.md` | yes | no task -- the phases of developing an extension |
| `devenv-develop-connect-apps.md` | yes | no task -- Microsoft Entra authentication and exploring REST APIs with a client tool |
| `devenv-develop-for-teams.md` | yes | no task -- the Microsoft Teams integration |
| `devenv-develop-for-teams-cards.md` | yes | no task -- Teams cards built from field groups and events |
| `devenv-develop-for-teams-check-session.md` | yes | no task -- detecting a Teams session and a Microsoft 365 licence |
| `devenv-develop-for-teams-tab-content.md` | yes | no task -- recommended content in the Teams tab configuration |
| `devenv-develop-for-teams-tabs.md` | yes | no task -- adding a Teams tab through the Graph API |
| `devenv-developing-for-multiple-platform-versions.md` | yes | no task -- the `platform` version in `app.json`; agiru reads BCApps as it stands |
| `devenv-differences-and-limitations-developing-pages-business-central-mobile-app.md` | yes | no task -- mobile form-factor limitations |
| `devenv-directory-app-json.md` | yes | no task -- `directory.app.props.json`, shared project metadata for the AL build |
| `devenv-disable-soap-microsoft-pages-feature-key.md` | yes | no task -- a feature key that removes SOAP endpoints from Microsoft pages |
| `devenv-embed-web-client-pages-in-websites.md` | yes | no task -- hosting the web client inside another site |
| `devenv-application-app-file.md` | yes | no task -- how `Microsoft_Application.app` bundles a solution's extensions; a packaging artefact of the BC build |
| `devenv-change-a-module.md` | yes | no task -- the process for changing a System Application module and getting it accepted |
| `devenv-export-permission-sets.md` | yes | no task -- exporting permission sets to XML through XMLport 9171 or the client; an administrative procedure over board:0559's objects |
| `devenv-develop-custom-api.md` | yes | routed to 0567 -- a 909-line walkthrough of building an API page and query; every mechanism in it is already in that item |
| `devenv-essential-al-methods.md` | yes | routed to 0571 -- a 29-line index into `methods-auto/`, which that pass covers in full |
| `devenv-events-example.md` | yes | routed to 0057 -- a worked example of publisher and subscriber, no mechanism the event root does not carry |
| `devenv-dotnet-controladdins.md` | yes | routed to 0035 -- declaring a .NET or JavaScript add-in assembly, which is the DotNet surface seen from the add-in side |
| `devenv-dotnet-serializing-dotnetframework-types.md` | yes | routed to 0035 -- making a .NET type serializable so it can cross the AL boundary |

| `devenv-differences.md` | yes | 0573 |
| `devenv-edit-in-excel-lists.md` | yes | no task -- the Edit in Excel system action needs an Excel add-in on the client and is off by default on premises; its gating is three permission sets, which board:0559 owns |

| `devenv-extension-advanced-example.md` | yes | no task -- a 917-line walkthrough building a sample extension |
| `devenv-extension-example.md` | yes | no task -- a 575-line first-extension walkthrough |
| `devenv-extension-moving-scope.md` | yes | no task -- moving an extension between Marketplace, per-tenant and DEV scopes |
| `devenv-extension-types-and-scope.md` | yes | no task -- what a global app, a per-tenant extension and a DEV extension are |
| `devenv-generating-delta-files.md` | yes | no task -- exporting C/SIDE deltas with `ExportToNewSyntax` |
| `devenv-get-started.md` | yes | no task -- setting up a sandbox and Visual Studio Code |
| `devenv-getting-started.md` | yes | no task -- getting started with System Application modules |
| `devenv-getting-started-developing-business-central-mobile-app.md` | yes | no task -- developing for the mobile app |
| `devenv-hotfixing-appsource-app.md` | yes | no task -- what may be in a Marketplace hotfix submission |
| `devenv-how-publish-and-install-an-extension-v2.md` | yes | no task -- publishing and installing an extension on a server |
| `devenv-implement-camera-al.md` | yes | no task -- the camera capability through a client add-in on a device |
| `devenv-implement-location-al.md` | yes | no task -- the location capability through a client add-in on a device |
| `devenv-location-options.md` | yes | no task -- `LocationOptions` is a client-side capability class, like `CameraOptions` |
| `devenv-implementation-tips-gestures-property.md` | yes | no task -- when to use a swipe gesture on a phone |
| `devenv-instrument-application-for-telemetry.md` | yes | no task -- event log against telemetry, and where each is logged |
| `devenv-instrument-application-for-telemetry-app-insights.md` | yes | no task -- custom telemetry events for Azure Application Insights |
| `devenv-instrument-application-for-telemetry-event-log.md` | yes | no task -- custom telemetry events for the Windows event log |
| `devenv-integrating-dynamics-365-for-sales-extension-development.md` | yes | no task -- enabling Dataverse tables for extension development |
| `devenv-introducing-business-central-mobile-app.md` | yes | no task -- what the mobile app is |
| `devenv-link-to-mobile-app.md` | yes | no task -- constructing a URL that opens the mobile app |
| `devenv-json-launch-file.md` | yes | no task -- `launch.json`, the Visual Studio Code debug and publish configuration |
| `devenv-keyboard-shortcuts.md` | yes | no task -- Visual Studio Code keyboard shortcuts |
| `devenv-migrate-from-dotnet-framework-to-dotnet-standard.md` | yes | no task -- porting a .NET add-in assembly between frameworks |
| `devenv-invoice-posting-example.md` | yes | no task -- extending G/L entry aggregation through BaseApp events; AL objects the transpiler translates like any other |
| `devenv-httpcertvalid-feature-key.md` | yes | routed to 0563 -- the pre-version-27 feature key for server certificate validation, which that item records as a superseded scheme |
| `devenv-mask-type-feature-key.md` | yes | routed to 0330 -- the feature key that introduced `MaskType` |
| `devenv-get-started-call-dotnet-from-al.md` | yes | routed to 0035 -- declaring a .NET package in `app.json` and using a type from AL |
| `devenv-inclient-designer.md` | yes | routed to 0551 -- Designer writes the page customizations that item describes, and its capabilities are the reason a customization holds no code |
| `devenv-inspecting-pages.md` | yes | routed to 0553 -- Page Inspection displays the control tree and the source table behind it, which is that item's metadata seen through the client |

| `devenv-mobile-app-barcode-scanning.md` | yes | no task -- barcode scanning through the device camera |
| `devenv-opening-business-central-tablet-or-phone-client-from-browser.md` | yes | no task -- a URL scheme that opens the mobile client |
| `devenv-troubleshooting-the-mobile-app.md` | yes | no task -- mobile app troubleshooting |
| `devenv-using-https-and-certificates-mobile-app.md` | yes | no task -- certificates for the mobile client |
| `devenv-multiroot-workspaces.md` | yes | no task -- Visual Studio Code multi-root workspaces |
| `devenv-work-workspace-projects-references.md` | yes | no task -- workspace, project and reference layout in Visual Studio Code |
| `devenv-optimize-visual-studio-code.md` | yes | no task -- editor performance settings |
| `devenv-rad-publishing.md` | yes | no task -- Rapid Application Development publishing |
| `devenv-txt2al-tool.md` | yes | no task -- converting C/SIDE text exports to AL |
| `devenv-troubleshoot-vscode-webclient.md` | yes | no task -- troubleshooting the editor against the web client |
| `devenv-snapshot-debugging.md` | yes | no task -- snapshot debugging a running server |
| `devenv-using-code-analysis-tool.md` | yes | no task -- running the analyzers, and the analyzer family is struck |
| `devenv-using-code-analysis-tool-with-rule-set.md` | yes | no task -- analyzer rule sets |
| `devenv-rule-set-syntax-for-code-analysis-tools.md` | yes | no task -- the rule-set JSON schema |
| `devenv-performance-toolkit.md` | yes | no task -- the BC performance toolkit, a load-testing harness against a server |
| `devenv-power-bi-report-parts.md` | yes | no task -- embedding Power BI reports; an external service |
| `devenv-power-bi-report-parts-legacy.md` | yes | no task -- the superseded Power BI part |
| `devenv-publish-code-customization.md` | yes | no task -- publishing a code-customized base application on premises |
| `devenv-retaining-data-after-publishing.md` | yes | no task -- what survives a publish, an administration concern |
| `devenv-running-container-development.md` | yes | no task -- developing against a container |
| `devenv-sandbox-overview.md` | yes | no task -- what a sandbox environment is |
| `devenv-set-up-an-environment.md` | yes | no task -- setting up a development environment |
| `devenv-security-settings-and-ip-protection.md` | yes | no task -- protecting an app's source; a publishing concern |
| `devenv-sell-apps-appsource.md` | yes | no task -- selling through Marketplace |
| `devenv-sign-extension.md` | yes | no task -- code-signing an `.app` |
| `devenv-supported-cipher-suites.md` | yes | no task -- the server's TLS cipher list |
| `devenv-unpublish-and-uninstall-extension-v2.md` | yes | no task -- unpublishing and uninstalling |
| `devenv-update-app-life-cycle-faq.md` | yes | no task -- app update lifecycle questions |
| `devenv-upgrade-appsource-app-in-prod.md` | yes | no task -- upgrading a Marketplace app in production |
| `devenv-upgrade-v1-to-v2-overview.md` | yes | no task -- migrating a v1 extension to v2, a format long superseded |
| `devenv-uplift-to-extensions.md` | yes | no task -- moving a code customization into extensions |
| `devenv-troubleshooting-overview.md` | yes | no task -- an index of troubleshooting pages |
| `devenv-troubleshooting-device-date-is-causing-connection-Issues.md` | yes | no task -- a clock-skew connection failure |
| `devenv-web-client-urls.md` | yes | no task -- the web client's URL parameters; a client surface, not an AL one |
| `devenv-work-sandbox-entitlements.md` | yes | no task -- entitlements in a sandbox, which board:0559 puts out of scope on premises |
| `devenv-translate-base-app-help.md` | yes | no task -- translating the help content, not the application |
| `devenv-migration-json-file.md` | yes | no task -- the cloud-migration configuration file |
| `devenv-reports-troubleshooting.md` | yes | no task -- diagnosing a report that fails on a server |
| `devenv-reports-troubleshoot-printing.md` | yes | no task -- diagnosing a printer that does not print |
| `devenv-walkthrough-developing-sales-rep-rolecenter-business-central-tablet-client.md` | yes | no task -- a role-centre walkthrough for the tablet client |
| `index.md` | yes | no task -- the family's landing page |
| `devenv-reference-overview.md` | yes | no task -- an index of the reference families |
| `integration-azure-overview.md` | yes | no task -- integrating with Azure services |
| `integration-infrastructure-overview.md` | yes | no task -- the integration infrastructure overview |
| `m365-integration-overview.md` | yes | no task -- Microsoft 365 integration |
| `ml-forecasting-api-overview.md` | yes | no task -- the Azure ML forecasting service |
| `ml-prediction-api-overview.md` | yes | no task -- the Azure ML prediction service |
| `ml-transparency-note.md` | yes | no task -- a responsible-AI transparency note |
| `power-pages-on-virtual-tables-overview.md` | yes | no task -- Power Pages over Dataverse virtual tables |
| `semantic-search-feature-key.md` | yes | no task -- a feature key for semantic search over an external index |
| `devenv-ncci-and-sql-server.md` | yes | routed to 0019 -- how SQL Server maintains a nonclustered columnstore index, which PostgreSQL does not have |
| `devenv-ncci-performance.md` | yes | routed to 0019 -- the performance case for the columnstore successor to SIFT |
| `devenv-ncci-tuning-and-tracing.md` | yes | routed to 0019 -- tuning and tracing that index |
| `devenv-new-module.md` | yes | routed to 0033 -- creating a System Application module, the process behind board:0033's module rules |
| `devenv-system-application-overview.md` | yes | routed to 0033 -- what the System Application is and how its modules layer |

| `devenv-pages-overview.md` | yes | 0574 |
| `devenv-page-object.md` | yes | 0574 |
| `devenv-page-type-usercontrolhost.md` | yes | 0574 |
| `devenv-page-discoverability.md` | yes | 0574 -- an index of five ways a user finds a page, each owned elsewhere; it adds one dependency, that a page reaches the Role Explorer by being in a role centre's navigation |

| `devenv-onafterdocumentready-event.md` | yes | 0575 |
| `devenv-onafterintermediatedocumentready-event.md` | yes | 0575 |
| `devenv-onafterdocumentprintready-event.md` | yes | 0575 |
| `devenv-onafterdocumentdownload-event.md` | yes | 0575 |
| `devenv-ongetfilename-event.md` | yes | 0575 |
| `devenv-onaftersetupprinters-event.md` | yes | 0575 |
| `devenv-oncustomdocumentmerger-event.md` | yes | 0575 |
| `devenv-oncustomdocumentmergerex-event.md` | yes | 0575 |

| `devenv-report-layout-declaration.md` | yes | 0576 |
| `devenv-multiple-report-layouts.md` | yes | 0576 |
| `devenv-report-design-overview.md` | yes | 0576 |
| `devenv-reports.md` | yes | 0576 |
| `devenv-reporting-options-overview.md` | yes | 0576 -- an analytics-persona overview whose only mechanism is the three layout types |

| `devenv-report-localization.md` | yes | 0577 |
| `devenv-report-performance.md` | yes | 0577 -- report telemetry; the only mechanism is the two format properties |
| `devenv-reports-obsoletion.md` | yes | 0577 -- `ObsoleteState`, `ObsoleteTag` and `ObsoleteReason` on a report and, from 2025 wave 1, on a LAYOUT; the preprocessor pattern is board:0552's |
| `devenv-reports-printing.md` | yes | 0577 -- the two printing events board:0575 owns; adds that without a printer extension there is no direct printing at all, only a PDF download |
| `devenv-reports-discoverability.md` | yes | 0577 -- Tell Me, role centres and captions, each owned elsewhere |
| `devenv-richtext-content-controls.md` | yes | routed to 0553 -- `ExtendedDatatype = RichContent`, 23 declarations, alone in its group, persisted as HTML with images inlined |

| `devenv-xml-comments.md` | yes | 0578 |
| `devenv-random-test-data.md` | yes | routed to 0564 -- BC's own tests seed deliberately with `SetSeed(1)` and the `Any` library, which is why a fixed default for `Randomize()` is wrong |
| `devenv-integration-record-refactoring.md` | yes | routed to 0013 -- integration record tables were removed in favour of `SystemId` and `SystemModifiedAt` |
| `devenv-json-files.md` | yes | routed to 0033 -- the `app.json` manifest, and the app id binds table names at runtime |
| `devenv-programming-in-al.md` | yes | routed to 0029 -- which objects have triggers, and that a `local` method cannot be run from another object |
| `devenv-query-overview.md` | yes | routed to 0550 -- the query object overview, no mechanism that item does not carry |
| `devenv-restapi-overview.md` | yes | routed to 0567 -- the REST surface this sweep puts out of phase 1-3 scope |
| `devenv-post-process-report-pdf.md` | yes | routed to 0575 -- patching a generated PDF in `OnAfterDocumentReady` |
| `devenv-reports-create-printer-extension.md` | yes | routed to 0575 -- a worked printer extension over the same two events |
| `devenv-report-ext-example.md` | yes | routed to 0576 -- a report extension adding layouts, which that item measures |
| `devenv-report-add-barcodes.md` | yes | routed to 0576 -- barcodes in an RDLC or Word layout, a layout-file concern |
| `devenv-report-barcode-fonts.md` | yes | routed to 0576 -- the barcode fonts a layout may reference |
| `devenv-pages-action-bar-improvements.md` | yes | routed to 0539 -- a list of pages whose action bar was redesigned; no mechanism of its own |
| `devenv-walkthrough-workflow-events-responses.md` | yes | routed to 0057 -- a workflow walkthrough over the BaseApp's workflow objects and the generic event mechanism |
| `devenv-syntax.md` | yes | no task -- the Visual Studio Code snippet list |
| `devenv-robust-coding-practices.md` | yes | no task -- failure modelling and defensive-coding advice, with no platform rule in it |
| `devenv-page-scripting.md` | yes | no task -- a client record-and-replay tool producing YAML recordings for a pipeline, not AL; what it captures is board:0553's control tree seen through the client |
| `devenv-migrate-table-fields.md` | yes | no task -- moving a field between extensions while preserving its data; a deployment-time operation driven by `migration.json` |
| `devenv-migrate-table-fields-up.md` | yes | no task -- the same, up the dependency graph |
| `devenv-migrate-table-fields-down.md` | yes | no task -- the same, down the dependency graph |
| `devenv-move-table-fields-between-extensions.md` | yes | no task -- the same operation in full, with the schema steps |
| `devenv-scenarios-moving-table-fields.md` | yes | no task -- the scenarios the above three cover |

| `devenv-page-type-configuration-dialog.md` | yes | 0579 |
| `integration-overview.md` | yes | 0579 -- an architect's map of the three web-service stacks and the Microsoft 365 integrations; every mechanism is either board:0567's endpoint or an external product |

**470 of 470 root pages read.**
**The per-page index at the end of this file is the AUTHORITY for this family's count**, and it
disagreed with the running counter by eight when it was first built -- the counter had been kept by
hand per pass and the index is derived from the items' own `Source:` lines. The index wins; the
counter is corrected to match. That is the same rule the sweep applies to a measured population.
 The remaining 359 are listed in the running order at the top of this
file and are worked by theme.

---

## `developer/` root, twelfth pass -- views and run-time filter surfaces

board:0030's subject and board:0018's. Six pages, two items.

**A view carries THREE things and only two have property items**: the filter (board:0475), the sort
(board:0352), and **layout changes -- "modifying page columns, moving them"** -- which no property page
covers. So a view is a partial page customization with a name and a filter, and board:0542 gives the
two mechanisms one representation rather than two.

**Source order, for the third time.** Views appear in declaration order, as do board:0538's role-centre
actions and board:0539's area contents. board:0539 found that ACTIONS have anchors, so AL declares
their merge order; **views have none**, so for views agiru must declare it.

**A `FilterPageBuilder` page has no page object.** It is assembled by AL at run time from table and
field identifiers and run modally -- the only such page in this sweep. board:0030's renderer therefore
needs a second entry point, "render this constructed descriptor", and that has to be in the design from
the start rather than retrofitted. board:0198's `[FilterPageHandler]` is its test seam, with the
signature board:0540 lists.

**A filter token is resolved by an AL event subscriber**, `OnResolveTextFilterToken` on the
`Filter Tokens` codeunit -- which is board:0512's finding in its clearest form: the runtime raises,
the transpiled BaseApp resolves, and `%mycustomers` is a name the runtime must not know. The
subscriber's `var Handled: Boolean` is board:0516's pattern, and a copied `var` means the raw token
reaches board:0509's parser as a literal.

And one default that runs the other way: the documented subscriber passes **`true, true`** for
`SkipOnMissingLicense` and `SkipOnMissingPermission`, so a token resolver the user cannot execute is
SKIPPED rather than failing the filter -- the opposite of board:0513's global default of `false`.

---

## `developer/` root, thirteenth pass -- namespaces, access, extension objects

board:0033's subject. Ten pages, two items.

**`Access = Internal` must NOT become C++ `private`, and the documentation is emphatic about why.**
"Access modifiers are ONLY taken into consideration at COMPILE TIME ... at runtime, ANY MODULE CAN
ACCESS THE TABLE by using reflection-based mechanisms such as `RecordRef`, or `TransferFields`. And the
`OnRun` trigger can be run on `internal` codeunits by using `Codeunit.Run`." Three named escape
hatches. board:0359 measured **2 532** `Internal` declarations and left this open; the answer is that
the check belongs to the GENERATOR and the emitted C++ leaves the object reachable -- the opposite of
the instinct, and stricter would break BaseApp code that reaches internal objects deliberately.

**A name is unique per KIND per namespace in AL, and per name in C++.** So AL allows a table and a
codeunit both called `Foo` in one namespace and C++ does not. The generated name must therefore carry
the kind -- board:0026 owns the mechanism and this page is why it is required rather than tidy.

**A namespace may span MODULES**, so it is not the app boundary; board:0033's app is the module and the
namespace cuts across it. CLAUDE.md's `scope.json` is a whitelist over namespaces, which makes the
namespace the transpiler's own selection key.

**The 2 000 000 000 boundary appears a second time.** board:0511 read it as the reserved SYSTEM FIELD
number range; here it is the system OBJECT range, and "system and virtual tables cannot be extended"
is what it means concretely -- board:0523's `Integer` (2000000026) and `Date` (2000000007) are inside
it.

**A tooltip on a TABLE FIELD is inherited by every page that uses the field.** board:0385 filed
`ToolTip` at 159 993 as a page-control property; this makes it a two-level fallback resolved by the
generator, with the control's own value winning. That changes 0385's shape from one member to a folded
chain.

One contradiction recorded rather than resolved: this page says **"the default value for ALL objects is
that they're extensible"**, while `devenv-extensible-property.md` says the default is **false on
enums**. board:0360 already asserts the per-kind default and its reading stands -- 1 226 enums
declaring `Extensible = false` would be redundant under this page's reading -- but the AL source
settles it.

---

## `developer/` root, fourteenth pass -- report layouts and the XMLport schema

board:0063's subject and board:0065's. Twelve pages, three items.

**An event named `OnAfter` fires BEFORE, and the documentation says so.**
`OnAfterSubstituteReport` "is called `OnAfterSubstituteReport` to match the pattern followed by other
events in the `ReportManagement` codeunit, but the subscriber will be INVOKED BEFORE the substitution
takes place." An implementation that followed the name would make substitution impossible. And its
`var NewReportId: Integer` is board:0516's pattern for the third time in this sweep -- a copied `var`
means every substitution is silently ignored and nothing raises.

**A report layout receives data the DATASET does not contain.** Two system data items, `ReportMetadata`
and `ReportRequest`, "aren't part of the report dataset but are only present in the layout XML" -- nine
named columns including the extension's id, name, publisher and version from `app.json`, the request
page's About title and text (board:0388) and the help link (board:0393). An implementation that fed the
layout only the dataset leaves all nine blank.

**The dataset SCHEMA is a published artefact in two directions.** The build generates the layout file's
skeleton from it, a human edits it in Word, and the layout binds by name -- so board:0396's
`IncludeCaption` entries are part of a contract rather than a convenience, and the schema is something
the generator emits rather than only a runtime structure.

**An XMLport schema is five node kinds with four structural rules**, all decidable: exactly one root,
the root is an element (a `textelement` when the format is XML), attributes before elements within a
parent, attributes are leaves. And a `tableelement` is a LOOP -- "the code nested inside is iterated
for all records" -- so the schema tree is also the control flow.

**The node kind discriminates four other properties.** board:0444's occurrence applies to elements,
board:0445's namespace prefix to element nodes only, board:0448's `XmlName` to all five. So the
descriptor is a tagged union of five shapes, not one node with optional fields, or the dispatch moves
to run time.

One partial resolution recorded as partial: board:0457 found `WordMergeDataItem` at 299 against
board:0452's **2** `WordLayout` declarations. This pass's "build the extension to GENERATE the Word
file" offers an explanation -- a layout may exist as a file added through the `rendering` section
without a `WordLayout` property -- but the AL source still settles it.

---

## `developer/` root, fifteenth pass -- the report and query objects

board:0063's subject and board:0064's. Eight pages, two items.

**A report column is one of FOUR things**: a field, a variable, an expression, or a text constant. Three
of them have no field behind them -- so board:0396's `IncludeCaption` applies only to the first, and
board:0491's `AutoFormat` has no field to look up a currency on for the other three. That makes the
dataset a tagged list of four column shapes, exactly as board:0548's XMLport schema is a tagged union of
five node kinds.

**A report's section ORDER is part of the syntax** -- properties, `dataset`, `requestpage`, `rendering`,
code -- so the parser enforces it rather than accepting any order.

**The `Integer` virtual table is how a QUERY becomes a report data item.** board:0523 found it is how AL
writes a `for` loop; this page gives it a second named use: a global query variable, an `Integer` data
item, and `OnPreDataItem` plus `OnAfterGetRecord` triggers pulling the rows. Two pages, one escape
hatch from table-shaped data.

**A query's data-item HIERARCHY is its join tree.** "The hierarchy ... determines the sequence in which
data items are linked, which in turn controls the results", and both `DataItemLink` and `SqlJoinType`
are set on the LOWER data item. So the `SELECT` is a depth-first walk, one `JOIN` per level, and
flattening the tree would need the order stored separately.

**A link names the parent by its ALIAS**, not by table -- `DataItemLink = FieldY = DataItem1.FieldX` --
so two data items over one table are distinguishable. board:0550's negative control is exactly that
case, which a table-keyed representation resolves to the same node.

**A codeunit is reachable two ways with different transaction behaviour**: `Codeunit.Run`, with
board:0077's commit-and-raise semantics, and a direct procedure call, with none. The same object, two
entry points -- stated plainly on the codeunit page and worth having beside board:0077.

---

## `developer/` root, sixteenth pass -- profiles, and the preprocessor nobody had counted

Eleven pages, two items, and the second one found a whole language feature.

**A page customization is LESS than a page extension and the documentation draws the line:** "you
CAN'T ADD VARIABLES, PROCEDURES, OR TRIGGERS. You can add actions, fields, and groups." So it is a
LAYOUT DELTA -- move, add, hide -- with nothing executable in it. board:0545's extension objects merge
at build time and carry code; a customization does not, which makes it a different mechanism rather
than a restricted one.

**AL HAS A PREPROCESSOR AND MICROSOFT'S OWN DEPRECATION PRACTICE IS BUILT ON IT.**
`devenv-deprecation-guidelines.md`: "we add the preprocessor statements `#if`, `#else`, `#endif`
surrounding the code to be obsoleted ... symbols, where the pattern is `CLEAN<Version>` ... **these
symbols AREN'T SHIPPED WITH THE PRODUCT.**" So an undefined symbol is FALSE and that is the shipping
default -- which is exactly what `src/al/Lexer.cpp:305` already does.

Measured over BCApps, 2026-09-04:

| directive | count |
|---|---|
| `#pragma` | 27 148 |
| `#endif` | 10 125 |
| `#if` | 9 946 |
| `#else` | 990 |
| `#region` | 807 |
| `#endregion` | 806 |

and the top conditions `#if not CLEAN28` 3 235, `#if not CLEAN27` 3 201, `#if not CLEAN29` 1 637.

**The number that mattered was inside the 27 148.** `ApplyDirectives` handles `if`/`else`/`endif` and
`continue`s past everything else, so every pragma is dropped. Splitting them: **`#pragma warning`
27 136**, which is a compiler concern and correctly ignored, and **`#pragma implicitwith` 14, in 12
files** -- 10 `disable`, 4 `restore`, every one a page, none in the Base Application. That is
board:0086's implicit-`with` resolution order with its off-switch removed.

**The honest reading is smaller than the alarming one and it is recorded that way.** `disable` turns
the implicit `with` OFF, so code under it is already qualified; dropping the pragma re-enables a
fallback that code does not use, and that is harmless while the implicit `with` sits LAST in the
resolution order. It becomes a defect exactly when it does not -- so the finding is not fourteen wrong
values, it is fourteen places with no second line of defence. Recorded in board:0552 as a
correctness requirement whose call-site population may be 0, because whether `scope.json` admits
`Apps/CZ`, `Apps/NA` and `Apps/NO` is a fact about `apps.json` and was not measured here.

**And the census answered its own ordering question the other way.** The item had named the pragma
split as its most consequential unmeasured number; measuring it moved `implicitwith` to SECOND, behind
the 9 946 `#if` that decide which BaseApp is translated at all. A measurement that reorders the item
that asked for it is the cheapest kind there is.

---

## `developer/` root, seventeenth pass -- the page layout is a tree, and nobody kept it

Five pages, three items, and the pass began by reading `src/gen/PageWriter.cpp` rather than another
`.md`.

**CLAUDE.md's phase 2 rests on "a page's layout is already `constexpr` metadata -- so a renderer walks
the control tree". Neither half is true yet.** `src/gen/PageWriter.cpp:52` flattens the layout into
three vectors keyed by name, and the container kinds match no predicate, so they are visited for their
children and then vanish. Counted over `~/Git/BCApps/src` by `^\s*<kind>\s*\(`:

| dropped | count |
|---|---:|
| `group(` | 32 177 |
| `area(` | 18 920 |
| `repeater(` | 4 387 |
| `cuegroup(` | 279 |
| `fixed(` | 230 |
| `grid(` | 53 |

**59 979 controls that decide the layout, and the AST already holds every one of them** --
`src/al/Ast.h:104` carries `kind`, `properties`, `triggers` and `children`, and the writer keeps only
the name. Plus one categorisation defect with a number: `IsField` accepts `systempart`, so **4 045
system parts are emitted as fields**.

**The sizing rules are what make the tree compulsory rather than tidy.** "When a ListPart is embedded
as the LAST part on the page, it expands to fill space"; "the Document page type allows the FIRST
ListPart to use extra vertical space". Both are questions about a node's position among its SIBLINGS,
and a flat list cannot answer either.

**A rule the source breaks fifty times, and the instinct it kills.** The page states as `IMPORTANT`
that entity-oriented pages must not contain a `Repeater` and that `List` pages must contain one.
Measured per file over the 6 961 single-page files: **30 entity-oriented pages carry a repeater**
(`Card` 13, `ListPlus` 8, `CardPart` 6, `Document` 3) **and 20 `List` pages carry none.** Four of the
thirty are in `Layers/W1/BaseApp`, so `scope.json` does not filter them away. CLAUDE.md says anything
decidable at translation time is a `static_assert`; **here that would reject fifty pages the platform
loads.** A `static_assert` is right for what the platform REFUSES, and this is guidance with a
degraded rendering behind it. The contradiction is recorded, not resolved.

**Three refusals ARE legitimate**, because the AL compiler itself errors: a `CardPart`/`ListPart`
embedding a part, a second `area(FactBoxes)`, and a FactBox part whose target is not a `CardPart` or
`ListPart`.

**A FactBox's LOADING ORDER is specified, and it is observable from AL.** Content first, then each
FactBox top to bottom, `Visible = false` never loaded, `OnOpenPage` run once and never again while the
page is open, and not asynchronous -- "a controlled sequence ... still within the same session". So
the determinism commitment is not at risk: the platform declares the order. The predecessor paid for
the row-following half twice (WI-1190 `GAINED 2`, WI-1228 `GAINED 1`), and WI-1228 records as REFUTED
the assumption that a FactBox is always filled by `SubPageLink` -- `LoadDataFromRecord` into a
`SourceTableTemporary` buffer is the other way.

**`systempart` has a fourth target the documentation does not list.** The page tabulates `Links`,
`Notes`, `Summary`; the source declares `Notes` 1 978, `Links` 1 933 and **`MyNotes` 134**, summing to
exactly the 4 045 measured. `Summary` appears zero times, which fits: it is on by default and BC never
needs to hide it.

**An `actionref` is a REFERENCE and the v20 copy semantics are the bug it fixed.** 18 275 of them
against 1 228 legacy `Promoted =`, fifteen to one. Four rendering rules follow from the link:
hiding the base hides the ref; **hiding the base's GROUP hides the ref even when the base is visible**;
a group whose every action is promoted stops rendering, recursively; and `Home`/`Process` unpacks when
it is the only promoted group. Rule two is the one a resolved copy passes and a reference fails --
which is why board:0555's negative control is that case alone.

**A second documentation contradiction, settled by the source.** `devenv-promoted-actions.md` says "up
to 10 categories" and stops at `Category10`; `devenv-promotedcategory-property.md` lists through
`Category12`; the source declares **`Category11` 13 times.** The property page and the source agree, so
the article is the page that is wrong. And the `PromotedCategory` values sum to 982 against a measured
983 -- one declaration whose value the value-extracting pattern does not reach. The missing row is
stated rather than absorbed.

---

## `developer/` root, eighteenth pass -- the query SELECT, and a gate that was wrong

Five pages, one item, and one CORRECTION to an item filed eight passes ago.

**A query has no `GROUP BY` and the grouping key is derived by exclusion** -- an aggregate on one
column groups by all the others. **With one exception the property page does not carry:** *"a column
that applies a DATE METHOD is still part of the group, unlike columns that apply an aggregate
method."* So `Method` is two families with opposite behaviour under one name, and the C++ shape that
gets it right is a `constexpr bool Aggregates(QueryMethod)` -- the exception becomes the ABSENCE of a
special case, because `Day`, `Month` and `Year` simply are not aggregates.

**board:0462's gate was wrong and is corrected.** It read *"a `Year` method on a zero date returns
1900"*, from `devenv-method-property.md`. `devenv-query-retrieve-date-data.md` carries the version
table:

| BC version | `Day` | `Month` | `Year` |
|---|---:|---:|---:|
| 26 and earlier | 1 | 1 | 1753 |
| **27 and later** | **0** | **0** | **0** |

BCApps is 30.0 and the demo database 28.4, both on the later side, **so the answer is 0**. The 1753 is
the SQL blank-date sentinel leaking through, and it stops leaking in 27. Neither page is wrong; the
property page describes the older behaviour and does not say so. **A documentation sweep that reads
only the property pages gets this backwards**, which is the argument for the concept pages the goal
already made -- here with a case number.

**`Average` is declared ZERO times in BCApps.** board:0462 is titled for its integer-truncation trap;
the trap is real, documented, and has no call site. It stays -- a documented behaviour without a gate
case is a gap -- but it is LAST of the seven methods. `Sum` 223 and `Count` 36 are 259 of 268 and are
the whole first pass; `Year`, `Month`, `Max`, `Day` 2 each and `Min` 1 are the remaining nine.

**The filter model is a four-position lattice and two of the positions are only on the concept page**:
a `ColumnFilter` on an AGGREGATED column becomes a `HAVING` rather than a `WHERE`, and a `filter` row
exists precisely to filter a field the dataset does not carry. A filter row is also a TYPED member --
`SetRange(Entry_Type, ItemMovements.Entry_Type::Sale)` resolves an option member through a control the
dataset never returns.

**A documentation self-contradiction inside one page, settled by the source.** The `Min` and `Max`
sections say the column "automatically changes to `Min_Quantity`"; the result table printed directly
beneath each says `Qty`, the declared name. Measured over `~/Git/BCApps/src` by
`\.(Min|Max|Day|Month|Year)_[A-Za-z0-9_]+`: **zero references, all five prefixes.** The source
disambiguates by hand instead -- `finishedAtDay`, `finishedAtMonth`, `finishedAtYear` over one field;
`MaxEntryNo` -- and the documentation's own worked example declares `column(Sum_Quantity; Quantity)`
and reads `ItemMovements.Sum_Quantity`, which a renaming platform would have made
`Sum_Sum_Quantity`. **AL sees the declared name.** Whether the exposed OData column is prefixed is
settled by neither source nor documentation and is recorded as unsettled.

**Two counts are NOT SEPARABLE and are printed as they are.** `column(` 105 669 and `dataitem(` 9 046
count report, query and XMLport controls together -- the pattern sees a line, not an enclosing object.
No rounded query share is offered in their place.

**And the query hole is deeper than the generator.** `src/al/Ast.h` has no query node at all, while
`src/gen/CodeunitWriter.cpp:129` already accepts `Query` as a VARIABLE type. The name exists, the
object does not, and nothing reports the difference -- board:0034's count seen from the inside.

---

## `developer/` root, nineteenth pass -- the report trigger order, printed as a listing

Two pages, two items. `devenv-report-triggers.md` does not describe the order, it PRINTS it -- 34
lines of `ROOT` and `CHILD` -- so it is a specification a gate compares against line for line, and
board:0302, 0303, 0306 and 0308 turn out to be four points on one constraint none of them names.

**A preview runs the whole report twice, and the documentation says so plainly**: *"before the report
is even executed, the `OnInitReport` trigger has already run twice."* The entire data-item walk runs
in the CHILD instance to build the preview and again in the ROOT to produce the document -- so every
`OnAfterGetRecord` fires twice per record. An implementation that reused the child's data would fire
none of the root's triggers, and 1 643 reports are written against them firing.

**The mode default is the opposite of what the page says.** Multiple-preview requires `SaveValues` AND
`AllowScheduling` both true; `devenv-allowscheduling-property.md` says its default is **true** and
`devenv-savevalues-property.md` says **false**, which lands in the preview & close cell -- while the
concept page asserts *"by default, reports use the multiple-preview mode."* Recorded, not resolved: two
pages state a default each, the third states a conclusion, and a conclusion is the thing that can be
wrong.

**The census is decided by `SaveValues`, not by `AllowScheduling`.** `SaveValues = true` **1 667**;
`AllowScheduling` declared **25 times in the whole tree, 24 of them `false`**. So roughly 1 643 reports
run everything twice and the ~445 that never declare `SaveValues` run once. **"Roughly" is doing work
and is left in**: the two properties are counted independently and the 24-object overlap is not
resolved per object, so 1 643 is an upper bound rather than a fact.

**Three counts are NOT separable and are printed as they are.** `OnAfterGetRecord` **8 137** counts the
page trigger of the same name; `requestpage` **2 267** against 2 135 reports counts XMLports and report
extensions; `report <id>` **2 135** is the tree, not CLAUDE.md's 668, which is the `scope.json` share --
a different question, not a smaller answer.

**And one paragraph under "General" is a NAME RESOLUTION rule, not a report one.** *"If you have two
methods with the same name, one defined in a report and the other in a table ... a call to the method
invokes the method that's defined in the TABLE."* The table wins over the object's own declaration,
from a source expression or a trigger.

**Measured: 52 reports, 25 distinct names** -- `CheckBalance` 13, `CheckGLAcc` 13, `GetLocation` 13,
`CheckICPartner` 12 -- built by intersecting each report's `procedure` names with the `procedure` names
of its own data items' tables, over 3 736 tables and 2 135 reports. **The number is an UPPER BOUND and
the item says so**: it does not check that the name is actually called from a source expression or a
trigger, which is the condition the rule attaches to. Most of the sites are the localisation
general-journal test reports, so the consequence is a journal test reporting the wrong errors.

The predecessor met the same precedence one object kind over -- WI-1086, *"an expression control loses
against a same-named table column"* -- and had to encode it in a language where nothing checked it.
Here the generator emits `Rec.CheckBalance()` explicitly, because C++ would pick the other one and the
deviation has to be visible rather than clever.

---

## `developer/` root, twentieth pass -- the entitlement ceiling, and a deferred number counted

Two pages, one item, and two earlier items closed out.

**board:0381 left a question open in its own words** -- *"whether agiru HAS a licence is the open
question and it is not this item's to answer"* -- and both concept pages answer it in one sentence,
stated twice without qualification: **"entitlements are ONLY USED IN THE ONLINE VERSION."** agiru is
one process on the user's own machine, so the entitlement object transpiles in full and gates nothing.
That is not a shortcut: an implementation that gated here would be MORE restrictive than BC.

**And the platform's composition rule makes the absence fall out rather than be a special case:**
*"actual permissions are the INTERSECTION between the permissions the user is ENTITLED to and the
permissions the user is ASSIGNED."* With no entitlement layer the entitled operand is the universe and
the intersection is the assigned set. board:0492's lattice is the `assigned` half; this names the
operator above it. **The formula is written down anyway**, with `Entitled` returning all bits, because
a function returning a constant is honest about which half is missing and a dropped operand would have
to be rediscovered.

**board:0483's deferred number is now counted.** That item recorded *"1 555 is not this property's
population ... the entitlement half is counted by file extension when the item is pulled"*. Counted
over the 206 `entitlement` declarations themselves: **206 `Type` declarations, so every entitlement
declares one.**

| `Type` | count |
|---|---:|
| `PerUserServicePlan` | **101** |
| `Role` | 56 |
| `ConcurrentUserServicePlan` | 16 |
| `ApplicationScope` | 14 |
| `Implicit` | 10 |
| `Application` | 9 |
| `FlatRateServicePlan`, `PerUserOfferPlan`, `Unlicensed`, `Group` | **0** |

**The concept page's examples cover a different six than the source uses.** It demonstrates `Role`,
`PerUserOfferPlan`, `Unlicensed`, `Group`, `Application` and `ApplicationScope` -- three of which the
source never declares -- and never shows `PerUserServicePlan`, the most common value in the tree by a
factor of two. `devenv-type-entitlement-property.md` is the enumeration with all ten; **the concept
page is a sample and not a representative one.** Which is the argument for reading both, from the
other direction than the last pass made it.

`RoleType` splits `Delegated` 32 / `Local` 24, summing to exactly the 56 `Role` entitlements.
**`ExcludedPermissionSets` is declared ONCE in 2.56 million lines** -- board:0379 and board:0492 built
a four-rule truth table around it, and the truth table is right; the population decides its order and
not its correctness.

**The measurement reordered the subject.** `NavApp.IsEntitled` and `NavApp.IsUnlicensed` are documented
with worked examples and have **0 call sites each**. The way BC's own code tests entitlement is
`Record.ReadPermission()` **1 009** and `Record.WritePermission()` **383** -- which return the
intersection and never name an entitlement. So the licence layer is 206 objects with no runtime
effect, and the two permission builtins are 1 392 call sites that decide whether BaseApp code takes a
branch. **They come first, and the entitlement object follows.**

And the blind gate is named: a `ReadPermission` that returns `true` unconditionally -- which is what
"there is no permission layer" produces today -- passes all 1 009 call sites. Only revoking an assigned
permission distinguishes "the intersection was computed" from "nothing was checked".

---

## The derived index had a hole, and it was structural

**The per-page index is derived from items' `Source:` lines. The 78 pre-existing roots have no
`Source:` line** -- they predate this sweep and carry the older `Type: root` / `State: open` header. So
**a page ROUTED INTO a pre-existing root was invisible to the index and to the counter**, which is
exactly the case the goal permits: *"a page whose task a pre-existing root already IS gets routed
there."*

Found by accident: this pass opened `devenv-read-isolation.md` as unread, and board:0012 already
carried a section headed *"`Record.ReadIsolation` IS A FOURTH DIAL"* with the page's own annotated AL
quoted in it and the parenthesis *"(read 2026-09-04, board:0071)"*.

**Swept for the rest.** Every root page named anywhere in `board/` and absent from the ledger:
**15 pages**, every one of them inside a root with no `Source:` header, every one of them quoted or
explicitly marked read. The counter moves 180 -> **195** and no item count changes, because a routed
page files nothing new.

| page | root |
|---|---|
| `devenv-read-isolation.md` | 0012 -- `ReadIsolation` as a fourth dial over the tri-state |
| `devenv-partial-records.md`, `-faq.md` | 0048 -- `SetLoadFields` and the widening set |
| `devenv-permissions-on-database-objects.md` | 0062 -- direct `RIMD` against indirect `rimd` |
| `devenv-number-sequences.md` | 0028 -- a sequence survives a rollback |
| `devenv-object-specifications-limitations.md` | 0081 -- every documented limit is a `static_assert` |
| `devenv-al-this-keyword.md` | 0026 -- `this` is a keyword the transpiler does not know |
| `devenv-extensible-enums.md` | 0053 -- a comma in a caption breaks the joined list |
| `devenv-extending-application-areas.md` | 0030 -- the area filter DENIES by default |
| `devenv-al-menusuite-functionality.md` | 0083 -- `UsageCategory` decides searchability |
| `devenv-oncompanyopencompleted.md` | 0057 -- the session's own lifecycle event |
| `devenv-ncci-overview.md` | 0019 -- the columnstore successor PostgreSQL does not have |
| `devenv-debug-upgrade-install-code.md`, `devenv-methodtype-property-upgrade-codeunits.md` | 0070 -- install and upgrade codeunits |
| `devenv-report-custom-render.md` | 0063 -- `External` rendering as a subscriber |

**THE CONVENTION, so it does not recur: a page routed to a pre-existing root gets its ledger row
written BY HAND, in the pass that reads it.** The derived index cannot find it, and a counter that
cannot see its own input is the blind gate this ledger's own rules name.

**Three ncci pages stay UNREAD and are not swept in with the rest.** board:0019 names
`devenv-ncci-and-sql-server`, `-performance` and `-tuning-and-tracing` in a parenthesis listing the
family; naming is not reading, and the distinction is kept rather than rounded up to a nicer number.
`devenv-ncci-overview.md` is different -- 0019 quotes it and marks it read.

---

## `developer/` root, twenty-first pass -- the one tree that is not a tree

Three pages, one item, and it CLOSES board:0553's open contradiction.

**A repeater's rows are flat and the hierarchy is an integer per row** -- `IndentationColumn` resolves
to an integer that is the indentation level, from a source-table field or a page variable. So the one
place a user sees a tree in a list is the one place the layout does NOT nest, which is the opposite of
everything else board:0553 establishes. It also has to stay that way: materialising the hierarchy
would hold the result set, which is what CLAUDE.md's streaming requirement forbids at 100 million
rows.

**And one layout decision in this whole sweep cannot be `constexpr`.** A collapsible hierarchy
*"ALWAYS INDENTS THE LEFT-MOST VISIBLE COLUMN ... the `IndentationControls` property is IGNORED. If
users customize the page by moving another column first, the moved column will be indented instead."*
The indented column depends on the viewer's personalisation. Named now rather than discovered after
the `constexpr` is written.

**31 of the 46 collapsible pages declare `IndentationControls` anyway**, counted per file over pages
carrying both properties -- among them `ALTestTool.Page.al`, the test runner's own page,
`PermissionSetTree`, `AssistedSetup` and `ManualSetup`. **BC's own source declaring something that
does nothing, 31 times.** CLAUDE.md says accepting a declaration and doing nothing with it is worse
than refusing it; this is the documented exception, and it costs a COUNTER rather than a
`static_assert` -- the generator reports how many it discarded, so the number is visible instead of
silent.

**board:0553's contradiction is resolved, by citation.** That item measured 30 entity-oriented pages
carrying a repeater against an `IMPORTANT` saying they must not, and concluded from the count alone
that a `static_assert` would be wrong. `devenv-repeater-controls.md` names the enforcement:
**UICop Warning AW0008** -- an analyzer finding, not a compiler error. The platform loads all 30. The
reading reached from the count is confirmed rather than only inferred, and the page even supplies BC's
own workaround: put the repeater in a `ListPart` and embed that.

Two refusals the web client DOES make, and both are `constexpr` facts: a `part` inside a `repeater`,
and a `field` inside one whose source is a `FlowFilter`.

**Populations**: `repeater(` 4 387, `IndentationColumn` 181, `IndentationControls` 168, `ShowAsTree`
46 (all `true`), `TreeInitialState` 10 (`CollapseAll` 9, `ExpandAll` 1), `FreezeColumn` 37, `Width`
248. So the plain flat repeater is 96 % of the subject and `ShowAsTree` is last of the three.

**And a population disagreement between two items turned out to be a scope difference.** board:0047
prints `FieldClass = FlowFilter` **490** and board:0339/0510 print **1 510**. 0047 is scoped to
`Layers/W1` and dated 2026-09-03, before the pattern was settled. Re-measured over the same subtree
with the settled pattern, 0047's numbers reproduce -- `FlowField` 2 153, `FlowFilter` 490,
`CalcFields` 3 604, `CalcSums` 1 319 -- with two drifts of a few counts (`CalcFormula` 2 149 against
2 150, `SetAutoCalcFields` 241 against 251) from the shell's wrapped `grep` and the leading dot. **The
scope is now stated in 0047 and the drifts are left visible**, because a reader comparing 490 with
1 510 would otherwise find a contradiction that is not there.

---

## `developer/` root, twenty-second pass -- a FastTab is a group, and the split is source order

Four pages, one item.

**There is no `fasttab()` in the layout grammar.** *"A FastTab IS A GROUP CONTROL directly within the
`content` area."* So the kind is POSITIONAL -- a `group` one level under `area(content)` -- and a
`group` inside a `group` is something else. board:0553's census counts 32 177 groups; which of them
are FastTabs is a question only the tree can answer, and flattening destroys exactly the parent that
answers it.

**Two distribution rules in one container, stated in one sentence each.** Fields are dealt into two
columns in SOURCE ORDER, left first, "as equal as possible" BY AREA. But *"when you group fields on a
FastTab, the GROUPS are distributed evenly between the left and right columns. FIELDS AREN'T."* An
implementation with one rule passes the field case and fails the group case -- which is why
board:0561's negative control is the group case and not the field case.

**The initial collapsed state is not a developer decision.** *"The first two parts or FastTabs are
automatically expanded. All other ... collapsed ... developers CAN'T SPECIFY the starting state."* A
positional rule with no property, and board:0554's FactBox rule makes it consequential: a collapsed
part does not load, so a FastTab's position in source order decides whether its triggers run on open.

**`Importance` is the largest number in the subject and it decides the order**: 16 781 declarations --
`Additional` **12 490**, `Promoted` 4 128, `Standard` 163. So on a collapsed FastTab three quarters of
what is declared is not visible, and a renderer ignoring the property shows every field on every card
in the product.

**A second batch of declarations the client drops, after board:0560's 31.** The web client does not
support `GridLayout = Rows` (**13 declarations**), `RowSpan` (6) or `ColumnSpan` (7) -- *"in the Web
client, fields can only be arranged in COLUMNS"*, the rest being Windows-client behaviour, and there is
no Windows client any more. **26 more declarations that do nothing**, treated the same way: counted by
the generator, never refused, because refusing would reject the BaseApp.

**And a third silent degradation**: a `grid` or `fixed` NOT placed inside a `group` *"will inherit
properties as if it were a typical Group control and NONE of the Grid or Fixed properties will
apply"*. Decidable at translation time, because the parent's kind is in the tree.

**One container overrides a field's own property.** *"Fields in a fixed layout are NOT EDITABLE even if
`Editable` is set to `true`."* `Editable` is declared 51 886 times; this is the only case in the sweep
where a property is overridden rather than combined.

**`CaptionML` is declared THREE times in 2.56 million lines**, against `Caption` at 288 491 -- and the
structural-group rule is written against `CaptionML`. Read literally, almost every group in BC is
structural, which is plainly not the intent. **The documentation names a property the source has
abandoned.** Recorded, not resolved; the sensible reading is that the rule means "has no caption" and
covers both spellings.

`grid(` 53 against `fixed(` 230: the "new and preferred" control is outnumbered four to one by the one
it replaces. And `ShowCaption` is 99.5 % `false` -- 8 595 of 8 636 -- so it exists only to remove a
caption.

---

## `developer/` root, twenty-third pass -- reading the page found the type already broken

One page, one item, and it is the sweep's first `Type: bug` since board:0349.

**`Notification` is one of the few AL types this tree has already BUILT**, so reading its concept page
was a comparison rather than a design exercise -- and `src/net/Notification.cpp` is 28 lines with three
defects in them.

**`AddAction` writes into the user's own key-value store.** `src/net/Notification.cpp:19` stores each
action as `SetData("Action" + n, caption + "|" + id + "|" + method)`. `SetData` and `GetData` are an
AL-VISIBLE map, so: `SetData('Action0', x)` overwrites the first action; `GetData('Action0')` hands AL
back `"caption|5|Method"` for a key it never set; and `HasData('Action0')` returns **true** for that
key -- on a method that exists precisely to answer that question. **487 `AddAction` call sites write
into the map 488 `SetData` calls read.**

The `|` separator has no escaping either, and a caption is free AL text.

**The door's own decision is fine and is not what is wrong.** `include/type/Notification.h:101` states
*"THE ACTION IS RECORDED AND NOT WIRED ... recording it lets a test see that the action was offered"*.
Recording it in the AL's namespace is not part of that decision.

**`Send()` does not send** -- it assigns a Guid and returns (`:24`). `grep -rn
SendNotificationHandler src/ include/` finds nothing, so board:0218's handler has no caller and **311
call sites do nothing observable**. `Recall()` is an empty body and board:0211's handler likewise, over
253 call sites. A UT case asserting the handler fired cannot pass -- and cannot fail loudly either.

**And both have the wrong signature.** The method pages give `[Ok := ] Notification.Send()` with
*"if you omit this optional return value and the operation does not execute successfully, A RUNTIME
ERROR WILL OCCUR."* `void Send()` cannot carry that: it is CLAUDE.md's named **value context** trap, on
a type that already exists, so it is a signature that is wrong today rather than a design question.

**One documented overload is missing**: the door's `\brief` says
`AddAction(Caption, CodeunitId, MethodName [, Tooltip])` and only the three-argument form is declared,
while `methods-auto/notification/` carries both files.

**Populations**: 731 `Notification` variables, `SetData` 488, `AddAction` 487, `Send` 311, `GetData`
310, `Recall` 253. **`NotificationScope::` is 321 declarations and every one is `LocalScope`** --
agreeing with the page's *"GlobalScope is currently not supported"*, so the unsupported scope has no
call site at all.

**One count is a LOWER bound and says so**: `.Send(` is 656 across the tree and not separable, because
`HttpClient` and `Email` declare a `Send` too; 311 is the qualified `Notification.Send` form only, and
no interpolation is offered for the rest.

**What the pass is really evidence for**: the sweep's value is not only in the objects that do not
exist yet. Four of the five findings here are in code that compiles, passes `make lint`, and is
documented -- and none of them would surface until a UT case asked a `Notification` a question.

---

## `developer/` root, twenty-fourth pass -- two levels of HTTP failure

One page, one item, and the method the last pass suggested: **read the concept page of a type that
already exists in `include/type/` and compare.** Where `Notification` yielded three defects,
`HttpClient` yields none -- eighteen `RefuseDoor` calls (`src/rt/Door.cpp:892`-`:999`) and correct
signatures throughout. **A loud refusal is the state CLAUDE.md asks for, and this is what it looks
like when the rule is kept.**

**The one thing an implementation gets wrong for free**: `Client.Get(...)` returning `true` means a
response ARRIVED, not that it was a good one. A 500 is a successful call. Folding the HTTP status into
the return value is the convenient shape and it makes every
`if not Response.IsSuccessStatusCode()` unreachable -- **122 call sites where a server error would
become a success.** That is board:0563's negative control and nothing else catches it.

**Headers are a MULTI-MAP, not a `Dictionary`.** BC's own example writes `Contains` then `Remove` then
`Add` to change `Content-Type`, because `Add` appends. board:0078's `Dictionary` is single-valued and
would silently drop the second `Set-Cookie`.

**And `GetHeaders` is a VIEW, not a copy** -- the example mutates what it hands back and expects the
change to reach the owner. 200 `GetHeaders` and 162 `ReadAs` call sites are `var` out parameters, the
first named trap in CLAUDE.md's table, closed in C++ only as long as the generator never copies.

**`HttpRequestMessage` 323 outnumbers `HttpClient` 148 more than two to one**, so `Send(Request,
Response)` is the normal form and the five verb shortcuts are the exceptions. `HttpStatusCode` 215
against `IsSuccessStatusCode` 122: BC reads the code more often than it asks the Boolean.

**One set of counts is excluded rather than printed.** The only pattern available for the per-verb
split is `Client\.<Verb>\(`, which matches a variable literally NAMED `Client`. It returns `Send` 99,
`Get` 39, `Post` 11, `Put` 6, `Delete` 5, `Patch` 2 -- every one a lower bound of unknown tightness.
The RATIO is kept, the absolute numbers are not offered as facts, and the ratio agrees with the
declaration counts independently.

**`UseServerCertificateValidation` has ONE call site in 2.56 million lines.** Validation is on by
default and from version 27 the per-call method is the only way off. agiru validates by default; the
pre-27 feature key is a superseded scheme and not a target.

**Anti-SSRF blocks internal addresses by default, and agiru is on premises by construction** -- so the
on-premises behaviour is the one to implement, as externalised configuration with the safe default
kept, rather than as a constant in the code.

---

## `developer/` root, twenty-fifth pass -- two pages, no new item, and that is the right answer

Two pages read, **zero items filed**, both routed into items that already own the theme. Recorded as
a pass rather than folded into the next one, because "one WI, one theme" cuts both ways: a page whose
finding belongs inside an existing item does not earn a new number just to make the count move.

**Routed to board:0524** -- `devenv-al-type-conversion-expressions.md`. That item carries the operand
matrix; this page carries the RANK the matrix cannot express: *"the system always converts at least
one of the operands to a MORE GENERAL data type"*, numbers `Char -> Integer -> Decimal` and strings
`Code -> Text`. So `text + code` is `text` -- the code operand widens, never the reverse. And one
sentence easy to read past: *"type conversion can occur even though two operands have the same
type"*, which is `Code[10] + Code[20]`, because the length is part of the type.

**In C++ most of it is already free**: `Decimal(std::int64_t)` is non-explicit
(`include/type/Decimal.h:57`) so `Integer + Decimal` promotes by itself, and **there is no
`Decimal(double)`** -- the no-binary-float invariant holding at the type level rather than by
convention. `Char` and the `Code`-to-`Text` widening are the two that need stating.

**Routed to board:0359** -- `devenv-protected-variables.md`. `protected var` makes a variable visible
to the object's EXTENSIONS. **Measured: 1 780 declarations; `internal var` and `local var` zero
each**, so two of `src/al/Parser.cpp:604`'s three branches are dead.

**The modifier is parsed and discarded**: `al::VarDecl` has no visibility member, and the same
collapse happens for procedures at `Parser.cpp:197`, where `internal` and `protected` fold into
`isLocal = false` -- **`internal procedure` 13 508 and `protected procedure` 1 252 are emitted
public.** Every object variable is public too: `WritePage` opens the class with `public:` and writes
the members there.

**The consequence runs the harmless way, which is why it is recorded and not urgent.** agiru is MORE
permissive than AL, not less, and its input is BCApps, which the AL compiler has already accepted --
so no BaseApp file is affected. It agrees with 0359's own conclusion for `Access = Internal`, where
being stricter than the platform would break `RecordRef` and `Codeunit.Run`. **What it costs is a
check the compiler could have made**, which is this tree's whole argument for leaving Python.

One count is not separable and is left out of the item rather than qualified into uselessness: a bare
`var` line is 350 865, and it counts local `var` blocks inside procedures together with object
globals. There is no line pattern that separates them.

---

## `developer/` root, twenty-sixth pass -- an omitted argument is not the zero value

One page, one item, and it started as a confirmation and turned into a bug.

**The confirmation first, because it is worth recording that a rule is KEPT.** *"If a parameter is
passed by value, then a COPY of the variable is passed ... if passed by reference, the method can
change the value of the variable itself."* `src/gen/CodeunitWriter.cpp:334` emits `Type &` for `var`
and `Type` -- by value, no `const`, no reference -- otherwise. **That is exactly right**, and the
absence of `const Type &` matters: it would be the obvious optimisation and it would break the AL
semantics that a by-value callee may modify its own copy.

**Then the rule about optional parameters**: *"the optional parameters may be OMITTED STARTING FROM
THE RIGHT"*, which is C++ default arguments exactly. So the SHAPE in the door is right.

**The VALUES are not.** `include/Builtins.h` carries **47 default arguments and every one is `= {}`**
-- zero, false, or the empty string. Checked against their method pages, five of them:

| door | documented default |
|---|---|
| `DMY2Date(Day, Month = {}, Year = {})` | the current month, the current year |
| `DWY2Date(WeekDay, Week = {}, Year = {})` | the current week, the year of the current week |
| `CalcDate(DateExpression, Date = {})` | *"the CURRENT SYSTEM DATE"* |
| `Randomize(Seed = {})` | *"the current system time, total milliseconds since midnight"* |
| `GlobalLanguage(NewLanguageID = {})` | -- the second family, below |

**Four wrong, and the information is lost at the CALL SITE** -- day 0, seed 0 and `0D` are all values a
caller could pass deliberately, so no body can recover the distinction afterwards.

**`Randomize()` is the sharpest**: seeded with 0 it produces the same sequence on every run. The item
is careful about what that does and does not violate -- CLAUDE.md's determinism invariant is about
postings producing the same entries twice, not about `Random`, so a frozen seed is simply the wrong
answer rather than a broken invariant.

**A second family, and it is a different defect.** Six builtins use an optional parameter to turn a
getter into a setter -- `GlobalLanguage`, `LockTimeout`, `LockTimeoutDuration`,
`CurrentTransactionType`, `ApplicationArea`, `CodeCoverageLog`. With `= {}` the body cannot tell
`GlobalLanguage()` from `GlobalLanguage(0)`, and 0 is a real LCID; `LockTimeout()` against
`LockTimeout(false)` collides over half the domain. **No default value fixes this one** -- it needs two
overloads, which is what AL's `[Optional]` notation means, and it is also what makes the completeness
counter able to see the difference.

**What was and was not checked, stated as such**: 47 defaults, **five checked, four wrong**. The other
42 are an unchecked population and the item's first task. Some are certainly fine --
`StopSession(SessionId, Comment = {})` really does default to an empty comment. **The rate is left at
"4 of 5 checked" rather than extrapolated over 47**, because a rate over five samples is not a
population.

All four refuse today (`src/rt/Builtins.cpp:55`, `:61`, `:181`, `:198`, `:367`), so nothing produces a
wrong date yet. **The defect is in the signature, which is checked-in code a body will inherit** --
filed as a bug rather than a task because the fix edits what exists.

---

## `developer/` root, twenty-seventh pass -- the smallest of the twelve object kinds

One page, one item, and the first count in this sweep that is EXACT rather than a bound.

**A `controladdin`'s `procedure` has no body.** It declares what the JavaScript provides, so the
object is an INTERFACE -- the same shape as an AL `interface`, with the implementation on the other
side of a language boundary. board:0479 filed its properties and board:0424 its ten sizing
properties; neither carries the surface, which is two lists of signatures and nothing else.

**Two directions, two call shapes.** AL calls in through the PAGE -- `CurrPage.ControlName.Method(...)`
-- so `CurrPage` needs a typed proxy per user control, a third thing it has to be after board:0553's
controls and board:0554's parts. JavaScript raises back through a `trigger` on the `usercontrol`,
matched BY NAME to the add-in's `event`, and from the JS side the call is
`InvokeExtensibilityMethod('CallBack', [args])` -- a string name and a positional array.

**Microsoft's own example declares `event Callback` and invokes `'CallBack'`.** AL is
case-insensitive so both are one symbol -- and that is CLAUDE.md's `identifier casing` trap in its
natural habitat, on a name crossing into a language that is NOT case-insensitive. The collapse has to
happen once, on the AL side, in the generator, and be recorded in the emitted metadata because the
boundary compares strings.

**An exact count, which is rare here.** Every file containing `^\s*event\s+<name>\s*\(` is one of the
20 `controladdin` files -- checked by listing both sets, not assumed. **51 events across 19 add-ins**;
`EarlyAccessPreviewBanner` declares none. `usercontrol(` **225**, `CurrPage.<ctl>.<m>(` **57**.

**And the kind is a CLOSED SET IN ONE DIRECTORY**: 16 of the 20 are in
`System Application/App/ControlAddIns/`, the other four one each in `ClientAddIns`, `INTaxEngine`,
`EDocument` and `UKMakingTaxDigital`. That is different from every other object kind in this board and
it changes what "implement it" means -- the twenty are enumerable and each can be decided on its own.
The item splits them into reachable-in-a-browser and needs-something-agiru-lacks (device cameras,
Power BI, Microsoft telemetry surveys), **and labels that split a judgement rather than a
measurement**; what is measured is that there are twenty and where they live.

**`ControlAddIn` is not parsed at all** -- no AST node, no parse path. Its only occurrence in `src/` is
`CodeunitWriter.cpp:129`, where it is recognised as a VARIABLE TYPE. The same half-state board:0556
found for `Query`: the name is known and the object is not.

**225 placements against 20 definitions** says the proxy on `CurrPage` is what most AL touches, so it
is what to build first -- and it is where the only type check the boundary has can live, because the
JavaScript side has none.

---

## `developer/` root, twenty-eighth pass -- a measurement that makes an ambiguity free

Two pages, one item, and one earlier item sized.

**A label carries three optional attributes in any order** -- `Comment`, `Locked`, `MaxLength` -- on
all three label-bearing forms. `src/al/Ast.h:24` is `struct LabelDecl { name; text; }`. **All three
are parsed away.**

**The `MaxLength` ambiguity, and the number that makes it cost nothing.** `label-data-type.md` says it
*"determines HOW MUCH OF THE LABEL IS USED"*, which reads as truncation.
`devenv-work-with-translation-files.md` shows it landing in the generated XLIFF as
`maxWidth="999" size-unit="char"` -- a constraint on the TRANSLATION, addressed to a translator.

**Measured: 42 101 labels carry a `MaxLength`, and the text of ZERO of them exceeds it.** Counted by
matching the quoted source and the attribute tail on each `Label` line, unescaping `''`, comparing
lengths. The `grep` count is 42 282, so 181 declarations the line pattern does not reach -- stated,
not closed.

**Zero over 42 101 does not settle which reading is right, and that is the finding.** It is consistent
with both. What it DOES settle is that the two implementations are indistinguishable over the entire
BaseApp: truncating and not truncating give the same string 42 101 times out of 42 101. **So the
decision is deferred with a number instead of guessed**, the XLIFF reading is taken meanwhile because
it is the one the compiler's own output demonstrates, and **the gate case that would tell them apart
has to be synthetic** -- a gate written from BaseApp labels alone would be green under both readings
and prove nothing. The same trap as a green negative control, reached from the data side.

**`Locked` is the one with a consequence: 22 202 attributes, `true` 22 196, `false` 6.** A locked label
must come back byte for byte in every language. It costs nothing today -- there is no translation
layer, so locked and unlocked behave identically -- and becomes 22 196 wrongly translated strings the
moment one arrives. Filed now with the number rather than discovered then.

**And board:0561's open question is closed by an `IMPORTANT` box.** That item found `CaptionML`
declared **3 times** against `Caption` at 288 491 and could only say "the documentation names a
property the source has abandoned". This page says why: **the `ML` versions of eight properties are
NOT INCLUDED IN THE XLIFF FILE** -- `AboutTitleML`, `AboutTextML`, `CaptionML`, `InstructionalTextML`,
`OptionCaptionML`, `PromotedActionCategoriesML`, `RequestFilterHeadingML`, `ToolTipML` -- **and neither
is `TextConst`.** The `ML` form carries translations inline; XLIFF carries them in a file; an app on
XLIFF cannot use `ML` for anything it wants translated. **`TextConst` went the same way: 10
declarations against 212 109 `Label`s.** It is a compatibility surface, not a mechanism.

**board:0382 sized.** `GenerateCaptions`: *"If the object already has a `Caption` property set, that
value is used."* `TableWriter.cpp:37` does exactly that for a FIELD; `TableWriter.cpp:551` writes
`.caption = <table>::kName` unconditionally for the TABLE. **3 765 tables declare a `Caption`, across
4 564 `.Table.al` files** -- roughly four in five declared and discarded.

---

## A COUNTING RULE THIS SWEEP HAD NOT WRITTEN DOWN

**A declaration count over `.al` lines can exceed the OBJECT count for a reason that has nothing to do
with the pattern: the same property may be declared at more than one LEVEL of one object.**

Found on `EntityName`: **854 declarations over 489 FILES**, against 374 `PageType = API` pages plus
113 `QueryType = API` queries plus exactly two non-API pages -- 489, exact.

**The first hypothesis was the preprocessor** -- board:0552 measures 9 946 `#if` in the tree, and a
versioned API is exactly where two branches of one property would sit. **Checked and refuted: of the
140 files declaring `EntityName` more than once, FOUR contain a preprocessor directive at all.**

The real answer is that an API page declares `EntityName` on the PAGE and again on every `part`
control -- `APIV2Items.Page.al` declares it eight times, once for itself and once per navigation
property. **So the excess is structure, not noise**, and it carries a finding: a part on an API page
is an OData navigation property, so the control tree is also the entity graph.

**The rule for every future number here**: when a per-line count exceeds the object count, the
question is not only "is the pattern too loose" but **"does this property exist at more than one level
of the object"** -- and the way to tell them apart is to count FILES and compare, which costs one
`grep -l`.

---

## `developer/` root, twenty-ninth pass -- the API pages

Three pages, one item, and the counting rule above came out of it.

**An API page has no renderer and still has to transpile.** board:0429 quotes the line -- *"cannot be
shown in the user interface"* -- and 374 page objects with source tables, repeaters, fields and
procedures do not stop being pages. **The scope position is stated rather than assumed: the OBJECT is
in scope, the HTTP endpoint is not a phase 1-3 target**, since none of the three phases is an OData
surface.

**The two OData action pages are not symmetric, and the asymmetry is the finding.** A BOUND action is
a declaration -- `[ServiceEnabled]` plus a `var WebServiceActionContext`. **An UNBOUND action is not a
declaration at all**: any public codeunit procedure, reached at
`POST /ODataV4/{serviceName}_{procedureName}`, with publishing done in the client. So the unbound half
needs nothing from the transpiler, which is recorded so the page is not read twice looking for a
mechanism that does not exist.

**Two refusals and one warning, and the severities must not be merged.** *"This page type CAN'T BE
EXTENDED"* and *"bound actions cannot be added by extending"* are errors; the naming rules split --
*"the compiler shows WARNINGS on casing violations and ERRORS on naming violations"*. So a
non-alphanumeric `EntityName` is a `static_assert` and a `PascalCase` one is a COUNTER. **Getting it
the other way round stops BC's own pages translating the moment one capitalises**, which is
board:0567's negative control.

`APIVersion` is a LIST -- `'v0.5', 'v1.0'` 98 of 481 -- and `APIPublisher` is `'microsoft'` 283 and
`'mock'` 12, every one in the tree. **295 `APIPublisher` against 374 API pages leaves 79 declaring
none**, and what those take instead is named as unsettled rather than inferred from a neighbour's
default.

---

## `developer/` root, thirtieth pass -- the no-task sweep, and what "no task" had to earn

**26 pages, zero items**, each with a one-sentence reason in its ledger row. The goal permits this and
it is the cheapest way through the remaining 257 -- but only if the reason is EARNED, so each page was
opened and its title, description and every `##` heading read before it was classified. **A title
alone is not enough**: three pages in the same batch that looked like Copilot documentation turned out
to describe LAYOUT GRAMMAR and were pulled out of this sweep instead (see the next pass).

The 26 fall into six groups:

| group | pages | why there is no task |
|---|---:|---|
| Copilot and AI | 16 | Azure OpenAI is an external service, and the `AOAI *` codeunits are BaseApp AL objects rather than platform primitives -- the transpiler translates them like any other codeunit |
| Marketplace and publishing | 5 | offer management, validation, update cadence: a process around the product, not behaviour in it |
| external services | 2 | Dataverse, Microsoft Entra |
| VS Code and tooling | 1 | Go To Definition |
| process | 1 | how to ask Microsoft for an extension point |
| navigational | 1 | `TOC.md`, the family's table of contents |

**The Copilot group is the one worth arguing for**, because it is the largest and the argument is not
"AI is out of scope". It is that **every AL-visible piece of it is already covered by something else**:
the `PromptDialog` page type is board:0429's, its layout areas are board:0553's census (`prompt` 7,
`promptoptions` 5, `prompting` 5, `promptguide` 2), and the AI module is a set of codeunits the
transpiler translates without knowing what they do. What is left over is an HTTP call to a service --
board:0563's subject -- with a model name in it.

---

## `developer/` root, thirty-first pass -- the nine pages the no-task sweep nearly took

Nine pages, one item. **Eight are filed under Copilot and the ninth under AI**, and the previous pass
classified twenty-six of their neighbours as no-task. These are LAYOUT GRAMMAR and were pulled out of
that sweep -- which is the whole argument for reading every page's headings rather than trusting its
title.

**Three layout areas that exist on no other page type**, each with a structural rule: `Prompt` and
`Content` take any control EXCEPT a repeater, `PromptOptions` takes ONLY option fields, and a page
without an `area(Prompt)` does not start in prompt mode. All decidable from board:0553's tree.

**A CLOSED action set, and the source confirms it exactly.** `systemaction(` is declared 40 times and
the argument is one of five names: **`Ok` 13, `Cancel` 13, `Generate` 9, `Regenerate` 3, `Attach` 2 --
summing to 40.** So the documented set is the used set, and **a `static_assert` over five names risks
rejecting nothing** -- rarer in this board than it sounds, since board:0553 and board:0560 both had to
settle for counters.

**One documented exclusivity the source does not keep.** The page says system actions "are only
supported by this page type". Counted per FILE: **13 files declare `area(SystemActions)` -- nine
`PromptDialog` and FOUR `ConfigurationDialog`** (the agent setup wizards). Recorded, not resolved: the
sentence says what a `PromptDialog` allows and not what a `ConfigurationDialog` allows, and
`devenv-page-type-configuration-dialog.md` is 630 lines and still unread. **What is settled is that a
`static_assert` restricting `systemaction` to `PromptDialog` would reject four pages the platform
loads.**

**A second thing a `constexpr` cannot hold**, after board:0560's collapsible indent column:
`PromptMode =` is declared **2 times** while `CurrPage.PromptMode` is written **6** -- the mode is
driven from code, so it is session state on the page object and the declared property only seeds it.

**Errors inside a prompt dialog follow different rules from everywhere else** -- only the LAST
`Message` is shown with a count of the total, an `Error` suppresses subsequent messages, and line
breaks are IGNORED. Three deviations inside one page type, and the count being carried means the
runtime keeps more than the last text.

**And `area(Prompting)` is a SEVENTH action area** on ordinary pages, beyond board:0539's six, whose
actions may only `RunObject` a `PromptDialog`. It renders differently per host page type -- a floating
bar on `List`, an icon top-right on `Card`, at the bottom on `StandardDialog` -- and **an action there
with no `RunObject` renders nothing**, which is a declaration that silently does nothing and therefore
a counter rather than a refusal.

`area(Prompt)` is declared 7 times over 9 PromptDialog pages, so two of the nine do not start in
prompt mode -- which the documentation describes as a consequence rather than an error, and the count
agrees with it.

---

## `developer/` root, thirty-second pass -- the second no-task sweep

**45 pages, zero items.** Same method as the thirtieth pass: title, description and every `##` heading
read before classifying, because a title alone has already been wrong once.

| group | pages |
|---|---:|
| Marketplace submission and publishing | 12 |
| Visual Studio Code and the AL tooling | 11 |
| deployment, tenant administration, lifecycle | 7 |
| debugging and profiling | 5 |
| telemetry, Teams, mobile, Codespaces, Dataverse | 6 |
| advice without a mechanism (add-in style, add-in performance, runtime choice, contributing) | 4 |

**Four of the 45 were kept out of the queue only after checking**, and they are the ones worth naming
because each looked like it had a task:

- **`devenv-debugging.md`** carries "Break on record changes" and "Debugging SQL behavior" -- both
  describe the DEBUGGER's view of behaviour that other pages specify, not behaviour of its own.
- **`devenv-connect-apps-filtering.md`** is OData `$filter` syntax in a URL. board:0567 put the HTTP
  endpoint out of phase 1-3 scope, so this is downstream of a decision already argued for -- and the
  row says so rather than repeating the argument.
- **`devenv-data-archive-extension.md`** documents the `Data Archive` codeunit's methods. It is a
  BaseApp AL OBJECT, so the transpiler translates it without knowing what it does; there is nothing
  for the runtime to learn. The same reasoning the Copilot group got in the thirtieth pass.
- **`devenv-choosing-runtime.md`** sets `runtime` in `app.json`, which gates which AL features the
  compiler accepts. agiru does not choose -- it reads BCApps on `main` as it stands, and the runtime
  version it implies is a fact rather than a setting.

**`devenv-control-addin-asynchronous-considerations.md` was NOT swept in with the other two add-in
pages** and is in the read queue: it is about what the SERVER can and cannot do while an add-in call
is outstanding, which is board:0565's boundary rather than style advice.

293 of 470. The remaining 177 are the pages with mechanisms in them.

---

## `developer/` root, thirty-third pass -- three pages, three routings, no new item

**`area(Sections)` is the role centre's NAVIGATION MENU**, and board:0553's area census listed
`sections` 204 among fifteen area arguments without saying whose it was. Checked per file: **all 204
are `PageType = RoleCenter`** -- read from each file's first `PageType` rather than inferred. Against
211 role-centre declarations, so almost every one has a navigation menu. Its children are groups over
actions, nested to any depth, and an action's `RunObject` may be a page, a report, an XMLport or a
codeunit; the target opens in the role centre's CONTENT AREA rather than as a window. Routed to
board:0538.

**The tooltip fallback is sized.** board:0545 recorded that a table field's `ToolTip` is inherited by
every page using the field and overridden by the page control's own. Measured by file extension:
**50 325 in `*.Table.al`, 92 903 in `*.Page.al`, 159 993 in all** -- so 16 765 sit in extensions,
reports and XMLports, which the extension split does not separate and which is stated rather than
apportioned. **The 50 325 are the fallback SOURCE**: a renderer reading only the control's own
property shows nothing wherever a page relies on inheritance. Routed to board:0385.

**`ContextSensitiveHelpPage` sits in three places, not one**: on a page it is a page property, on a
report and an XMLport it is a REQUEST PAGE property. A generator putting it only on `PageDef` drops
two thirds of its call sites -- of which there are 53 in total. Routed to board:0393.


---

# `developer/methods-auto/` -- 1 876 pages, one mechanical pass

**The check the goal prescribes, run over every page**: the documented `## Syntax` block against the
C++ signature in `include/`. Not one item per page -- the overload pages of one method are ONE unit,
and a page earns an item only where the door DISAGREES.

## What was read, and it adds up

| | count |
|---|---:|
| `.md` files under `methods-auto/` | **1 876** |
| `library.md` at the family root -- an index | 1 |
| pages inside the 134 type directories | 1 875 |
| of those, pages carrying a `## Syntax` block | **1 741** |
| pages without one: `*-data-type.md` | 101 |
| pages without one: `*-option.md` | 33 |

101 + 33 + 1 741 = 1 875, so **every page is accounted for by KIND rather than by assumption.** The
134 without a syntax block describe a type or an option rather than a method, have nothing to compare
against a signature, and carry no task of their own.

The 1 741 syntax blocks collapse to **1 300 distinct `Type.Method` over 93 types** -- the difference
being the overload pages the goal counts as one unit (`record-insert--method.md`,
`record-insert-boolean-method.md` and `record-insert-boolean-boolean-method.md` are one).

**THE GOAL'S OWN ESTIMATE IS CORRECTED HERE.** It quotes 1 435 distinct methods over 135 types, taken
from the H1 titles before any syntax block was parsed. Parsing them gives **1 300 over 93**: the 135
counted DIRECTORIES rather than types with methods, and the 1 435 counted the 134 data-type and option
pages as methods of their own. 1 300 + 134 + 1 = 1 435, so the two numbers differ by exactly the pages
that are not methods, and the smaller one is the right one.

## The method, and the three things it cannot see

For each type, the headers declaring the most of that type's documented names are chosen -- up to
four, BY HIT COUNT rather than by a hand-written map -- plus `Builtins.h` and `BuiltinsWritten.h`,
because AL documents `Dialog.Message` and `System.StrLen` under a type while the door declares them
globally. A type whose chosen headers cover fewer than half its names is marked UNRESOLVED and
contributes nothing but its count. Then, per method:

| column | test |
|---|---|
| **in the door** | at least one declaration of the name in the chosen headers or the builtins |
| **absent** | no declaration of that name ANYWHERE in `include/` |
| **elsewhere** | declared in `include/`, but not in the headers chosen for this type |
| **`void`** | the documentation gives a return value and every declaration returns `void` |
| **arity** | no declaration is variadic or callable at the documented MINIMUM argument count |

**Three limits, each found by checking rather than assumed:**

1. **A variadic template satisfies a documented optional list.** `Record::SetFilter(const Field &,
   std::string_view, const Arguments &...)` covers `SetFilter(Field, Text [, Value])`. **Handled** --
   the parser records `variadic` and the arity test accepts it. Found by hand-checking
   `Record::SetFilter` and `Record::GetFilter` after they appeared as gaps.
2. **A declaration wraps across lines.** `::agiru::Date` on one line and `CalcDate(...)` on the next
   is the door's ordinary style, and a line-anchored pattern misses it. **Handled** -- the door parser
   scans for `Name(`, balances the parentheses, and walks BACKWARDS to the previous `;`, `{`, `}` or
   `:` for the return type. Found because `CalcDate` and `DMY2Date` were reported absent while
   board:0564 had quoted them at `Builtins.h:72` and `:243`.
3. **The `elsewhere` column is the residue of the header chooser, not a defect.** `StrLen` and
   `MaxStrLen` are in `type/Text.h`, `Commit` in `runtime/Error.h`, `StrSubstNo` in `runtime/Record.h`,
   `ArrayLen` in `type/AlArray.h` -- all real, none in the headers the chooser picked for their type.
   **21 of them**, and they are counted apart from the 74 so that neither number pretends to be the
   other.

## The coverage table

| AL type | methods | pages | in the door | absent | elsewhere | `void` | arity |
|---|---:|---:|---:|---:|---:|---:|---:|
| `record` | 81 | 111 | 81 | 0 | 0 | 0 | 0 |
| `recordref` | 76 | 89 | 76 | 0 | 0 | 0 | 0 |
| `system` | 71 | 78 | 69 | 0 | 2 | 1 | 2 |
| `variant` | 67 | 67 | 67 | 0 | 0 | 0 | 0 |
| `report` * | 38 | 62 | 0 | 31 | 7 | 0 | 0 |
| `xmlelement` | 33 | 48 | 33 | 0 | 0 | 0 | 6 |
| `text` | 32 | 38 | 29 | 0 | 3 | 0 | 0 |
| `fieldref` | 31 | 70 | 31 | 0 | 0 | 0 | 0 |
| `jsonobject` | 31 | 66 | 31 | 0 | 0 | 2 | 0 |
| `testpage` | 30 | 30 | 27 | 3 | 0 | 0 | 4 |
| `database` | 29 | 31 | 28 | 0 | 1 | 0 | 0 |
| `file` | 28 | 48 | 28 | 0 | 0 | 0 | 4 |
| `jsonarray` | 28 | 90 | 28 | 0 | 0 | 0 | 0 |
| `xmlnode` | 27 | 32 | 27 | 0 | 0 | 0 | 3 |
| `testrequestpage` | 25 | 25 | 17 | 8 | 0 | 0 | 3 |
| `xmldocument` | 25 | 38 | 25 | 0 | 0 | 0 | 6 |
| `jsonvalue` | 24 | 37 | 24 | 0 | 0 | 0 | 0 |
| `testfield` | 24 | 25 | 24 | 0 | 0 | 0 | 0 |
| `testpart` | 20 | 20 | 19 | 1 | 0 | 0 | 3 |
| `debugger` | 19 | 19 | 19 | 0 | 0 | 0 | 1 |
| `page` | 19 | 29 | 19 | 0 | 0 | 1 | 5 |
| `session` | 19 | 22 | 19 | 0 | 0 | 0 | 1 |
| `xmldocumenttype` | 19 | 27 | 19 | 0 | 0 | 0 | 3 |
| `errorinfo` | 18 | 22 | 18 | 0 | 0 | 1 | 13 |
| `xmlattribute` | 18 | 24 | 18 | 0 | 0 | 0 | 4 |
| `xmlport` * | 18 | 21 | 0 | 14 | 4 | 0 | 0 |
| `label` | 17 | 20 | 17 | 0 | 0 | 0 | 0 |
| `navapp` | 17 | 17 | 17 | 0 | 0 | 3 | 7 |
| `textconst` | 17 | 20 | 17 | 0 | 0 | 0 | 0 |
| `httpclient` | 16 | 18 | 16 | 0 | 0 | 0 | 3 |
| `query` | 15 | 22 | 8 | 7 | 0 | 0 | 2 |
| `xmlprocessinginstruction` | 15 | 20 | 15 | 0 | 0 | 0 | 3 |
| `list` | 14 | 18 | 14 | 0 | 0 | 1 | 0 |
| `xmldeclaration` | 14 | 19 | 14 | 0 | 0 | 0 | 6 |
| `jsontoken` | 12 | 14 | 12 | 0 | 0 | 0 | 0 |
| `xmlcdata` | 12 | 17 | 12 | 0 | 0 | 0 | 4 |
| `xmlcomment` | 12 | 17 | 12 | 0 | 0 | 0 | 4 |
| `xmltext` | 12 | 17 | 12 | 0 | 0 | 0 | 4 |
| `filterpagebuilder` | 11 | 12 | 11 | 0 | 0 | 0 | 3 |
| `httprequestmessage` | 11 | 12 | 11 | 0 | 0 | 1 | 1 |
| `textbuilder` | 11 | 13 | 11 | 0 | 0 | 0 | 2 |
| `datatransfer` | 9 | 9 | 9 | 0 | 0 | 0 | 3 |
| `dialog` | 9 | 11 | 9 | 0 | 0 | 0 | 2 |
| `httpheaders` | 9 | 14 | 9 | 0 | 0 | 1 | 0 |
| `mediaset` | 9 | 9 | 7 | 2 | 0 | 1 | 0 |
| `notification` | 9 | 10 | 9 | 0 | 0 | 2 | 0 |
| `requestpage` | 9 | 9 | 9 | 0 | 0 | 0 | 3 |
| `sessionsettings` | 9 | 9 | 9 | 0 | 0 | 0 | 7 |
| `dictionary` | 8 | 10 | 8 | 0 | 0 | 1 | 0 |
| `httpresponsemessage` | 8 | 8 | 8 | 0 | 0 | 1 | 0 |
| `xmlnamespacemanager` | 8 | 8 | 8 | 0 | 0 | 0 | 1 |
| `cookie` | 7 | 7 | 7 | 0 | 0 | 0 | 0 |
| `media` | 7 | 8 | 4 | 3 | 0 | 0 | 0 |
| `moduleinfo` | 7 | 7 | 7 | 0 | 0 | 0 | 0 |
| `numbersequence` | 7 | 8 | 7 | 0 | 0 | 0 | 4 |
| `webserviceactioncontext` | 7 | 7 | 7 | 0 | 0 | 0 | 0 |
| `bigtext` | 6 | 8 | 6 | 0 | 0 | 0 | 2 |
| `blob` | 6 | 6 | 4 | 2 | 0 | 0 | 0 |
| `date` | 6 | 6 | 5 | 0 | 1 | 0 | 0 |
| `instream` | 6 | 14 | 6 | 0 | 0 | 1 | 0 |
| `testhttpresponsemessage` | 6 | 6 | 6 | 0 | 0 | 0 | 1 |
| `version` | 6 | 7 | 6 | 0 | 0 | 0 | 1 |
| `httpcontent` | 5 | 9 | 5 | 0 | 0 | 0 | 0 |
| `isolatedstorage` | 5 | 11 | 5 | 0 | 0 | 0 | 2 |
| `taskscheduler` | 5 | 6 | 5 | 0 | 0 | 0 | 2 |
| `testfilter` | 5 | 5 | 5 | 0 | 0 | 0 | 0 |
| `time` | 5 | 5 | 4 | 0 | 1 | 0 | 0 |
| `xmlattributecollection` | 5 | 10 | 5 | 0 | 0 | 0 | 0 |
| `enum` | 4 | 4 | 2 | 2 | 0 | 0 | 0 |
| `keyref` | 4 | 4 | 4 | 0 | 0 | 0 | 0 |
| `sessioninformation` | 4 | 4 | 4 | 0 | 0 | 0 | 0 |
| `testhttprequestmessage` | 4 | 4 | 4 | 0 | 0 | 1 | 0 |
| `companyproperty` | 3 | 3 | 3 | 0 | 0 | 0 | 0 |
| `datetime` | 3 | 3 | 3 | 0 | 0 | 0 | 0 |
| `guid` * | 3 | 5 | 0 | 1 | 2 | 0 | 0 |
| `moduledependencyinfo` | 3 | 3 | 3 | 0 | 0 | 0 | 0 |
| `productname` | 3 | 3 | 3 | 0 | 0 | 0 | 0 |
| `secrettext` | 3 | 3 | 3 | 0 | 0 | 0 | 0 |
| `testaction` | 3 | 3 | 3 | 0 | 0 | 0 | 0 |
| `fileupload` | 2 | 3 | 2 | 0 | 0 | 0 | 0 |
| `outstream` | 2 | 23 | 2 | 0 | 0 | 0 | 0 |
| `recordid` | 2 | 2 | 2 | 0 | 0 | 1 | 1 |
| `xmlnametable` | 2 | 2 | 2 | 0 | 0 | 0 | 0 |
| `xmlnodelist` | 2 | 2 | 2 | 0 | 0 | 0 | 0 |
| `biginteger` | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `boolean` | 1 | 3 | 1 | 0 | 0 | 0 | 0 |
| `byte` | 1 | 3 | 1 | 0 | 0 | 0 | 0 |
| `codeunit` | 1 | 4 | 1 | 0 | 0 | 0 | 0 |
| `decimal` | 1 | 3 | 1 | 0 | 0 | 0 | 0 |
| `duration` | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `integer` | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| `xmlreadoptions` | 1 | 1 | 1 | 0 | 0 | 0 | 1 |
| `xmlwriteoptions` | 1 | 1 | 1 | 0 | 0 | 0 | 1 |
| **93 types** | **1300** | **1741** | **1205** | **74** | **21** | **19** | **128** |

`*` marks a type whose headers could not be resolved: **`report` 38 methods and `xmlport` 18** -- the
two object kinds with no runtime type at all (board:0063, board:0065), which is that hole seen from
the method side -- and **`guid` 3**, where `include/type/Guid.h` exists and its three documented
methods (`CreateGuid`, `CreateSequentialGuid`, `ToText`) match nothing in it. **Their 59 methods are
counted as absent and should be read as UNKNOWN.**

## What the pass filed

**Three items, and only three**, because the goal asks for a work item only where the door
contradicts the documentation:

| item | family | count | how sure |
|---|---|---:|---|
| board:0569 | documented getter and setter, door has the SETTER ONLY | **50** | five spot-checked by hand, five confirmed |
| board:0570 | documented return value, door returns `void` | **19** | five spot-checked, five confirmed |
| board:0571 | documented method with no declaration anywhere in `include/` | **74**, of which 59 are `report`, `xmlport` and `guid` | a lower bound |

**The other 78 arity gaps are NOT filed as an item and the reason is stated rather than left
implicit.** Roughly half of them are one shape -- `AddAfterSelf`, `AddBeforeSelf`, `ReplaceWith`,
`Add`, `AddFirst`, `ReplaceNodes` across the eleven `Xml*` types, all documented at two arguments
where the door declares one -- which is a MISSING OVERLOAD rather than a missing default, and belongs
with whatever item takes the XML DOM. The rest are genuine optional-parameter gaps of the same family
as board:0564 and are listed in board:0569's ordering as the next mechanical pass.

**A counter reporting 0 over N is an abort**: every column above is non-zero over 1 300, and the two
that could have been silently zero -- `void` and `arity` -- were each checked by hand on five members
before being believed.

---

## `developer/` root, thirty-fourth pass -- ten pages, one item, seven routings

**The largest single number this sweep has produced about the .NET surface.** Measured over
`~/Git/BCApps/src` by `:\s*DotNet\s+<name>`: **7 050 `DotNet` variable declarations across 847
files**, and the top of the distribution is one library --

| .NET type | declarations |
|---|---:|
| `XmlNode` | 1 756 |
| `XmlDocument` | 957 |
| `XmlNodeList` | 347 |
| `XmlNamespaceManager` | 68 |
| `XmlAttribute` | 56 |
| `XmlElement` | 44 |

-- **3 228 of 7 050, or 46 %, is the .NET XML DOM**, for which agiru already has six AL-native
equivalents with almost no gaps in the methods-auto pass. **It does not follow that they substitute**:
`DotNet XmlDocument` is `System.Xml.XmlDocument` and AL's `XmlDocument` is a different class, and the
BaseApp chose the .NET one in 957 places deliberately. What follows is that nearly half the DotNet
work is one library of known shape. Routed to board:0035.

**And how the BaseApp reaches .NET is the opposite of the instinct.** `devenv-create-a-wrapper-module.md`
shows the pattern: a public facade codeunit with no logic (`Regex`) over an `Access = Internal`
implementation holding the `DotNet` variable (`Regex Impl.`). **So `Regex` is an AL OBJECT the
transpiler translates like any other, and what agiru owes is `DotNet Regex` underneath it** -- not a
`Regex` module.

**`analysisviews` is a third page section** beside `layout` and `actions`, pointing at a JSON file
exported from the client, and it is allowed on a `pagecustomization` -- which otherwise carries only
layout and actions. Routed to board:0553.

**Every control add-in method must be `void`**, and that is a platform refusal rather than a
convention: *"all calls between the AL code running on the server and the script method are
ASYNCHRONOUS ... methods in the control add-in interface MUST BE OF TYPE VOID."* It explains
board:0565's asymmetry -- a round trip is two one-way messages because the boundary cannot carry one.
Routed there as a `static_assert`.

**board:0572 is the only new item**: an app packages arbitrary FILES, declared by `resourceFolders` in
`app.json` and read by `NavApp.GetResource` -- which is how a Word or RDLC layout travels with an app,
so it is a precondition for board:0547 rather than a subject of its own. Three hard limits (16 MB per
file, 256 MB per app, 256 files) are board:0081's `static_assert` shape. And the name space is PER
APP: two apps may carry the same resource name and each reads its own, which makes the store a per-app
map and is board:0572's negative control.

**Its population is NOT measured and the reason is given rather than a number invented**:
`resourceFolders` lives in `app.json`, which the `.al` pattern does not reach, and the API is 2026
release wave 2, so the call sites in a tree pinned at BCApps `main` may be none.

---

## `developer/` root, thirty-fifth pass -- the third no-task sweep, and one group that had to be argued

**31 pages, zero items** -- 26 with a reason, 5 routed into items that already own them.

**Eight of the 31 are one group and it is the group worth arguing for**: `devenv-extend-edocuments`,
`-exchange-rates`, `-best-price-calculations`, `-document-sharing-onedrive`, `-email`,
`-item-charges`, `-shopify`, `-templates`. Each is a long page -- Shopify 721 lines, price
calculations 836, e-documents 620 -- about how to extend a BASEAPP FEATURE.

**They carry no task for the same reason `devenv-data-archive-extension.md` did not**: every mechanism
in them is an AL object. An interface to implement, an enum to extend, an event to subscribe to, a
codeunit to call. **The transpiler translates all of it without knowing what it does**, and what makes
the extension work is the generic machinery those pages presuppose -- board:0057's events,
board:0033's interfaces, board:0053's extensible enums -- every one of which is already on the board
with its own population.

**That is a claim that could be wrong, so it was checked rather than assumed**: each of the eight was
opened and its headings read for a PLATFORM mechanism rather than an application one. The closest any
came was `devenv-extending-email.md`'s "email view policies", which is a BaseApp table and a
subscriber, not a runtime rule.

The remaining 18 no-task pages are the familiar groups -- Teams 5, overviews and lifecycle 4, build
and packaging 4, hosting, mobile, SOAP, Entra, and the permission-set export procedure.

**Five were routed rather than dismissed**: the 909-line custom-API walkthrough into board:0567, the
essential-methods index into board:0571, the events example into board:0057, and both `.NET` pages into
board:0035 -- add-in assemblies and type serialisation being the DotNet surface seen from two other
sides.

---

## `developer/` root, thirty-sixth pass -- a page written as history that is a rule list

Two pages, one item. **`devenv-differences.md` is addressed to a C/SIDE developer and reads as
history; it is the AL compiler's own rule list**, and eight of its statements are checks a transpiler
can make.

**Five properties require another property**, with their populations measured 2026-09-04:
`ValidateTableRelation` **2 240** needs `TableRelation`; `RunPageMode` **1 232** needs `RunObject`;
`PromotedCategory` 983 and `PromotedIsBig` 488 need `Promoted` 1 228; `SourceTableTemporary` 680 needs
`SourceTable`. Five conditional `static_assert`s of the shape board:0483 already has two of -- and
worth having because the failure without them is SILENT: `RunPageMode` on an action with no
`RunObject` is a mode for a page that never opens.

**A name must be unique across controls, actions AND methods on a page, and the documentation gives
the reason**: *"actions and fields could have the same names before, but that would have PREVENTED
PAGE TESTABILITY ACCESS."* That is board:0540's problem, made a compile error by the platform.
**agiru resolves the same collision by RENAMING** -- `src/gen/PageWriter.cpp:240` collapses names
across all three lists with a `taken` set seeded from the TestPage surface. Where AL refuses, the
transpiler should refuse, because a rename produces a member the AL cannot address.

**Three lexical facts, and one of them is a suffix**: a date literal is `yyyy-mm-ddD` and nothing
else; an integer literal above `999999999999999` is a **Decimal**, and `L` is the BigInteger suffix
(**measured: 9 occurrences**); and on a table a fractional `Min`, `Max` or `InitValue` is a Decimal and
therefore invalid on an Integer field.

**Two earlier findings are settled from the other direction.** board:0538's `area(Sections)` is
C/SIDE's `ActivityButtons` -- the full rename table explains six of board:0553's fifteen area
arguments at once. And board:0566's `CaptionML` finding is confirmed by counting the other renamed
spellings: **`CaptionML` 3, `TooltipML` 0, `AutoFormatExpr` 0, `DataCaptionExpr` 0, `ProviderID` 0.**

**One backward-compatibility note is a trap rather than history**: *"we continue to support adding
non-part pages as parts."* board:0554 proposes refusing exactly that -- but for the `FactBoxes` area
specifically, where the documentation does say it errors. **The difference is one word, and a
`static_assert` written from the wrong page would reject working BaseApp pages.**

**And the item's own class note is the uncomfortable one**: every check it proposes refuses something
the AL compiler already refused, so the expected finding count over BCApps is ZERO -- which is
indistinguishable from a check that never runs. All six gate cases are therefore synthetic.

---

## `developer/` root, thirty-seventh pass -- the fourth no-task sweep

**29 pages, zero items** -- 24 with a reason, 5 routed. 368 of 470, and the remaining 102 are what is
left after four sweeps have taken the tooling, the Marketplace, the device capabilities and the
walkthroughs.

| group | pages |
|---|---:|
| walkthroughs and getting-started | 6 |
| publishing, scopes, hotfixes, deltas | 6 |
| device capabilities: camera, location, gestures, mobile | 6 |
| telemetry | 3 |
| Visual Studio Code | 2 |
| Dataverse, .NET assembly porting | 2 |
| one BaseApp posting example | 1 |

**Two feature-key pages were routed rather than dismissed**, and the distinction is worth keeping:
a feature key is a SWITCH over behaviour this board already owns.
`devenv-httpcertvalid-feature-key.md` is the pre-27 way to turn off server certificate validation --
board:0563 records that as a superseded scheme -- and `devenv-mask-type-feature-key.md` is what
introduced `MaskType`, which board:0330 owns.

**Three pages were routed because the CLIENT is showing something this board models.**
`devenv-inclient-designer.md` and `devenv-inspecting-pages.md` are tools, but what they display is
board:0553's control tree and board:0551's page customization -- Designer WRITES the customizations,
and Page Inspection SHOWS the tree and its source table. **They are evidence about the model rather
than about the tool**, which is why they route rather than sweep.

---

## `developer/` root, thirty-eighth pass -- the fifth and last no-task sweep

**55 pages, zero items** -- 50 with a reason, 5 routed. **423 of 470**, and what remains is 47 pages
that are all one of four subjects: reports, pages, the AL language itself, and four singles.

| group | pages |
|---|---:|
| deployment, environments, publishing, signing, upgrade | 17 |
| Visual Studio Code and the analyzers | 9 |
| external services -- Azure, Microsoft 365, ML, Power BI, Power Pages, semantic search | 8 |
| mobile | 4 |
| troubleshooting | 4 |
| indexes and landing pages | 2 |
| a load-testing toolkit, a cloud-migration file, a walkthrough | 3 |

**Five routed**: the three remaining `ncci` pages into board:0019 -- board:0019 quotes the overview and
names the other three as a family, and the twelfth pass explicitly refused to count them read on that
basis, so they are read now and routed rather than left as a rounding error -- and the two System
Application module pages into board:0033, which now holds the module rules, the process for making a
new one, and what the layering means.

**A note on `devenv-work-sandbox-entitlements.md`**, because it is the one page in this sweep whose
title suggests a task: it describes which entitlements a sandbox grants, and board:0559 established by
citation that entitlements are ONLINE ONLY and gate nothing on premises. **The reason for sweeping it
is a decision this board already made, not a judgement made here.**

---

## `developer/` root, thirty-ninth pass -- a page has five sections

Four pages, one item. 427 of 470.

**A page object has three sections beyond `layout` and `actions`**: `views` (67 sections, 181 `view(`
entries, *"only for pages of type ListPage"*), `analysisviews` (**19** -- which corrects board:0553's
routing note that said the population was not measured), and the AL code block. **And the ORDER
matters**: *"the order in which the sections appear matters"*, which board:0549 already found for a
report. So section order is a parser rule for two object kinds, not a convention.

**`Extensible` is the gate on every extension and it is nearly half the declarations**: 2 285 in all,
**`false` 1 225 against `true` 1 060.** That makes board:0567's API refusal and board:0568's
`PromptDialog` refusal SPECIAL CASES of one rule -- both are pages the platform treats as
`Extensible = false` whatever they declare -- and board:0574's negative control is exactly that: a
gate written as a list of page types passes every other case while having to grow forever.

**`UserControlHost` is 157 pages with almost nothing in them**: one `usercontrol` in `area(Content)`,
**no actions at all**, not extensible. **Measured: 157 files declare the type and ZERO declare an
`actions` section** -- the source keeps the restriction over 157 chances to break it, so the
`static_assert` risks nothing. At 157 it is larger than `PromptDialog` 9 and `ConfigurationDialog` 4
together, which is surprising for a type whose whole content is one control; the page says it is for
embedding Power BI, which board:0565 has already classified as unreachable. **The page still has to
transpile, and the constraint is what makes it cheap.**

One more hard number for board:0081's shape: **an extension object's name may be at most 30
characters.**

---

## `developer/` root, fortieth pass -- eight events on a named codeunit, and a census

Eight pages, one item. 435 of 470.

**All eight report-pipeline events share one sentence: "Publisher: Codeunit 44 `ReportManagement`."**
The RUNTIME raises an event on a NAMED BaseApp codeunit -- and CLAUDE.md's fourth invariant is that
neither transpiler nor runtime ever names a concrete AL object. **This is the first place in the sweep
where the documentation and an invariant point in opposite directions.**

board:0575's answer keeps both: the runtime raises a RUNTIME event, and the transpiler emits a shim
into the app that declares `ReportManagement` which forwards it to the AL publisher. **So the name
lives in `apps/`, which is generated, and never in `src/`.** It cannot be avoided altogether, because
the subscribers are AL and their `[EventSubscriber]` attribute names codeunit 44 -- the binding is
written into the BaseApp and cannot be changed.

**Every one of the eight has a `var` out parameter, and the contract makes the trap expensive**:
*"the content in the `TargetStream` will be DISCARDED if the `Success` parameter is `false` upon
return."* A copied `var` here is not a wrong value, it is a lost document.

**The population is two or three occurrences each** -- the publisher plus one or two subscribers. The
extension points exist and BC barely uses them.

**And one name in board:0557's trigger listing is not an AL event at all.** That listing's last line
is `ROOT OnMergeDocumentReport`; measured, the name appears **ZERO times in 36 673 `.al` files**.
Reading the listing as a list of AL events would have produced a ninth event that does not exist.

**The census, counted because the pattern was already written, and routed to board:0057:**
`[IntegrationEvent(` **94 269** across 5 461 files, `[EventSubscriber(` **11 135**, `[BusinessEvent(`
**4**, `var IsHandled` **35 392**.

**CLAUDE.md says "the BaseApp wires hundreds of `[EventSubscriber]`s inside itself."** It is 11 135.
**The sentence's point stands and stands harder; its number is two orders of magnitude low**, and the
real one now sits in the event root where the dispatch is designed.

---

## `developer/` root, forty-first pass -- a report declares its layouts twice, and board:0457 is closed

Five pages, one item. 440 of 470.

**board:0457 left a contradiction as its first task**: `WordMergeDataItem` **299** against
board:0452's **2** `WordLayout` declarations, with the note that either the property is declared on
reports whose layout comes from elsewhere or one of the counts is wrong.

**Settled, and neither count was wrong.** The 2 is the LEGACY property. The current form is a
`rendering` section of named layouts, measured:

| | count |
|---|---:|
| `rendering` sections | 669 |
| `layout(` entries | **884** |
| `Type = RDLC` | 641 |
| `Type = Word` | **182** |
| `Type = Excel` | 61 |
| `Type = Custom` | **0** |

641 + 182 + 61 = 884 exactly, so the census is complete. **184 Word layouts against 299
`WordMergeDataItem`**, and the remaining gap is the property doing its job -- one layout, several data
items.

**Both declaration forms have to be built, and the migration is split by TYPE.** `RDLCLayout` 768
against `Type = RDLC` 641; `WordLayout` **2** against `Type = Word` 182; `ExcelLayout` 1 against 61.
**A transpiler supporting only the recommended syntax loses 768 RDLC layouts** -- and loses them
SILENTLY, because a report with no layout is legal when it is processing-only and merely renders
nothing when it is not. That is board:0576's negative control.

**`MimeType` is 0 and `Type = Custom` is 0**, which agree: the property is documented as supported
only with `Custom`. `Summary` 855 against 884 layouts, so 29 layouts fall back to their NAME -- which
makes a layout name a user-visible string and decides what the legacy form's synthesised name may be.

**And `DefaultRenderingLayout` 646 against 669 `rendering` sections leaves 23 without a default**,
consistent with the documented rule that a report EXTENSION may not set it.

---

## `developer/` root, forty-second pass -- the rendering route, and two dials that look like one

Six pages, one item, and **the user's instruction recorded in the layout items**: reports are
TRANSLATED to XSL-FO and Apache FOP is the engine.

**That is now written into board:0576 and board:0547**, and it decides what a layout type means: an
RDL definition and a Word `.docx` are two descriptions of a page, and XSL-FO is the third that FOP can
print -- **one rendering engine and two translators into it**, rather than three page engines. The
Excel row is deliberately different and is argued rather than assumed: an `.xlsx` is written, not
rendered, and the evidence is board:0557's request-page button table, where Excel offers `Download`
while RDLC and Word offer `Print` and `Preview`.

**board:0577 is the other finding, and it is a precedence chain with the USER at the top.**
`Report.Language` picks the WORDS and `Report.FormatRegion` picks the SEPARATORS -- two dials, and the
documentation is explicit because they are easy to conflate. Five places may set them: the object
definition, the Report Limits and Settings page, a report trigger, the instance, and the request page,
**each overriding the one before it.** That is the reverse of every other property on this board,
where the object declares and the runtime obeys.

**Measured: `currReport.Language` 378, `currReport.FormatRegion` 321 -- and `FormatRegion` as a
PROPERTY, zero.** The documentation lists the object definition first and the source never uses it, so
the bottom of the chain is unexercised and the whole population is the two code forms. Both methods
are among board:0571's 74 absent, so **699 call sites reach two methods that do not exist.**

**Its negative control is the conflation**: fold the two into one locale -- what almost every other
system does -- and every case passes except the one that renders an English month name beside a German
decimal separator.

**And a control routed to board:0553**: a Rich Text control is `ExtendedDatatype = RichContent`,
**23 declarations of 2 745**, must be ALONE IN ITS GROUP, and is persisted as HTML with images inlined
-- so it is a `Blob` the client renders and not a new storage mechanism. The distribution of the other
`ExtendedDatatype` values is recorded there: `PhoneNo` 1 359, `Email` 831, `URL` 219, `Masked` 104,
`Barcode` 96, `Ratio` 67, `Person` 39, `Task` 4, `Document` 2, `None` 1.

---

## `developer/` root, forty-third pass -- the last twenty-two

**22 pages: one item, thirteen routings, eight with a reason. 468 of 470.**

**board:0578 is the item, and it does not resolve what it finds.** AL has documentation comments --
`///` with an XML tag set that maps onto Doxygen almost one for one. Measured over `~/Git/BCApps/src`:

| | count |
|---|---:|
| `///` lines | **542 639**, across 5 768 files |
| `<param` | 136 416 |
| `<summary>` | **111 885** |
| `<returns>` | 11 254 |
| plain `//` lines | 719 565 |

**1.26 million comment lines, and CLAUDE.md says a generated file carries none.** For the 719 565
plain comments that is right for the same reason `make` strips `src/`. **For the 542 639 documentation
lines the SAME document argues the other way**: the reader of `apps/` is a model with AL in its
training data and no knowledge of agiru, and `include/` requires Doxygen on every public name because
of it. The item states the tension with the number, lays out three positions and their costs, and
names the one measurement that would decide it -- what a comment block costs in the door's parse time,
which is one `make tree` pair. **It recommends translating, conditional on that measurement, and
records that dropping is what happens if nobody chooses.**

**Four routings carry a finding rather than a pointer:**

- **board:0564** -- BC's own tests seed deliberately, with `SetSeed(1)` or the `Any` library. That is
  what makes a fixed default for `Randomize()` wrong: **the tests already have a way to be
  deterministic and it is not the default of `Randomize`.**
- **board:0033** -- *"the app ID is used AT RUNTIME TO BIND TABLE NAMES. Changing the app ID results in
  data from old tables not being used."* So an app id is part of a table's identity in the DATABASE,
  not only in the package. And `idRange` is a compile-time check with the range in the manifest.
- **board:0029** -- the full list of objects that have triggers, and one reachability rule: *"if the AL
  code is in a `local` method, you can't run it from another object."* board:0359 records that
  `Access = Internal` must NOT become `private`; **`local` is the case where the restriction is real**,
  and the two must not be collapsed.
- **board:0013** -- BC removed its integration-record tables and told extensions to use `SystemId` and
  `SystemModifiedAt` instead. The system fields are the platform's own identity mechanism, which is why
  a rowversion that is merely present is worse than none.

The five field-migration pages are one operation -- moving a field between extensions while preserving
its data -- and it is deployment-time work driven by `migration.json`, which an earlier sweep already
recorded. `devenv-page-scripting.md` is a client record-and-replay tool producing YAML for a pipeline;
what it CAPTURES is board:0553's control tree, which is why the row says so.

---

## `developer/` root, forty-fourth pass -- 470 of 470, and a counter that lied

**The last two pages, one item -- and the way they were found is the pass's other finding.**

**`unread.sh` reported zero unread pages while the counter said 468 of 470.** The script tested
whether a filename appears anywhere in `ledger.md`; two pages were NAMED IN PROSE -- one in
board:0568's pass, which said `devenv-page-type-configuration-dialog.md` was "630 lines and still
unread", and the other in a routing note -- and the script counted that as read.

**The counter was right and the script was wrong**, which is the reverse of the thirty-first pass's
lesson and the same class of defect: **a check that tests the wrong thing agrees with success.** The
fix is one line -- match a ledger ROW, `^\| \`<name>\` \|`, not a mention -- and the verification
now reports 470 rows over 470 files.

**board:0568's deferred question is answered and its conclusion stands.** That item measured 13 files
declaring `area(SystemActions)` -- nine `PromptDialog`, four `ConfigurationDialog` -- against a
documentation sentence saying system actions are *"only supported by this page type"*, and deferred the
question to the unread page. **The two page types use the area differently**: `PromptDialog` allows
five names WITH `OnAction` triggers; `ConfigurationDialog` allows `OK` and `Cancel` ONLY and *"the
triggers for these actions can't be defined as they're defined by the platform"* -- only `Caption` and
`Enabled`. So the check is per page type, not per area, and the `static_assert` 0568 refused to write
would still have been wrong.

**A `ConfigurationDialog` has four mandatory properties**, two of them with required VALUES:
`SourceTableTemporary` must be `true` -- the page's whole transaction model, settings collected into a
temporary record and applied in `OnQueryClosePage` -- and `Extensible` must be `false`, making it the
third page type that cannot be extended.

**And its toggle is the fourth POSITIONAL rule in this sweep.** A card is a root-level group; it gets a
toggle when its FIRST field is a Boolean with `ShowCaption = false`. After board:0553's "the LAST
ListPart expands", board:0561's "the first two FastTabs are expanded" and board:0560's "the left-most
visible column is indented", **position is load-bearing in BC's layout model** -- which is the argument
for keeping the tree, arrived at from a fourth direction.

---

# `developer/` root is finished: 470 of 470

87 items filed from this family, and every page carries a row: an item number, a routing into an item
or one of the 78 pre-existing roots, or a one-sentence reason why it has no task.
