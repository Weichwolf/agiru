Type:     bug
Status:   open
Parent:   0077
Area:     rt
Source:   developer/triggers-auto/codeunit/devenv-onrun-codeunit-trigger.md
Verdict:  implementiert
Class:    silent-wrong-data

# A codeunit's `OnRun` runs inside a boundary, and the boundary neither commits nor raises

`OnRun` "runs when a codeunit is run" -- through `Codeunit.Run(id)`, through `CODEUNIT.RUN`, or by
assigning the codeunit and calling `Run()`.

## The IST-state -- the trigger IS called, and what surrounds it is wrong

`include/runtime/Codeunit.h:186` opens a `detail::Scope`, calls `OnRun` and returns `true` or
`false`. `src/gen/CodeunitWriter.cpp:89` emits the trigger and registers `InvokeTest<..., &X::OnRun>`
where the codeunit declares one. So the trigger reaches the runtime.

**What the boundary does is where this becomes a `bug` rather than a task.** board:0077 records the
documented behaviour from `codeunit-run-method.md`: `Codeunit.Run` **COMMITS** when its answer is
used and **RAISES** when it is discarded. `Codeunit.h:173` does neither -- it opens a scope, runs,
and returns a Boolean whatever the caller does with it.

Two consequences, both silent:

- `Codeunit.Run(X)` as a STATEMENT, whose `OnRun` raises: AL propagates the error, agiru swallows it
  and returns `false` to nobody.
- `if Codeunit.Run(X) then` where `OnRun` succeeded: AL has committed everything the codeunit wrote,
  agiru has not -- so a later rollback undoes work CLAUDE.md's first invariant says is durable.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`trigger OnRun()` on a codeunit: **6 971 declarations.** `Codeunit.Run(` at **4 312 call sites**, of
which the value-using form -- `if ... Codeunit.Run` -- is **1 268**.

## The choice

The value context decides the boundary, and the generator already knows it (CLAUDE.md's tabulated
trap names the contexts). `Run()` keeps its Boolean for the `if` form and commits on success; the
statement form re-raises. Both are board:0077's subject -- this item is the CALL SITE, which exists,
pointed at a boundary that does not do what the page says.

## Ordering

Behind board:0077, which owns the two rules. Nothing else blocks: the trigger and the scope are
both there.

## Gate, and its negative control

`if Codeunit.Run(X) then` where `OnRun` inserted a row and a later boundary rolls back: the row
SURVIVES. `Codeunit.Run(X)` as a statement where `OnRun` raises: the error reaches the caller.

**The negative control is the surviving row** -- today both cases pass a test that only checks the
return value, which is why this is filed as a bug against a trigger that already runs.

## `TableNo` IS WHAT GIVES `OnRun` ITS `Rec`

`devenv-codeunit-object.md` (read 2026-09-04, routed here) is short and settles one thing this item
depends on:

```al
codeunit 50113 CreateCustomer
{
    TableNo = Customer;
    trigger OnRun();
    begin
        CheckSize(Rec);
    end;
    procedure CheckSize(var Cust: Record Customer) ...
}
```

> "The codeunit can be used BOTH as a direct call to `codeunit.run(customer)` AND as a call to the
> procedure inside the codeunit, `createcustomer.CheckSize(customer)`."

**So `TableNo` is what puts a `Rec` on a codeunit**, and the two entry points differ in more than
convenience -- board:0077 records that `Codeunit.Run` carries commit-and-raise semantics that a direct
procedure call does not. **One object, two doors, two transaction behaviours**, and `TableNo` is the
declaration that makes the first one possible.

The page also names `this` as the codeunit self-reference, which board:0026 owns.
