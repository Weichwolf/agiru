# 0742 — Reproduce quality inspections, restrictions and dispositions

Status: queued | Priority: P2
Depends on: 0727 tracked-item/vendor/location setup, 0731 stock/tracking contracts,
0720 list/card/part/dialog/permission contracts and 0726 container execution.
0732 supplies production/assembly output; 0736 supplies workflow/job scheduling;
0063 supplies genuine certificate output. These gate their respective variants,
not the first manual purchase inspection. BC reference capture is independent.
Next: verify the sandbox's installed app/version and permissions; create an isolated
marked purchase lot, inspect it before receipt, finish PASS/FAIL and verify restrictions.

## Processes and implementation

- Results, typed measurements, templates and generation rules; manual and scheduled
  inspections, basic/directed receipts, production/assembly output and reinspections.
- Separate result from Open/Finished status. Exercise newest versus finished-only
  selection, result priorities, Block/Allow finished only/Allow and lot/serial/package
  blocking/unblocking. Inspect own filtered workflows, never change unrelated stock.
- Quarantine movement/internal put-away, transfer, negative adjustment, purchase return
  and tracking reclassification; entire tracked quantity, specific/default-base quantity,
  sample, passed and failed quantities. Require sufficient matching source inventory.
- Add the app/library/tests to the qualified application graph and transpile through
  existing generic metadata, record, event, page and report primitives. One library per
  app; no hand-written quality business rules or licensing bypass. Keep native API tests.

## Sources and acceptance

- User docs `bf5ffffa9b026e146d29f13a242daa5334ddf0d8`, `business-central/qms-*.md`:
  overview/setup/templates/rules, manual/scheduled creation, simple/warehouse receipts,
  production output, result grades, workflows, lot blocking and noncompliant processing.
- BCApps main `d99152ee35f0ca8cfec43ba6334b7247a0ee6b17`,
  `src/Apps/W1/Quality Management/{app,Test Library,test,Demo Data}/`;
  application manifests say 30.0.0.0. Raw AL files/objects: 218 app, 3 library,
  23 test and 12 demo (256 total); 18 test codeunits contain 748 test methods.
  Configured-app population: zero; approved product exclusions: zero. No app root is
  registered in `apps.json`; this is a coverage gap, not a product exclusion.
  Reproduce with `make census AGIRU_BC_SOURCE=/home/cosmo/Git/BCApps/src` and select
  the `Apps/W1/Quality Management/` prefix in `build/scope-inventory.json`.
  The full-tree census exits nonzero: conditional variants remain refused in
  `Apps/IN/INFADepreciation/app/src/table/FixedAssetShift.Table.al`,
  `Apps/W1/SalesOrderAgent/app/src/Setup/SOASetup.Table.al` and
  `Layers/FR/BaseApp/Bank/BankAccount/BankAccount.Table.al`; none is a quality source.
  Qualify compatible source/symbol/demo versions before claiming equivalent execution.
  `src/DisabledTests/Quality_Management-Tests/Quality_Management-Tests.DisabledTest.json`
  has 16 upstream entries, including a whole API codeunit; retain their actual method
  identities in 0058, not an automatic skip or approved exclusion.
- Reference and agiru replay unexecuted. Use isolated `AGIRU-QMS-` prerequisites, not
  edits to shared Contoso demo records. Missing app/permissions/source behaviour stays visible.
- Reopen and independently verify measurement types/values, result/status, inspection-source
  links and tracking restrictions. Test concurrent finish/reinspection, denied permissions,
  repeated actions and failing posting; no partial or duplicate inventory/financial effects.
- Reconcile inspection, tracking, reservation, item/value/warehouse/G/L entries and return
  documents independently. Compare certificate dataset and genuine PDF, not a dataset dump.
  CMD/MCP perform complete sequences; sample real browser dialogs, tracking and certificate.
  Durable acceptance cases belong in `test/ui/` and `test/reporting/`.
