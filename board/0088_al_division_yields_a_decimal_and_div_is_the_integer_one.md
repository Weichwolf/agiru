Type: root
State: open
Area: gen

# AL's `/` yields a DECIMAL, `DIV` is the integer one, and agiru emits both as C++ `/`

`src/gen/BodyWriter.cpp:43` maps the AL operators onto C++ ones, and two rows of that table are
wrong in opposite directions:

```cpp
Operator{.al = "/",   .cpp = "/", .precedence = 6},
Operator{.al = "div", .cpp = "/", .precedence = 6},
Operator{.al = "mod", .cpp = "%", .precedence = 6},
```

## What the platform documents

`devenv-al-operators.md` lists the arithmetic operators and distinguishes them by name:

| AL | meaning |
|---|---|
| `/` | **Division** |
| `div` | **Integer division** |
| `mod` | Modulus |

and `devenv-al-arithmetic-operators.md` gives the operand table: `/`, `div` and `mod` are all
defined for **Byte, Char, Option, Integer AND Decimal**. So `div` is not "the integer types' `/`" --
it is a DIFFERENT OPERATION that both numeric groups support.

**The AL source settles what `/` returns, and it does it twice over.** Measured 2026-09-04 over
`~/Git/BCApps/src`:

- `Round(Counter / CounterTotal * 100, 1)` -- `GenerateFileFEC.Codeunit.al:170`,
  `CreateCustomerContracts.Report.al:27`, `SubBillingProgressTracker.Codeunit.al:88` and 7 928 more
  `Round(... / ...)` sites. Both operands are Integer counters. **If `/` were integer division, this
  expression would be 0 for every `Counter < CounterTotal`** and every progress bar in the BaseApp
  would sit at zero until the last record.
- `FilesPerZip := TempNameValueBuffer.Count() DIV 10` -- `SAFTExportMgt.Codeunit.al:661`. **`DIV`
  exists precisely because `/` is not it**, and a developer who wanted truncation wrote `DIV`.

That is the finding stated as a rule: **`/` is a decimal divide whatever its operands are, and `DIV`
truncates whatever its operands are.**

## What agiru produces

| AL | agiru emits | on Integer operands | on Decimal operands |
|---|---|---|---|
| `A / B` | `A / B` | **C++ truncates** -- `7 / 2` is 3 where AL gives 3.5 | correct |
| `A DIV B` | `A / B` | correct | **the full quotient** -- `5.5 DIV 2` is 2.75 where AL gives 2 |
| `A MOD B` | `A % B` | correct | **fails to compile** -- `%` is not defined for a class |

**The first row is silent-wrong-data with the largest blast radius in the tree.** `7 926`
assignments contain a `/`, and every one of them whose operands are integral produces a truncated
value that then flows into an amount, a percentage or a quantity. Nothing throws.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`, all `.al`

| | |
|---|---:|
| `:= ... / ...;` assignments | **7 926** |
| `Round(... / ...)` | **7 931** |
| ` DIV ` | 567 |
| ` MOD ` | 677 |

## The choice

**The operator table stops being a string map and starts naming FUNCTIONS**, because two of the
three cases cannot be spelled with a C++ operator that means the same thing.

- `/` emits `agiru::Divide(a, b)`, whose overloads return `Decimal` for every numeric pair. The
  integral overloads convert first, so `7 / 2` is `Decimal(3.5)` -- and `agiru::Decimal` already has
  the division board:0008 is about, "filling up to 28 decimal places and normalising"
  (`include/type/Decimal.h:111`), which is the CLR rule this needs.
- `div` emits `agiru::IntegerDivide(a, b)`, which truncates toward zero and returns the operands'
  own integral type -- and for a `Decimal` pair returns the truncated `Decimal`, which is what the
  operand table says `div` does there.
- `mod` emits `agiru::Modulus(a, b)`, so the Decimal case exists at all rather than failing to
  compile.
- **The names are not AL's**, and that is deliberate: AL has no name for these, `Divide` reads as
  itself, and CLAUDE.md's rule is that where idiomatic C++ cannot produce the AL shape "the
  deviation is VISIBLE and uniform rather than clever". A macro spelling `DIV` would be the clever
  version and worse.

**What it costs**: every arithmetic expression in the generated tree becomes a call rather than an
operator, which is why only the three that are WRONG change. `+`, `-` and `*` stay operators,
because C++ means by them what AL does -- with the overflow question left to board:0073, where it
already is.

**The ordering**: this comes before anything that computes an amount, which is most of the tree. It
is also cheap -- one table, three functions -- and it is a prerequisite for board:0072's `Round`,
because `Round(a / b, 1)` cannot be right while `a / b` is not.

## Gate, and its negative control

`7 / 2 = 3.5`, `7 DIV 2 = 3`, `7 MOD 2 = 1`, `5.5 DIV 2 = 2`, `5.5 MOD 2 = 1.5`, and the BaseApp's
own shape `Round(1 / 4 * 100, 1) = 25`. Each comes from the operator table or from a call site named
above, so none can be back-filled from the output.

**The negative control is `7 / 2`**: it must NOT be 3. A gate written with operands that divide
evenly passes today and proves nothing, which is how a defect this size stays invisible -- every
test that divides an amount by a quantity that happens to divide is green.

Classification: **silent-wrong-data**, and the `DIV`-on-Decimal row is the same. The `MOD`-on-Decimal
row is a compile error, so it is **activation** and nothing regresses.
