# 0737 — Reproduce contact-to-opportunity relationship management

Status: queued | Priority: P2
Depends on: 0727 master data, 0720 list/card/actions and 0726.
Next: marked company/person contact → interaction/opportunity → quote/customer.

## Processes and acceptance

- Contact conversion/linkage, profiles/classification, segments/campaigns,
  interactions/tasks and opportunity stages/won/lost analysis.
- Docs `bf5ffffa9b026`, `business-central/marketing-relationship-management.md`,
  `marketing-create-contact-companies.md`, `marketing-processing-sales-opportunities.md`
  and linked tasks. Dynamics 365 Sales integration is excluded, native CRM is not.
- Reference/replay unexecuted. Independently verify relationships, stages/amounts,
  generated documents and interactions; never email real contacts during capture.
- CMD/MCP/container execution and browser samples use generated AL semantics;
  permissions, duplicate-contact and conversion-failure cases remain counted.
Files: generated relationship-management objects and `test/ui/`.
