Type:     task
Status:   open
Parent:   0035
Area:     gen
Source:   ~/Git/BCApps/src/Layers/W1/BaseApp/Finance/VAT/Registration/AltCustVATRegFacade.Codeunit.al; include/runtime/Implementation.h
Verdict:  measured
Class:    silent-wrong-data

# A call that returns an interface is reached through the arrow

`AltCustVATRegFacade` writes `AltCustVATRegOrchestrator.GetAltCustVATRegDocImpl().Init(...)`: a
procedure on ANOTHER codeunit returns `Interface "Alt. Cust. VAT Reg. Doc."`, and the member is
called on the result. The generator spells it `GetAltCustVATRegDocImpl().Init(...)`, and
`Implementation<I>` reaches its object through `operator->`, so the unit does not compile --
40 UT cases wait on it (`ERM VAT VIES Lookup UT` and `Alt. Cust. VAT Reg. UT`, 2026-09-09).

## What the references say

- `BodyWriter` already spells the arrow for a variable of Interface type (`IsHandle`) and for a
  call to THIS object's procedure that returns one (`ReturnsAHandle`); what it cannot see is the
  return type of another object's procedure, because the object index (`TableRef.procedures`)
  maps a name to its identifier and nothing else.
- Listing the interface's members on `Implementation<I>` is not possible generically: the
  wrapper is one template over every interface.

## The choice

The index records, per procedure, whether it returns an Interface, and the arrow decision
reads it for a member call whose base is a codeunit variable. The alternative -- resolving the
call's type through the callee's header -- is what the index exists to avoid.
