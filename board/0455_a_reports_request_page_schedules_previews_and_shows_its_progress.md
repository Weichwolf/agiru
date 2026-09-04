Type:     task
Status:   open
Parent:   0063
Area:     gen, rt
Source:   developer/properties/devenv-allowscheduling-property.md, developer/properties/devenv-showprintstatus-property.md, developer/properties/devenv-usesystemprinter-property.md, developer/properties/devenv-maximumdatasetsize-property.md, developer/properties/devenv-maximumdocumentcount-property.md, developer/properties/devenv-topnumberofrows-property.md
Verdict:  fehlt
Class:    activation

# A report schedules, shows progress, and stops at a declared size

**Six pages, one item**: the report-level switches that govern a RUN rather than a layout. Each is one
value with no interaction except the two the documentation names, and six files would each carry a
paragraph.

> **AllowScheduling** (default **true**): whether the report can be scheduled to run in the
> background. **"Together with `SaveValues`, this property determines whether the report supports
> MULTIPLE PREVIEWS. When both are true, users can preview as many times as they like without the
> request page closing. If either is false ... the request page includes a Preview and Close button
> instead of Preview."** -- board:0413 records the same pairing from the other side.
>
> **ShowPrintStatus** (default **true**): whether a status window is shown. **"The window also
> contains a Cancel button that will cause the processing and printing of the report to TERMINATE.
> If you set it to false, the user will not be able to stop the report prematurely."** And: **"If
> `ProcessingOnly` is true, there will be NO status dialog box, even if `ShowPrintStatus` is true."**
>
> **UseSystemPrinter** (default false): which printer is suggested.
>
> **MaximumDatasetSize**: the maximum number of rows. **"At runtime, this property will OVERRIDE the
> hard limit set by the Default Max Rows setting for the server instance. The server also includes
> the Max Rows (HARD LIMIT) setting, which this property WON'T override."**
>
> **MaximumDocumentCount**, **TopNumberOfRows**: further caps.

**`ShowPrintStatus` is a cancellation channel, not a progress bar.** "The user will not be able to
stop the report prematurely" makes it the only documented way to abort a running report, which under
board:0045's 100-million-row tables is not cosmetic.

**And there are two limits with different authority**: a per-report maximum that overrides one server
setting and cannot override another. That is a three-way `min` with a declared precedence, and it is
the kind of thing an implementation reduces to one number and gets wrong.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`AllowScheduling =` **25** · `MaximumDatasetSize =` **12** · `TopNumberOfRows =` **4** ·
`ShowPrintStatus =` **2** · `UseSystemPrinter =` **2** · `MaximumDocumentCount =` **1**.

**Forty-six declarations across six properties**, all necessarily against the default. So the DEFAULTS
are the work here -- every report schedules, shows a status window with a Cancel button, and stops at
the server's row limit.

## The IST-state

Reports have no generator (board:0063, board:0034); there is no server-instance configuration, so the
two row limits have no source.

## The choice

Six fields on the report descriptor with their documented defaults. **`ShowPrintStatus`'s cancellation
is the only one with runtime substance** and it needs a cancellation token the data-item loop checks
-- which is CLAUDE.md's timeout-guard rule arriving as a user-facing feature.

The row limits are one resolved number per report, computed by the generator where it can be and by
the session where the server settings are.

## Ordering

Inside board:0063. `ShowPrintStatus`'s cancellation with the data-item loop; the rest are descriptor
fields.

## Gate, and its negative control

A running report is cancelled from its status window and stops; a report declaring
`ShowPrintStatus = false` offers no cancel.

**The negative control is the `ProcessingOnly` report** -- it must show NO status dialog even
declaring `ShowPrintStatus = true`, which is the documentation's own exception and the case an
implementation reading one bit gets wrong.
