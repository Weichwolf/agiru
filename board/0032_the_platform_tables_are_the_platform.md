Type: root
State: open
Area: rt, gen

# The platform tables are the platform, not an app

87 tables are named by 381 declarations in the source roots this tree reads, and no AL file declares
any of them (measured 2026-09-02, after the index repair; before it the same count read 192 and
1 342, and 105 of those were an indexing defect rather than a gap). What is left is BC's own:

| named | table | kind |
|---|---|---|
| 39 | `User` | stored |
| 39 | `Field` | VIRTUAL |
| 21 | `Integer` | VIRTUAL |
| 18 | `Date` | VIRTUAL |
| 14 | `AllObjWithCaption` | VIRTUAL |
| 13 | `All Profile` | stored |
| 12 | `Company` | stored |
| 8 | `Tenant Permission` | stored |

## The hypothesis that was refuted before it was built

**Declare them as `.al` in an agiru-owned app and transpile them like anything else.** It looked
right -- it would have kept "the runtime knows no AL object" without an argument -- and it is wrong:
a transpiled table can do exactly nothing that these tables exist to do. `Field` produces one row
per field of every table in the catalogue; `Integer` produces a range from the FILTER applied to it;
neither has storage, and generated code cannot reach the catalogue at all. Writing them in AL would
have produced a table shaped like the answer with none of the behaviour.

**They are not AL objects. They only look like AL objects to AL code**, and internally they do
things `apps/` cannot. `devenv-virtual-tables.md` says it outright: "A virtual table contains system
information. You can't change the data in virtual tables. You can only read the information.
Virtual tables aren't stored in the database, but are computed by Business Central at runtime."

**And the invariant does not forbid this.** "THE RUNTIME KNOWS NO AL OBJECT" forbids the runtime
naming `Sales Line`. `Field` (2000000041) is platform surface in the same sense `SystemId`
(2000000000) is, and the runtime already names all five system fields for that reason. The line is
between the PLATFORM, which the runtime IS, and the APPLICATION, which it must never know.

## The two kinds, which the predecessor also separated

`~/Git/openerp/openerp/runtime/base/system_tables.py` (1 794 lines) and `virtual_metadata.py` (273)
split them the same way, and its comments carry what each cost:

- **Stored system tables** -- `User`, `Company`, `All Profile`, `Record Link`, the permission set --
  are ordinary tables the platform ships. They need a DECLARATION and nothing else: schema, insert,
  filters and keys all work the moment one exists.
- **Virtual tables** -- `Field`, `Integer`, `Date`, `AllObj`, `AllObjWithCaption`,
  `Table Relations Metadata`, `Table Metadata`, `Page Metadata` -- have no storage. Their rows are
  COMPUTED, and `insert`/`modify`/`delete` must refuse. openerp notes the two access costs it
  measured: `Get(Id)` materialises one row and is the common case, while a scan materialises
  everything and is deliberately not the default path.

`Integer` is the one to build first and the documentation makes it small enough to be a seam rather
than a project: ID 2000000026, ONE field, integers from -1 000 000 000 to 1 000 000 000, and "by
applying a filter to the Integer virtual table, you can easily get a subset or range of numbers
that can be used to control looping". It has no dependencies -- no catalogue, no session, no
database -- so it proves the mechanism alone. `Field` needs the table catalogue (board:0025) and
comes after it.

## What has to be decided, and it is not decided yet

- **How a row source is chosen.** A generated record has no virtual functions, so a virtual table
  cannot simply override `Get`. The seam is in the metadata or in the class, and which one it is
  decides whether `RecordRef.Open(2000000026)` reaches the same rows as `Rec: Record Integer` does.
- **Where the generator learns the names.** `Record "Field"` must resolve into `agiru::` and not
  `agiru::app::` -- the same discrimination a parameter named after its type already needs. A list
  of platform table names belongs beside `TypeName()`'s list of AL type names, which is the same
  kind of thing: the platform's own vocabulary, written down once.

## What is true when this closes

- `Rec: Record Integer` with `SetRange(Number, 1, 10)` walks ten rows.
- A write to a virtual table REFUSES with the documented reason.
- The 87 shrink, and the count in the transpiler's report is what says by how much.
- No name of an application table appears anywhere in `src/`.
