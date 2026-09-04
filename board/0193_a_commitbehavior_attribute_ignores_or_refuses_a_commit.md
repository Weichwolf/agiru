Type:     task
Status:   open
Parent:   0012
Area:     gen, rt
Source:   developer/attributes/devenv-commitbehavior-attribute.md
Verdict:  fehlt
Class:    silent-wrong-data

# A `[CommitBehavior]` attribute makes a `Commit` inside the scope IGNORED or an ERROR

`[CommitBehavior(Behavior: CommitBehavior)]` on a method. Two values: **`Ignored`** -- a `Commit`
called anywhere inside that method's scope does nothing -- and **`Error`** -- it raises.

**This is the one attribute that can silently change what a posting makes durable**, which is why it
belongs under board:0012 and why its class is `silent-wrong-data`. CLAUDE.md's first invariant says
"a `Commit()` makes what came before it durable and no later rollback may undo that". An `Ignored`
commit means the AL that called `Commit` did NOT get that guarantee -- deliberately, because the
caller wrapped it -- and a runtime that honours the `Commit` anyway makes durable what BC would have
rolled back.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**183 `[CommitBehavior` declarations.**

## The IST-state

`include/type/CommitBehavior.h` exists as a door header with the two members. The attribute parses
into the raw list and is dropped; `Commit` behaves the same everywhere.

## The choice

The generator emits, at the top of the annotated method's body, a scoped guard that pushes the
behaviour onto the session and pops it on exit -- so it covers the whole DYNAMIC scope, including
everything the method calls, which is what "inside the method scope" means. `Commit` consults the
top of that stack:

| top of stack | `Commit()` |
|---|---|
| nothing | commits |
| `Ignored` | returns, having done nothing |
| `Error` | raises |

**Why a stack and not a flag.** Methods nest, and an `Ignored` scope inside an `Error` scope has to
restore the outer behaviour on the way out. A flag would leak the inner value to the caller.

## Ordering

After board:0012's transaction boundary exists. Before any posting item, because 183 sites decide
whether a `Commit` is honoured and a posting run crosses several of them.

## Gate, and its negative control

A method marked `Ignored` that inserts, commits and then raises: the row must be GONE, because the
commit did nothing. The same method unmarked: the row must survive.

**The negative control is that pair** -- a runtime that drops the attribute makes both cases keep
the row, which looks correct until something rolls back.
