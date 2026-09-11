# 0703 A var array parameter is a view through the element's base

**The finding (board:0688 and the census in board:0702, 47 units).** AL hands one array to
another's `var` parameter across two kinds of element difference, and BC compiles both:

| the parameter | the argument | where |
|---|---|---|
| `var CaptionSet: array[32] of Text[80]` | `array[32] of Text[100]` | `MatrixManagement.GeneratePeriodMatrixData` |
| `var PeriodRecords: array[32] of Record Date temporary` | `array[32] of Record Date` | the same procedure, from `AbsOverviewbyPeriodMatrix` |
| `var Errors: array[100] of Text` | `array[100] of Text[250]` | `Consolidate.GetAccumulatedErrors` (board:0688) |

The door modelled a `var` array as `AlArray<T, 0> &` -- a reference to a view whose element type
is exactly `T` -- so every one of these was a compile error and its translation unit stayed out of
the slice.

**The reference.** `Text<N>` derives from `Text<0>` and `Temporary<R>` from `R`, and in both the
declared length or temporariness is a property of the OBJECT, not of the bytes an element is
reached through: a `Text<100>` assigned through a `Text<0> &` still refuses a 101st character, and
a `Temporary<Date>` inserted through a `Date &` still lands in the buffer, because the runtime asks
the record's state and not its static type (`TempOf(record)` in `src/rt/Navigate.cpp`).

**The choice.** `AlArray<T, 0>` is a VIEW that reaches its elements through a typed accessor: the
sized array that owns the storage indexes ITS element type and hands back the base reference, so
there is no arithmetic over a base pointer (the undefined behaviour board:0688 stopped at) and
nothing is copied. A `var` array parameter is that view BY VALUE -- `AlArray<Text<0>, 0>` -- with
the element unsized and un-`Temporary`d by the generator (`Unsized` in `src/gen/CodeunitWriter.cpp`),
and every write the callee makes lands in the caller's elements, which is what `var` means. `A := B`
through the view copies elements and never rebinds.

**What the same round carries beside it**, each a census class or a predecessor finding:

- `Option := Variant` and `Option = Variant` -- three equally good conversions were an ambiguity in
  nine units; one route named after the source is none.
- `X.SetValue := Y` / `X.Value := Y` on a test-page control -- AL's property-assignment syntax
  through a chain of any depth becomes the call, not `SetValue() = Y`.
- a page reaches its source table's PROCEDURES and platform METHODS without `Rec.` -- `Mark(false)`,
  `Reset()`, `UpdateStatus()` -- the way it already reached its fields; a report's dataitem the same.
- `SetFilter` on a `TestFilter` and a `FieldRef` takes a VALUE with a text form (a Guid, a date).
- `RecordRef.Copy(RecordRef)` and `RecordRef.Copy(Record)` are written.
- `DelChr(Text, Text, Char)` and `Page.GetBackgroundParameters()` returning the dictionary it
  documents (17 units).
- the `Text` half of board:0695 begins: `GetFilter`, `GetFilters`, `TableCaption`, `JsonValue.AsText`,
  `JsonObject.GetText`, `TextBuilder.ToText` return `::agiru::Text<0>` and never `std::string`.

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
