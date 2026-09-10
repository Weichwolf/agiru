# `NavUserAccountHelper` answers for one tenant on premises

**Finding (2026-09-10).** `NavUserAccountHelper.IsAzure()` was an absent .NET member; `Identity
Management`, `Azure AD Auth Flow` and the `User Card` ask it first, and 12 UT cases refused there.
The BaseApp names 13 members of the type.

**Reference.** The type is the platform's own (`Microsoft.Dynamics.Nav.Runtime`), undocumented;
its questions are about where the session runs, and on premises every one has a fixed answer.

**Choice.** Rebuilt in `include/dotnet/NavUserAccountHelper.h`: not Azure, not Windows
authentication, an admin session, a super user, no assigned permission sets; setting an Entra
object id is a no-op; what asks a directory or a registration refuses. `UserName()` answers empty
until the value layer has a hook to the session's user (owed here).
