Type:     task
Status:   open
Parent:   0062
Area:     gen, rt
Source:   developer/devenv-entitlements-and-permissionsets-overview.md, developer/devenv-entitlement-object.md
Verdict:  fehlt
Class:    activation

# An entitlement is a ceiling, and it is never lowered on premises

board:0381 filed `ObjectEntitlements` and left one question open in its own words: *"whether agiru HAS
a licence is the open question and it is not this item's to answer."* **These two pages answer it in
one sentence, and it is a citation rather than a preference.**

## What the platform guarantees

> **"Entitlements are ONLY USED IN THE ONLINE VERSION of Business Central."**
> -- `devenv-entitlements-and-permissionsets-overview.md`
>
> **"Entitlements can only be used with the online version of Business Central."**
> -- `devenv-entitlement-object.md`

Said twice, on two pages, without qualification. agiru is one process and one PostgreSQL on the user's
own machine, so **the entitlement object is transpiled and represented and never restricts anything.**
That is not a gap and it is not a shortcut -- it is what the platform does off line, and an
implementation that gated on entitlements would be MORE restrictive than BC.

**And the composition rule says exactly how a ceiling behaves when it is absent:**

> **"Being entitled defines the MAXIMUM permissions a user is entitled to. Actual permissions are the
> INTERSECTION between the permissions the user is ENTITLED to and the permissions the user is
> ASSIGNED."**

`assigned ∩ entitled`. With no entitlement layer the entitled set is the universe and the intersection
is the assigned set -- so the on-premises behaviour falls out of the same formula rather than being a
special case. **board:0492's lattice is the `assigned` half of this expression** and stands unchanged;
this names the operator above it.

## Three further rules

**An entitlement may name only permission sets from ITS OWN APP.**

> "An entitlement can only include permission set objects which reference the objects that are
> included WITHIN THE SAME APP. This is to ensure that the entitlements included with one app can't
> alter or redefine the entitlements included with another app."

**That is a translation-time refusal**, and it is the app boundary CLAUDE.md already claims the linker
enforces -- here as a rule the generator can check before the linker ever sees it, because both
objects' apps are known.

**Permission sets come in three types and only one of them is an AL object.**

| type | where it lives | editable |
|---|---|---|
| **System** | AL `PermissionSet` / `PermissionSetExtension` objects | no |
| Extension | an XML file shipped with an app | no |
| **User-Defined** | the `Tenant Permission Set` and `Tenant Permission` TABLES | yes |

So the transpiled objects are the System half, the tenant tables are ordinary data with ordinary rows,
and a user copying a System set produces a User-Defined one. **The distinction is a column on the
tenant table, not a second mechanism.**

**And the switch that decides which wins has a default.** `UsePermissionSetsFromExtensions` defaults to
`true`: "the server will be retrieving all System permission sets and permissions from the AL objects
... the permissions data, in case it's still present in the application database, will be
DISREGARDED." agiru has no data-sourced permissions at all, so it is permanently on the `true` side
and the setting does not need to exist -- recorded so the absence is a decision rather than an
oversight.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

| | count |
|---|---:|
| `permissionset <id>` | **1 124** |
| `permissionsetextension <id>` | 720 |
| `Permissions =` | 4 020 |
| **`entitlement <name>`** | **206** |
| `ObjectEntitlements =` | 203 |
| `Assignable =` | 1 116 |
| `IncludedPermissionSets =` | 968 |
| `RoleType =` | 56 |
| **`ExcludedPermissionSets =`** | **1** |

**The entitlement `Type` distribution, counted over the 206 declarations themselves** -- the number
board:0483 deferred, since `Type =` is 1 555 across all object kinds and no line pattern separates
them:

| `Type` | count | documented in the property page |
|---|---:|---|
| `PerUserServicePlan` | **101** | yes |
| `Role` | 56 | yes |
| `ConcurrentUserServicePlan` | 16 | yes |
| `ApplicationScope` | 14 | yes |
| `Implicit` | 10 | yes |
| `Application` | 9 | yes |
| `FlatRateServicePlan` | **0** | yes |
| `PerUserOfferPlan` | **0** | yes |
| `Unlicensed` | **0** | yes |
| `Group` | **0** | yes |

206 declarations, 206 `Type` declarations -- every entitlement declares one, so the census is complete
and the property is effectively mandatory.

**Six of the ten values are used and four have no call site.** And the concept page's own examples
cover a DIFFERENT six: it demonstrates `Role`, `PerUserOfferPlan`, `Unlicensed`, `Group`,
`Application` and `ApplicationScope` -- three of which the source never declares -- and never shows
`PerUserServicePlan`, which is the most common value in the tree by a factor of two. **The property
page `devenv-type-entitlement-property.md` is the enumeration; the concept page is a sample, and it
is not a representative one.**

`RoleType` splits **`Delegated` 32, `Local` 24**, summing to 56 -- exactly the `Role` count, so every
`Role` entitlement qualifies itself.

**All 206 entitlements live in four trees**: `System Application/App`, `Business Foundation/App`,
`Apps/W1` and `Layers/W1`. None in a localisation layer.

**`ExcludedPermissionSets` is declared ONCE in 2.56 million lines.** board:0379 and board:0492 built a
four-rule truth table around exclusion; the truth table is right and its population is one. That
decides its order, not its correctness.

**Two builtins are documented and never called.** `NavApp.IsEntitled(name[, appId])` and
`NavApp.IsUnlicensed()`: **0 call sites each**. The way BC's own code tests entitlement is the other
one the page shows -- `Record.WritePermission()` **383** and `Record.ReadPermission()` **1 009**, which
return the intersection above and do not name an entitlement at all. So the two entitlement builtins
are a documented gap with no caller, and the two permission builtins are the real surface.

## The IST-state

- **`Entitlement` has no generator and no AST node** -- board:0034's hole, the same shape board:0556
  found for queries and board:0557 for reports.
- **`board:0062` records that no permission gate exists**, so there is nothing above which a ceiling
  would sit.
- **`ReadPermission` and `WritePermission` are not implemented**, and at 1 009 and 383 call sites they
  are the part of this subject with a population. A missing `ReadPermission` that returns `true`
  passes every one of those call sites silently.

## The choice

**The entitlement object is transpiled in full and consulted by nothing, and the reason is written
into the generated file's own structure rather than into a comment.**

```cpp
enum class EntitlementType : std::uint8_t {
  PerUserServicePlan, FlatRateServicePlan, Role, ConcurrentUserServicePlan,
  Application, ApplicationScope, Implicit, PerUserOfferPlan, Unlicensed, Group
};
```

`constexpr` data in `.rodata`, all ten values because the property page enumerates ten, with
board:0483's two conditional `static_assert`s -- `RoleType` only with `Role`, `GroupName` only with
`ConcurrentUserServicePlan`.

**Why all ten and not the six with call sites:** the completeness measure is documented syntax block
against C++ signature, and an enumerator costs nothing. The four unused ones are where an extension
would land.

**`ReadPermission` and `WritePermission` return `assigned ∩ entitled`, and off line the entitled half
is the universe** -- so they return board:0492's assigned mask, unmodified. **The intersection is
written down anyway**, as `Assigned(obj) & Entitled(obj)` with `Entitled` returning all bits, because
the formula is the specification and a function that returns a constant is honest about which half is
missing. A version that simply omitted the operand would have to be rediscovered if agiru ever grows
a licence.

**Refuse at translation time:** an `ObjectEntitlements` naming a permission set from another app. Both
apps are known to the generator, and the platform's stated reason -- one app must not redefine
another's entitlements -- is the app boundary this tree already enforces with the linker.

## Ordering

**After board:0492's assigned-permission lattice** -- there is no intersection until one operand
exists. **Before board:0381 and board:0483**, whose open question this closes.

**`ReadPermission` and `WritePermission` come FIRST of everything here**, at 1 009 and 383 call sites
against an entitlement object that gates nothing. That is the measurement reordering the subject: the
licence layer is 206 objects with no runtime effect, and the two permission builtins are 1 392 call
sites that decide whether BaseApp code takes a branch.

## Gate, and its negative control

1. an entitlement of each of the six used types transpiles and carries its `Type`
2. `RoleType` without `Type = Role` fails to transpile
3. an `ObjectEntitlements` naming a permission set from another app fails to transpile
4. `Rec.ReadPermission()` returns the ASSIGNED mask's read bit -- the entitled operand is all ones
5. revoking the assigned read permission makes case 4 return `false`

**The negative control is case 5, and case 4 alone is the blind gate.** An implementation that returns
`true` unconditionally -- which is what "there is no permission layer" produces -- passes case 4 and
every one of the 1 009 call sites. Case 5 is the only one that distinguishes "the intersection was
computed" from "nothing was checked".

**Second control, for case 3:** put the named permission set in the same app. Case 3 must then
transpile -- an implementation that refuses every `ObjectEntitlements` also passes case 3 as written.

## Class

`activation`. `ReadPermission` returning `true` today means 1 392 call sites take the permitted branch;
making it compute the real answer sends some of them the other way, and BaseApp code behind a
permission check has never run. Full A/B over the UT suite, and on a loss the deeper root is
board:0492's mask rather than this item.
