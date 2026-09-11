# 0703 Three predecessor threads, read and not yet ported

The array view, the property assignment through a chain, the un-hidden base overloads and the rest
of the round this item was filed for landed in commit 49c8d18 (1 788 -> 1 800) and are closed with
it. What stays open is what the predecessor's newest snapshot (bossERP.zip, 2026-09-11, diffed
against the tar of the previous one) named and this tree has not measured:

**From the predecessor's newest state (bossERP.zip, 2026-09-11), read and NOT ported:**

- WI-1401: `CurrPage.Update` fires ONLY `OnAfterGetCurrRecord`, never `OnAfterGetRecord` -- a
  diagnosis confirmed by measurement there, but the activation gained 0 and lost 3 to WI-1411, so
  `Page.Update` here stays the no-op it is until the totals root below is understood.
- WI-1411 (open there): a document line's `Inv. Discount Amount` is written to 0 by a second pass
  of `SalesLine.UpdateVATOnLines` whose `TempVATAmountLine` no longer carries the discount. Our 12
  cases saying `Cannot apply an invoice discount ...` and 6 saying `InvDiscountAmount_General shows
  no field to read` are in that neighbourhood, and the predecessor's instrumentation plan
  (discount in the buffer per call, the base amount, the caller) is the way in.
- WI-1235: a part fed through `CurrPage.Part.PAGE.SetRecords(TempRec)` must be readable from the
  TestPage; our runtime's `SetRecords` on a part is unmeasured.
- WI-1413 (profiles dropped by the parser): already done here -- 44 profiles written into
  `All Profile` on every run.
