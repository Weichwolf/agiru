Type: root
State: open
Area: rt

# `xRec` is the stored image of the record, and every place that reads it gets the right answer

A table names two records and declares neither: `Rec` is the object and `xRec` is what that record
was before the change. 5 353 places in W1's BaseApp read it, across 508 generated files. The name
resolves and a mechanism is there; the IMAGE is not, so a body that reads `xRec` raises.

## Reference

`devenv-oninsert-table-trigger.md` and its neighbours say when a trigger runs and name the pair. The
platform supplies `xRec`, which is why no `.al` file says where it comes from.

**THE PREDECESSOR GOT THIS WRONG FOUR TIMES AND MEASURED EACH ONE.** The four items are the
specification here, and the first of them refutes the obvious design outright:

| item | what was wrong | what it cost |
|---|---|---|
| WI-781 | `xRec` in OnModify was a copy of `Rec` | 45 triggers dead; GAINED 8 over 209 ids when fixed |
| WI-1078 | outside a trigger `xRec` mirrored `self`, so every `Rec.F <> xRec.F` was trivially false | GAINED 9 of 183 |
| WI-1137 | the image was right and `Copy` read it out of a `__slots__` object, copying nothing | neutral, and it was silent-wrong-data |
| WI-1156 | the image was refreshed on load and on modify but NOT after an insert | a second journal line's OnModify cleared the FIRST line's entry; GAINED 2 |

**WI-1078 IS THE ONE THAT DECIDES THE DESIGN.** `xRec` is not a trigger-scoped thing the platform
hands in: the BaseApp reads it in ordinary table PROCEDURES too. `ProdOrderComponent.UpdateBin`
writes `Comp2 := Comp; Comp2.GetDefaultBin()`, and `GetDefaultBin` exits when quantity, item,
location, variant and routing link all match `xRec` -- which makes the whole idiom dead code unless
`xRec` is the record's own STORED image rather than a mirror of itself.

So the rule is not "who invoked the trigger". It is:

- **Every record carries the image it was last read, inserted or modified as.** Refreshed on
  hydration, at the end of a successful `Modify`, and -- WI-1156 -- at the end of a successful
  `Insert`.
- **A record variable nobody has touched has an EMPTY image**, not a mirror of itself. WI-1078
  separates the cases: `Init`/`Clear` set a blank image, a load sets the row, a fresh variable has
  none.
- **`Copy` copies the image with the record**, or the next `xRec` read is silently wrong.

## The choice

**What is built is the WRONG SHAPE and is recorded as such.** `detail::PushBefore`/`PopBefore` and
`detail::Before<T>()` in `runtime/Table.h` are a per-thread stack pushed by whoever invokes a
trigger -- which is exactly the trigger-scoped design WI-1078 refutes. It compiles, it refuses
loudly, and it is replaced by the image.

The image is a member of the GENERATED class, not of `Table<Derived>`: the base holds no data,
because that is what keeps every generated class standard-layout and `offsetof` over the field table
depends on it. A `std::shared_ptr<Derived>` beside the fields costs eight bytes per record and
nothing when no image was ever taken; the field table names only the field members, so the layout
the descriptors address is unchanged. The name comes from the interior-underscore seam, which no AL
name can reach.

WI-781 measured that only 45 of ~1580 tables declare their own `OnModify`, so the capture can be
`if constexpr`-narrowed later -- but not before it is right, because WI-1078 shows the readers are
not only triggers.

## Gate

A field trigger that reads `xRec` after a `Validate` sees the value the field had before. A record
read, changed and modified twice in a row sees its OWN previous row and not the one read before it
(WI-1156). A fresh variable's `xRec` is blank and not a mirror (WI-1078). `Copy` carries the image
(WI-1137). The negative control removes the capture and each must go red.

## `xRec` MAY SHARE STATE WITH `Rec`, AND THE PLATFORM SAYS SO

`devenv-system-defined-variables.md` (read 2026-09-04, board:0071) lists the six variables the
platform declares -- `Rec`, `xRec`, `CurrPage`, `CurrReport`, `RequestOptionsPage`, and `CurrFieldNo`
("**Retained for compatibility reasons**") -- and adds a warning this item should carry:

> **Avoid modifications to the `xRec` variable because the record might share some of the underlying
> state with the `Rec` variable** for performance and compatibility reasons, and **changes can
> unexpectedly propagate to the `Rec` variable.**

**So BC does not guarantee that `xRec` is an independent copy.** That matters here in both
directions: it means a runtime that makes `xRec` a full copy is SAFER than the platform and cannot
be wrong about a read; and it means any AL that writes to `xRec` is relying on undefined behaviour,
so reproducing the sharing would reproduce a hazard rather than a semantic.

The decision follows: **`xRec` is an independent copy here**, and the deviation is named. It costs one
record's worth of bytes at the trigger boundary, which board:0006 counts, and it removes a class of
defect the predecessor spent four items on (WI-781, WI-1078, WI-1137, WI-1156).

`CurrFieldNo` being "retained for compatibility" is worth noting too: it is the field number of the
current field during a validate, and the BaseApp still branches on it, so it is carried without being
extended.
