# 0728 — Reproduce quote-to-cash, corrections and returns

Status: queued | Priority: P0
Depends on: 0727 customer/item/posting setup and 0720 document/part/dialog/posting contracts;
0726 container endpoint. BC reference capture can proceed now on own test records.
Next: marked customer → quote → order → partial/full shipment → invoice → payment/application.

## Processes and implementation

- Quotes/orders/invoices, prices/discounts/VAT/dimensions, payment terms, reservations,
  blanket/recurring lines, combined shipments, prepayments, drop/special orders.
- Preview/post/release/reopen, credit warnings, unpaid cancellation/correction,
  paid returns/credit memos/refunds and document archives/attachments.
- Use generated AL header/line and posting actions; retain exact Decimal values,
  SaveRecord/delayed insertion and header-before-part boundaries. No sales-specific runtime fixes.

## Evidence and acceptance

- Docs `bf5ffffa9b026`, `business-central/sales-manage-sales.md` and its process links;
  `sales-how-sell-products.md`, `sales-how-invoice-sales.md`, `ui-post-sales.md`,
  `sales-how-send-partial-shipments.md`, `sales-how-process-sales-returns-cancellations.md`.
- Sandbox reference and agiru replay not yet executed. Capture every posted document
  and entry identity, before/after quantities, receivable/VAT/revenue/COGS and applied balances.
- Independently reconcile shipment/invoice/customer/detail/VAT/G/L/item/value entries;
  debit equals credit, no doubled posting, expected remaining quantities/amounts.
- External CMD/MCP execute every step against container clones; sample real browser
  document/lines/confirmations/reports. Independent failure/rollback/retry cases must reject.
Files: generated sales pages/codeunits, shared runtime, `test/ui/`.
