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
- 0742 owns quality inspections triggered by production/assembly output; expose the
  qualified output/tracking contract to that family without making this WI depend on it.
- Use existing generated AL algorithms; generic runtime semantics own fixes.
  Preserve deterministic ordering, exact units/quantities/costs and concurrent posting boundaries.

## Evidence and acceptance

- Refreshed predecessor 1987/1989: replay forecast/MPS/MRP request initialization
  with Combined MPS/MRP disabled; do not assume SaveValues supplied a prior test's MPS.
  `<90M>` is ninety months, not ninety days. Qualify routing send-ahead quantities,
  concurrent capacities and wait-time DateTime arithmetic across midnight/outside
  working hours against original `SCM Plan-Req. Wksht` and `SCM Capacity Requirements`
  methods. Keep these hypotheses separate from demonstrated agiru faults.

- Docs `bf5ffffa9b026`, `business-central/production-planning.md`,
  `production-manage-manufacturing.md`, `assembly-assemble-items.md` and their linked tasks.
- Reference/replay unexecuted. Independently reconcile demand/supply order linkage,
  component/output/capacity/value/G/L entries, scrap and finished-order variances.
- Record prerequisites unavailable in the live company explicitly; never claim execution
  from an opened page. CMD/MCP/container execution plus representative browser samples required.
Files: generated planning/assembly/production objects and `test/ui/`.
