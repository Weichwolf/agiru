# 0727 — Reproduce setup and master-data processes

Status: queued | Priority: P0
Depends on: 0720 list/card/edit/lookup/save and authorization contracts plus 0726
for agiru execution; BC reference exploration can proceed before those contracts.
Next: finish rebuilding native ERP consumers of 0720's stored-platform binding and
rerun the three retained Item workflows; then expand invalid/duplicate/lookup/dimension and remaining
setup/master-data workflows.

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

- Current original-client prerequisite regression, 2026-10-09:
  `make erp-client-test JOBS=2` passes
  preparation 22/22 and client 26/26, with zero failures/skips/cancellations, exit 0.
  Original Customer List/Card/edit/create cases pass across external CMD/MCP and
  actual Chromium. All three stale-page cases now return native failure diagnostics
  and durable failed receipts without changing any committed field/audit/rowversion
  or ledger fingerprint. The shared runtime repair belongs to 0720.
  Three additional cases normalize ` ch ` to `CH` on own created Customers and
  refuse the absent Country/Region `AGIRUX`. Independent SQL preserves the full row,
  entire Customer population and four ledger populations; a fresh card retains `CH`.
  Three blocking cases save Ship/Invoice/All/blank through CMD/MCP/Chromium and
  independently reopen each value. SQL proves exact ordinals, modifier, advancing
  rowversion, unchanged creation identity, remaining fields, other Customers and
  four ledger populations. Original OnModify updates Last Modified Date Time and
  Last Date Modified; independent SQL clock bounds qualify these rather than hiding
  their effects. Original dropdown discovery now proves the exact ordered ordinal,
  AL member and caption arrays across all three clients. Production declarations and
  native ABI consumers are rebuilt. Three additional privacy cases qualify the original
  confirmation: true sets All; the pending question saves nothing; explicit No produces
  PageValidation with empty AL text and preserves the full row/audit/rowversion; explicit
  Yes saves false/Ship. SQL verifies failed/complete original command receipts, closed
  answer 0, unchanged remaining fields/other Customers/four ledger populations and
  independently reopened values. Real Chromium covers both branches. My Settings
  timezone/Today, same-card continuation after root validation failure, privacy or
  block enforcement on sales/journal posting and BC visual parity remain unqualified.
  These are native validation proofs, not a newly executed BC-sandbox comparison.
  The retained eleven original cases verify the 40-row window, exact Unicode values, permissions
  and durable receipts; modal 1380 exposes all
  three templates. CMD/MCP/Chromium explicitly select the second template, create one
  customer through original AL, and independently verify creator plus five inherited
  posting/payment/currency fields. Name/Address/Country/Credit Limit persist, and a fresh
  original List/Card independently reopens the exact typed values and zero balance.
  SQL verifies no new customer ledger entries and unchanged counts/full-row fingerprints
  across Customer/G/L/Item/Value ledgers. The generic state repair and production rebuild
  are qualified in 0741; 0720 qualifies optimistic-write protection and this replay.
  This accepts this Customer sequence on the stated native seed,
  not the whole process family, visual BC parity or a matching-tenant financial A/B.
  Choosing a template automatically is not an implementation.
- Original Vendor creation now passes through CMD/MCP and actual Chromium:
  List 27 → explicit non-default template in modal 1379 → Card 26 →
  Name/Address/Country → independent List/Card reopen. All three seed templates remain
  visible; no insertion precedes consent. Original AL allocates the key and transfers
  five posting/payment/currency fields. SQL proves creator/modifier, exact full-row/
  rowversion preservation on reopen, unchanged other Vendors and all Customers, and
  unchanged counts/full-row fingerprints of Vendor/Detailed Vendor/Customer/G/L/Item/
  Value entries. Durable cases: `test/ui/erp-client.mjs`; existing shared drivers serve
  both master-data families. No new C++ business mask or native semantic fix was needed.
  References at the pinned revisions below: developer `methods-auto/page/page-{runmodal-,
  getrecord,settableview,update}-method.md`, `triggers-auto/page/devenv-onnewrecord-page-trigger.md`;
  BCApps `Layers/W1/BaseApp/Purchases/Vendor/{VendorList.Page,VendorCard.Page,
  SelectVendorTemplList.Page,VendorTemplMgt.Codeunit}.al`; user
  `includes/create_new_vendor.md`; predecessor 1503/1761/1954. This accepts the native
  Vendor sequence, not a fresh BC-sandbox comparison, purchase posting or the full family.
- Item workflows are retained, not accepted: the expanded `make erp-client-test JOBS=2`
  run has preparation 22/22, client 26/29, three failures, no skips/cancellations,
  outer exit 2. After the native consumer rebuild at 1d41c43 (2483 seconds, exit 0),
  CMD/MCP/Web each open original List 31, then New fails before modal 1378:
  `Entity Text.ReadPermission` is misclassified as an unimplemented .NET member.
  Fixture/client input hashes and original Company source remain unchanged.
  The earlier PageWindowProvider refusal at 6a1556c is superseded by this new blocker.
  0720 now qualifies the actual missing platform-table binding from verified System
  declarations: GenNativeBinding 199 checks, NativeStorage 84 SQL/runtime checks,
  three source defects rejected. Both AL permission forms observe revocation without
  bypasses. Production declarations are regenerated; the native rebuild and fresh
  Item replay remain required. The last accepted client measurement is still 26/29.
  Item List declares OnFindRecord/OnNextRecord: stored Rec
  normally, temporary attribute/pick selection conditionally. Preserve both sources,
  custom navigation and bounded windows; removing the declaration check alone would
  silently bypass AL. `test/ui/erp-client.mjs` retains List 31 → modal 1378 → Card 30,
  explicit template, inherited fields/base unit, exact Decimal price, independent
  reopen/full-row/audit checks and unchanged seven ledger/warehouse populations.
  Those later Item steps remain unexecuted. All 26 existing cases still pass.
  0720 now qualifies custom navigation, cross-provider Copy/seek and bounded windows:
  generated navigation 473/473, dispatcher 122/122, all forty-seven compiled execution
  defects rejected, source hashes unchanged. SQL/default-Next/temporary providers retain
  7/40/80 bounds, raw anchors, per-row images/calculations, descending/filter order and
  provider selection without SQL substitution. Original Item opening now succeeds,
  but creation remains unaccepted. Custom visits are not an AL SQL scan bound.
  References at the pinned revisions below: developer
  `triggers-auto/page/devenv-{onfindrecord,onnextrecord}-page-trigger.md`; BCApps
  `Layers/W1/BaseApp/Inventory/Item/{ItemList.Page,ItemCard.Page,Item.Table,
  SelectItemTemplList.Page,ItemTemplMgt.Codeunit}.al`; user
  `includes/create_new_item.md`; predecessors 1228/1767/1858/1868.
- Docs revision `bf5ffffa9b026e146d29f13a242daa5334ddf0d8`, under
  `~/Git/dynamics365smb-docs/business-central/`: `setup.md`,
  `sales-how-register-new-customers.md`, `includes/create_new_customer.md`,
  `purchasing-how-register-new-vendors.md`, `inventory-how-register-new-items.md`,
  `finance-setup-finance.md`, `ui-create-number-series.md`, `finance-dimensions.md`,
  `receivables-how-block-customers.md` (distinct Ship/Invoice/All restrictions) and
  `admin-responding-to-requests-about-personal-data.md` (restricting data processing).
- Validation contract: developer `f928288ee840334be73142e5fc0202c0e19b246d`,
  `methods-auto/code/code-data-type.md`, `methods-auto/record/record-validate-method.md`,
  `properties/devenv-validatetablerelation-property.md` and
  `triggers-auto/field/devenv-onvalidate-field-trigger.md`. BCApps main
  `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`,
  `Layers/W1/BaseApp/Sales/Customer/{Customer.Table,CustomerCard.Page}.al`:
  field 35 is Code[10] related to Country/Region with default validation enabled.
  Predecessor WIs 962/1132 distinguish relation-validation diagnostics/order from
  lookup metadata; disabling validation never removes the lookup contract.
  Enum/time contracts at those revisions: `methods-auto/enum/enum-frominteger-method.md`,
  `devenv-extensible-enums.md`, `methods-auto/system/system-{currentdatetime,today}-method.md`,
  `methods-auto/dialog/dialog-confirm-method.md` (omitted default is No);
  `Layers/W1/BaseApp/Sales/Customer/CustomerBlocked.Enum.al` declares 0/space, 1/Ship,
  2/Invoice, 3/All. Customer field 39 validates privacy blocking; OnModify calls
  SetLastModifiedDateTime. Predecessor 1576 separates typed Option identity from
  display captions; 548 retains blocked opening-balance failures due to missing setup,
  not proof of correct transaction blocking.
  `test/ui/erp-client.mjs` retains all twenty earlier cases plus three privacy cases;
  `test/ui/browser-client.mjs` shares revision-or-dialog-state waits for setters/actions.
  `include/runtime/{PageSession,ErrorValue}.h` declares the uncoded validation classifier
  and unchanged AL text. `test/ui/erp-fixture.sh` migrates private write ownership only
  on its owned clone.
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
