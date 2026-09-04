Type:     task
Status:   open
Parent:   0055
Area:     rt, gen
Source:   developer/devenv-error-dialog.md, developer/devenv-actionable-errors.md, developer/devenv-error-handling-guidelines.md
Verdict:  fehlt
Class:    activation

# An error dialog has four parts, and an actionable error carries its fix

**Three pages, one item**: what the dialog is made of, how an error offers a remedy, and the wording
rules. They describe one surface and board:0055 owns it.

## The dialog is four parts, and the AL developer supplies two

> 1. **a title (optionally)**
> 2. **a message directed to the user**
> 3. a **Copy Details** action
> 4. a yes/no question as to whether the message was helpful

**Parts 3 and 4 are the platform's**, so the runtime owns them and no AL code produces them. Part 1
comes from `ErrorInfo.Title` and part 2 from `ErrorInfo.Message` -- and a plain `Error('text')` supplies
only part 2.

**And the Copy Details content is specified** -- the messages, then `DetailedMessage` if the `ErrorInfo`
overload was used, then:

> internal session ID · Application Insights session ID · client activity id · **time stamp on error**
> · user telemetry id · **AL call stack**
>
> `Report1(Report 50101).OnPostReport(Trigger) line 2 - ReportErrors by Default publisher`

**The AL call stack line format is a specification**: object name, object type and id in parentheses,
member, kind, line number, publisher. That is what `ErrorInfo.Callstack()` (board:0518) returns and it
is reproducible only if the generated code carries AL line numbers -- which nothing in this tree does
yet, and which is the item's largest open question. **Recorded, not answered.**

## An actionable error carries a Fix-it action

> ```AL
> FixitErrorInfo.Title('The line dimension value isn''t valid');
> FixitErrorInfo.Message(StrSubstNo('The dimension value must be blank for %1 for Vendor %2', dimension, vendorCode));
> FixitErrorInfo.DetailedMessage('Add some text to help the person troubleshooting this error.');
> FixitErrorInfo.AddAction('Set value to blank', Codeunit::FixitCodeunit, FixitCodeunitMethodName);
> Error(FixitErrorInfo);
> ```

**`AddAction(caption, codeunit, method name)`** -- so an error dialog can RUN AL. The button invokes a
codeunit method by name, which means the error carries a callable reference and the dialog is not a
message but a small form with actions.

**That is the mechanism board:0506's `TestField` navigation is the built-in case of**: `TestField(Field)`
gets a "Show [Record]" button for free, and `AddAction` is how AL adds its own. Two paths to one
dialog feature.

**And the method is named by STRING** -- `FixitCodeunitMethodName` -- so the runtime resolves a
procedure by name on a codeunit at run time. That is a reflective call, which this tree otherwise
avoids; the generator can turn it into a `constexpr` name-to-function table per codeunit, which is the
same shape board:0512's dispatch needs. **One mechanism, two consumers.**

## The wording guidelines are for the AL author, not the runtime

`devenv-error-handling-guidelines.md` is user-experience advice -- how to phrase a message so a user can
act on it. **It produces no runtime obligation**, and it is read and recorded rather than turned into a
task, except for one thing it settles: the error TEXTS in the BaseApp are intended behaviour and tests
compare them, which CLAUDE.md already states and this page's existence confirms.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`AddAction` and the `ErrorInfo` overload of `Error` are method calls; board:0028 owns the census.
**Stated rather than guessed** -- and the `AddAction` count is worth taking, because it sizes the
reflective-call requirement.

## The IST-state

board:0055 records the error surface. `include/runtime/Error.h` carries the error type;
`src/rt/Builtins.cpp` refuses the dialog family (board:0035). No AL line numbers are emitted, so the
call stack cannot be produced.

## The choice

`ErrorInfo` as a value type carrying title, message, detailed message, the five addressing fields
(board:0518), custom dimensions and an action list. `Error(ErrorInfo)` raises it; the renderer
(board:0030) draws four parts.

**The action list holds `{ caption, codeunit id, procedure index }`, resolved by the generator** from
the string where it is a literal -- which it is in the example -- and refused where it is not, until
something needs the dynamic form.

**The AL call stack is deferred and named**: it needs line numbers in the generated code, and that is a
generator decision with a cost across 7 885 translation units.

## Ordering

Behind board:0055's error type and board:0030's dialog rendering. The call stack behind a decision
about line numbers that has no item yet -- **this is where it is named.**

## Gate, and its negative control

An `Error(ErrorInfo)` with a title, a message and one action renders four parts and the action's button
runs the named codeunit method; a plain `Error('text')` renders a message and no title.

**The negative control is the plain `Error`** -- it must produce NO title, and an implementation that
fills the title from the message produces a dialog with the same sentence twice, which looks tidy and
is not what BC does.
