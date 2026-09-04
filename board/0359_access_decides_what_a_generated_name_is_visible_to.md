Type:     task
Status:   open
Parent:   0033
Area:     gen
Source:   developer/properties/devenv-access-property.md
Verdict:  fehlt
Class:    activation

# `Access` decides what a generated name is visible to

> **Version**: runtime 4.0. Applies to: Codeunit, Query, Table, **Table field**, Enum Type,
> Interface, Permission Set.
>
> **Public** -- accessible by any other code in the same module and in **other modules that reference
> it**. **Internal** -- accessible only by code in the same module, **not from another module**.
>
> **For table fields there are two additional settings**: **Local** -- only by code in the same table
> or table extension where the field is defined. **Protected** -- only by code in the same table or
> table extensions of that table. Public is the default.

Four levels on a field, two on an object, and **C++ has all four already**: `public`, and for a field
`private` with the table's own extensions as the only other reader. AL's module is agiru's app, which
is a library (CLAUDE.md), so `Internal` is the one that does not fall out of a class -- a C++ class
member cannot be visible to a library and not to its clients.

**That gap is the item.** The candidates are: emit `Internal` names into a header the app's own
sources include and the door does not re-export; or rely on the linker, which enforces DIRECTION and
not visibility (board:0033 says so in as many words); or accept the deviation and record it. Not
decided here, because the answer depends on what `apps/<app>` looks like once one is generated, and
none is.

**And `Access` is what makes the app boundary checkable at all.** CLAUDE.md's reason for one library
per app is that "the linker enforces what AL declares -- the Base Application may not know an
extension". That check is about OBJECTS; this property is the same statement about names inside an
object, and 3 738 declarations make it the more common of the two.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`Access =`: **3 738 declarations** -- **2 532 `Internal`**, 1 206 `Public`.

**Two thirds of them are `Internal`.** So the common case is the one C++ does not express directly,
which is the opposite of the convenient distribution.

## `protected var` IS THE SAME QUESTION FOR A VARIABLE, AND IT IS DROPPED

`devenv-protected-variables.md` (read 2026-09-04, routed here): the `protected` keyword *"can be used
to make variables accessible between tables and table extensions, pages and page extensions and
reports and report extensions ... also between extensions if they belong to apps which depend on each
other."* Two `var` sections, `protected var` first and plain `var` after.

**Measured 2026-09-04 over `~/Git/BCApps/src`: `protected var` 1 780.** And `internal var` and
`local var` are **zero each** -- `src/al/Parser.cpp:604` accepts all three spellings, so two of its
three branches are dead.

**The modifier is parsed and discarded.** `AtProtectedVar` recognises the section and the keyword is
skipped; `al::VarDecl` (`src/al/Ast.h:29`) has no visibility member, so a `protected var` and a plain
`var` are indistinguishable downstream. For PROCEDURES the same collapse happens at
`src/al/Parser.cpp:197` -- `internal` and `protected` fold into `isLocal = false`, which makes them
public: **`internal procedure` 13 508 and `protected procedure` 1 252** join the 251 809 public ones.

**And every object variable is emitted PUBLIC**: `WritePage` (`src/gen/PageWriter.cpp:275`) opens the
class with `public:` and writes `MemberDeclarations` there, before the `private:` section that holds
only local procedures.

**The consequence runs the harmless way and that is why this is recorded rather than urgent.** agiru
is MORE PERMISSIVE than AL, not less: code that AL would reject compiles here. The transpiler's input
is BCApps, which the AL compiler has already accepted, so no BaseApp file is affected. It costs
something only for AL written by hand against agiru, which does not exist -- and it agrees with this
item's own conclusion for `Access = Internal`, where being stricter than the platform would break
`RecordRef` and `Codeunit.Run`.

**What it does cost is a check the compiler could have made.** A `protected` that reached the
generated C++ as `protected` would let `-Werror` catch an extension reaching a variable AL does not
share -- one more class of defect moved from a run to a build, which is this tree's whole argument.

## The IST-state

Not among the nine properties the generator consumes (board:0067). Every generated name is `public`,
so 2 532 declarations of `Internal` are silently ignored -- and CLAUDE.md's own rule is that `private`
is the default and a public data member is an invariant nobody can hold.

## The choice

`Local` and `Protected` on a field are `private` and `protected` on the generated member, which is
exact and free. `Public` is `public`. **`Internal` is the open one** and it is a board:0033 question
rather than a `TableWriter` one.

**Do not map `Internal` onto `public` and move on.** That is the state today and it is what makes the
declaration invisible; if the decision is that C++ cannot express it, the deviation is recorded here
and counted, per CLAUDE.md's rule that a deviation is visible rather than clever.

## Ordering

The field half now -- `Local` and `Protected` are two lines in `TableWriter` and they close 
board:0033's smallest gap. The `Internal` half behind the first generated app.

## Gate, and its negative control

A field declaring `Access = Local` is `private` in the generated class and a use from outside the
table fails to compile; `Protected` compiles from a table extension and not from elsewhere.

**The negative control is the compile failure** -- a gate that only checks the generated text passes
on a member spelled `private` in a class that has no access control anywhere, and only an actual
failing translation unit proves the visibility.
