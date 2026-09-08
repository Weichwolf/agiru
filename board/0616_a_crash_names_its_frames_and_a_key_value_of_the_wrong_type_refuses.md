Type:     root
Status:   open
Area:     rt, cli
Source:   three UT codeunits that stopped reporting after board:0614's round, 2026-09-08
Class:    activation

# A crash names its frames, and a key value of the wrong type refuses

**THREE CODEUNITS LEFT THE DENOMINATOR AND NOTHING SAID SO: 1 708 became 1 605.** The harness
counted a codeunit when it printed `N of M passed` and skipped it otherwise, so a segmentation
fault between two tests looked like a SMALLER SUITE rather than a crash -- which is CLAUDE.md's
"baseline that falls by accident" with the denominator moving instead of the numerator.

**THE HARNESS COUNTS A SILENT CODEUNIT AS A LOSS AND NAMES IT.** That is the first fix and it is
not optional: a measurement that can shrink its own population proves nothing.

## The crash printed nothing, so the runner learned to print

`agiru` installs a handler for `SIGSEGV`, `SIGBUS`, `SIGFPE`, `SIGILL` and `SIGABRT` that writes
its frames with `backtrace_symbols_fd` and leaves with `128 + signal`. Two details are load-bearing
and both were paid for in one round:

- **IT RUNS ON AN ALTERNATE STACK** (`sigaltstack` + `SA_ONSTACK`). The first version used
  `std::signal` and printed NOTHING for the second crash, because that crash was a STACK OVERFLOW
  and a handler on the overflowed stack cannot run.
- **IT IS GUARDED BY `__has_include(<execinfo.h>)`** and does nothing where that header is absent,
  because the target is every architecture this builds for and not this box.

## Two defects, and both were silent-wrong-data one step from a crash

**1. `AsText` AND `Format` CALLED EACH OTHER FOREVER.** `AsText<T>`'s last branch was
`return Format(value)`, and the non-Variant `Format<T>` is defined as `return AsText(value)` -- so
any type with no text conversion, no `ToText()`, not arithmetic and not an enumeration recursed
until the stack ran out. `Format(FieldRef)` is such a type and the BaseApp writes it. The fallback
REFUSES now, by name; a chain that cannot terminate is not a fallback.

**AND A `FieldRef` RENDERS AS ITS VALUE**, which is what AL's `Format(Any)` does with one.

**2. `Get(RecordId)` WROTE A `RecordId` OVER A `Code[20]`.** `AssignKey` reinterpret-casts the
field's storage to the CALLER's type, so `ItemCategory.Get(RecId)` -- which the BaseApp writes in
`ItemCategoryManagement.GetLastChildCode` -- copied a `std::string` and a `std::vector` over twenty
bytes of a record. That is memory corruption whose crash arrives three frames later, in
`std::char_traits<char>::copy`.

Two fixes, and the second is the one that matters:

- **`Record.Get(RecordId)` IS ITS OWN OVERLOAD**, and it reads the key values the id carries.
  `record-get-method.md` does not spell it out; it spells out the near miss -- a key field of type
  RecordID cannot be fetched this way, "because RecordId already is the primary key itself and not
  one of the fields that forms it, as the method expects".
- **`AssignKey` REFUSES A VALUE WHOSE TYPE IS NOT THE FIELD'S.** `FieldTypeOf<Key>::kType` against
  `def->type`, where the trait exists. The same mistake anywhere else is now an error naming the
  field instead of a corrupted record.

**AND A `RecordId` CARRIES THE STORAGE FORM OF ITS KEY**, not the display form, because
`Get(RecordId)` has to read it back: `FieldText` renders an Option as its member NAME and
`SetFieldText` reads an ordinal. That is the same pair `TransferFields` was corrected for one round
earlier, and the third time it has cost a round.

## What proves it

Every UT codeunit reports a line: 62 of 62, and the denominator stays at 1 708. The negative
control is the harness itself -- with the LOST branch removed, the three codeunits vanish from the
count instead of appearing in it.
