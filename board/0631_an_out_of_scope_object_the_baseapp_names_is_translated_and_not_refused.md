Type:     decision
Status:   active
Parent:   0034
Area:     scope
Source:   scope.json; ~/Git/openerp WI-990, WI-1033; devenv-integration-dataverse.md
Verdict:  measured
Class:    activation

# An out-of-scope object the BaseApp names is translated, and not refused

**724 of the 1 640 red UT cases on 2026-09-09 -- 31.5 % of the whole suite -- end in one message:
`the .NET member X.Y is named by AL and not rebuilt here`.** X is not a .NET type. It is an AL object
that `scope.json` carves out, named from an object that is in scope, and the generator hands the
reference to `dotnet/Refused.h` because it knows no other home for a name it did not translate.

| named object | namespace | reached from | red UT cases |
|---|---|---|---|
| `Codeunit "CRM Integration Management"` | `Microsoft.Integration.Dataverse` | `Item.OnInsert`, `Resource.OnInsert`, four role-centre pages | 372 |
| `Query "Role Center from Plans"` | `System.Azure.Identity` | `Conf./Personalization Mgt.GetDefaultProfileID`, every session's role centre | 234 |
| `Codeunit "Plan Ids"` | `System.Azure.Identity` | `LibraryE2EPlanPermissions`, `UserListTests` | 57 |
| `Codeunit "Privacy Notice"` | `System.Privacy` | `ERM General Journal UT` | 26 |
| `Record "Feature Key"` | platform table 2000000211, no AL source | Price List UTs | 20 |
| `Codeunit "Feature Telemetry"` | `System.Telemetry` | Price List UTs | 15 |

**The first row is the finding.** `Item.OnInsert` asks whether Dataverse integration is enabled, and
the answer on every on-premises BC is `false`, read out of `CRM Connection Setup."Is Enabled"`. The
codeunit is INSTALLED there; it is not TALKING to anything. Excluding it makes an `Item.Insert` refuse,
which no BC user has ever seen -- so the exclusion, meant to remove a cloud bridge, removed a guard
that every document foundation table walks through.

## What the references say

- **WI-990 (openerp)**: the user drew the border -- CRM stays whole except Outlook, e-invoicing
  stays whole including transport, the cloud bridges go. The reason recorded there is COST: each
  object in the predecessor was a hand-checked dictionary of descriptors and an object nobody uses
  was a day nobody had. That reason does not transfer: here an object costs a transpile and a
  compile, both counted in seconds, and the gate for whether it works is `-Werror`.
- **WI-1033 (openerp)**: the same failure seen from the other side. A reference to an untranslated
  object became a `_NoopProxy` that walked into an assert message as `<_noopproxy>`. The predecessor
  chose SILENT; this tree chose LOUD (`Refused.h`), which is why the count is visible at all. Both
  are wrong: the right answer for `IsIntegrationEnabled` is `false`, and the only place that can
  say so is the codeunit itself.
- **The platform documentation**: `devenv-integration-dataverse.md` describes an integration that
  is set up, enabled and synchronised at run time -- a configuration, not a build.

## The choice

**Widen the whitelist by the namespaces an in-scope object NAMES, and keep out only what nothing in
scope names.** Measured over the W1 BaseApp: `Microsoft.Integration.Dataverse` (63 objects),
`Microsoft.Integration.D365Sales` (91), `Microsoft.Integration.SyncEngine` (40),
`System.Azure.Identity` (60), `System.Privacy`. `PowerBI` (33), `Graph` (37), `FieldService` (16)
and `Booking` stay out -- 242 in-scope files name one of the first five and none of the last four
on a document path. A codeunit that is installed and disabled is BC's own on-premises picture, and
the standalone ERP this tree is for IS an on-premises install.

**What it is not**: a runtime that answers `false` for a name it recognises. THE RUNTIME KNOWS NO AL
OBJECT, and `Refused.h` is the proof that a name without a translation is a hole, not a value.

**The `Feature Key` row is a different item**: table 2000000211 is a platform table with no AL
source, like `Field` and `AllObj`, and belongs to the virtual-table census (board:0004's `platform`
schema), not to the scope.

## Taken 2026-09-09

The six namespaces go into `scope.json`'s whitelist: the five above and `System.Telemetry`, which
the table did not carry -- `Feature Telemetry.LogUptake` is called on the price-list paths and
24 cases die on it (build 35). `System.Azure` and `Microsoft.Integration` stay in the exclude
list; the longer include wins on the dot boundary, which is what the whitelist's own rule says.
The generated sources that compile join the slice; the ones that do not are the next census.

## The measurement to take

An activation, so a full A/B: transpile with the five namespaces added, `make tree`, and the UT
milestone before and after on the seeded template. The cost side is the tree's compile time,
read from `build/times.log`, and the number of new roots `make gap` reports -- Dataverse carries
`DotNet` proxies (`CRM Integration Record`, the `CDS` tables) that may open holes of their own.

## The gate, and its negative control

`Item.Insert()` on the seeded template runs through. The control is the scope file: with
`Microsoft.Integration.Dataverse` removed again, the same Insert refuses at
`CRMIntegrationManagement.IsIntegrationEnabled` -- a gate that is green both ways proves the
call was never reached.

## Two of the 78 UT codeunits are out of scope themselves

`Graph Collect Mgt Item UT` and `O365 Integration Record UT` live under `Tests/Graph`, a
namespace-less folder the scope excludes with the Graph bridge, so the binary registers 76 of the
78 and the runner reports them as "printed no total" on every measurement. They stay in the
denominator, which is counted from the text; they turn green only with this item's decision.

## 2026-09-09: the .NET census after the widening

Of the 292 sources the six namespaces generate, 220 compiled at once and 72 did not; after the
exception hierarchy, the `Variant`-taking absent type, `NavTenantSettingsHelper.GetPlatformVersion`,
`Duration` to `Decimal` and the refused `Message`/`InnerException`/`StrPos`, 3 more compile. The
rest is the Dataverse bridge's own .NET surface (`CrmHelper`, `GenericDictionary2` walked with
`foreach`, `Type` handed around as a value) and the Entra user sync (`UserInfo.AssignedPlans`,
`Roles` with `RoleTemplateId`). `CRM Integration Management` compiles, and its 427 cases wait on
`CDS Integration Impl.` behind it, which is the next census.
