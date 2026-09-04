Type:     task
Status:   open
Parent:   0027
Area:     al, gen, rt
Source:   developer/devenv-interfaces-in-al.md, developer/devenv-interfaces-in-al-extend.md, developer/devenv-interfaces-in-al-operators.md
Verdict:  teilweise
Class:    activation

# An interface extends others, and `is` and `as` test and cast it

**Three pages, one item**: interfaces, their extension, and the two operators that exist because of the
extension. board:0027 is "an AL interface is an abstract class" and these three are what it has to
carry.

## The contract, and what the compiler checks

> "an interface ... is a **syntactical contract** ... **The interface itself DOESN'T CONTAIN ANY CODE,
> only signatures, and CAN'T ITSELF BE CALLED from code.**"
>
> **"The AL compiler CHECKS to ensure that implementations adhere to assigned interfaces."**
>
> "You can declare **variables as a given interface** to allow passing objects that implement it, and
> then call implementations **in a polymorphic manner.**"
>
> **"Interfaces can only contain PROCEDURE DECLARATIONS"** -- diagnostics AL0584, AL0585, AL0612.

**A pure abstract class with no data and no bodies**, which is what board:0027 says. The compiler's
adherence check is C++'s override check, so it comes free -- **provided the generated implementation
uses `override`**, which is the thing to verify.

**Four analyzer rules are named and each is a translation-time check** this tree gets for free from
C++: only procedures (AL0584/0585/0612), no naming conflicts with built-ins (AL0616), no duplication
across multiple interfaces (AL0587/AL0675), **no circular references (AL0852)** -- which C++ also
rejects, but with a worse message.

## Extension is multiple inheritance of pure interfaces

> ```AL
> interface IFooBar extends IFoo, IBar { procedure FooBar(); }
> codeunit 10 TheImplementor implements IFooBar   // must implement IFoo, IBar, IFooBar
> ```
>
> **"The new interface INHERITS ALL THE METHODS from the interfaces it extends"**, and an implementor
> "must provide implementations for ALL the methods defined in the extended interfaces as well."
>
> "`TheImplementor` can be used as **both `IFoo`, `IBar`, AND `IFooBar`**."

**Multiple inheritance of abstract bases, which C++ has directly** -- and the diamond it invites is
what AL0587 and AL0675 refuse: "when implementing multiple interfaces avoid duplication". So AL
forbids the ambiguity C++ would resolve with virtual inheritance, which means **non-virtual multiple
inheritance is the right shape** and the diamond is a translation error before it is a C++ problem.

## `is` and `as` are runtime type tests, and that is the hard part

> ```AL
> if intf is IBar then ...        // on an Interface variable
> if v is IBar then ...           // on a VARIANT
> exit(intf as IBar);             // "Throws an error if 'intf' doesn't implement 'IBar'"
> exit(v as IBar);                // on a Variant
> ```

**Four forms: `is`/`as` over an interface variable and over a `Variant`.**

**`intf is IBar` is `dynamic_cast` and it needs RTTI**, which this tree has not had to decide about.
CLAUDE.md's reason for leaving Python is that a compiler can check what a test run otherwise finds --
and here AL deliberately defers a check to run time. So RTTI is required, or an equivalent: a
`constexpr` per-implementation table of which interfaces it satisfies, checked by id. **The table is
the better answer**, because every implementation and every interface is known at translation time, and
it avoids enabling RTTI across 7 885 translation units for four operator forms.

**The `Variant` forms are board:0037's and board:0075's**: a `Variant` holding a codeunit must answer
"do you implement `IBar`", so the variant's payload carries the same interface-id set.

**`as` throws on failure** -- board:0055's wording, and it is the one place in this item with a runtime
error.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0467: `Implementation` **571** on enum values, `DefaultImplementation` **40**,
`UnknownValueImplementation` **9**. board:0468: `AssignmentCompatibility` **573** over 1 436 enums.

**`implements`, `extends` on an interface, and the `is`/`as` operators are SYNTAX, not property
declarations**, so this sweep's pattern does not count them. Their counts belong to this item and are
its first task -- **stated rather than guessed**, and `is`/`as` at zero would make the interface-id
table unnecessary.

## The IST-state, and it is why this is `teilweise`

`src/gen/CodeunitWriter.cpp:143`, `:463`, `:499`, `:532` -- the generator looks up
`objects.interfaces` by lowered name in four places, and `src/gen/EnumWriter.cpp:59` and `:83` do the
same for an enum value's implementation. **So interfaces are parsed, indexed and resolved.**

board:0027 records the three-level fallback. **Whether `extends` on an interface is parsed, and whether
`is`/`as` are, is this item's first check** and is not measured here.

## The choice

An interface becomes a pure abstract class; `implements` becomes public inheritance; `extends` becomes
multiple inheritance of the extended interfaces, **non-virtual**, with the duplication cases refused at
translation time as AL refuses them.

**`is` and `as` read a `constexpr` interface-id set per implementing object** rather than RTTI --
`static_assert`-checkable, zero cost when unused, and it works identically for the `Variant` forms.

**`as` on a failing cast raises** with BC's wording.

## Ordering

board:0027's core. The interface-id table is behind board:0467's three-level fallback, which needs the
same per-object metadata.

## Gate, and its negative control

A codeunit implementing `IFooBar` is usable as `IFoo`, `IBar` and `IFooBar`; `intf is IBar` is true for
it and false for an implementor of `IFoo` alone; `intf as IBar` on the latter raises.

**The negative control is the `IFoo`-only implementor** -- `is` must return FALSE, and an
implementation that answers from the static type of the variable rather than the dynamic type of the
object returns true for anything, which passes every positive assertion.
