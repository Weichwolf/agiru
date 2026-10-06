# 0732 — Reproduce planning, assembly and manufacturing

Status: queued | Priority: P2
Depends on: 0731 stock/tracking and 0727 BOM/resource/calendar setup;
0720 worksheet/document contracts and 0726. BC exploration is independent of agiru readiness.
Next: marked BOM and demand → planning worksheet → supply order → consumption/output.

## Processes and implementation

- Forecast/MPS/MRP/requisition, replenishment parameters/order tracking;
  assembly-to-stock/to-order, production BOM/routing/capacity/calendars.
- Planned/firm/released/finished production, material/capacity journals, scrap,
  subcontracting, reservations and costing. Premium licensing is not an agiru restriction.
- Use existing generated AL algorithms; generic runtime semantics own fixes.
  Preserve deterministic ordering, exact units/quantities/costs and concurrent posting boundaries.

## Evidence and acceptance

- Docs `bf5ffffa9b026`, `business-central/production-planning.md`,
  `production-manage-manufacturing.md`, `assembly-assemble-items.md` and their linked tasks.
- Reference/replay unexecuted. Independently reconcile demand/supply order linkage,
  component/output/capacity/value/G/L entries, scrap and finished-order variances.
- Record prerequisites unavailable in the live company explicitly; never claim execution
  from an opened page. CMD/MCP/container execution plus representative browser samples required.
Files: generated planning/assembly/production objects and `test/ui/`.
