# 0734 — Reproduce repairs, service contracts and invoicing

Status: queued | Priority: P2
Depends on: 0727 customer/item/resource setup, 0720 service document/part/dialog
contracts, 0726 and stock/finance posting contracts. Reference capture can proceed now.
Next: marked service item → order/assignment → parts/time → shipment/invoice.

## Processes and acceptance

- Service estimates/orders, fault/repair statuses, skills/allocation/loaners,
  warranties/pricing, contracts/renewal/cancellation and periodic invoicing.
- Docs `bf5ffffa9b026`, `business-central/service-service.md`,
  `service-plan-service.md`, `service-deliver-service.md`,
  `service-fulfill-service-contracts.md` and linked tasks. No Premium restriction in agiru.
- Reference/replay unexecuted. Independently reconcile service/item/resource/value,
  customer/VAT/G/L entries, contract dates/amounts and remaining quantities.
- Run external CMD/MCP over the container endpoint, browser samples and refusal/
  rollback cases. Reuse AL service execution, not a separate C++ service engine.
Files: generated service objects and `test/ui/`.
