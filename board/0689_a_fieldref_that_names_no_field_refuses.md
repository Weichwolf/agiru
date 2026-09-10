# 0689 A FieldRef that names no field refuses

**The finding.** Chain 106's milestone came back with a DENOMINATOR THAT MOVED -- 2 268 of 2 296,
77 codeunits instead of 78 -- because `API Setup UT` left with **exit 139**, a segmentation fault,
and took its 28 procedures out of the count. A moved denominator is an abort and not a pass, so
the +60 it also reported is not a milestone until this is closed.

**The cause.** `FieldRef::Value()` read `def_->type` on a FieldRef that
`DataTypeManagement.FindFieldByName` had never bound -- the AL ignores that method's Boolean and
asks for the value regardless, which is legal AL and a null read here. Every other accessor was
the same: `Number()`, `Name()`, `Caption()`, `Length()`, `Class()`, the enum accessors, and eight
places in `src/rt/RecordRef.cpp`.

**The door had already promised otherwise**, in its own words beside the default constructor: "AL
DECLARES ONE BEFORE IT HAS ONE ... so the declared state is empty and every question asked of it
before the assignment refuses rather than reading a null." The comment was right and the code did
not do it -- a silent place with a Doxygen paragraph in front of it.

**The choice.** `Def_()` and `Table_()` are the only ways to the two pointers, and each throws an
AL `Error` when it is empty. A crash is the one failure mode a test suite cannot report around:
it takes the whole worker with it, which is why one unbound FieldRef cost 28 procedures rather
than one.

**Measured.** Chain 107: the denominator must be 2 296 over 78 again, and `API Setup UT` must
report a total.
