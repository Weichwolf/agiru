Type: task
Area: gen, rt, net

# The predecessor is re-read at 886 items, and its findings are taken rather than repeated

`~/Git/openerp` was unpacked again on 2026-09-06 at **886 board items, 327 of them done, 2 276 of
2 289 green** -- against the 773/287/2 260 the earlier copy carried. The 113 new items are 113
semantics somebody already got wrong, measured and corrected.

## What was taken in the same round

| openerp | the finding | what it changed here |
|---|---|---|
| WI-1305 | `[SendNotificationHandler(true)]` carries `HandlerIsOptional`; their parser threw the parenthesised form away and fell back to the procedure's NAME | `TestHandler::optional`, and an optional handler is excluded from the "named and never ran" failure |
| WI-1385, 1206, 1173 | a DISCARDED `FindSet`/`FindFirst`/`FindLast` must raise; the `GAINED 1, LOST 49` that rejected it in September was their own runtime calling `FindSet` where BC does not | `detail::Found` raises when nobody reads the answer |
| `runtime/errors.py` | `DB:NothingInsideFilter` ("There is no *X* within the filter.") is a DIFFERENT message from a keyed `Get`'s `DB:RecordNotFound`, and `Assert.AssertNothingInsideFilter` matches the first | both texts, apart, with the table's name in each |

## What was checked and needed nothing

- **WI-1341** (text builtins return a value where the documentation says "an error is returned"):
  `InsStr` position < 1, `ConvertStr` unequal lengths, `PadStr` negative length and `SelectStr`
  past the end all already raise here.
- **WI-1008** (an Option assigned to an Integer must coerce to the ordinal): holds, and a gate
  compiled and ran it.
- **WI-1365** (a dialog proxy swallowing every unknown name into a no-op): a `__getattr__`
  catch-all is a Python hole; `Dialog` is a closed class here.

**That is the point of the fourth reference and the reason to re-read it whole rather than by
subject:** three findings taken, three checks answered in minutes, and none of the six cost a round.

## What is left standing from the new items

`Query.TopNumberOfRows` writing a field the engine does not read (WI-1349), `TestPage` missing its
state checks (WI-1342) and `JsonArray.IndexOf` absent (WI-1359) are all in surfaces agiru refuses
outright, so they are notes for when those surfaces are built rather than defects here.
