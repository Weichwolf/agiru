# 0729 — Reproduce procure-to-pay and purchase returns

Status: queued | Priority: P1
Depends on: 0727 vendor/item/posting setup, 0720 document/lookup/dialog contracts and 0726.
Next: marked vendor → quote/order → receipt → invoice → payment/application.

## Processes and implementation

- Quote conversion, partial/combined receipts, prices/discounts/VAT/dimensions,
  item charges, prepayments, recurring/blanket orders and drop/special-order linkage.
- Preview/release/approve/post, incoming documents/attachments, invoice correction,
  purchase returns/credit memos/refunds. Generic electronic-document protocols remain in scope.
- Reuse AL document/line/posting execution and exact Decimal; uncertain writes need
  reconciled command receipts, not unconditional retry or auto-confirmation.

## Evidence and acceptance

- Docs `bf5ffffa9b026`, `business-central/purchasing-manage-purchasing.md`,
  `purchasing-how-record-purchases.md`, `purchasing-how-to-combine-receipts.md`,
  `purchasing-how-process-purchase-returns-cancellations.md` and their linked variants.
- Reference/replay unexecuted. Verify posted receipt/invoice/vendor/detail/VAT/G/L,
  item/value and charge entries independently, including partial quantities and applied balances.
- Prove balanced ledger effects, inventory/cost linkage, correct corrections and no
  partial posting after failure. External CMD/MCP/container and browser samples share one contract.
Files: generated purchasing objects, shared runtime, `test/ui/`.
