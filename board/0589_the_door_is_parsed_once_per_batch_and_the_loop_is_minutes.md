Type:     task
Status:   open
Area:     build, gen
Verdict:  teilweise
Class:    build time

# The door is parsed once per batch, and the loop is minutes rather than an hour

## Measured 2026-09-06, clang++-19, one 1 022-line table source, machine under a parallel build

| phase | seconds |
|---|---:|
| ExecuteCompiler | 8.2 |
| Frontend | 7.7 |
| of it: parsing included headers (`Source`) | 6.6 |
| of it: `std::vformat_to` instantiations | ~1.0 |
| Backend | 0.5 |

`-O2 -g` against `-O0`: no difference. **The cost is the door parse, per file, 424 files in the
slice**; a full slice rebuild after a door edit is 15-20 minutes on two cores, and a door edit is
the ordinary edit.

## The choice

- **Unity build for the slice** (`UNITY_BUILD ON`, batch 16): the door is parsed once per batch.
  Every generated source keeps its file-scope constants (`kInCatalogue`, `kTestMethods`,
  `kSubscriptions`) in a namespace of its own, so a batch has nothing to collide on. Expected
  3-4x on a full rebuild; the measurement goes here.
- **`<format>` out of the door**: the `std::vformat_to` instantiations are ~12 % of every file
  and CLAUDE.md already forbids `<format>` in the door; the header that pulls it in is found
  through the time trace's parent chain, not by grep (no door header names it).
- **The transpiler writes only on difference** (`WriteFile` already does), so ninja sees the
  files that changed and not the tree.
- The census runs its compiles two at a time; the loop adds in bulk (`bulk.sh`) and one by one
  only what bulk could not settle.

## Not the choice

C++20 modules: the same invalidation on a door edit, an immature CMake/clang path, and nothing a
unity batch does not already give for this tree's shape.

## Measured 2026-09-06, after the first round

| step | before | after | what changed |
|---|---:|---:|---|
| full slice build, 429 sources, 2 cores | ~20 min | 437 s to the first error (unity, batch 16); 1 172 s green but shared with a transpile | `UNITY_BUILD ON`, `Variant` special members out of line, `<memory>` out of the door |
| transpile, whole tree, unloaded | 284 s | **31 s** | `FieldIdentifier` memoised per table (it rebuilt the collision set per call); `Shadowed(table)` once per table; `CodeunitNames` once per procedure; `BodyIncludes` without `std::regex` and with a header index instead of a linear scan of every object per match; `Resolve` without a `LowerKey` allocation per candidate |
| one 1 022-line table source, `-O0`, machine shared | 8.2 s | **2.9 s** (front end 2.6, header parse 2.3, instantiation 1.0, back end 0.2) | the field tables, keys and `TableDef` moved from every table header into its source (`TableDefinitions`), `Variant`'s special members out of line, `<memory>` out of the door |

`make` logs every build to `build/times.log`; `chain2.sh` logs the transpile there too.

## Measured 2026-09-06, unity live

`CMAKE_CXX_SCAN_FOR_MODULES` was on: every source got a second preprocessor pass for modules
nobody uses, and CMake refuses to unity-batch a scanned source -- so `UNITY_BUILD` had been a
line in CMakeLists and nothing in `build.ninja` for a day. With the scan off and groups keyed
by app/module in buckets of 16:

| step | before | after |
|---|---:|---:|
| full slice build, 440 sources | 984 s | **560 s** |
| one source added to the slice (`settle.sh` round) | 30-120 s | **5 s** |

The rule that came out of it stands in CLAUDE.md: a build option is proved by `build.ninja`.

