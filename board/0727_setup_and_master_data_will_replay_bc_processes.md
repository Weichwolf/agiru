# 0727 — Reproduce setup and master-data processes

Status: queued | Priority: P0
Depends on: 0720 list/card/edit/lookup/save and authorization contracts plus 0726
for agiru execution; BC reference exploration can proceed before those contracts.
Next: expand customer invalid/duplicate/lookup/dimension cases and the remaining
setup/master-data workflows; vendor/item creation follows the accepted original
template → customer → edit → independent reopen regression.

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

- Refreshed predecessor 1966/2000: qualify country composition from BCApps
  `.config/views_config.json` and original GDL build scripts: W1 → DACH → CH,
  higher-layer replacement, excluded-view paths and placeholder handling. Pin selected
  owners/body hashes and country tests; apply agiru adaptations only after composition
  with explicit conflicts. Never silently overwrite localized posting or install
  successful licensing/service stubs. 0058 owns scope, 0073 binding and 0721 deployment.
- Predecessor 2002 retains a RapidStart defect after field-name collision repair:
  ImportPackageXML → ApplyPackage inserts keys but loses Decimal data. Add the original
  DuplicatedXMLFields workflow with independent field-number/RecordRef/SQL checks;
  generic Init/Insert/Modify/alias ownership is the repair, not an object-specific patch.
  Source: `~/Git/openerp/scripts/transpiler/layer_view.py`,
  `scripts/analysis/patch_target_overlap.py`, `test/openerp/runtime/test_layer_view.py`;
  BC `ERM RS Package Operations::ImportPackageWithDuplicatedXMLFields`.

- Current prerequisite regression, 2026-10-09: `make erp-client-test JOBS=2` passes
  preparation 22/22 and client 11/14, with three failures and none skipped/cancelled.
  Original Customer List/Card/edit/create cases pass across external CMD/MCP and
  actual Chromium; all three stale-page conflict cases incorrectly modify the row.
  Independent SQL verifies the defect; 0720 owns the generic observed-version repair.
  The eleven passing cases verify the 40-row window, exact Unicode values, permissions
  and durable receipts; modal 1380 exposes all
  three templates. CMD/MCP/Chromium explicitly select the second template, create one
  customer through original AL, and independently verify creator plus five inherited
  posting/payment/currency fields. Name/Address/Country/Credit Limit persist, and a fresh
  original List/Card independently reopens the exact typed values and zero balance.
  SQL verifies no new customer ledger entries and unchanged counts/full-row fingerprints
  across Customer/G/L/Item/Value ledgers. The generic state repair and production rebuild
  are qualified in 0741. This accepts this Customer sequence on the stated native seed,
  not the whole process family, visual BC parity or a matching-tenant financial A/B.
  Choosing a template automatically is not an implementation.
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
  The corresponding agiru Customer sequence now passes above; the international seed's
  explicit CUSTOMER EU COMPANY differs from this CH reference's template. No matching-
  tenant, posting or source-collation proof is claimed. Eight BC screenshots
  and state captures stay privately under `~/.local/share/agiru/bc-reference/2026-10-06/`.
- Mark all own records/documents; record company/version/work date, inputs, chosen
  template, saved IDs and defaults. Never alter unrelated setup or send real email/payments.
- Reopen independently and verify exact stored values, no unintended ledger entries,
  SQL/company isolation and invalid-input refusal. Equal client transcripts alone fail acceptance.
- Durable agiru cases belong in `test/ui/`; retain exact fixture seed and SQL expectations.
