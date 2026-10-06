# 0727 — Reproduce setup and master-data processes

Status: queued | Priority: P0
Depends on: 0720 list/card/edit/lookup/save and authorization contracts plus 0726
for agiru execution; BC reference exploration can proceed before those contracts.
Next: create one marked BC customer from a template, validate/change/reopen it,
then reproduce that sequence through external CMD/MCP and sample the agiru browser.

## Processes and implementation

- Company information, numbering, posting/VAT groups, dimensions, payment/shipment
  codes, currencies, locations and units; customer/vendor/item/bank/resource cards.
- Templates, Code normalization, Option/Enum values, lookups, required fields, duplicate
  keys, blocked records, defaults, dimensions and private/company permission boundaries.
- Inventory user-doc processes from `across-business-functionality.md`, area overview
  links and local-functionality topics. Every mapped process belongs to a family WI;
  unmapped/unsupported/unexecuted cases stay visible, never treated as exclusions.
- Reuse AL declarations and generated record validation/page triggers, not C++ business
  masks or client-side posting rules. Page IDs/URL/bookmarks are server-owned identities.

## Evidence and acceptance

- Docs revision `bf5ffffa9b026e146d29f13a242daa5334ddf0d8`, under
  `~/Git/dynamics365smb-docs/business-central/`: `setup.md`,
  `sales-how-register-new-customers.md`, `includes/create_new_customer.md`,
  `purchasing-how-register-new-vendors.md`, `inventory-how-register-new-items.md`,
  `finance-setup-finance.md`, `ui-create-number-series.md`, `finance-dimensions.md`.
- 2026-10-06 sandbox access and Customers page 22 are verified; no process is accepted
  yet. Captures/screenshots stay privately under `~/.local/share/agiru/bc-reference/`.
- Mark all own records/documents; record company/version/work date, inputs, chosen
  template, saved IDs and defaults. Never alter unrelated setup or send real email/payments.
- Reopen independently and verify exact stored values, no unintended ledger entries,
  SQL/company isolation and invalid-input refusal. Equal client transcripts alone fail acceptance.
- Durable agiru cases belong in `test/ui/`; retain exact fixture seed and SQL expectations.
