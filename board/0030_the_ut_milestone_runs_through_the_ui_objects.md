Type: root
State: open
Area: rt, gen

# The UT milestone runs through the UI objects

Measured 2026-09-02 over `src/Layers/W1/Tests`, on the files the milestone's population lives in
(`*UT.Codeunit.al`, 81 files; the transpiler counts 86 codeunits in them):

| type the file names | UT files | tree-wide files it blocks |
|---|---|---|
| **TestPage** | **41** | 24 |
| Page | 24 | 28 |
| Report | 18 | 20 |
| Notification | 8 | 34 |
| Dialog | 6 | 39 |
| DotNet | 6 | 140 |
| JsonObject | 2 | 31 |
| XmlDocument | 1 | 11 |
| Interface | 1 | 53 |

**The two columns disagree, and the left one is the goal.** `DotNet` blocks more generated files
than anything else in the tree and appears in SIX of the milestone's files; `TestPage` blocks a
third as many files tree-wide and appears in HALF of them. Ordering the work by what the whole tree
fails to compile would put the UI objects last and reach 6 UT files per unit of effort instead of
41.

## What this costs to admit

The `/goal` route reads "filter language and transaction boundary, then temporary records,
FlowFields, remaining field types, then CRONUS, then the tests themselves". **The UI objects are not
in it, and about half the milestone stands on them.** That is not a reason to reorder the route --
the record layer is under the page layer and has to be right first -- but it is a reason to stop
treating pages as something that comes after the milestone.

A `TestPage` is not a page: `testpage-data-type.md` gives it its own surface -- `OpenEdit`, `First`,
`Next`, `GoToRecord`, field access by name, `Action`, and the handler attributes
(`[PageHandler]`, `[ModalPageHandler]`, `[ConfirmHandler]`) that let a test answer a dialog the code
under test raises. Half of that is a TEST harness rather than a UI, and the harness half is what 41
files need.

## The reference this stands on

`~/Git/openerp` reached 97.0 % of the same subset, so it has an answer: `openerp/runtime/` carries
`_page_registry.py`, `filter_page.py` and a `builtins/_recordref.py`, and `openerp/base/**/page/`
holds the generated pages. **Grep there before deriving any of this from scratch** -- in particular
how a `[PageHandler]` is bound to the page it answers, which is the part with no obvious C++ shape.

## What the predecessor already answered, measured 2026-09-02

`~/Git/openerp/openerp/runtime/base/test_page.py` is 4 740 lines and `page.py` is 1 367, and the
module docstring of the first states the dependency that decides the order here:

> field access ... `set_value(v)` / `value(v)` -> `record.validate(field, v)` -- drives the real
> `OnValidate` trigger and its error path.

**A TestPage field write IS a Validate**, which is trigger dispatch (board:0029). So the UI half of
this item cannot start before that one, and the 41 files do not need a page renderer -- they need
`Validate` plus navigation over a record set.

The same file names its own scope boundary, and it is a MEASURED one rather than an opinion: v1 left
"ModalPageHandler dispatch and real action-`OnAction` execution" out, `<action>.invoke()` was a
no-op, and the suite still reached 97.0 %. So the handler machinery is not what most of the 41 files
are waiting for.

**One thing there is NOT to be copied.** That module degrades everything it cannot resolve -- a page
out of scope, a control with no matching field -- to a silent no-op handle, and says so outright:
"activating this wrapper never *removes* a code path that previously survived." That is the
`activation` trade taken in the other direction, and it is why a 97 % number can hide a wrong one:
CLAUDE.md's "a failure is loud" and "accepting a declaration and doing nothing with it is worse than
refusing it" both point the opposite way. Here an unresolvable control is an ERROR naming the page
and the control.

## What is true when this closes

- A `page` object translates, with its source table, its fields and its actions.
- A `TestPage` drives one: open, move, read and write a field by AL name, invoke an action.
- A handler attribute binds a procedure to the page it answers, and a page opened with no handler
  bound is an ERROR naming the page rather than a silent pass.
- The count of UT codeunits that translate is a baseline beside the others.
