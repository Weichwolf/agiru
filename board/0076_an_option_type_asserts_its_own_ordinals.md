Type: leaf
State: open
Area: net, build

# A system option type asserts its own member count and ordinals at translation time

`methods-auto/` documents a large family of SYSTEM OPTION types -- `Action`, `AuditCategory`,
`ClientType`, `DataScope`, `ErrorType`, `ExecutionMode`, `TelemetryScope`, `TransactionModel`,
`Verbosity` and their neighbours -- and each page is one table: the members, in order. An AL option
is zero-based and sequential in exactly that order, so the ordinal of every member is a
DOCUMENTED FACT and every one of them is decidable when the tree is compiled.

`include/type/Action.h` and `include/type/AuditCategory.h` transcribe them correctly today and
**nothing asserts that they still do**. A member inserted in the middle -- which is how the
documentation itself changes, since BC appends and occasionally reorders -- shifts every ordinal
after it, silently, and an option field written with one ordinal is read as another.

## Why this is a `static_assert` and not a test case

CLAUDE.md is explicit, and this is the plainest instance of it in the tree:

> **Anything decidable at translation time is a `static_assert`, never a test case.** Field counts,
> sort order, layout, enum exhaustiveness. The transpiler EMITS them beside every object, so a
> mis-generated table is a translation error rather than a lookup that quietly finds nothing.

An option's member count and its last member's ordinal are both `constexpr`. A test case for them
would run seconds later than the compiler could have refused, and would have to be written 60 times.

## The population

The system option types are the pages named `<type>-option.md` under `methods-auto/`; the sweep
counts them as it reaches each type (board:0071). `Action` has 9 members, `AuditCategory` 19. They
are written by hand into `include/type/`, from the page, and the page is the only thing that says
whether the transcription is right.

## The choice

- **Beside each option enum, two lines**: a `static_assert` on the count, and one on the ordinal of
  the LAST member. Both are free at run time and both fail at the point of the edit.
- **The count comes from the documentation page**, quoted in the assert's message with the file name,
  so a reader who sees it fail knows which page to open.
- **`agiru::OptionTraits<E>::kMembers` already exists** for generated AL options -- the system
  options use the same shape rather than a second one, which also gives them `Format` and
  `OptionCaption` for free (board:0053).

## Gate

The asserts themselves are the gate: they are checked on every build of the door.

**Negative control**: insert a member into the middle of one option and require the build to FAIL.
If it compiles, the assert is on something else -- a count that happens to match after an insert
plus a delete is the case that makes a count-only assert blind, which is why the last ordinal is
asserted too.
