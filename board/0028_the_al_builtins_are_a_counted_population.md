Type: arc
State: open
Area: gen, rt, net

# The AL builtins are a counted population

The generator translates no builtin at all. `CurrentDateTime`, `StrSubstNo`, `CopyStr`, `Format`,
`Evaluate`, `CreateGuid` -- AL writes them as free calls and the generator emits the name unchanged,
which is an undeclared identifier in every file that uses one.

## The population is measured, not guessed

`~/Git/openerp/openerp/runtime/builtins/` holds **165 `_al_*` functions**, and that is the same
question answered once already on the same BaseApp:

| file | builtins |
|---|---|
| `_string.py` | 41 |
| `_system.py` | 26 |
| `_datetime.py` | 22 |
| `_format.py` | 14 |
| `_math.py`, `_clear.py` | 10 each |
| `_error.py` | 9 |
| `_enum.py` | 8 |
| `_recordref.py`, `_array.py` | 5 each |
| `_guid.py` | 4 |
| `_core.py` | 3 |

That list is the WORKLIST and the DENOMINATOR both, and it is a measured one: it is what a 97 %-green
run over the UT subset actually needed. It is not the specification -- `methods-auto/` is -- but a
builtin that is not in it is one no BaseApp path reached.

## Two answers taken from the predecessor rather than re-derived

- **`UserSecurityId()` returns the blank GUID.** `_system.py:409` returns
  `'00000000-0000-0000-0000-000000000000'` and 97 % of the UT subset went green over it. So the
  audit fields `SystemCreatedBy` and `SystemModifiedBy` do not block on an authentication story
  (board:0013). It becomes a value the SESSION carries, defaulting to blank, rather than a constant
  in a function -- a session field is honest, a hardcoded GUID is not.
- **`CurrentDateTime` is the wall clock**, `_datetime.py:386`. No work date, no session offset.

## The choice

**A builtin is a free function in `agiru::`, named as AL names it**, and it lands in the tier its
type lives in -- `StrSubstNo` beside the string types in `src/net`, `CurrentDateTime` beside
DateTime, `CurrFieldNo` in `src/rt` because it needs the session. The generator emits the call
unqualified and the door's `using` makes it resolve, so a generated line reads exactly as the AL
line does.

`_format.py` is the one that is not a function but a LANGUAGE: `Format(Value, Length, FormatStr)`
carries BC's own format specifiers, and openerp needed 14 functions and a spec parser for it.
~~It gets its own item when it is reached~~ -- **it is reached, and it has one: board:0066.**
board:0007 holds the decimal half of it.

## The rest of the surface, ranked over the milestone rather than guessed at, 2026-09-04

Counting the `[Test]` procedures of the 80 UT codeunits (board:0058) that name each subject, so that
nothing below sits on the board as an unranked "missing":

| subject | procedures | codeunits | `Layers/W1` | who owns it |
|---|---:|---:|---:|---|
| `Validate` | 750 | 47 | -- | board:0029, board:0043 |
| `TestField` | 484 | 43 | -- | present |
| `asserterror` | 348 | 53 | -- | present; board:0055 for the text |
| `StrSubstNo` | 184 | 35 | 14 800 | **board:0066** |
| `Format` | 108 | 20 | 11 992 | **board:0066** |
| `CalcDate` / `DateFormula` | 54 | 11 | -- | this item |
| `CalcFields` / `CalcSums` | 44 | 10 | -- | board:0047 |
| `InStream` / `OutStream` | 27 | 6 | 1 710 declarations | this item -- `File` alone carries 59 door refusals |
| `TempBlob` / `Blob` | 25 | 4 | -- | board:0017 |
| `TransferFields` | 9 | 5 | 857 | this item |
| `File.` | 6 | 2 | -- | this item |
| `Notification` | 4 | 3 | -- | board:0054 |
| `ErrorInfo`, collectible errors | 0 | 0 | 382 / 32 | unclaimed, and correctly last |
| `IsolatedStorage`, `NumberSequence`, `TaskScheduler` | 0 | 0 | 36 / 41 / -- | unclaimed, and correctly last |

**Three things this ranking settles.** `Validate` and `TestField` dominate and both already have
items. `ErrorInfo` and the collectible-error API are a documented feature
(`devenv-actionable-errors.md`, `devenv-error-collection.md`) with 21 refusing door members and
**zero** reach into the milestone -- so they stay unclaimed on purpose rather than by oversight. And
the streams are the one middle case: small in the milestone, 1 710 declarations in W1, and the
largest single refusing type in the door after the Json and Xml families.

## What is true when this closes

- A counter beside the AL surface baseline: builtins reachable against 165.
- The generator resolves a builtin call by NAME and refuses an unknown one loudly, rather than
  emitting an identifier that the C++ compiler will refuse two steps later with no AL name in the
  message.

## `NumberSequence` IS A DATABASE SEQUENCE AND ITS VALUE SURVIVES A ROLLBACK

`devenv-number-sequences.md` (read 2026-09-04, board:0071) settles what the nine refusing
`NumberSequence` methods (`coverage/methods-auto-mnop.md`) are refusing:

> Business Central number sequences are built on **SQL Server sequences**, which means that they are
> **not associated with any tables**. ... Numbers are used sequentially, but **numbers can be
> skipped** ... gaps ... can occur when transactions are rolled back or numbers are allocated but not
> used.

and, on `Current`:

> Gets the current value from the number sequence, without doing any increment. **The value is
> retrieved out of transaction. The value will not be returned on transaction rollback.**

**PostgreSQL's `CREATE SEQUENCE` is the same object with the same guarantee**, including
`nextval`'s immunity to rollback -- so this is one of the few AL primitives that maps onto the
target database without a decision. `Insert`, `Exists`, `Delete`, `Next` and `Current` become DDL and
one function call each.

**And it is the counter-example that makes CLAUDE.md's rule concrete.** "Nothing in a process is
authoritative -- a number series, a lock, the rowversion ... shared state lives in the database or it
is wrong the moment a second tier starts." A `NumberSequence` is that state done right: it is in the
database, it is outside the transaction, and it is why `Allow Gaps in Nos.` exists at all -- the
page states the trade plainly, that continuous numbering locks the `No. Series Line` row "until the
transaction completes. This can potentially block other users from being able to work."

So the application's `No. Series` (continuous, locking) and the platform's `NumberSequence`
(gapped, non-blocking) are two mechanisms with a documented reason to choose between, and only the
second is agiru's to build.
