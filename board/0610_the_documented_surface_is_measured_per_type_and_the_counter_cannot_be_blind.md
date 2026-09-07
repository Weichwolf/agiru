Type:     finding
Status:   open
Area:     rt, gen, doc
Source:   the first run of `scripts/al_surface.py` this session, 2026-09-07
Class:    silent-wrong-data

# The documented surface is measured per TYPE, and the counter cannot be blind

**THE COMPLETENESS COUNTER WAS BLIND, AND THEN IT WAS SATURATED.** Both failures were live at once,
and the second was hiding behind the first.

## Blind

`scripts/al_surface.py` reads the door out of doxygen's XML under `build/doc/xml`. Nothing on the
way to a build makes that directory -- `make` does not run doxygen -- so on a fresh checkout or
after `make clean` the read returned an EMPTY SET and the report said:

```
al surface  1253 documented methods over 93 types
            1174 implemented by the predecessor, 0 here
```

`test/surface-baseline` says `1173 1253`, so the tree had measured 1 173 once and now answered 0.
`make lint FULL=1` would have called that a shrink of 1 173 -- loud, but only in the FULL run, and
the plain report printed a table of zeros without a word. It is CLAUDE.md's blind gate exactly: the
step that was supposed to do the work never ran, and everything downstream reported a number.

**The counter now REGENERATES what it measures** -- doxygen runs when the XML is missing or older
than `include/` -- and it ABORTS rather than returning zero, twice: no `index.xml` after doxygen
ran, and no names parsed out of it.

## Saturated

With the read repaired the report said **1 253 of 1 253**, which is not a measurement either. The
comparison was against ONE BAG of every name in `include/`: `Insert` counted for `XmlElement`
because `Record` has an `Insert`. A measure that cannot go down has stopped measuring.

**It compares per TYPE now**, following base classes, with the free functions beside it because AL
documents a static on the type (`Text.CopyStr`, `Session.CurrentClientType`) and the door writes
exactly those as free functions. Three AL types are spelled apart in the door and the script NAMES
them rather than counting their whole method list as missing -- `Record` is `Table<Derived>`,
`TestPart` is a `TestPage`, `RequestPage` is a `Page`. That is CLAUDE.md's rule that a mechanical
pass cannot tell a naming defect from a gap.

**The honest number was 1 226 of 1 253, and 27 gaps in five types:**

| type | missing |
|---|---|
| `TestRequestPage` | 21 of its 25 -- `Caption`, `Editable`, `Expand`, `FindFirstField`, `FindNextField`, `FindPreviousField`, `First`, `GetValidationError`, `GoToKey`, `IsExpanded`, `Last`, `New`, `Next`, `Preview`, `Previous`, `Print`, `SaveAsExcel`, `SaveAsPdf`, `SaveAsWord`, `SaveAsXml`, `ValidationErrorCount` |
| `MediaSet` | `ExportFile`, `Remove` |
| `TestPart` | `Enabled`, `Visible` |
| `Version` | `Create` |
| `System` | `Time` |

**25 of the 27 are written and the counter reads 1 251 of 1 253, per type.** The two that stand
are `TestPart.Enabled()` and `TestPart.Visible()`, and the reason is a measurement:

**A PART'S METHOD AND A PAGE'S CONTROL SHARE ONE MEMBER NAMESPACE.** Putting them on `TestPage`
-- where a part lives, since AL declares one as `TestPage <Subpage>` -- hid the CONTROLS of that
name: 40 pages declare a control called `Enabled` and 16 declare one called `Visible`, and
`CurrExchRateServiceCard.Enabled.Editable()` stopped compiling. Taken back with the number in it.
AL keeps `TestPart` a separate type for exactly this reason, so the fix is a separate type here
too, and it is not two methods' worth of work: a part carries a page's whole surface.

## The one that was not a gap but a defect: `System.Time()`

**`Time` IS AN AL TYPE AND AN AL BUILTIN, and C++ cannot hold a class and a function of one name in
one namespace.** So `gen_builtins.py` skipped the builtin -- it skips whatever another door header
declares -- and the door had no `Time()`. But the generator emitted `Time()` anyway, because the
door's callable set is SCRAPED and the `Time` CLASS has a constructor, so `Time` looked callable.

C++ read `Time()` as a value-initialised `Time`. **178 call sites got 0T where AL gives the clock**
(measured 2026-09-07), and two of them say what that costs:

```cpp
TimeLocked = Time();                              // RecordLinkImpl, timing a lock
if (InTransaction && Time() > TimeLocked + 1000)  // ... against a clock that never moves
SetSeed(Time() - Time{});                         // Any, seeding a generator with zero
```

It compiled, it ran, and it returned a `Time`. Same signature as board:0608 one type over.

**The door spells the builtin apart -- `CurrentTime()` -- and the GENERATOR translates AL's name
into it**, through one entry in `kSpelledApart` in `src/gen/Door.cpp`. Two things had to move for
that to work, and the second is the more useful:

- the bare no-argument builtins were emitted from the hand-kept `kNoArgument` list VERBATIM, so no
  spelling map ever saw them. They go through `BuiltinSpelling` now, which is what makes
  `kSpelledApart` a mechanism rather than a special case.
- IT IS A BUILTIN MAP AND NOT THE GENERAL ONE. Putting `time -> CurrentTime` into
  `AsTheDoorSpellsIt` renamed `DateTime.Time()` and `FieldType::Time` as well -- 22 errors in one
  build. A member keeps AL's spelling; only a BARE builtin is translated.
- `kNoArgument` itself is still a list somebody has to remember to fill (CLAUDE.md's trap). It has
  62 entries against `system-*-method.md`'s 71 documented methods; the gap between those two
  numbers is the next item, and it is derivable rather than typed.

## What proves it

`python3 scripts/al_surface.py` prints 1 251 of 1 253 with `TestPart` as the only row, and answers
with an ABORT rather than a zero when `build/doc/xml` is missing. The negative control for the
`Time` half is the count: a bare `Time()` in `apps/` was 178 and is 0, and `CurrentTime()` is 167 --
the eleven that are not in the difference are the sites where AL wrote `Time` as a member of
something else, which is the rename staying where it belongs.
