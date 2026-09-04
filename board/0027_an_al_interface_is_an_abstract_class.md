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

## AN INTERFACE CAN EXTEND ANOTHER, AND `is` / `as` TEST AND CAST IT

`devenv-interfaces-in-al-extend.md` and `devenv-interfaces-in-al-operators.md`, read 2026-09-04
(board:0071), add two things this item does not carry -- and both land on the DERIVATION this item
already chose:

- **An interface may extend another interface**, so the 207 interfaces are a hierarchy rather than a
  flat set. In C++ that is inheritance between abstract classes, which the choice above already
  gives for free.
- **`is` and `as` are operators on an interface variable AND on a `Variant`:**

  ```al
  if intf is IBar then ...          // does this implementation also satisfy IBar?
  exit(intf as IBar);               // "throws an error if 'intf' doesn't implement 'IBar'"
  ```

  `is` is `dynamic_cast<IBar *>(p) != nullptr` and `as` is the same cast with a raise on null -- so
  they need the vtable this item's `implements`-as-inheritance already puts on the 691 implementing
  codeunits, and **nothing else**. That is worth recording because it is the one place where the
  "no virtual functions" rule for TABLES could have been mistaken for a rule about codeunits: `as`
  cannot work without RTTI, and a table never needs it.
- The `Variant` form means `Variant` must be able to answer "does the value you hold implement this
  interface", which is a question `include/type/Variant.h` cannot ask today.

## What is true when this closes

- `.Interface.al` is parsed and counted in the population baseline beside tables, codeunits and
  enums.
- An interface emits a header with pure virtual functions and no source.
- A codeunit that `implements` one derives from it, and a missing procedure is a COMPILER error
  rather than a runtime lookup -- which is the trade this tree made by leaving Python.
- An interface-typed variable dispatches to the codeunit assigned to it, and how it is held is
  written down in one sentence with the cost named.

## THE ENUM-TO-INTERFACE BINDING IS A THREE-LEVEL FALLBACK, read 2026-09-04 (board:0071)

`src/gen/EnumWriter.cpp:ImplementationBodies` emits a `switch (value)` over the values that declare
`Implementation` and ends in `throw Error("this value of X names no implementation of Y")`. The
platform documents TWO more levels below that switch, and both are declarations the parser already
sees:

| the case | property | pages | agiru today |
|---|---|---|---|
| the value names its own | `Implementation` on the VALUE | `devenv-implementation-property.md` | the `switch` case |
| the value names none | **`DefaultImplementation` on the ENUM** | `devenv-defaultimplementation-property.md` | **throws** |
| the ORDINAL matches no declared value | **`UnknownValueImplementation` on the ENUM** | `devenv-unknownvalueimplementation-property.md` | **throws** |

The property page's own example is the whole rule in six lines:

```al
e := SomeEnum::Yes;  ifoo := e;  ifoo.Foo();  // => YesFooImpl,     from Implementation on the value
e := SomeEnum::No;   ifoo := e;  ifoo.Foo();  // => DefaultFooImpl, from DefaultImplementation
e := 2;              ifoo := e;  ifoo.Foo();  // => UnknownFooImpl, from UnknownValueImplementation
```

**The third line is the one that decides the shape.** `e := 2` is an ordinal no value declares --
which is ordinary once an enum is `Extensible` and an extension has been removed -- so the binding
cannot be a switch over declared values with a `throw` underneath it. It is a lookup with two
defaults, and the `default:` label is where the second one goes.

Measured 2026-09-04 over `~/Git/BCApps/src`: **40 `DefaultImplementation`** and **9
`UnknownValueImplementation`** declarations, against 629 `Implementation` bindings. Small, and every
one of them is a live path that currently raises instead of dispatching.

## AND THE DISPATCH ALLOCATES ON EVERY ASSIGNMENT

`ImplementationOf` emits `return new agiru::app::<Codeunit>{};` (`src/gen/EnumWriter.cpp:107`) and
`Implementation<I>::Forget()` deletes it. Nothing leaks -- but an interface assignment inside a loop
is one heap allocation per turn, which is the rule CLAUDE.md states as "no allocation on the hot
path", and a BC posting routine assigns an interface variable per document line.

**The shape that removes it:** a codeunit with no state is a value, and the binding can be a
`constexpr` table of `I *(*)()` factories indexed by ordinal, returning a pointer into a
`thread_local` instance per implementing codeunit -- one per session, not one per assignment. That
also makes the fallback levels above a lookup rather than a control-flow construct: the table's
entry for an unbound ordinal IS the default, filled in by the generator.

**One thing this item must settle before that is built.** `Implementation`'s copy constructor holds
NOTHING -- "A copy holds nothing of its own" (`include/runtime/Implementation.h:32`) -- so
`A := B` between two interface variables leaves `A` unbound and the next `A.Foo()` raises "this
interface variable has no implementation assigned yet". AL's own semantics for that assignment are
not stated on any page read so far, and the answer decides whether the holder can be a non-owning
pointer at all. **It is named here rather than assumed**, because a factory table makes the copy
free and the current design makes it impossible -- and picking the wrong one is a rewrite.
