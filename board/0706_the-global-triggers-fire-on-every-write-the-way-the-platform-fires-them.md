Type:     task
Status:   open
Area:     rt
Source:   developer/devenv-events-global.md
Verdict:  fehlt
Class:    activation

# 0706 The global triggers fire on every write, the way the platform fires them

**The finding (2026-09-11, read while porting the sign-in, board:0704's round).** The platform
codeunit `Global Triggers` publishes `GetDatabaseTableTriggerSetup`, `OnDatabaseInsert`,
`OnDatabaseModify`, `OnDatabaseDelete` and `OnDatabaseRename` (`devenv-events-global.md`), and the
BaseApp subscribes to all five in `GlobalTriggerManagement` -- which fans out to the Change Log,
the CRM/Dataverse integration and the API webhook notifications. Nothing in `src/rt` raises them
(`grep -rn "Global Triggers" src/rt` is empty), so those three subsystems are dead: a Change Log
entry is never written, `Integration Record` rows are never maintained by the trigger path, a
webhook notification is never queued. The predecessor found the same hole at the same moment
(openerp WI-1135, comment 1: "codeunit 'Global Triggers' wird von unserer Runtime NIE publiziert")
and did not close it.

**Reference.** `devenv-events-global.md`: the setup event is raised ONCE per table per session --
"the OnDatabaseInsert, OnDatabaseModify ... triggers are only fired for tables where the
GetDatabaseTableTriggerSetup event set the corresponding parameter to true" -- and the write
events run AFTER the table's own trigger, inside the same transaction, with a `RecordRef` to the
row. The AL comment in `JobQueueInactivityDetect.Codeunit.al:150` (`Item.Insert(); // calls
COD1.OnDatabaseInsert`) shows the BaseApp relying on the trigger firing regardless of `RunTrigger`.

**The shape.** `Table::Insert/Modify/Delete/Rename` in the runtime ask a per-session cache
`TableTriggerSetup(TableId)` -- filled on first use by raising `GetDatabaseTableTriggerSetup` with
four `var Boolean`s -- and raise the matching `OnDatabaseX(RecRef)` when the flag is set. The
cache is per SESSION (the setup depends on the user's Change Log setup) and never per process.

**Class: activation.** Every subscriber that starts running is new code on the posting path, so
this is a full A/B with the case list on a loss; `Change Log` UT cases are the ones expected to
move first.
