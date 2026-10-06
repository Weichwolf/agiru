# 0740 — Reproduce subscription contracts and recurring billing

Status: queued | Priority: P2
Depends on: 0727 customer/vendor/item setup, 0728/0729 invoicing contracts,
0720 document/scheduling operations and 0726.
Next: marked subscription/contract → recurring billing proposal → invoice → renewal/cancellation.

## Processes and acceptance

- Customer/vendor subscription contracts, service commitments, billing periods,
  price changes, deferrals, renewals, termination and usage-based variants.
- Docs `bf5ffffa9b026`, `business-central/SRB/` and
  `business-central/finance-recurring-invoicing.md`; inventory actual process articles
  before executing. Customer subscription business is not agiru product-license gating.
- Reference/replay unexecuted. Independently verify period boundaries/proration,
  billed versus unbilled amounts, sales/purchase documents, deferrals and ledger effects.
- Prove external CMD/MCP/container execution and browser samples; duplicate billing,
  stale proposals, cancellation and failed posting must not leave unintended effects.
Files: generated subscription-billing objects and `test/ui/`.
