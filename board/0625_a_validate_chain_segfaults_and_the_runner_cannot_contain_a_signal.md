Type:     root
Status:   open
Area:     rt
Source:   `Price Source UT` after the compile-fix batch, 2026-09-08
Class:    activation

# A validate chain segfaults, and the runner cannot contain a SIGNAL

**`Price Source UT` DIES WITH SIGSEGV IN A GENERATED VALIDATE CHAIN**, and the frames name it:

```
Job_Table::BillToCustomerNoUpdated(Job_Table&, Job_Table&)      <- the fault, +0x2127 in
Job_Table::OnValidateBillToCustomerNo()
Job_Table::SellToCustomerNoUpdated(Job_Table&, Job_Table&)
Job_Table::OnValidateSellToCustomerNo()
LibraryJob_Codeunit::CreateJob(Job_Table&, Code<20>)
PriceSourceUT_Codeunit::NewJob(Guid&)
```

**IT IS NOT THE `bad_array_new_length` OF board:0624.** That one is an exception and the runner
catches it now; this is a SIGNAL, and nothing in the process can contain it -- the codeunit leaves
the run and takes its 115 procedures with it.

## What it cost, and what it is measured against

| | before the batch | after |
|---|---|---|
| UT | 344 of 1 708 over 62 | **301 of 1 996 over 68** |
| `Price Source UT` | 56 of 115 | **dead** |

The batch itself is a gain: six codeunits that did not compile now do, and the denominator moved
288 procedures towards the milestone's own 2 291. What it also did is reach this crash, which was
there and unreachable.

**THE FALLBACK `Variant` CONSTRUCTOR IS NOT THE CAUSE** -- removing it leaves the crash exactly
where it was (measured 2026-09-08). The other candidates from the same batch, in the order they
touch generated code: `Guid(const char *)` becoming explicit with the generator emitting the
conversion where AL converts; `List<T>::Add` and `Notification::Id` taking what MAKES their type;
`Date += Integer`; `File::Write` collapsing to two constrained templates.

## What is wanted

**FIRST, THE ADDRESS.** The slice is built with `-g0`, so a frame is a symbol and an offset and
nothing more. A build with line tables for one library -- or `addr2line` against a `-g` slice --
turns `+0x2127` into a line of `Job.Table.al`, and that is the whole distance between this item and
a fix.

**AND THE RUNNER SHOULD SURVIVE IT.** `agiru run-tests --isolate` runs a codeunit per process and
already survives a segfault at the codeunit boundary (board:0612); what the ordinary run does is
lose the rest of the codeunit. A case that dies on a signal should be a failed CASE, which needs
the child-process isolation to be per case or a handler that can longjmp out -- and the second is
not something to write without a reason.

## Standing: the free site is `Modify()` inside the trigger, and `xRec` was the stored image (2026-09-08)

ASan names it in one block: `heap-use-after-free` at `Job.cpp:2241`, reading `XJob.BillToCustomerNo`;
freed by `CaptureImage()` from `Modify()` at `Job.cpp:1539` (`CopyDefaultDimensionsFromCustomer`),
called INSIDE the same `OnValidate`. The generated trigger binds `XRec = this->StoredImage()`,
which is a reference into the held image, and a `Modify` under the trigger replaces that image
and frees the old one under the reference.

**THE SEMANTIC WAS WRONG BEFORE IT WAS UNSAFE.** `devenv-system-defined-variables.md`: `xRec`
holds the original values of the record BEFORE THE CHANGES -- not the record as last read or
modified. The predecessor paid the same lesson as openerp WI-1242: an `OnValidate` that validates a
second field is the BaseApp's normal shape, and the inner trigger's `xRec` still carries the values
from before the OUTER one began; a snapshot per call handed the inner trigger a mixed record.

**THE FIX IS IN THE DOOR AND THE GENERATOR IS UNCHANGED.** `Validate` already copies the record
into a `before` local and pushes it as the trigger's before-image; the stack now records WHICH
record variable each entry belongs to, `detail::OutermostBefore(owner)` finds the outermost entry
of one variable, and `StoredImage()` answers that entry while a trigger of this record runs. The
`before` local outlives every nested trigger, and the held image can be replaced freely.
`Before<T>()` (board:0042) keeps its meaning over the innermost entry.

Class: silent-wrong-data (a mixed `xRec` in a cascade) plus the crash. Measured: pending the
full build (`ut_xrec.log`).

## What proves it

`Price Source UT` reports 115 again. The negative control is the frame list: with the fault fixed,
the same run must not print `agiru: signal 11`.
