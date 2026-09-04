Type: root
State: open
Area: rt

# An error carries its code beside its text, and both are BC's own

`Assert.ExpectedError` is the second-largest primitive in the milestone after `Validate`: **276
sites in 43 of the 78 UT codeunits.** It compares `GetLastErrorText()` against an expected
substring, so every diagnostic the runtime raises is a value a test reads. `Assert.ExpectedErrorCode`
adds 29 sites in 13 codeunits, and `GetLastErrorCode()` is a door refusal today
(`src/rt/Builtins.cpp`).

## Where the 276 expected texts come from, measured 2026-09-04

| origin | count |
|---|---|
| found verbatim as a `Label` under `Layers/W1` outside the tests | 130 |
| not found in one -- a platform diagnostic, an app outside W1, or the test's own label | 130 |
| not statically resolvable (built by `StrSubstNo` from a computed part) | 16 |

**Half of the work is already done by translating the BaseApp**, provided `StrSubstNo` and
`Error(Label, ...)` substitute identically -- which is what makes `Format` and the placeholder
rules load-bearing rather than cosmetic. The other half is a SMALL CLOSED SET of platform
diagnostics, and those are the runtime's own words:

| shape | raised by | in the runtime |
|---|---|---|
| `<Caption> must have a value in <Table>: <key>. It cannot be zero or empty.` | `TestField(Field)` | present, `src/rt/Record.cpp` |
| `<Caption> must be equal to '<value>'  in <Table>: <key>. Current value is '<actual>'.` | `TestField(Field, Value)` | present -- and the DOUBLE SPACE before `in` is BC's, confirmed against `ERMCostAccAllocations`' own label |
| `<Caption> must not be <value> in ...` | `FieldError(Field)` | present |
| `The <Table> does not exist. Identification fields and values: <key>` | a discarded `Get`/`Find` | **absent** |
| `The <Table> already exists. Identification fields and values: <key>` | `Insert` over a duplicate key | **absent** |

## The codes are a closed set, and the whole of Layers/W1 uses seven

`system-getlasterrorcode-method.md`: "the type of the last error"; it is NOT translated, which is
why a test may compare it. Counted over `Layers/W1`:

| code | sites | raised when |
|---|---|---|
| `DB:ClientInsertDenied` | 28 | a permission refusal on insert |
| `DB:RecordNotFound` | 7 | a read that found nothing |
| `DB:NothingInsideFilter` | 6 | a find over a filter that matches no row |
| `DB:RecordExists` | 2 | an insert over an existing key |
| `DB:ClientDeleteDenied` | 2 | a permission refusal on delete |
| `DB:PrimRecordNotFound` | 1 | the primary-key `Get` that found nothing |
| `DB:NoFilter` | 1 | a find on an unfiltered empty table |

## The choice

`Error` gains a CODE beside its text -- a `std::string_view` into `.rodata`, empty for an AL
`Error()` and set by the runtime's own diagnostics. `GetLastErrorCode()` reads it from the same
place `GetLastErrorText()` reads the text, so `asserterror` fills both or neither. It is not a
second exception type: AL has one error with two accessors, and two types here would make every
`catch` site choose.

**THE DISCARD CONTEXT IS THE OTHER HALF OF `Get`.** `Table<Derived>::Get` returns `bool` and never
raises, which is right for `if Rec.Get(...) then` and wrong for a bare `Rec.Get(...);` -- AL raises
there, with the text above and `DB:RecordNotFound`. That is CLAUDE.md's own `value context` trap and
it is what makes `Assert.AssertRecordNotFound` testable at all. The generator already knows the
context it emits into; the runtime needs the raising form beside the answering one.

## The gate

One case per row of both tables: the text verbatim, and the code beside it. The negative control is
an `Error()` raised from AL, which must leave the code EMPTY -- a runtime that stamped a code on
every error would pass a text-only check and fail this one.
