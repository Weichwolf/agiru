# 0730 — Reproduce journals, banking, tax and period closing

Status: queued | Priority: P1
Depends on: 0727 finance/bank/dimension setup, 0720 journal/filter/posting contracts and 0726.
Next: marked balanced general journal → preview → post → independent G/L register reconciliation.

## Processes and implementation

- General/recurring journals and allocations, payments/application/unapplication,
  bank reconciliation and file exchange, cash flow, reminders/finance charges.
- VAT/non-deductible VAT, currencies/exchange adjustment, deferrals, budgets,
  cost accounting, intercompany/consolidation and year-end closing.
- Include documented country-specific financial variants; unavailable setup/native
  behaviour is a gap, not license gating. Reuse exact Decimal and durable AL Commit boundaries.

## Evidence and acceptance

- Docs `bf5ffffa9b026`, `business-central/finance.md`, `ui-work-general-journals.md`,
  `finance-how-post-transactions-directly.md`, `bank-manage-bank-accounts.md`,
  `finance-work-with-vat.md`, receivables/payables and local-functionality links.
- Reference/replay unexecuted. Preserve account/currency/rate/date/dimension inputs;
  independently reconcile balanced G/L/VAT/bank/customer/vendor entries and registers.
- Verify remaining/application amounts, bank statement differences, period refusal,
  reversal and Commit-followed-by-error durability. No actual bank transfer or real external payment.
Files: generated finance objects, transaction/Decimal primitives, `test/ui/`.
