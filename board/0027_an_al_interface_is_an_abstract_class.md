Type: arc
State: open
Area: al, gen, rt

# An AL interface is an abstract class

The transpiler does not read `.Interface.al` at all. Measured over BCApps on 2026-09-01:

| | |
|---|---|
| interface objects | 207 |
| procedures they declare | 822 |
| codeunits declaring `implements` | 699 |
| interface-typed declarations | 1 945 |
| generated files blocked by the unknown type name `Interface` | 53 |

## What the references say

`devenv-interfaces-in-al.md`: an interface declares procedure SIGNATURES and no bodies; a codeunit
`implements` one or more and must supply every one; an interface-typed variable is assigned a
codeunit and dispatches to that codeunit's implementation. An enum value may carry
`Implementation = "Interface" = Codeunit`, which is how the BaseApp picks an implementation from
data -- the parser already reads the enum's interface list (28 of 576 BaseApp enums carry one).

An interface has no state and no instance: it is a pure signature set. That maps onto C++ exactly --
an abstract class with pure virtual functions, and `implements` as inheritance -- and this is one of
the few places where AL and C++ agree without a deviation.

## The one thing that is NOT decided by that

**How an interface-typed VARIABLE is held.** AL writes `NoSeriesSingle: Interface "No. Series -
Single"` and then assigns a codeunit to it; the variable is a value with dynamic dispatch behind it.
C++ cannot store a polymorphic value in an automatic variable without either a pointer or a copy of
a known type. The candidates, none free:

| way | costs |
|---|---|
| `IFace &` | needs the codeunit to outlive the variable; AL's assignment from an enum's `Implementation` produces no such object |
| `IFace *` | the same lifetime question, and a null an AL reader has no name for |
| `std::unique_ptr<IFace>` | one allocation per assignment, and the session arena is what this tree allocates from |
| an arena handle | fits the arena, and is a name AL does not have |

`openerp` had no problem to solve here -- Python dispatches on the object -- so it is a hint about
nothing at all. This is the first place in this tree where a C++ answer has to be INVENTED rather
than derived, which is why it gets an item instead of a commit.

## What this collides with

**A codeunit has no virtual functions today** and that is deliberate for TABLES -- `offsetof` over
the field table needs standard layout. It is NOT a constraint on codeunits, which carry no field
table. So `implements` may add a vtable to the 699 codeunits that declare it, and to no others.

## What is true when this closes

- `.Interface.al` is parsed and counted in the population baseline beside tables, codeunits and
  enums.
- An interface emits a header with pure virtual functions and no source.
- A codeunit that `implements` one derives from it, and a missing procedure is a COMPILER error
  rather than a runtime lookup -- which is the trade this tree made by leaving Python.
- An interface-typed variable dispatches to the codeunit assigned to it, and how it is held is
  written down in one sentence with the cost named.
