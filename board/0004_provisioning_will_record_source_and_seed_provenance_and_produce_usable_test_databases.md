# 0004 — Provisioning will record source and seed provenance and produce usable test databases

Status: open | Priority: P1 | Reviewed: 2026-09-22

## Current evidence

`BC_VERSION` is 28.4.53241.0. `scripts/provision.sh` claims it checks the source version but only runs three scripts and prints that transfer is not implemented. A seeded runner and import scripts already exist. Imported CRONUS schemas and the runner public schema are different layouts.

`seed_demo.py` previously ran reader and writer through `sh -c 'psql ... | psql ...'`: the shell returned the writer's success even when the reader failed. The transfer now starts two explicit processes, checks both exits, and reports the failing side. It also exits nonzero when any table refused, even if earlier tables copied rows, so an interrupted or partly seeded target cannot be advertised as complete. Two focused process fixtures cover reader exit 17 with writer exit 0 and a partly populated two-table seed. No live seeded database was changed while the frozen AL run was using it. Existing partial databases still lack durable identity.

New seeds now write `public.agiru_seed_provenance` as `building` before any row transfer, and only a zero-refusal finish changes it to `complete`. Its UUID and JSON record identify the artefact version and SHA-256, BCApps revision, scope hash, source/target schema hashes, company and transfer counts. A second seed attempt on the same target fails before copying. The UT runner reads this row, refuses `building`, and records the complete identity in its run metadata; legacy databases without the row still report `null` rather than inventing one. Two disposable PostgreSQL database probes verified the start/duplicate/finish transitions and the runner's absent/building/complete decisions. The existing `agiru_seeded` template remains legacy until it is rebuilt and sealed with provenance. The current frozen run uses its earlier metadata code and cannot claim this identity.

## Implementation for Sol

1. Record artefact version/checksum, BCApps commit and scope hash in database metadata. Compare schemas and publish unmapped columns explicitly; do not claim the artefact and current main are version-matched.
2. Make provision orchestrate the actual download, restore, conversion, schema and seed steps with checked exit statuses. Keep imported data immutable and create disposable test clones.
   Seed a fresh target: provenance deliberately refuses a second transfer into the same database. Build a new template, validate its `complete` record, then seal it and point the AL runner at it; do not add an identity row to the old partly populated template after the fact.
3. Remove early-return coupling between platform catalogue tables in ProvisionInstalled: an existing AllObj row must not prevent Field/Page Metadata from being repaired. Make each catalogue population idempotent.
4. Derive the work date from documented seed policy and record it in run results. Keep test gate DSNs distinct from masters and refuse destructive operations on templates.

## Acceptance

Provision twice, compare catalogue/data counts and identity, then clone and run a representative AL codeunit. Interrupt a transfer and prove restart does not advertise a complete seed. Check empty and partially populated catalogue tables.

## References

Repository: scripts/provision.sh, seed_demo.py, cronus_to_pg.py, pg_master.sh, src/rt/Storage.cpp::ProvisionInstalled, RunnerDatabase.cpp. AL: platform metadata consumers. Predecessor: WI-832 (master held open), WI-847 (demo working date).
