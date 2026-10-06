# 0733 — Reproduce project budgeting, usage, WIP and invoicing

Status: queued | Priority: P2
Depends on: 0727 customer/resource/dimension setup, 0720 document/worksheet contracts,
0726 and qualified finance posting. Stock/project purchases use 0729/0731 contracts.
Next: marked project → tasks/planning → time/item usage → WIP → invoice.

## Processes and acceptance

- Resource pricing/capacity, time-sheet submission/approval, project budgets/supplies,
  usage journals, WIP methods, partial invoicing and project completion.
- Docs `bf5ffffa9b026`, `business-central/projects-manage-projects.md`,
  `projects-how-create-jobs.md`, `projects-how-record-job-usage.md`,
  `projects-how-invoice-jobs.md`, `projects-understanding-wip.md` and linked tasks.
- Reference/replay unexecuted. Verify planning/budget versus actual quantities,
  project/resource/item/customer/G/L entries and WIP independently, including reversals.
- Reuse AL project runtime; prove external CMD/MCP against container clones and browser
  samples, exact amounts, authorization and failure boundaries. No copied project business rules.
Files: generated project/resource objects and `test/ui/`.
