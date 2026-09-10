# `Codeunit.Run(Rec)` hands the record over with its filters, and back

**Finding (2026-09-10).** Once `Rec := Other` copied fields only (board:0659), `Codeunit.Run(
Codeunit::"Export Payment File (Yes/No)", GenJnlLine)` lost the caller's `SetRange` on the way
into the codeunit's `Rec`: its `OnRun` opens with `Rec.FindSet()` and read the first journal line
of the whole table (`Payment Export Validation UT.GenJnlLineDocNoGapWithNoSeries`, chain 87).

**Reference.** `codeunit-run-method.md`: the record parameter is passed by reference -- the
codeunit works on the caller's record, filters and position included, and the caller sees what
the codeunit left.

**Choice.** `Run(Rec)` / `Ok_Run(Rec)` copy the record in and back out with `Copy` (fields,
filters, key and position), through a handle where the caller holds one. Silent-wrong-data.
