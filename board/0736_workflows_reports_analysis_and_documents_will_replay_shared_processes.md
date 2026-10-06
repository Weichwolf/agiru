# 0736 — Reproduce approvals, reports, analysis and document exchange

Status: queued | Priority: P1
Depends on: 0720 dialogs/files/chart/analysis/request-page contracts, 0726;
0063 layout/rendering contracts only for genuine output acceptance, not BC exploration.
Next: marked purchase approval → request → delegate/approve/reject → posting restriction;
retain explicit user permissions and own workflow ownership.

## Processes and acceptance

- Workflow templates/conditions/responses and multi-user approvals, task/job queues,
  archives/comments/attachments, incoming documents and generic data exchange.
- Report request options/filter/SaveValues/layout selection/scheduling/download;
  genuine PDF/workbook output, role-center charts/drilldown and ledger analysis
  with saved/renamed/copied/shared/imported/exported definitions.
- Docs `bf5ffffa9b026`, `business-central/ui-across-business-areas.md`,
  `across-workflow.md`, `across-income-documents.md`, `across-data-exchange.md`,
  `analysis-mode.md`, `ui-work-report.md`, `admin-job-queues-schedule-tasks.md`.
- Earlier BC attachment/chart/view/template samples are reference evidence only;
  full approval/report/job execution and agiru parity are not accepted.
- Verify exact file bytes, approvals/restrictions, committed queue effects, independent
  report/aggregate totals, copied-view independence and user/company isolation.
  Generic export/SMTP protocols remain in scope; Microsoft cloud integrations do not.
Files: shared page/command/report/query runtime, `test/ui/`, `test/reporting/`.
