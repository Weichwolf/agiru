Type:     bug
Status:   open
Area:     gen
Source:   the first full build over the grown slice, 2026-09-07
Class:    silent-wrong-data

# A generated class name is unique across every app, and a name that folds onto another is refused

**Four C++ class names are declared twice in `agiru::app::tables` and `agiru::app::codeunits`**, and
the build says so as `redefinition of` the moment two of them meet in one translation unit. They
were invisible while the slice was 2 064 sources and the unity groups were small; at 6 784 they meet.

| C++ name | AL objects behind it | cause |
|---|---|---|
| `PermissionSetBuffer_Table` | `9009 "Permission Set Buffer"` (base), `9862 "PermissionSet Buffer"` (system) | **two different AL names fold to one identifier** |
| `TestTableA_Table` | `"Test Table A"` in `system_test_library` and in `tests` | the same AL name in two apps |
| `TestTableB_Table` | the same | the same |
| `ServicePriceManagement_Codeunit` | `6080 "Service Price Management"` (`Microsoft.Service.Pricing`) and `136105 "Service - Price Management"` (`Microsoft.Service.Test`) | **the fold again**: the hyphen goes the way the space does |

## Two causes and they do not share a fix

**The first is the folding.** `Identifier` drops the spaces, so `"Permission Set Buffer"` and
`"PermissionSet Buffer"` become one name. That is the `identifier casing` trap in CLAUDE.md's table
with a different character doing the damage: the map from AL name to C++ name is NOT injective, and
nothing checks that it is.

**The second is that an AL object name is unique per APP and not per tenant.** BC allows
`"Test Table A"` in two apps -- different ids, and AL tells them apart by the `namespace` each file
declares. The generator flattens every table into `agiru::app::tables`, so the namespace that
distinguishes them in AL is thrown away exactly where it was needed.

## What is taken today, and what it costs

**Nothing is fixed here yet.** The three slice entries whose partner is also in the slice are out of
`test/slice` so the build can go green on everything else -- `system/.../PermissionSetBuffer.cpp`,
`system_test_library/.../TestTableA.cpp` and `.../TestTableB.cpp`. They return when this closes.
`ServicePriceManagement` needs no entry removed: only the base one is in the slice.

## What the AL namespaces fix and what they do not, measured 2026-09-07

The namespaces are being mirrored (the round after this item was filed), and the four split
cleanly by whether that is enough:

| collision | the two AL namespaces | mirroring fixes it |
|---|---|---|
| `ServicePriceManagement_Codeunit` | `Microsoft.Service.Pricing` / `Microsoft.Service.Test` | **yes** |
| `TestTableA_Table`, `TestTableB_Table` | different apps, different namespaces | **yes** |
| `PermissionSetBuffer_Table` | `System.Security.AccessControl` in BOTH | **no** -- same namespace, and the fold makes `"Permission Set Buffer"` and `"PermissionSet Buffer"` one name |

**So the fold defect survives the namespace round and this item stays open for it.**

## The choice, and why it is not made in this round

**Mirroring the AL namespace is the answer that fixes both causes** -- `agiru::app::tables` becomes
`agiru::app::tables::<the AL namespace>`, name equality with AL improves rather than degrades, and a
name can then only collide where AL itself would refuse. It is also a change to every generated file
and every reference between them, so it is its own round with its own measurement.

**The cheap alternative is a suffix on collision**, and it is rejected as the primary fix: the
suffix would have to be stable under insertion, which means every colliding name carries it (not
just the second one), and a reader then meets `PermissionSetBuffer_9009_Table` for a reason the AL
source does not show.

**What must hold either way: the generator DETECTS the collision instead of emitting it.** A map
that is not injective is a defect the compiler finds late and only sometimes -- `ServicePriceManagement`
is a redefinition that no translation unit happens to see today. The detection is cheap: the
identifiers are already indexed, and a second entry under one name is an abort with both AL names
and both ids in the message.
