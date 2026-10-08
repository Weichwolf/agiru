# 0727 — Reproduce setup and master-data processes

Status: queued | Priority: P0
Depends on: 0720 list/card/edit/lookup/save and authorization contracts plus 0726
for agiru execution; BC reference exploration can proceed before those contracts.
Next: resolve the native modal-template contract in 0720/0741, then reproduce the
captured template → customer → edit → reopen sequence through external CMD/MCP and
sample the agiru browser; expand the remaining master-data cases.

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

- Prerequisite regression: `make erp-client-test JOBS=2` passes 7/7 original Customer
  List/Card/edit cases across external CMD/MCP and actual Chromium, independently
  checking the 40-row SQL window, exact Unicode saves, permissions and durable receipts.
  This does not replay the captured New/template sequence or accept this process family.
- The expanded eight-case run retains those seven passes; New fails explicitly at
  original `Select Customer Templ. List` (1380), without inserting a customer.
  The seed has three templates; choosing one automatically is not an implementation.
- Docs revision `bf5ffffa9b026e146d29f13a242daa5334ddf0d8`, under
  `~/Git/dynamics365smb-docs/business-central/`: `setup.md`,
  `sales-how-register-new-customers.md`, `includes/create_new_customer.md`,
  `purchasing-how-register-new-vendors.md`, `inventory-how-register-new-items.md`,
  `finance-setup-finance.md`, `ui-create-number-series.md`, `finance-dimensions.md`.
- BC reference, 2026-10-06, company CRONUS CH: Customers 22 → New → three-template
  selection → DEBITOR MANDANT → Customer Card 21 creates own C00060. Saved Name
  `AGIRU 261006 - Customer`, Address `AGIRU Testweg 6`, Country CH and Credit Limit
  1234.56. A fresh bookmarked navigation defaults to View, not Edit; its five exact
  field values independently match, with displayed limit `1,234.56` and zero balance.
  Keep accepted machine values distinct from localized display text. Own customer
  remains for subsequent sales reference; unrelated customers/setup were unchanged.
  No agiru replay, posting or independent SQL proof is accepted yet. Eight screenshots
  and state captures stay privately under `~/.local/share/agiru/bc-reference/2026-10-06/`.
- Mark all own records/documents; record company/version/work date, inputs, chosen
  template, saved IDs and defaults. Never alter unrelated setup or send real email/payments.
- Reopen independently and verify exact stored values, no unintended ledger entries,
  SQL/company isolation and invalid-input refusal. Equal client transcripts alone fail acceptance.
- Durable agiru cases belong in `test/ui/`; retain exact fixture seed and SQL expectations.
