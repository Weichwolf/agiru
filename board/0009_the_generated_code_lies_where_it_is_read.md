Type: arc
State: open
Area: gen
Tags: target

# The generated code lies in memory the way a posting run walks it

9 300 compiled AL objects are a large text segment. On the target (board:0006) that is acceptable
only because the kernel pages in what is touched -- which makes the LAYOUT of the generated code an
architectural concern rather than an output detail.

## Reference

**Platform documentation / AL source**: not affected. This is a property of the generator, not of AL.

**Predecessor**: had no equivalent. Python loaded every module into the heap at import, which is why
the image cost a gigabyte and why layout could not help. The freedom to decide where code lies is
new here and is not automatically used.

**The mechanism:** a posting run walks Sales-Post, then the ledger codeunits, then dimensions, then
number series. If those lie scattered across a 200 MB segment, each call touches a fresh 4 KB page
from a slow microSD. If the generator emits them adjacently, the same run stays inside a few
hundred pages.

**What is not yet known:** how large the segment actually becomes, and how far apart the objects of
one run lie under the naive layout. Both are measurable as soon as the generator emits more than one
object, and neither should be guessed at before then.

**The candidates**, none of them measured:

| layout | ordering by |
|---|---|
| naive | object id -- what the AL source happens to give |
| by namespace | the BaseApp's own module structure |
| by call graph | what actually calls what, taken from the AST |

## What will be true

- [ ] The size of `.text` per AL object is measured, and the total for the BaseApp is quoted rather
      than estimated.
- [ ] The resident share of `.text` during a posting run is measured on the target.
- [ ] The layout is chosen on that measurement, and the reason stands beside it.
- [ ] Proof: the same posting run under two layouts, resident pages counted both times.
- [ ] **Negative control**: shuffle the object order at random and require the resident-page count
      to rise. If it does not, layout buys nothing here and the item closes as refuted -- which is a
      result, not a failure.
