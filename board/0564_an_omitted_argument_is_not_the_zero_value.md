Type:     bug
Status:   open
Parent:   0028
Area:     rt
Source:   developer/devenv-al-methods.md, developer/methods-auto/system/system-dmy2date-method.md, developer/methods-auto/system/system-dwy2date-method.md, developer/methods-auto/system/system-calcdate-dateformula-date-method.md, developer/methods-auto/system/system-randomize-method.md
Verdict:  deklariert
Class:    silent-wrong-data

# An omitted argument is not the zero value

`devenv-al-methods.md` states the optional-parameter rule and it maps onto C++ default arguments
exactly:

> "The optional parameters may be OMITTED STARTING FROM THE RIGHT. If a method has three optional
> parameters, then you can't omit the second parameter without omitting the third."

**So the shape in the door is right. The VALUES are not.** `include/Builtins.h` carries **47 default
arguments and every one of them is `= {}`** -- value-initialised, which is zero, false, or the empty
string. AL's own documentation names a different default for at least four of them, and `{}` also
cannot express "the caller omitted this" for a second family.

## Four wrong defaults, each with its page

| door | documented default |
|---|---|
| `Builtins.h:243` `DMY2Date(Day, Month = {}, Year = {})` | *"If you omit this optional parameter, the CURRENT MONTH will be used"* / *"the current year"* |
| `Builtins.h:265` `DWY2Date(WeekDay, Week = {}, Year = {})` | *"the current week is used"* / *"the year of the current week"* |
| `Builtins.h:72`, `:80` `CalcDate(DateExpression, Date = {})` | *"The default is the CURRENT SYSTEM DATE. If you omit this optional value, the current system date is used."* |
| `Builtins.h:508` `Randomize(Seed = {})` | *"If you omit this optional parameter, `Randomize` uses the CURRENT SYSTEM TIME (total number of milliseconds since midnight)."* |

`{}` gives day 0, month 0, week 0, year 0, date `0D`, seed 0. **Every one of those is a value the
caller could also pass deliberately**, so the body cannot recover the distinction later either -- the
information is lost at the call site.

**`Randomize()` is the sharpest of the four**: seeded with 0 it produces the SAME sequence on every
run, which is the exact opposite of what the AL asked for. It is worth being precise about the
consequence -- CLAUDE.md's determinism invariant is about postings producing the same entries twice,
not about `Random`, so a fixed seed is not a violation of it. It is simply the wrong answer to
`Randomize()`, and a test that asserts two runs differ would be the one that catches it.

## A second family, and it is a different defect

Six builtins use an optional parameter to turn a GETTER into a SETTER:

```cpp
::agiru::Integer GlobalLanguage(::agiru::Integer NewLanguageID = {});      // :407
::agiru::Boolean LockTimeout(::agiru::Boolean LockTimeout = {});           // :675
::agiru::Integer LockTimeoutDuration(::agiru::Integer LockTimeoutDuration = {});  // :682
CurrentTransactionType(const ::agiru::TransactionType & = {});             // :583
std::string ApplicationArea(std::string_view ApplicationArea = {});        // :792
::agiru::Boolean CodeCoverageLog(::agiru::Boolean NewIsActive = {}, ...);  // :144
```

`system-globallanguage-method.md` gives one signature -- `[LanguageID := ] System.GlobalLanguage([NewLanguageID: Integer])`
-- and *"gets AND SETS the current global language setting."* **With `= {}` the body cannot tell
`GlobalLanguage()` from `GlobalLanguage(0)`**, and 0 is a real LCID (language-neutral).
`LockTimeout()` against `LockTimeout(false)` is the same question with a Boolean, where the collision
is total: `false` is half the domain.

**This family is a different defect from the first.** The first is a wrong VALUE; this is a missing
DISTINCTION, and no default value fixes it -- it needs two overloads, which is what AL's own
`[Optional]` notation means.

## What was and was not checked

**47 `= {}` defaults in `include/Builtins.h`. FIVE were checked against their method pages; FOUR are
wrong and one (`GlobalLanguage`) is the second family.** The other 42 are an unchecked population and
checking them is this item's first task -- one `grep` per method page, mechanical.

**Some of the 42 are certainly fine**: `StopSession(SessionId, Comment = {})` really does default to
an empty comment, `GetCollectedErrors(Clear = {})` to `false`. **The number is stated as 4 of 5
checked rather than extrapolated to the 47**, because a rate over five samples is not a population.

## The IST-state

**All four are refusing today** -- `src/rt/Builtins.cpp:55`, `:61`, `:181`, `:198`, `:367` -- so
nothing produces a wrong date or a fixed seed yet. **The defect is in the signature, which is
checked-in code that a body will inherit**, and it is filed as a bug rather than a task because the
fix edits what exists.

## The choice

**For the first family: the default is an EXPRESSION, not a literal.** C++ default arguments may call
functions, and they are evaluated at each call, which is exactly AL's semantics:

```cpp
::agiru::Date CalcDate(::agiru::DateFormula DateExpression, ::agiru::Date Date = ::agiru::Today());
::agiru::Date DMY2Date(::agiru::Integer Day,
                       ::agiru::Integer Month = ::agiru::Date::CurrentMonth(),
                       ::agiru::Integer Year = ::agiru::Date::CurrentYear());
```

**Why not a sentinel plus a branch in the body:** a sentinel is a magic number, which
`readability-magic-numbers` forbids and CLAUDE.md's "every number carries its origin" forbids twice.
The expression says what the documentation says, in the place the documentation says it.

**Why not `std::optional`:** the parameter's AL type is `Integer`, and CLAUDE.md's name-equality
invariant says a reader who knows AL must recognise the signature. `std::optional<Integer>` is a
different type in the AL vocabulary's terms.

**Careful about one thing:** `Today()` in a default argument is evaluated in the CALLER's context and
at call time. That is right for AL and it must not be hoisted into a static initialiser, which would
freeze the date at load.

**For the second family: two overloads, not a default.**

```cpp
::agiru::Integer GlobalLanguage();                       // get
::agiru::Integer GlobalLanguage(::agiru::Integer NewLanguageID);  // set, returns the previous
```

That is what `[NewLanguageID]` in the AL syntax block means, and it is the only shape that keeps
`LockTimeout(false)` distinguishable from `LockTimeout()`. **It also makes the completeness counter
right**: the door currently shows one signature where the documentation shows an optional parameter,
and the counter cannot see the difference.

## Ordering

**The audit of the remaining 42 first**, because it is mechanical and it sizes the rest. **Then the
six getter/setters**, which are a compile-time change with a `-Werror` blast radius and no runtime
behaviour to A/B. **Then the four defaults**, which land with the bodies board:0028 is opening
anyway.

## Gate, and its negative control

1. `DMY2Date(5)` returns the fifth of the CURRENT month and year, not `0D` and not a runtime error
2. `CalcDate('<1D>')` computes from today
3. two calls to `Randomize()` in different seconds produce different sequences
4. `GlobalLanguage()` returns the current LCID and does NOT set it
5. `GlobalLanguage(0)` SETS the language to 0 and returns the previous value

**The negative control is case 5, and it is the only one that separates the two families.** Give
`GlobalLanguage` a default of `Today()`-style cleverness or any other single default and cases 1
through 4 stay green while case 5 silently becomes a getter -- the caller asked to set 0 and nothing
happened. It is also the case a test is least likely to write, because 0 looks like "nothing".

**Case 3 needs care not to be a green negative control**: two `Randomize()` calls in the SAME
millisecond legitimately seed identically. The gate seeds explicitly on either side of a sleep, or it
asserts the seed rather than the sequence.

## Class

`silent-wrong-data`. `DMY2Date(5)` would return a date -- `0D` or a date in year 0 -- rather than
raising, and `Randomize()` would return successfully with a frozen sequence. Nothing throws in either
case, which is why the four sit in the door unnoticed today.

## THE TEST LIBRARIES SEED DELIBERATELY, WHICH IS THE OTHER HALF OF `Randomize`

`devenv-random-test-data.md` (read 2026-09-04, routed here) shows what BC's own test code does with
randomness, and it is the mirror image of this item's fourth defect:

> "In most of the codeunits in the application test libraries, you find an `Initialize` method that
> often contains `RandomTestRunner.SetSeed(1)` ... **the same sequence of records is created each
> time.**"
>
> "Use the **Any** library ... **this module generates the same set of numbers, allowing you to
> reproduce test failures.**"

**So a FIXED seed is what a test wants and asks for explicitly**, through `SetSeed` or the `Any`
library. That is what makes `Randomize()` with no argument having a fixed default WRONG rather than
merely unusual: **the tests already have a way to be deterministic, and it is not the default of
`Randomize`.** An implementation that froze the seed would give the tests what they already have and
take away the only thing `Randomize()` is for.

It also names the gate's shape: a test that asserts two `Randomize()` calls differ must not race the
clock, and BC's own answer -- seed explicitly when you want repeatability -- is the same one this
item's gate uses.
