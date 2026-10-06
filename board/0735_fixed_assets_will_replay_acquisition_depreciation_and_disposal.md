# 0735 — Reproduce the fixed-asset lifecycle

Status: queued | Priority: P2
Depends on: 0727 depreciation-book/posting setup, 0720 journal/report contracts,
0726 and qualified finance posting.
Next: marked asset → acquisition → depreciation → revaluation → disposal.

## Processes and acceptance

- Multiple depreciation books/methods, acquisition from purchase/journal, maintenance,
  insurance, budgets, write-down/appreciation, transfers and disposal/reversal.
- Docs `bf5ffffa9b026`, `business-central/fa-manage.md`, `fa-how-acquire.md`,
  `fa-how-setup-depreciation.md`, `fa-how-revalue.md` and linked disposal/report tasks.
- Reference/replay unexecuted. Independently calculate expected depreciation/book values
  and reconcile FA/G/L entries, dates, gains/losses and book-specific integration.
- Prove external CMD/MCP/container execution, report values and browser samples;
  exact Decimal/date semantics and refusal/rollback cases remain mandatory.
Files: generated fixed-asset objects and `test/ui/`.
