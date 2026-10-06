# 0731 — Reproduce stock, costing and warehouse flows

Status: queued | Priority: P1
Depends on: 0727 item/location/bin setup, 0720 worksheet/document/lookup contracts and 0726.
Sales/purchase integration uses the qualified 0728/0729 shipment/receipt contracts.
Next: marked item → purchase receipt → movement/transfer → pick/shipment → physical count.

## Processes and implementation

- Item journals, physical inventory orders/recordings, transfer/reclassification,
  reservations/availability, serial/lot/package tracing, bins and replenishment.
- Basic and directed receipts/put-away/movements/picks/shipments; undo/correction,
  FEFO, partial quantities, cost adjustment and inventory-to-G/L reconciliation.
- Reuse AL planning/warehouse posting and typed key/filter/query primitives;
  bound scan/pivot/output blocks, never materialize the entire ledger client-side.

## Evidence and acceptance

- Docs `bf5ffffa9b026`, `business-central/inventory-manage-inventory.md`,
  `design-details-warehouse-management.md`, `inventory-how-count-inventory-with-documents.md`,
  `inventory-how-work-item-tracking.md`, `inventory-how-adjust-item-costs.md` and linked tasks.
- Reference/replay unexecuted. Independently verify location/bin/tracking quantities,
  item/value/warehouse entries, reservations, applied costs and corresponding G/L balances.
- External clients use container clones; sample barcode/lookup/document/ledger browser views.
  Over-pick/duplicate tracking/stale inventory/failure cases must refuse without partial effects.
Files: generated inventory/warehouse objects and `test/ui/`.
