Type: root
State: open
Area: gen

# AL does NOT short-circuit, and both operands of `and` / `or` run

`src/gen/BodyWriter.cpp:44` maps them onto C++'s short-circuiting operators:

```cpp
Operator{.al = "or",  .cpp = "||", .precedence = 1},
Operator{.al = "and", .cpp = "&&", .precedence = 2},
Operator{.al = "xor", .cpp = "!=", .precedence = 3},
```

**`&&` and `||` skip their right operand when the left decides the answer. AL does not.**

## The reference, and it is the predecessor's own refuted hypothesis

`~/Git/openerp/board/1057_evaluate-in-and-or-bedingung-schreibt-nicht-zurueck.md`, `status: done`,
**`measured: 42/53 -> 46/53 (GAINED 4, LOST 0; Kontrollen 40/40)`**:

> Der Emitter hoistete den by-ref-Rueckschreibvorgang von Evaluate nur fuer nackte Aufrufe und ein
> einzelnes `not`, nicht ueber `and`/`or` -- begruendet mit Kurzschluss-Auswertung. **WIDERLEGT: AL
> kurzschliesst nicht**, und der Emitter erzeugt `_al_and(A,B)`, wertet also beide Operanden als
> Argumente aus.

The call site it names is the shape that makes this expensive:

```al
if not Evaluate(D, V) and not Evaluate(D, V, XmlFormat) then
```

`ConfigValidateManagement.EvaluateValueToDecimal`. The SECOND `Evaluate` writes `D` through a `var`
parameter. Skip it and `D` stays 0 -> a Purchase Line Quantity of 0 -> `TestField(Quantity)` fails
while validating a Line Discount Amount -- **three objects away from the `and` that caused it**.

**This is exactly what CLAUDE.md says the predecessor's board is authoritative about**: that a
semantic has a trap in it at all, and which call site walks into it. It was filed there as a
refutation of the belief this tree's operator table currently encodes.

## What the platform documentation says, and what it does NOT

`devenv-al-boolean-operators.md` gives `not`, `and`, `or`, `xor` with their result types and nothing
about evaluation order. `devenv-al-operators.md` calls them "logical conjunction" and "logical
disjunction". **`grep -rn -i "short-circuit"` over the whole of `dev-itpro` returns nothing**
(measured 2026-09-04). So the documentation neither promises short-circuiting nor forbids it -- and
the predecessor's measurement is the evidence that decides it, which is the case CLAUDE.md's rule
covers: read the FINDING, not the fix.

`xor` is the same question one step further: `!=` on two `bool`s evaluates both, so that row is
already right, and it is the shape the other two should have.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`, all `.al`

| | |
|---|---:|
| ` and ` / ` or ` occurrences | **327 929** |
| of them with a CALL as the right operand -- ` and \| or ` then optional `not` then `Name(` | **1 870** |
| of them where that call is `Evaluate` -- the predecessor's own case | 12 |

**1 870 is the population that can differ**, and only a subset of those has a side effect. But the
side effects are the ones that matter: a `var` parameter written, a record positioned by a `Find`, a
`TryFunction` whose last error is set. Every one of them is invisible when it is skipped.

## The choice

**`and`, `or` and `xor` emit function calls, for the same reason board:0088's `/` does.**

- `a and b` emits `agiru::And(a, b)`, `a or b` emits `agiru::Or(a, b)`. Both take `Boolean` by
  value, so **C++'s argument evaluation forces both operands to run** -- which is precisely the
  mechanism the predecessor's `_al_and(A, B)` used, and it needs nothing else.
- They are `constexpr` and `[[nodiscard]]`, so the cost is nothing after inlining: the compiler sees
  `return a && b` over two already-evaluated `bool`s.
- **The order of evaluation of the two arguments is UNSPECIFIED in C++**, and that is the one thing
  this must not inherit. AL executes left to right, so the two operands are evaluated into named
  locals in order and then combined -- which the generator does anyway when an operand is a call
  whose `var` parameter has to be written back.
- `not` stays `!`, because it has one operand.

**Why not the alternative.** Hoisting the side effects out of the expression -- the predecessor's own
fix, `_hoist_in_expr` -- keeps `&&` and moves the calls above the `if`. It works, it is what a Python
emitter had to do, and here it is strictly worse: it requires the generator to KNOW which operands
have side effects, and "a call with a `var` parameter" is only the easiest kind. A function call
evaluates everything, always, and needs no analysis.

**What it costs**: `if A and B` becomes `if (agiru::And(A, B))` in 327 929 places, which is a
readability cost on generated code that no human reads, and zero at run time. The alternative costs
a correctness analysis that has to be right every time.

## Ordering

Before board:0061 (`TryFunction`) and before board:0066's `Evaluate`, because both of those are
`var`-writing calls that AL routinely puts on the right of an `and`. After nothing.

## Gate, and its negative control

An AL fragment whose right operand has an observable side effect -- the predecessor's own shape is
the case, and it costs nothing to reproduce:

```al
if not Evaluate(D, '1') and not Evaluate(D, 'x') then ...
```

`D` must hold what the FIRST `Evaluate` wrote, and a second case where the LEFT operand is false
must still run the right one and leave its mark.

**The negative control is the second case**: restore `&&` and it must go red. A gate whose right
operand is pure passes under both emissions, which is why the case is written around a `var`
parameter.

Classification: **silent-wrong-data.** The skipped operand raises nothing; the wrong value surfaces
in a different object, which is what the predecessor measured and why it cost four test methods.
