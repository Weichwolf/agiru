Type: root
State: open
Area: al, gen

# The transpiler translates every AL object kind

The UT suite is the PROOF, not the target. The target is a complete Business Central in C++, and
that makes "what the transpiler must be able to do" a question with a measurable answer rather than
a horizon. `~/Git/openerp/scripts/transpiler` is the floor: 18 894 lines over 46 modules that
reached 97.0 % of the UT subset, with a generator per object kind.

Measured over BCApps and over both trees on 2026-09-02:

| AL object | files in BCApps | agiru | openerp |
|---|---|---|---|
| Codeunit | 14 417 | yes | yes |
| Page | 6 967 | **yes** | yes (`page_gen`, 1 553 lines) |
| Table | 4 720 | yes | yes |
| **Report** | **2 142** | no | yes (`report_gen`, 995) |
| Enum | 1 470 | yes | yes |
| **PermissionSet** | **1 127** | no | no |
| **Query** | **464** | no | yes (`query_gen`, 465) |
| **XmlPort** | **384** | no | yes (`xmlport_gen`, 550) |
| Interface | 209 | **yes** | yes (`interface_gen`) -- board:0027 |
| **Entitlement** | 206 | no | no |
| **Profile** | 59 | no | no |
| **ControlAddIn** | 20 | no | no |

**Five of twelve kinds.** Page landed on 2026-09-02 -- 2 962 of 2 962 in the read roots, parsed and
written -- and Interface with it. What is left is not the tail either: Report is the largest of the
seven and PermissionSet the second.

## Counted over the READ ROOTS rather than the tree, 2026-09-04

The table above counts BCApps whole; the number that decides the work is what `apps.json` actually
reads. 11 627 `.al` files, of which **2 075 are of a kind with no generator**, and each now has an
item:

| kind | files in the read roots | item |
|---|---:|---|
| **Report** | **668** | board:0063 |
| PermissionSet (all spellings) | 495 | board:0062 |
| PageExt | 168 | board:0033 -- **merged now** |
| Query | 164 | board:0064 |
| TableExt | 98 | board:0033 -- **merged now** |
| Entitlement | 75 | board:0062 |
| EnumExt | 64 | board:0033 -- **merged now** |
| XmlPort (all spellings) | 51 | board:0065 |
| Profile | 44 | -- |
| ControlAddIn (all spellings) | 16 | -- |
| ReportExt | 14 | board:0063 |
| ProfileExt, PageCust | 1 each | -- |
| `dotnet.al` | 28 | board:0035 |

`.Namespace.al` (67 files) is not an object: it is a namespace declaration carrying the BaseApp's
own module documentation, and it needs no generator.

## THE COUNT ITSELF HAS A HOLE, AND IT IS THE FAILURE MODE THIS TREE NAMES

`UntranslatedKinds` (`src/tc/Main.cpp:179`) walks a HAND-WRITTEN array of eleven suffixes. CLAUDE.md
lists that shape as a measured failure mode -- "a list somebody has to remember to fill: one entry
point sets it, the others get an empty one and emit nothing" -- with the guard that it must FIND
ITSELF. It does not, and three kinds fall through it:

| what | files | why it is missed |
|---|---:|---|
| `dotnet.al` | **28** | the array holds `dotnet`, so the suffix compared is `.dotnet.al`; the files are named `dotnet.al` with a `/` before them and never match |
| `.PageCust.al` | 1 | the array holds `pagecustomization`; the file suffix is `PageCust` |
| `.ProfileExt.al` | 1 | not in the array at all |

So 30 files in three kinds are reported as zero rather than as a hole, which is the one thing
CLAUDE.md says a kind with no generator may never be.

**What is NOT a defect, checked in the same pass:** `SourcesEndingIn` compares the suffix
case-insensitively, so the 60 files BCApps names `.codeunit.al`, `.page.al`, `.enum.al`,
`.table.al`, `.interface.al` and `.xmlport.al` in lower case are read like any other. That was the
suspicion; the code answers it.

**The repair is the guard rather than three more entries.** The kinds come from the file names the
roots actually hold -- every distinct `<Kind>` in `<Name>.<Kind>.al`, minus those a generator claims
-- so a kind nobody has seen before is counted the first time it appears.

**The layout grammar is done and the rest inherits it.** A page's `layout` and `actions` turned out
to be ONE shape -- `<kind>(<name>[; <source>]) { properties triggers children }` -- read by kind
rather than by a case per keyword, with `(` versus `=` separating a control from a property. A
report's `dataset`, a query's `elements` and an xmlport's `schema` are the same shape, so the
parser for each is a header and a writer rather than a grammar.

## And the body is the bigger half

| | lines |
|---|---|
| `agiru` `src/gen/BodyWriter.cpp` | 445 |
| `openerp` `generator/body_emitter/` | 4 075 |

That is the AL statement and expression translation -- the actual CODE inside a procedure, as
against its signature. A nine-fold gap, and it is the half that decides whether a translated
codeunit DOES anything. The 3 927 lines of `src/al` and `src/gen` together are a fifth of the
predecessor's 18 894, and the predecessor was 97 % on a subset rather than complete.

## What this item is for

It is not a plan; it is the DENOMINATOR. Every other generator item -- board:0027 for interfaces,
board:0030 for pages, board:0033 for extensions -- is a row of the table above, and the count of
kinds translated belongs beside the counts of objects parsed in the run summary. A transpiler that
reads 4 029 codeunits and no page is not 4 029 objects along; it is three kinds of twelve.

## What is true when this closes

- Every AL object kind BCApps declares has a generator, and the run summary counts them by kind.
- A kind with no generator is REPORTED by name and count, not skipped in silence.
- **The list of kinds is DERIVED from the file names in the read roots, not written by hand**, so a
  kind nobody anticipated is counted the first time one appears. Negative control: add a file of an
  invented kind and require the run summary to name it.
- The body emitter translates the statement and expression grammar the BaseApp uses, measured
  against the same corpus rather than against the cases that happen to be written.
