Type:     task
Status:   open
Parent:   0033
Area:     al, gen
Source:   developer/devenv-namespaces-overview.md, developer/devenv-namespaces-structure.md, developer/devenv-using-access-modifiers.md, developer/devenv-compilation-scope-overview.md
Verdict:  teilweise
Class:    silent-wrong-data

# A namespace scopes a name, and `Access` is compile-time only

**Four pages, one item**: namespaces, their adoption, the access modifiers and the compilation scope.
They are one subject -- what a name means and who may see it -- and board:0359 filed `Access` from the
property page and left the `Internal` half to board:0033.

## The two compiler rules that make a namespace matter

> **"You can only have ONE OBJECT OF A KIND with the same name IN A MODULE."**
> **"You can only have ONE OBJECT OF A KIND with the same name IN A NAMESPACE."**
>
> "An AL file declares a namespace **at the beginning of the file**, and **all objects in the code file
> belong to that namespace. A given object can only belong to ONE namespace**, but the same namespace
> can be used for **multiple AL files ... and for MULTIPLE MODULES.**"
>
> The `using` directive goes **"after the namespace declaration and before any object declarations. THE
> ORDER OF THE `using` DIRECTIVES DOESN'T MATTER."**

**A namespace may span modules**, so it is not the app boundary -- board:0033's app is the module and
the namespace cuts across it. Two independent scoping mechanisms, and CLAUDE.md's `scope.json` is "a
whitelist over AL NAMESPACES", which makes the namespace the transpiler's own selection key.

**And a name is unique per KIND per namespace** -- so two tables may not share a name in one namespace,
but a table and a page may. That is exactly C++'s problem in board:0026 ("a generated name never
collides with its class"), and the AL rule is weaker than C++'s: **AL allows a table and a codeunit
called `Foo` in one namespace; C++ does not allow two classes called `Foo` in one C++ namespace.**

**So the generator's name mangling must carry the KIND**, and board:0026 is where that lives. This
page is the reason it is required rather than tidy.

## `Access` is not a security boundary, and the documentation says so twice

> **"In AL, access modifiers are primarily intended for designing APIs and CANNOT BE USED AS A SECURITY
> BOUNDARY."**
>
> **"Access modifiers are ONLY TAKEN INTO CONSIDERATION AT COMPILE TIME.** For example, at compile
> time, a table with `Access = Internal` can't be used from other modules ... **but AT RUNTIME, ANY
> MODULE CAN ACCESS THE TABLE by using reflection-based mechanisms such as `RecordRef`, or
> `TransferFields`. And the `OnRun` trigger CAN BE RUN on `internal` codeunits by using
> `Codeunit.Run`.**"

**Three named escape hatches**: `RecordRef`, `TransferFields`, `Codeunit.Run`. So `Access = Internal`
is advisory at run time in BC.

**In C++ it would be absolute** -- a `private` member is not reachable by any means -- **and that is a
divergence in the strict direction that would BREAK code.** board:0359 measured `Access` at **3 738**
declarations, **2 532 of them `Internal`**, and the BaseApp reaches internal objects through
`RecordRef` by design.

**So `Internal` must NOT become C++ `private` or an unexported symbol.** It is a compile-time check the
GENERATOR performs against AL's rules, and the emitted C++ leaves the object reachable -- which is the
opposite of the instinct and is what this page settles. board:0359 left the question open; this is the
answer, and it is recorded as such.

> Setting `internal` is linked to the `internalsVisibleTo` setting in `app.json`.

**A per-app friend list**, which the generator reads and which makes the check pairwise rather than
absolute.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0359: `Access =` **3 738** -- `Internal` **2 532**, `Public` **1 206**.

**`namespace` and `using` are declarations, not `Name = Value` properties, so this sweep's pattern does
not reach them** -- stated rather than guessed. CLAUDE.md records that `scope.json` is a whitelist over
namespaces, so the count of distinct namespaces in scope is already a known quantity to the
transpiler.

## The IST-state, and it is why this is `teilweise`

CLAUDE.md: `scope.json` selects by namespace, so **the transpiler already reads namespace
declarations** -- that half works. board:0359 records that `Access` is among the properties the
generator does not consume. board:0026 owns the name-collision mechanism and
`src/gen/TableWriter.cpp:96` implements it for fields.

**Whether the generator maps an AL namespace onto a C++ namespace, and whether the kind is part of the
mangled name, is this item's first check** and is not measured here.

## The choice

An AL namespace becomes a C++ namespace, and the OBJECT KIND is part of the generated class name --
because AL's uniqueness rule is per kind and C++'s is not.

**`Access` is checked by the GENERATOR and does not change the emitted C++ accessibility**, with
`internalsVisibleTo` read from `app.json` as the friend list. `Local` and `Protected` on a FIELD do
become `private` and `protected` (board:0359), because those are not reachable by AL's three escape
hatches either.

## Ordering

With board:0026's naming and board:0033's app boundary. Ahead of board:0359's field half, which is a
different decision on the same property.

## Gate, and its negative control

Two objects of different kinds with the same name in one namespace both transpile and do not collide;
a table with `Access = Internal` is still reachable through `RecordRef` from another module.

**The negative control is the `RecordRef` reach** -- an implementation that makes `Internal` a C++
access restriction is stricter than BC and breaks 2 532 declarations' worth of BaseApp code that
reaches internal objects deliberately. Every compile-time gate passes; the failure is at link or run
time in the transpiled BaseApp.
