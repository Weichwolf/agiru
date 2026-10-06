# 0738 — Reproduce employee, absence and expense processes

Status: queued | Priority: P2
Depends on: 0727 employee/posting setup, 0720 list/card/journal contracts, 0726;
expense posting/application uses 0730's finance contract.
Next: marked employee → absence registration/analysis → expense → reimbursement journal.

## Processes and acceptance

- Employee qualifications/confidential data, absence units/reasons/registration,
  expense/reimbursement and employee ledger application/reversal.
- Docs `bf5ffffa9b026`, `business-central/hr-manage-human-resources.md`,
  `finance-how-record-reimburse-employee-expenses.md` and linked absence tasks.
- Reference/replay unexecuted. Independently reconcile absence totals, employee/detail/
  bank/G/L entries and access control. No real payment or unrelated personal-data modification.
- Exact amounts and quantities, unauthorized reads/writes and rollback cases must
  match across external CMD/MCP and actual browser samples of the container server.
Files: generated employee/finance objects and `test/ui/`.
