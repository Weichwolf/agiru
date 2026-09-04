Type:     task
Status:   open
Parent:   0553
Area:     gen, rt
Source:   developer/devenv-page-type-promptdialog.md, developer/devenv-page-promptguide.md, developer/devenv-page-prompt-error-handling.md, developer/devenv-page-prompting-floating-actionbar.md, developer/copilot-create-promptdialog.md, developer/copilot-customize-generate-mode.md, developer/copilot-design-prompt-mode.md, developer/copilot-design-content-mode.md, developer/ai-build-experience.md
Verdict:  fehlt
Class:    activation

# A PromptDialog has three layout areas and a closed action set

**Nine pages, one item.** Eight of them are filed under Copilot and the ninth under AI, and the
no-task sweep of the same batch nearly took them with it. **They are LAYOUT GRAMMAR**: three area
names that exist on no other page type, two action areas instead of board:0539's six, and a fixed set
of five actions. The Azure OpenAI half is out of scope; this half is not.

## The areas, and their rules are structural

| area | takes | refuses |
|---|---|---|
| `area(Prompt)` | any control -- the input to Copilot | **repeater** |
| `area(Content)` | any control -- the output | **repeater** |
| `area(PromptOptions)` | **only option fields** | everything else |

> "To enable the full copilot experience, you must use the `Prompt`, `Content`, and `PromptOptions`
> areas. **The page must have an `area(Prompt)` with one or more controls that accept user input** to
> have the `PromptDialog` page start with a prompt."

**Every one of those is decidable at translation time** from board:0553's tree: the area kind, the
child control kinds, and whether a field's source is an option.

## The actions are a CLOSED SET, and the source confirms it exactly

> "Unlike other page types, `PromptDialog` pages can only specify TWO action areas: `SystemActions`
> and `PromptGuide` ... The `SystemActions` area only allows you to define a FIXED SET of actions
> called system actions, which are only supported by this page type. These system actions are
> `Generate`, `Regenerate`, `Attach`, `Ok` and `Cancel`."

**Measured 2026-09-04 over `~/Git/BCApps/src`: `systemaction(` is declared 40 times and the argument
is one of exactly five names -- `Ok` 13, `Cancel` 13, `Generate` 9, `Regenerate` 3, `Attach` 2.**
40 = 13 + 13 + 9 + 3 + 2, so the census is complete and the source uses the documented set and nothing
else. **A `static_assert` over five names, with no risk of rejecting BC's own pages**, which is rarer
in this board than it sounds -- board:0553 and board:0560 both had to settle for counters.

**`PromptGuide` renders only when `PromptMode = Prompt`**, and its actions are ordinary actions whose
`OnAction` writes a prompt text into the prompt field. The documentation is explicit that this is AL
work and not a platform feature: *"The AL logic for prompt guides must COMPUTE AND INSERT the prompt
text into the prompt input."* So a prompt guide is a normal action with a convention, and the only
thing the runtime owes it is the mode-dependent visibility.

## One documented exclusivity that the source does not keep

The page says system actions "are only supported by this page type". **Measured per FILE: 13 files
declare `area(SystemActions)` -- NINE are `PageType = PromptDialog` and FOUR are
`PageType = ConfigurationDialog`** (`PayablesAgentSetup`, `CustomAgentSetup`, `SOASetup`,
`ExpenseAgentSetupWizard`).

**Recorded, not resolved at the time.** The sentence is a statement about what a `PromptDialog`
allows, and it does not say what a `ConfigurationDialog` allows.

**ANSWERED by board:0579, and the two page types use the area DIFFERENTLY:**

| | `PromptDialog` | `ConfigurationDialog` |
|---|---|---|
| allowed system actions | `Generate`, `Regenerate`, `Attach`, `Ok`, `Cancel` | **`OK` and `Cancel` only** |
| `OnAction` trigger | **yes** | **NO** -- *"the triggers for these actions can't be defined as they're defined by the platform"* |
| what may be set | the trigger | `Caption` and `Enabled` |

So *"only supported by this page type"* is wrong as written, **the conclusion below stands unchanged**
-- a `static_assert` restricting `systemaction` to `PromptDialog` would reject four pages the platform
loads -- and the check becomes per page type rather than per area.

## `PromptMode` is a property AND a runtime member

> "`PromptMode` is by default `Prompt` ... The other options are `Generate`, which triggers generating
> the output, and `Content`, which shows the output. **You can programmatically set this property by
> setting the variable `CurrPage.PromptMode`** before the page is opened."

**`PromptMode =` is declared 2 times; `CurrPage.PromptMode` is written 6 times.** So the property is
almost always left at its default and the mode is driven from code -- which makes it session state on
the page object, not `constexpr` metadata, and it is the second thing in this sweep (after
board:0560's collapsible indent column) that a `constexpr` cannot hold.

**`Extensible = false` is mandatory** -- *"to ensure that the page isn't extended so that customers can
trust the AI experience"* -- so a `pageextension` over a `PromptDialog` is a third refusal alongside
board:0567's two over API pages.

## Errors inside a prompt dialog follow different rules

`devenv-page-prompt-error-handling.md`, and this is behaviour a test can see:

> "**If the code throws more than one message, only the LAST message is shown**, but the user is
> informed about the TOTAL NUMBER of issues. **If an error is thrown, any subsequent message is
> suppressed.** If the error or message contains LINE BREAKS, **these line breaks are IGNORED**, as
> opposed to when they're rendered in dialogs."

Three deviations from `Message` and `Error` everywhere else in the product, all inside one page type.
board:0517 and board:0518 own the error shapes; **this is a third rendering of them**, and the
"informed about the total number" part means the count is carried, not just the last text.

**The generate-mode caption is `Dialog.Open()` and `Dialog.Update()`** -- the ordinary `Dialog` type,
used as a progress caption inside the prompt dialog rather than as a window. So nothing new is needed
for it, which is worth recording so the page is not read twice.

## `area(Prompting)` is a SEVENTH action area, on ordinary pages

> "To create prompt actions, you must create a new area in the `actions` section ... set to
> `area(Prompting)`. **Only objects of the `PromptDialog` page type can be run from a prompting
> area** ... the prompt actions only appear if you specify the `RunObject` property to the page."

board:0539 places an action in one of six areas; **this is a seventh**, it sits on `List`, `Worksheet`,
`Card`, `Document`, `ListPlus`, `ListPart` and `StandardDialog`, and it renders differently on each --
a floating action bar on `List` and `Worksheet`, an icon in the upper-right on `Card`, at the bottom
on `StandardDialog`.

**Two more translation-time checks**: an action in `area(Prompting)` whose `RunObject` is not a
`PromptDialog` page, and one with no `RunObject` at all -- the latter renders NOTHING, which is a
declaration that silently does nothing and therefore a counter rather than a refusal, since the
documentation describes it as a condition rather than an error.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

| | count |
|---|---:|
| `Extensible =` | 2 285 |
| `ShowMandatory =` | 2 165 |
| `InstructionalText =` | 1 000 |
| `MultiLine =` | 717 |
| `DataCaptionExpression =` | 690 |
| `systemaction(` | **40** -- `Ok` 13, `Cancel` 13, `Generate` 9, `Regenerate` 3, `Attach` 2 |
| files with `area(SystemActions)` | **13** -- 9 `PromptDialog`, 4 `ConfigurationDialog` |
| `IsPreview =` | 10 |
| files with `PageType = PromptDialog` | **9** |
| `CurrPage.PromptMode` | 6 |
| files with `area(Prompting)` | 5 |
| files with `area(PromptGuide)` | 2 |
| `PromptMode =` | **2** |

board:0553's area census gives the layout side: `prompt` 7, `promptoptions` 5, `prompting` 5,
`promptguide` 2.

**`area(Prompt)` is declared 7 times over 9 PromptDialog pages**, so two of the nine have no prompt
area and therefore do not start in prompt mode -- which the documentation describes as a consequence
rather than an error, and the count agrees with it.

## The IST-state

Nothing here exists. `PageType` is not read (board:0553), no area survives `Flatten`
(`src/gen/PageWriter.cpp:52`), and `systemaction` IS in `IsAction` (`PageWriter.cpp:33`) -- so the 40
system actions land on the same flat vector as every other action, with nothing saying they are the
closed set.

## The choice

**No new mechanism: three `AreaKind` enumerators, one `ControlKind::SystemAction`, and a
`SystemActionKind` enum of five.**

```cpp
enum class AreaKind : std::uint8_t { Content, FactBoxes, Processing, ..., Prompt, PromptOptions, Prompting, PromptGuide, SystemActions };
enum class SystemActionKind : std::uint8_t { Generate, Regenerate, Attach, Ok, Cancel };
```

**Why an enum and not the name:** the set is closed, the source uses exactly those five, and an enum
makes the renderer's `switch` exhaustive under `-Werror`. A sixth name appearing in a future BCApps is
then a translation error rather than an action that renders as nothing.

**`PromptMode` is a member on the page object, not in `PageDef`** -- `CurrPage.PromptMode` is written
6 times and read by the renderer, so it is session state. The declared `PromptMode =` seeds it.

**Three `static_assert`s and two counters:**

| check | severity | why |
|---|---|---|
| a `repeater` in `area(Prompt)` or `area(Content)` | error | the documentation refuses it outright |
| a non-option field in `area(PromptOptions)` | error | likewise |
| a `pageextension` over a `PageType = PromptDialog` page | error | `Extensible = false` is mandatory |
| an action in `area(Prompting)` with no `RunObject` | **counter** | it renders nothing; described as a condition, not an error |
| a `systemaction` outside a `PromptDialog` | **counter** | four `ConfigurationDialog` pages do it |

## Ordering

**Inside board:0553**, after the tree and the area kinds. **After board:0429's `PageType`.**

**This is the LAST page type to build, at 9 pages**, and it is in the board now for one reason: its
area names are already in board:0553's census, so leaving it out would mean a `AreaKind` enum that has
to be extended later rather than written once.

## Gate, and its negative control

1. a `PromptDialog` with `area(Prompt)`, `area(Content)`, `area(PromptOptions)` and an
   `area(SystemActions)` holding `Generate` and `Ok` transpiles, and the two system actions carry
   their enumerators
2. a `repeater` in `area(Content)` **fails to transpile**
3. a non-option field in `area(PromptOptions)` **fails to transpile**
4. `systemaction(Improve)` -- a sixth name -- **fails to transpile**
5. an `area(Prompting)` action running a `Card` page **fails to transpile**; running a `PromptDialog`
   does not
6. two `Message()` calls inside a system action's `OnAction` render the SECOND one, with a count of 2

**The negative control is case 6.** Every other case is a structural check the compiler makes; case 6
is the only behaviour, and an implementation that renders both messages -- which is what every other
page type does -- passes 1 through 5 and fails only this. It is also the case a structural test cannot
reach.

**Case 4 is the blind-gate guard for the closed set**: a `SystemActionKind` parsed by string
comparison with a fall-through would accept `Improve` and emit nothing, and cases 1, 2, 3, 5 and 6
would all stay green.

## Class

`activation`. Nine pages, no renderer, nothing to regress. The risk is entirely in the three refusals
firing where they should not -- particularly case 5, whose condition is a property (`RunObject`) on an
action in an area, and `make apps` over the whole tree is the A/B before anything renders.
