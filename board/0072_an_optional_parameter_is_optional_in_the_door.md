Type: root
State: open
Area: net, rt, build

# Every parameter the documentation marks optional is optional in the door

AL calls a method with fewer arguments than its longest form constantly -- `AddText(String)`,
`GetSubText(Var, 3)`, `Message(Text)`, `Insert()`. The documentation marks each such parameter
`*[Optional]*` and brackets it in the syntax block. **The door's CLASS half declares no default
argument at all**, so every one of those call forms fails to compile.

## Measured 2026-09-04

| | |
|---|---:|
| documented signature pages | 1 741 |
| of them carrying at least one `*[Optional]*` parameter | **864 -- 49.6 %** |
| syntax blocks with a bracketed argument | 923 |
| default arguments in `include/type/*.h` (111 headers) | **0** |
| default arguments in `include/Builtins.h` | 47 |

The split is the finding: the FREE-FUNCTION half of the door has optional tails and the TYPE half
has none. `scripts/door.py` emits a class member per documented signature and never a default;
`scripts/gen_builtins.py` does emit them.

`BigText.AddText(String: Text [, Position: Integer])` becomes
`void AddText(std::string_view String, ::agiru::Integer Position);` -- and the AL the BaseApp
actually writes, `MyBigText.AddText('ABCDEFG')`, does not compile.

## What the references say

Each page states the omitted parameter's behaviour, and it is never "pass a zero":

| page | what omitting it means |
|---|---|
| `bigtext-addtext-string-integer-method.md` | the string is added AT THE END |
| `bigtext-getsubtext-text-integer-integer-method.md` | the sub text runs to the END of the variable |
| `record-changecompany-method.md` | change back to the CURRENT company |
| `boolean-totext-boolean-method.md` | `Format(value, 0, 0)` rather than `Format(value, 0, 9)` |

So a default is a documented VALUE, not an absence, and writing `AddText(s, 0)` at the call site
would be wrong twice: `Position < 1` is a documented run-time error.

**AND ONE DEFAULT IS NOT A VALUE AT ALL: IT IS A CALL INTO THE APPLICATION.**
`system-round-method.md` on the omitted `Precision`:

> The method `ReadRounding` in Codeunit 45 is called ... By default, the `ReadRounding` method
> returns the **Amount Rounding Precision** field from the `GLSetup` table. If you have customized
> Codeunit 45 and it does not implement `ReadRounding`, then the precision is specified as 2 digits
> after the decimal.

`Round(Amount)` is the most-called shape of the most-called arithmetic builtin in an ERP, and its
default comes from a company's setup row through a BaseApp codeunit. `include/type/Decimal.h:195`
already notices this and declines to default the parameter, which is right: **the runtime may not
name `GLSetup`**, ever (CLAUDE.md's third invariant).

So this one resolves the way board:0025 resolved the table catalogue: the DOOR installs a function
pointer at start-up, the runtime calls through it, and a tree with no BaseApp linked falls back to
the documented two decimals. It is the only default in the 864 that cannot be read off its own page,
and it is worth building the mechanism for one rather than hard-coding `0.01` and being wrong for
every company that set something else.

**board:0046 is the same defect seen from the generator's side** -- "the missing optional tails stay
missing -- `Message(String)` alone is one error in `Currency`". That item is about the door generator
not reproducing the committed header; this one is about what the header owes, and it is 864 pages
wide rather than one function.

## The choice

- **The door declares the optional parameter with its DOCUMENTED default**, not with a placeholder.
  Where the default is a value (`Format(value, 0, 0)`), it is that value; where omitting changes the
  BEHAVIOUR (append rather than insert), the door declares the short overload as its own signature
  and the two share one body.
- **`scripts/door.py` learns what `gen_builtins.py` already knows.** The optionality is in the
  documentation's own syntax block, so it is read from there rather than decided per method.
- **A default that cannot be read from the page is a translation ERROR**, named, rather than a
  guessed `{}`. A wrong default is silent-wrong-data in 864 places.

## Gate

The short form of a documented signature compiles and does what the page says it does, for one
method of each shape: an appended `AddText`, a `GetSubText` to the end, a `ChangeCompany` back to the
current company, a `ToText` without `Invariant`.

**Negative control**: remove one default and require the short call to fail to COMPILE -- which is
today's state, and is why the gate has to be a compiled call rather than a runtime assertion.
