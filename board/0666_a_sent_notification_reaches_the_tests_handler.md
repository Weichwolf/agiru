# A sent notification reaches the test's `[SendNotificationHandler]`

**Finding (2026-09-10).** `Notification.Send()` recorded nothing and told nobody; a test whose
handler enqueues the message for a later `DequeueText` failed with "Queue underflow" (12 UT
cases: Price List Line UT's out-of-sync notification, VAT Return Period UT, Test OAuth 2.0 UT).
The handler kind, its thunk and the generated handler procedure all existed; the value type in
`src/net` had no way to reach the runtime's handler table.

**Reference.** `devenv-notifications-developing.md`; the test framework's
`[SendNotificationHandler]` receives `var Notification` for every `Send`.

**Choice.** `Notification::OnSend(sink)` is a hook the value type carries; the runtime installs
it with the handlers (`HandlerTable::Install`) and the sink dispatches to the
`SendNotification` handler through the same thunk the dialogs use, which now knows the
`Notification &` shape. A notification sent with no handler installed still answers true, as a
client without a handler would show it.
