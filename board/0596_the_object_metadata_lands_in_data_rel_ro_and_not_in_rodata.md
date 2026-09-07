Type:     task
Status:   open
Parent:   0006
Area:     gen, rt
Source:   measured on the built slice, 2026-09-07
Verdict:  offen
Class:    silent-wrong-data

# The object metadata lands in `.data.rel.ro` and not in `.rodata`, and that is per-process memory

CLAUDE.md states the design in one sentence: *"Object metadata is STATIC CONST DATA, emitted by the
transpiler, never built at startup. Field descriptors, relations, keys, captions, the `OnValidate`
map -- `constexpr` arrays in `.rodata`, demand-paged, shared between processes, zero startup cost,
zero heap."*

**Measured on `build/libagiru_slice.so` at 2 055 slice sources, 2026-09-07:**

| section | size |
|---|---:|
| `.rodata` | 2.5 MB |
| **`.data.rel.ro`** | **21.0 MB** |
| relative relocations | **176 466** |
| relocation table | 4.9 MB |

**The metadata is in the wrong section and the reason is `std::string_view`.** Every name, caption,
filter, expression and AL-written property in `FieldDef`, `TableDef`, `KeyDef`, `ControlDef`,
`PageDef` and `CodeunitDef` is a `string_view`, which is a POINTER and a length. A pointer into the
image is not a constant the linker can fold: it needs a relocation, so the object holding it cannot
sit in `.rodata` and lands in `.data.rel.ro` -- read-only after the dynamic loader has WRITTEN it.

Three consequences, and the third is the one that matters at 10 000 sessions:

- **176 466 relocations are applied at load**, before `main`. That is the startup cost the design
  says is zero.
- **`.data.rel.ro` is COPY-ON-WRITE and every page of it is written**, so it is private to the
  process rather than shared between them. `.rodata` would be shared.
- **It is 21 MB at a QUARTER of the tree.** The slice is 2 055 of 7 885 generated sources, and the
  whole BaseApp is the target. Linear extrapolation is 80 MB per process of metadata that was meant
  to cost nothing -- which is the predecessor's gigabyte problem in a smaller size and the same
  shape (board:0006).

## What would fix it, and what it costs

**An OFFSET into one string blob instead of a pointer.** A `string_view` becomes a
`{std::uint32_t offset; std::uint32_t size;}` into a single `constexpr char[]` per app, and the
metadata is then position-independent data with no relocation at all -- `.rodata`, shared,
demand-paged, exactly as the design says.

**What it costs is the READER**, and that is the argument against: `field.name` stops being a
`string_view` and becomes a call, `Name(field)`. CLAUDE.md's own rule is that a reader who knows AL
must be able to write the next file from having read one, and an indirection through a blob is the
kind of cleverness the macro was thrown out for.

**A cheaper half-measure exists and should be measured first**: `-Wl,-z,pack-relative-relocs`
(`DT_RELR`), which compresses relative relocations by roughly 10:1 in the TABLE but still writes
every page at load. It fixes the 4.9 MB table and not the 21 MB of private memory.

## What is NOT the finding

**The numbers are not the page metadata's fault.** board:0553's control trees are in the slice for
107 pages of 2 717, so they are a small part of the 21 MB; the field tables of 1 194 tables are most
of it. The finding is about the SHAPE of every metadata struct in the tree, and it was there before
the page tree and would be there without it.

## Gate

`readelf -S` on the built library, with `.data.rel.ro` and the relocation count beside the slice
size, recorded like a baseline. Today: 21.0 MB and 176 466 at 2 055 sources.
