Type:     task
Status:   open
Area:     gen, tc
Source:   `AltCustVATRegFacade` in the not-in-slice ranking, 2026-09-08 (9 compile errors, 2 UT cases)
Class:    compile-only

# A procedure that returns an Interface is called through with an arrow

```al
AltCustVATRegOrchestrator.GetAltCustVATRegDocImpl().Init(SalesHeader, xSalesHeader);
```

The generator decides `->` against `.` from the OWNER of the member access: a variable whose
declared type is `Interface` is a handle (`IsHandle`), and so is a call to one of the UNIT'S OWN
procedures that returns one (`ReturnsAHandle`). A call into ANOTHER codeunit's procedure is
neither, so the body says `.Init(...)` on an `Implementation<I>` that has no such member.

## The reference

The predecessor resolved every member at run time and never had the question. Here the door's
`Implementation<I>` is a pointer-like holder with `operator->`, on purpose: an interface variable
holds whichever codeunit was assigned, and C++ has no way to forward an unknown member through a
value.

## The choice

**THE INDEX CARRIES A PROCEDURE'S RETURN TYPE, or at least whether it is an Interface.**
`TableRef::procedures` is a map from lower-cased name to identifier and knows nothing else; the tc
fills it from a name scan and not from a parse. The generic shape is the one `ReturnsAHandle`
already has for the unit's own procedures, extended to the callee's unit through the index --
which means the tc records, per procedure, the AL return type it declares. Reports and queries
will want the same map for `SetTableView` (board:0063).

Not done in the round that found it: two UT cases behind nine errors, and the fix is a tc index
change with its own blast radius. Filed so the next `make gap` over `AltCustVATRegFacade` does not
rediscover it.

## What proves it

`apps/base/finance/vat/registration/codeunit/AltCustVATRegFacade.cpp` compiles and enters the
slice; the negative control is a procedure returning a Codeunit, which must keep `->` as today.
