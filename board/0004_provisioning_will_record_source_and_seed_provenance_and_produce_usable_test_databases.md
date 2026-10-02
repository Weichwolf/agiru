# 0004 — Provisioning will record source and seed provenance and produce usable test databases

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

`BC_VERSION` is 28.4.53241.0. `scripts/provision.sh` claims it checks the source version but only runs three scripts and prints that transfer is not implemented. A seeded runner and import scripts already exist. Imported CRONUS schemas and the runner public schema are different layouts.

## Implementation for Sol

1. Record artefact version/checksum, BCApps commit and scope hash in database metadata. Compare schemas and publish unmapped columns explicitly; do not claim the artefact and current main are version-matched.
2. Make provision orchestrate the actual download, restore, conversion, schema and seed steps with checked exit statuses. Keep imported data immutable and create disposable test clones.
3. Remove early-return coupling between platform catalogue tables in ProvisionInstalled: an existing AllObj row must not prevent Field/Page Metadata from being repaired. Make each catalogue population idempotent.
4. Derive the work date from documented seed policy and record it in run results. Keep test gate DSNs distinct from masters and refuse destructive operations on templates.

## Acceptance

Provision twice, compare catalogue/data counts and identity, then clone and run a representative AL codeunit. Interrupt a transfer and prove restart does not advertise a complete seed. Check empty and partially populated catalogue tables.

## References

Repository: scripts/provision.sh, seed_demo.py, cronus_to_pg.py, pg_master.sh, src/rt/Storage.cpp::ProvisionInstalled, RunnerDatabase.cpp. AL: platform metadata consumers. Predecessor: WI-832 (master held open), WI-847 (demo working date).
