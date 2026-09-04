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

**Three of the seven cannot be raised until there is a permission system**, and none of those three
is reachable from the milestone: `DB:ClientInsertDenied` (28) and `DB:ClientDeleteDenied` (2) are
permission refusals, and the count of them inside the 80 UT codeunits is **0** (measured 2026-09-04;
66 of those codeunits set `TestPermissions = Disabled`, which runs every test as SUPER). So this
item's own delivery is the four database codes, and the two permission codes arrive with
board:0062.

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

## `Error('')` IS AN IDIOM AND NOT AN OVERSIGHT, read 2026-09-04 (board:0071)

`methods-auto/dialog/dialog-error-string-joker-method.md` states it in one sentence:

> By calling the method with an **empty** string the execution of AL code **ends without displaying
> a message**.

**1 990 call sites write `Error('')`** (measured 2026-09-04 over `~/Git/BCApps/src`), and the most
important of them is the reason Preview Posting works at all. Codeunit 19 `Gen. Jnl.-Post Preview`
ends its `OnRun` with it unconditionally:

```al
Preview(PreviewSubscriber, PreviewRecord);
LastErrorText := GetLastErrorText();
if not IsSuccess() then
    ErrorMessageMgt.LogError(PreviewRecord, LastErrorText, '');
Error('');
```

The whole posting RUNS, its entries are captured by the preview event handler into TEMPORARY tables,
and then `Error('')` rolls the database back and returns to the caller with nothing written and
nothing displayed. `ui-how-preview-post-results.md` is that feature seen from the user's side: "you
can choose the **Preview Posting** button to review the different types of entries that will be
created when you post."

Three obligations follow, and each fails silently if it is missed:

- **An empty error text is still an ERROR.** A runtime that treats `Error('')` as "no message, so
  nothing happened" turns Preview Posting into a posting -- which is the worst failure this tree can
  produce, because the user asked to see what WOULD happen and the ledger changed.
- **It must not be rendered.** No dialog, no message, no line in a log that a test then compares.
  board:0054's `MessageHandler` must not be called for it either.
- **Temporary rows survive the rollback**, because they never reached the database. That is what the
  preview captures, and it constrains board:0047: a `Temporary<T>` backed by a real table -- even a
  session-scoped one -- makes this feature impossible.

`Error('')` also raises the question this item exists for from the other end: an error with no text
still carries a CODE and a call stack, which `GetLastErrorCode` and `GetLastErrorCallStack` read.
The wording is what is empty, not the error.

**The gate is Preview Posting's own shape** and it is buildable before any posting exists: a
procedure that inserts a row, then raises `Error('')`, inside a boundary -- the row must be gone, the
caller must continue, and nothing must have been displayed. **The negative control is the insert**:
remove the rollback and the case must go red, because a gate that only checks "no message appeared"
passes over a runtime that never rolled anything back.

## `FieldError` ON A ZERO NUMERIC SAYS THE WRONG SENTENCE

`devenv-calcfields-calcsums-fielderror-fieldname-init-testfield-and-validate-methods.md` (read
2026-09-04, board:0071) gives `FieldError`'s four message forms with worked examples:

| the field | the message |
|---|---|
| a Code field holding a wrong value | `Class must not be OTHER in Item No.='70000'.` |
| **a text or code field holding the empty string** | `You must specify Class in Item No.='70000'.` |
| **a NUMERIC field that is empty** -- "it's treated as if it contains the value 0 (zero)" | **`Class must not be 0 in Item No.='70000'.`** |
| any field, with a custom text | `Class must be greater than 4711 in Item No.='70000'.` |

**`src/rt/Record.cpp:110` gets three of the four right and the third wrong.** It branches on
`IsBlank(record, *def)`, and `IsBlank` (`src/rt/Record.cpp:78`) answers TRUE for an `Integer` of 0,
a `Decimal` of zero, an `Option` at ordinal 0 and a `Boolean` that is false. So
`Rec.FieldError(Quantity)` on a zero quantity produces

> You must specify Quantity in Sales Line Document Type='Order',...

where BC produces

> Quantity must not be 0 in Sales Line Document Type='Order',...

**`Assert.ExpectedError` matches a SUBSTRING of the text**, so this is not cosmetic: 21 test
procedures in the read roots assert on `must not be` and 7 on `You must specify` (measured
2026-09-04), against **5 297 `FieldError` call sites**.

**The fix is not in `IsBlank`**, which is right where it is also used: `TestField` must fail on a
zero numeric, and its own wording ("It cannot be zero or empty") says so. The fix is that
`FieldError` branches on the field's TYPE rather than on blankness -- `Code` and `Text` take the
"You must specify" form, everything else takes "must not be " plus the field's rendered value, which
for a zero Integer is `0` and is what `FieldText` already produces.

**What the page does NOT settle, and it is said rather than guessed**: a blank `Date`, `Option`,
`Guid` or `DateFormula` is neither a text nor a number, and no form is documented for them. The
BaseApp's own `FieldError` calls on those fields are the evidence to look at when this is worked;
until then the current behaviour is preserved for them and the change is confined to the numeric
types the page names.

**Gate**: the page's own three examples, verbatim, on a Code field with a value, a Code field that is
empty, and an Integer field that is zero. **The negative control is the third** -- it passes today
with the wrong sentence, and a gate that only covers the first two is green over the defect.

Classification: **silent-wrong-data** in the sense that matters here: the run does not fail, it
fails DIFFERENTLY, and a test comparing the text is the only thing that notices.

## THE SAME PAGE FIXES THREE MORE BEHAVIOURS

- **`Init` does not initialise the PRIMARY KEY.** "If a default value for a field has been defined by
  using the **InitValue** property, this value is used for the initialization. Otherwise, the default
  value of each data type is used. ... **Init doesn't initialize the fields of the primary key.**"
  Two rules, both `constexpr`-decidable: `InitValue` is a field property (board:0067) and the primary
  key is `keys[0]`.
- **A failing `TestField` DISCARDS the record's changes**: "an error message is displayed and a
  run-time error is triggered. This means that **any changes that were made to the record are
  discarded**." So it is a boundary and not only a throw -- the same shape `asserterror` already has
  in `include/runtime/Error.h`.
- **`TestField(Field)` and `TestField(Field, Value)` produce DIFFERENT DIALOGS.** The one-parameter
  form shows a **Show [Record]** button that navigates to "a Card-type page whose `SourceTable`
  matches the record's table" -- resolved by the PLATFORM, not by `ErrorInfo`; the two-parameter form
  shows only OK. That table-to-Card-page lookup is decidable at translation time and is the same
  index board:0083 builds, so it costs one more column rather than a search.

## A LABEL CARRIES `Comment`, `Locked` AND `MaxLength`, AND ONLY THE THIRD IS ENFORCEABLE HERE

`devenv-using-labels.md` (read 2026-09-04, board:0071): a label is a text constant plus three
optional, comma-separated, order-independent parameters:

| parameter | |
|---|---|
| `Comment` | notes about the placeholders -- translator guidance, no runtime effect |
| `Locked` | `true` means "do not translate"; the default is `false` |
| **`MaxLength`** | "Determines how long the label can be. **If no maximum length is specified, the string can be any length.**" |

**`MaxLength` is a `static_assert`** (board:0081): the label's own text is a translation-time
constant and so is the bound, so a label longer than its declared maximum is a build error rather
than a truncation nobody sees. `Locked` and `Comment` are metadata the generator carries for the
translation files and does not act on.

A label appears in four places: as the value of a PROPERTY (`Caption`, `ToolTip`, `OptionCaption`,
`AdditionalSearchTerms`, `InstructionalText`, `PromotedActionCategories`, `RequestFilterHeading`), as
a `Label` VARIABLE, as a REPORT label, and as a PAGE label. All four are declarations, all four end
up in `.rodata`, and the first list is seven of board:0067's properties whose values are not plain
strings but this small grammar.
