Type: root
State: open
Area: gen, al

# An implicit `with` opens the source record, and the symbol lookup has a DOCUMENTED ORDER

`src/gen/BodyWriter.cpp:249` already says the shape of this item:

```cpp
case al::StmtKind::With:
  throw std::runtime_error("AL `with` needs the members it opens to be resolved first");
```

The parser reads the statement (`src/al/Statements.cpp:124`) and the generator refuses it, loudly and
correctly. What the tree has not recorded is that **the explicit `with` is the small half**, and
that the platform documents the resolution rule the refusal is waiting for.

## The population, measured 2026-09-04 over `~/Git/BCApps/src`

| | |
|---|---:|
| explicit `with ... do` statements | **1 066** |
| `SourceTable =` -- a page or a report with an implicit `with` around the WHOLE object | **6 301** |
| `TableNo =` -- a codeunit with an implicit `with` around `OnRun` | **1 578** |
| apps declaring `NoImplicitWith` in `app.json` | 130 of 879 |

**749 of 879 apps still get the implicit `with`**, and the 6 301 pages are where it hurts: a page
field's SOURCE EXPRESSION is inside the scope, so `field(Name; Name)` means `field(Name; Rec.Name)`
and every page in the tree depends on it.

## What the platform documents

`devenv-deprecating-with-statements-overview.md` gives both the scopes and the lookup.

**The scopes:**

| where | what is opened | over what |
|---|---|---|
| `with X do` | `X` | the statement's body |
| a codeunit with `TableNo` | `Rec` | the `OnRun` trigger's body |
| a page with `SourceTable` | `Rec` | **the ENTIRE object** -- "Everywhere inside the page object the fields and methods from the source tables are directly available without any prefix", and "it's not only the code in triggers and procedures that is spanned by the implicit with on the source `Rec`; also the source expressions for the fields are covered" |

**The lookup, quoted, and the order is the whole rule:**

1. the opened record's table
   - user-defined members on the table AND on its `tableextension`s
   - platform-defined members, "for example, `FindFirst()` or `Modify()`"
2. the enclosing object
   - its own user-defined members
   - its platform-defined members
3. globally defined members

> The first time the search for `IsDirty` finds the name, **it doesn't continue to the next
> top-level group**. That means that if a procedure named `IsDirty` is introduced in the Customer
> table (platform or application) that procedure is found instead of the procedure in `MyCodeunit`.

**That last sentence is why AL is deprecating the feature and why agiru must implement it anyway.**
The BaseApp on disk was compiled under these rules; translating it under any other rule silently
binds a different procedure. The failure is not a compile error -- both procedures exist -- it is
`IsDirty()` returning the wrong answer, which is **silent-wrong-data at 1 066 explicit sites and
across 6 301 objects implicitly**.

## The choice

**This is a generator problem and not a runtime one**, which is the good news: every input is a
declaration and the answer is decided once, at translation time.

- The generator already builds an `Objects` catalogue and a `Names` scope. **A `with` pushes one
  more frame onto that scope**, carrying the opened variable's TABLE, and `Names` resolves an
  unqualified identifier against the frames in the documented order before falling back to the
  enclosing object. `with X do S` then emits `S` with each resolved name written as `X.<member>`, and
  the C++ output carries no scope construct at all -- which is what makes this safe: **the deviation
  is resolved away rather than reproduced.**
- **The page and codeunit cases are the same frame pushed by the object writer** rather than by a
  statement, so one mechanism covers 1 066 + 6 301 + 1 578 sites. `PageWriter` pushes it for the
  whole object including the control source expressions; `CodeunitWriter` pushes it for `OnRun` only
  when `TableNo` is declared.
- **`tableextension` members are in the FIRST group**, which means the frame must see the MERGED
  table (board:0033 merges extensions at translation time -- so the order of those two passes is
  fixed by this rule, not by taste).
- **A name that resolves nowhere is an ERROR naming the identifier and the frames searched.** It is
  the same rule board:0030 states for an unresolvable page control, and it is what stops this from
  becoming a silent qualification.
- **`NoImplicitWith` in `app.json` turns the implicit frames off for that app**, and it is read where
  `apps.json` is already read. 130 apps declare it; treating them the same as the other 749 binds
  the wrong symbol in exactly the apps that were written to avoid it.

**`#pragma implicitwith disable` / `restore` is the file-local form and it is RARE but real**: 14
occurrences against 27 138 `#pragma warning` (measured 2026-09-04). The lexer already makes every
directive a token and drops the ones it does not act on (`src/al/Lexer.cpp:342`), so this one costs a
third case there -- and dropping it silently would bind the wrong symbol in 14 places that were
written to prevent exactly that.

## Gate, and its negative control

The page's own two examples, which are written to fail: a codeunit with `TableNo = Customer` whose
`OnRun` calls `IsDirty()` while the codeunit ALSO declares `IsDirty()` -- the call must bind the
TABLE's member when one exists and the codeunit's when it does not; and a page whose
`field(Name; Name)` must emit `Rec.Name`.

**The negative control is the shadowing case**: add an `IsDirty` to the table and require the
generated call to CHANGE. A gate written with no name collision passes over a resolver that searches
the enclosing object first, which is the natural way to write it and the wrong order.

Classification: **activation** -- the explicit sites do not translate today, so nothing regresses
there; the implicit sites are an A/B, because a page that translated by accident may stop.
