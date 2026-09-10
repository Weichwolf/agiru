# `Record Link` carries the system fields, and a table connection is named by a Guid too

**Finding (2026-09-10).** `CRM Int. Table. Subscriber` did not compile: it reads
`RecordLink.SystemId`, which the platform `Record Link` table did not declare, and passes a
`Guid` where `SetDefaultTableConnection` takes text (AL converts). 6 UT cases stopped there.

**Reference.** `devenv-table-system-fields.md`: every table carries the five system fields;
`database-setdefaulttableconnection-method.md`.

**Choice.** `Record Link` gets the system fields through `WithSystemFields`, like `Company`; the
connection builtin gains the Guid overload beside the text one in the written builtins, both
refusing on premises as before. board:0664's rule for the other absent-object refusals stands.
