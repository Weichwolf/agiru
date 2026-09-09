# A self-hosted tenant is paid, and a started session runs here

**Finding (2026-09-10).** With `NavTestExecution` answered, `EnvironmentInformation` stops on the
next platform table it reads, `Tenant License State` (2000000189), through `FindLast` (47 UT
cases, Match Bank Reconciliation, Price List Header UT, Suggest Price Lines UT). Beside it,
`Session.StartSession(SessionId, CodeunitId, Company, Record)` refused 22 cases in the price
list UTs, which start a background session to run a codeunit over a record and then read what
it wrote.

**Reference.** `Tenant License State` is a system table with `Start Date`, `End Date` and
`State` (`Evaluation, Trial, Paid, Warning, Suspended, Deleted, LockedOut`), read by the
Environment Information module through its last row. `session-startsession-*-method.md`: the
codeunit runs in a new session; the caller continues at once.

**Choice.** `include/platform/TenantLicenseState.h`, registered like the other platform tables,
and `ProvisionInstalled` writes one `Paid` row from 1980 to 2079, which is what an on-premises
installation answers. `StartSession` runs the codeunit synchronously in the caller's session
and transaction through `RunCodeunitByNumber`, counting the sessions it hands back; the
deviation is written on the door -- a test that expects the work NOT to be there yet sees it,
and a session layer is board:0035's. Activation on both counts, measured by the milestone.
