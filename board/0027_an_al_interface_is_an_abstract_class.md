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

## HOW IT IS BUILT, and the one question that looked open is answered by a property

Measured over BCApps on 2026-09-02:

| | |
|---|---|
| interfaces | 207, declaring 822 procedures |
| codeunits that `implements` | 691 |
| enums that `implements` | 115, carrying 424 `Implementation =` bindings |
| interface-typed variables | 1 769 |
| implementing codeunits with `SingleInstance = true` | **18** |

**1. The interface is an abstract class and the implementation derives from it.** Nothing is invented
here: 822 pure virtual functions, 691 inheritances, and the compiler refuses an implementation that
is missing one -- which is the trade this tree made by leaving Python.

**2. The dominant assignment is from an ENUM, not from a codeunit.** The BaseApp writes
`NoSeriesSingle := NoSeriesLine.Implementation;` -- an enum-typed FIELD assigned to an interface
variable, with the enum value carrying the binding:

    enum 397 "No. Series Implementation" implements "No. Series - Single"
    { value(0; Normal) { Implementation = "No. Series - Single" = "No. Series - Stateless Impl."; } }

So the generator emits, per (enum, interface) pair, a switch over the ordinal. The set is CLOSED at
translation time, so an ordinal with no `Implementation` clause is a translation error rather than a
null at run time, and there is no registry, no `std::function` and no type erasure anywhere in it.

**3. WHERE THE IMPLEMENTATION LIVES IS DECLARED, NOT DEDUCED.** This looked like the item's hard
part -- an enum value names a codeunit TYPE and no object exists to point at, so something has to
decide the lifetime, and sharing one instance is only safe if the implementation is stateless. That
question does not have to be asked: `devenv-singleinstance-property.md` -- "Sets whether a single
instance of the codeunit and codeunit variables are instantiated ... The default is false." AL
declares it per codeunit, 673 of the 691 implementations take the default, and 18 do not. A fresh
instance goes in the session arena, whose frame gives it the interface variable's lifetime; a
`SingleInstance` one is the session's. And `SingleInstance` is needed for `Codeunit.Run` anyway, so
this is not machinery built for interfaces.

**4. The deviation is the arrow.** AL writes `NoSeriesSingle.GetNextNo(...)` and C++ writes
`NoSeriesSingle->GetNextNo(...)`, because the variable holds a pointer to the base and 822
signatures cannot be forwarded by a wrapper. It is visible, uniform, and it reads as itself: an
interface variable IS a handle to something else, and AL's dot is the one place AL hides an
indirection that C++ shows.

**5. Dispatch is one virtual call**, which is what C/SIDE did natively.

## What is true when this closes

- `.Interface.al` is parsed and counted in the population baseline beside tables, codeunits and
  enums.
- An interface emits a header with pure virtual functions and no source.
- A codeunit that `implements` one derives from it, and a missing procedure is a COMPILER error
  rather than a runtime lookup -- which is the trade this tree made by leaving Python.
- An interface-typed variable dispatches to the codeunit assigned to it, and how it is held is
  written down in one sentence with the cost named.
