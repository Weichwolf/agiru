Type:     arc
Status:   open
Parent:   0037
Area:     rt, gen
Source:   include/runtime/Implementation.h; include/runtime/Codeunit.h (Instance<T>); ~/Git/BCApps/src/Layers/W1/BaseApp/Sales/Document/SalesLine.Table.al:5458 (GetLineWithPrice)
Verdict:  measured
Class:    activation

# A codeunit instance is shared by what refers to it

`SalesLine.GetLineWithPrice` assigns a LOCAL codeunit to a `var` interface parameter and
returns. In BC the instance lives on: a codeunit variable is a reference to an instance the
platform keeps alive while anything refers to it. Here `Implementation<I>` borrowed the address
and read a dead vtable one frame later (SIGSEGV in three UT codeunits, chain 54, 2026-09-09);
it now takes a copy, which is right for this call and wrong in general -- state the variable
changes after the assignment does not reach the interface, and two interfaces assigned from one
variable are two instances.

## What the references say

- `devenv-interfaces.md`: an interface variable "refers to" the codeunit assigned; the codeunit
  is not copied.
- `Instance<T>` (board:0037) already holds a codeunit lazily and "a copy holds nothing",
  because two codeunit VARIABLES are two instances in AL -- true for `Codeunit X; Codeunit Y`,
  but a by-value codeunit PARAMETER and an interface assignment are the same instance.
- The Python predecessor made every object variable a reference and paid for it in copies of
  records (WI-1095), never in codeunits.

## The choice, and what it is part of

A codeunit instance becomes a shared, reference-counted object: `Instance<T>` and
`Implementation<I>` both hold a count on it, assignment shares, and the last holder frees. That
is one third of the recursion work the user asked for (2026-09-09): copy-on-write for a record's
STATE, a large per-session stack, and shared codeunit instances -- each a measurement first:

- `sizeof` of the largest tables and codeunits (measured: Sales Line 5 680 B, Sales-Post 6 640 B,
  Price Calculation - V16 40 B -- codeunit globals are already `Instance<T>` handles, so the
  Python version's 1 400 KB -> 14 KB is this tree's starting point);
- the stack high-water mark over a posting UT, before and after a 256 MB session stack;
- `perf` over a posting UT for the share of `RecordState` copies.

## Measured

The copy: chain 55 follows chain 53's 1 083 of 2 296; the shared instance is measured when it
lands.
