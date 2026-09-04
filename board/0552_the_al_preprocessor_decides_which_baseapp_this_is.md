Type:     task
Status:   open
Parent:   0069
Area:     al, gen
Source:   developer/devenv-obsolete-objects.md, developer/devenv-deprecation-guidelines.md, developer/devenv-deprecation-timeline.md, developer/devenv-deprecating-with-statements-overview.md, developer/devenv-app-discontinue.md
Verdict:  teilweise
Class:    silent-wrong-data

# The AL preprocessor decides which BaseApp this is, and 9 946 blocks depend on it

**Five pages, one item**: how objects and symbols are obsoleted, Microsoft's own deprecation practice,
the timeline, the `with`-statement deprecation and app discontinuation. board:0069 owns the obsolete
state and board:0355/0356 the reason and tag; **this is the mechanism that surrounds them**, and it is
a language feature nobody has filed.

## Microsoft's deprecation practice IS the preprocessor

> "When we obsolete code, we **add the preprocessor statements `#if`, `#else`, and `#endif` surrounding
> the code to be obsoleted.**"
>
> "Use one of the following **preprocessor symbols, where the pattern is `CLEAN<Version>`**, such as
> `CLEAN15`, `CLEAN16`, `CLEAN17`, `CLEAN18`."
>
> **"NOTE: These symbols AREN'T SHIPPED WITH THE PRODUCT."**
>
> "For tables and fields, we also use **`CLEANSCHEMA`** in a second phase **to delete the SQL
> schema.**"
>
> "The version to use in the symbol **matches the `<major>` of the `ObsoleteTag`.**"
>
> ```al
> #if not CLEAN18
>     [Obsolete('Replaced by SetParameters().', '18.0')]
>     procedure SetParams(...) ...
> #endif
> ```

**"These symbols aren't shipped with the product" is the load-bearing sentence.** Every symbol is
UNDEFINED, so `#if not CLEAN28` is TRUE and the obsolete code is INCLUDED. Defining `CLEAN28` is how
Microsoft builds the version that drops it.

**So the preprocessor decides which BaseApp agiru transpiles**, and the default -- all symbols
undefined -- is the one that keeps every obsolete object.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

Directives, counted over `.al` files:

| directive | count |
|---|---:|
| `#pragma` | **27 148** |
| `#endif` | **10 125** |
| `#if` | **9 946** |
| `#else` | 990 |
| `#region` | 807 |
| `#endregion` | 806 |

and the conditions, most frequent first: **`#if not CLEAN28` 3 235**, `#if not CLEAN27` 3 201,
`#if not CLEAN29` 1 637, `#if not CLEANSCHEMA25` 268, `#if CLEAN27` 197, `#if not CLEANSCHEMA26` 193.

**9 946 conditional blocks and 27 148 `#pragma`s.** That is not a corner of the language -- it is
present in a large fraction of the BaseApp, and **179 more `#endif` than `#if`** is worth noting: the
difference is exactly the count of `#endif` closing an `#ifdef`-like form or a measurement artefact of
line-leading whitespace, and it is **stated rather than reconciled** here.

**Both polarities occur**: `#if not CLEAN27` (3 201) keeps code until 27 is defined, `#if CLEAN27`
(197) keeps code only once it is. So a symbol is not merely "drop this" -- it switches between two
implementations.

## The IST-state, and it is why this is `teilweise`

**`src/al/Lexer.cpp` already implements the preprocessor**, and better than expected:

- `src/al/Lexer.cpp:249` -- `if (c == '#') { return LexDirective(start); }`
- `src/al/Lexer.cpp:342` -- `ApplyDirectives` keeps a stack of `bool` and drops tokens inside a false
  block, handling `if`, `else` and `endif`
- `src/al/Lexer.cpp:263` -- `ConditionReader` evaluates `or`, `and`, `not`, parentheses and `true`,
  with a depth limit of 64 and `LexError("a #if condition nests too deeply")`
- `src/al/Lexer.cpp:305` -- an unknown word evaluates to **`false`**, which is exactly "not shipped with
  the product"

**So `#if not CLEAN28` is true, the obsolete code is kept, and that matches BC's default.** The
mechanism is right and the item is `teilweise` for what is around it:

- **`#pragma` at 27 148 is not handled by `ApplyDirectives`** -- it falls through the `if`/`else`/`endif`
  chain and is `continue`d, so it is DROPPED silently. **Measured 2026-09-04 over the same tree:
  `#pragma warning` 27 136 and `#pragma implicitwith` 14.** The warning pragmas are a compiler concern
  and dropping them is correct. **The fourteen `implicitwith` are not**, and they are the finding below.
- **`#region` and `#endregion`** are folding markers and correctly dropped.
- **There is no way to DEFINE a symbol**, so `CLEANSCHEMA` and the 197 `#if CLEAN27` blocks can never
  be selected. That is correct today and is a knob the transpiler will need when it targets a version.

## `#pragma implicitwith` is the one that changes meaning

board:0086 is "an implicit `with` opens the record and the lookup has an order", and AL's
`#pragma implicitwith` turns that off per file.

**Measured 2026-09-04: 14 pragmas in 12 FILES -- 10 `disable`, 4 `restore`.** So six of the twelve
disable to the end of the file and never restore, and the pragma is a REGION marker rather than a file
switch: a `disable`/`restore` pair scopes it to the lines between them.

Every one of the twelve is a PAGE, and none is in the Base Application:

| tree | files |
|---|---|
| `Apps/CZ/CoreLocalizationPack` | 8 |
| `Apps/NA/EnvestnetYodleeBankFeeds` | 2 |
| `Apps/NO/ImportNOPayroll` | 1 |
| `Apps/W1/APIReportsFinance` | 1 |

`src/al/Lexer.cpp:342` drops all fourteen, so those twelve files are resolved like every other one.

**What that costs is smaller than it looks, and the honest reading is worth more than the alarming
one.** `disable` turns the implicit `with` OFF, so code written under it is already fully qualified.
Dropping the pragma RE-ENABLES a fallback that file's code does not use -- which is harmless as long as
the resolver reaches the implicit `with` only after every other scope has failed. **It is a defect
exactly when the resolution ORDER puts `Rec` ahead of something else**, which is board:0086's subject
and the reason that item exists. So this is not "fourteen wrong values"; it is fourteen places where
board:0086's order is the only thing standing between correct and silent, with no pragma to fall back
on.

**And the scope question is open**: all twelve sit in localization and API apps, and whether
`scope.json` admits them is a fact about `apps.json` rather than about the lexer. If it does not, the
population inside the transpiled tree is 0 -- which would make this a correctness requirement without
a call site, and that is recorded rather than assumed either way.

## The choice

`ApplyDirectives` gains a symbol table (empty by default, settable by the transpiler) and a `#pragma`
handler that dispatches by pragma name rather than dropping. Unknown pragmas are counted, not silently
ignored -- CLAUDE.md: accepting a declaration and doing nothing with it is worse than refusing it.

**The `CLEAN<n>` symbols stay undefined**, which is BC's own default and keeps every obsolete object --
board:0069's requirement that a `Removed` field keeps its column.

## Ordering

The `#pragma` census is done: 27 136 `warning`, 14 `implicitwith` in 12 files, all pages, none in the
Base Application. **So `implicitwith` is NOT first.** The `#if` chain is what decides which BaseApp
gets translated -- 9 946 of them, `#if not CLEAN28` alone 3 235 -- and that is already implemented and
needs only the symbol table made explicit. `implicitwith` follows, and it follows board:0086 rather
than leading it: without the resolution order settled there is nothing for the pragma to switch off.

## Gate, and its negative control

A file with `#if not CLEAN28 ... #endif` keeps the guarded code; with `CLEAN28` defined it drops it;
an unrecognised `#pragma` is counted rather than discarded.

**The negative control is one of the ten `#pragma implicitwith disable` sites** -- a bare name inside
its scope must resolve differently from the same name outside it, and an implementation that drops all
pragmas gives both the same answer. The control goes red only if board:0086's resolution order is
implemented first; until then it is green for the wrong reason, and that is why the ordering above puts
it second.
