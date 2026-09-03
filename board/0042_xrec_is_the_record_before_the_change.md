Type: leaf
State: open
Area: rt

# `xRec` holds what the record was, and every trigger that reads it gets the right answer

A table trigger names two records and declares neither: `Rec` is the object it runs on and `xRec` is
what that record was BEFORE the change. 5 353 places in W1's BaseApp read it, across 508 generated
files. The name resolves and the mechanism is there; what is missing is the IMAGE, so a trigger that
reads `xRec` raises instead of answering.

## Reference

`devenv-oninsert-table-trigger.md` and its neighbours describe when a trigger runs, and
`devenv-al-table-triggers.md` names the pair. The platform supplies `xRec`; the object does not
declare it, which is why nothing in a `.al` file says where it comes from.

What it holds depends on WHO invoked the trigger, and that is the whole item:

| invoker | `xRec` is |
|---|---|
| `Validate(Field, Value)` | the record as it was before the assignment -- a copy the runtime already has in hand |
| `Insert(true)` | a BLANK record: there is nothing before an insert |
| `Modify(true)` | the row as the DATABASE holds it, which means a read before the write |
| `Delete(true)` | the same as Modify |
| a page's own OnModify | the row the page read, which the page still holds |

`~/Git/openerp/` carries `xrec` as a dict copied at the same four points, and its backlog has no
entry against it -- so the shape is right and only the Modify case costs a read.

## The choice

`detail::PushBefore`/`PopBefore` and `detail::Before<T>()` are in `runtime/Table.h`, and the
generator binds `const T &XRec = detail::Before<T>();` at the top of any body that names it. The
stack is per THREAD and it NESTS, because a trigger runs AL code that validates another record.

What is left is for the four invokers to push the right image. Validate is the largest by far and
also the cheapest, because the runtime holds the record already; it lands with Validate itself.
Modify's before-image is a read, and the isolation rules for it are board:0012's.

**Not a member of the record.** A `Table<Derived>` holds no data -- that is what keeps every
generated class standard-layout, and `offsetof` over the field table depends on it.

## Gate

A trigger that reads `xRec` after a `Validate` sees the value the field had before; one outside a
trigger raises with the sentence naming why. The negative control is the second half: without the
push, the first check must go red.
