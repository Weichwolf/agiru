# Assigning one record handle to another keeps the rows apart

**Finding (2026-09-10).** `Gen. Jnl.-Post Line` writes `TempGLEntryPreview := TempGLEntryBuf;
TempGLEntryPreview.Insert()` for every entry in its buffer. Both globals are held by
`Instance<Temporary<...>>`, and `Instance::operator=(const Instance &)` CLONED the other handle:
the clone's `StateHandle` copy shares the temporary store, so the preview's first `Insert` found
its own row already there -- "The G/L Entry already exists ... 2821" in 29 UT cases (Payment
Export Validation UT 19, Credit Transfer Register UT 5, ERM Remittance Report UT 3), and the
same shape behind "The Price Asset already exists" (11). No SQL `INSERT` preceded the refusal,
which is what told the temporary store apart from the table.

**Reference.** AL `Rec2 := Rec` copies the fields, the filters and the position and never the
rows (`record-copy-method.md`: only `Copy(From, true)` shares), which `StateHandle::operator=`
already does for plain variables; the handle's assignment bypassed it.

**Choice.** `Instance::operator=` routes a record-typed handle through `T::operator=` on the
held instance and keeps the clone for codeunit handles, whose two variables are two instances.
Gate case in `TemporaryGate`. Silent-wrong-data.
