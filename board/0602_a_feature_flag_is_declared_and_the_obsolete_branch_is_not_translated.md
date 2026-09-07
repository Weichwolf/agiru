Type:     bug
Status:   open
Area:     al, gen
Source:   -Winfinite-recursion on a generated body, 2026-09-07
Class:    silent-wrong-data

# A feature flag is declared, and the branch BC removes is not translated

**The transpiler translates code the AL compiler throws away.** `CarryOutAction` came out of the
generator as

```cpp
Boolean CarryOutAction_Codeunit::AsmOrderChgAndReshedule(RequisitionLine_Table RequisitionLine) {
  return AsmOrderChgAndReshedule(RequisitionLine);
}
```

which `-Winfinite-recursion` refuses -- and rightly, because the AL says exactly that:

```al
    procedure AsmOrderChgAndReshedule(RequisitionLine: Record "Requisition Line"): Boolean
    begin
        exit(AsmOrderChgAndReshedule(RequisitionLine));
    end;
#endif
```

It sits inside `#if not CLEAN26`. BC's own build defines `CLEAN26`, so the AL compiler never sees
this body; ours does.

## The mechanism is one line

`ApplyDirectives` in `src/al/Lexer.cpp` DOES read the directives and drop the dead branch -- the
defect is which branch is dead. `ConditionReader::ReadPrimary` ends with

```cpp
if (word == "not")  { return !ReadPrimary(); }
if (word == "true") { return true; }
return false;                          // every unknown symbol
```

An undefined flag is false, so `#if not CLEAN26` is TRUE and the obsolete half is kept. The default
is exactly backwards for the shape that dominates the tree.

## Population, measured over `~/Git/BCApps/src` 2026-09-07

| condition | occurrences |
|---|---:|
| `not CLEAN28` | 3 306 |
| `not CLEAN27` | 3 228 |
| `not CLEAN29` | 1 654 |
| `not CLEANSCHEMA25` | 288 |
| `not CLEANSCHEMA27` | 200 |
| `CLEAN27` | 197 |
| `not CLEANSCHEMA26` | 196 |
| `not CLEAN26` | 179 |

**10 127 `#if` blocks in 3 462 files**, and 990 `#else`. The great majority are `not CLEANnn`, so
the tree currently carries the OBSOLETE side of nearly every one of them -- and where an `#else`
follows, it carries the obsolete side INSTEAD of the current one.

## The choice

**The flags are a declared input, like `scope.json`.** `app.json` carries `preprocessorSymbols` in
BC's own projects (empty in the one that declares it here), so the set is a project fact and not a
guess: the version being translated is 30.0, so `CLEAN25` through `CLEAN29` and the `CLEANSCHEMA`
siblings are what a 30.0 build defines. The transpiler reads them from `apps.json` beside the app's
other declarations and every unknown symbol stays false.

**And the count is a gate.** How many objects lose a member, and how many bodies change, is an A/B
over the slice -- this is an `activation` in the other direction, since it REMOVES code that
compiles today. The obsolete branch is what `Obsolete` properties already mark, so a member that
disappears here should be one the documentation calls removed.
