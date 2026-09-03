# 0050 -- AL's two-same-typed-parameter signatures have a home the lint accepts

`Record.FindSet(ForUpdate, UpdateKey)` is documented -- `record-findset-boolean-boolean-method.md`
-- and takes two adjacent `Boolean`s. `bugprone-easily-swappable-parameters` refuses it, and it is
right to in general: two parameters of one type are a transposition away from silent wrong
behaviour.

Three ways out, and none of them is available today:

| way | why not |
|---|---|
| a strong type for one parameter | AL's type IS `Boolean`, and CLAUDE.md's naming invariant says a type named differently breaks the documentation check for every method of that type |
| `NOLINT` on the line | it costs a number in `test/todo-baseline`, and a baseline may only FALL |
| `bugprone-easily-swappable-parameters.MinimumLength: 3` | it would stop flagging `TableId, FieldNo` pairs, which is the reason `.clang-tidy` keeps the check on at all |

So the overload is NOT DECLARED, which is a hole with a count rather than a decision taken quietly:
15 call sites under `Layers/W1`, 3 files in the whole W1 test tree, **0 in the 78 UT codeunits**.
Phase 1 does not reach it and phase 3 does.

It is not one method. `record-insert-boolean-boolean-method.md` is the other side of the same
shape, and CLAUDE.md already singles that file out: "the SystemId rule lives in
`record-insert-boolean-boolean-method.md`, not in the file next to it". AL's surface has this shape
wherever a method takes two flags, so the answer has to be general.

**The choice, when it is taken.** The most likely one is a check option that is narrow in the right
direction -- flag two same-typed parameters unless BOTH names appear verbatim in the AL
documentation's syntax block for that method. clang-tidy has no such option, so it is either a
small clang-tidy check written for this tree, or an entry in `.clang-tidy` under
`CheckOptions` with the AL page cited beside it, following the `misc-include-cleaner.IgnoreHeaders`
precedent already there: narrow the finding at the narrowest point available, never switch the
check off.

Until then a call site that needs the two-argument form fails to COMPILE, with the method name in
the diagnostic -- which is the loud failure, not a silent one.
