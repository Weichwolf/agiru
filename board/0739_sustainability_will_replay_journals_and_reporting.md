# 0739 — Reproduce sustainability journals and reporting

Status: queued | Priority: P2
Depends on: 0727 accounts/units/dimensions, 0720 journal/report/analysis contracts,
0726; layout output qualification belongs to 0063.
Next: marked sustainability account/factors → journal → posting → ledger/report reconciliation.

## Processes and acceptance

- Emission/water/waste measures and factors, procurement links, budgets,
  carbon credits, ESG reporting and documented calculation variants.
- Docs `bf5ffffa9b026`, `business-central/finance-manage-sustainability.md`,
  `finance-sustainability-journal.md`, `sustainability-esg-reporting.md` and linked tasks.
  Power BI integration is excluded, native reporting/analysis is required.
- Reference/replay unexecuted. Independently calculate quantities/factors/emissions,
  verify posted entries/dimensions and report totals, including adjustment/reversal.
- Reuse generated AL and exact values, prove external CMD/MCP/container execution and
  browser samples; absent features remain visible, never accepted as successful no-ops.
Files: generated sustainability objects, shared query/report runtime and `test/ui/`.
