# 0692 AL `File` is a file

**The finding.** Every method of AL's `File` type was a declaration with no body: the def-unit
machinery gave each one a stub that throws, and 46 UT cases stop at
`the AL procedure agiru::File::CreateTempFile() is declared and its source is not in the slice`.
Eight of those were GREEN before chain 115 -- by accident, because an earlier step refused and the
test's `asserterror` matched that refusal; with the earlier step working they reach the real gap.

**The shape it is written to.** The header already decided it: `Blob held_`, `name_`, `position_`,
`open_`, `textMode_`, `writeMode_`. So a file is its BYTES while it is open, and `Close` writes
them; `CreateInStream` and `CreateOutStream` hand out streams over the same Blob, which is what
makes `File.Open(Name); File.CreateInStream(In)` work without a second copy.

- **`CreateTempFile()` makes a name nothing else holds**, under the system temporary directory,
  and creates the file -- `file-createtempfile-method.md` says it creates AND opens one, so
  `Exists` answers true straight after and `IsPathTemporary` answers true for the name.
- **`Read(var Text)` is a LINE in text mode** and returns how many bytes it consumed, the line
  break counted, which is what the BaseApp's import loops test against zero.
- **`Pos()` is one-based and `Seek` is zero-based**, as `file-pos-method.md` and
  `file-seek-method.md` have it.
- **The static half answers rather than raising**: `Exists`, `Erase`, `Copy`, `Rename` return a
  Boolean, so a file that is not there is `false` and not an error.

**The proof.** `test/gate/FileGate.cpp`: a temporary file is made, written, closed, opened again,
read back line by line in order, and erased; a stream over an open file reads what the file holds.
The negative control was run -- with the creation neutered the gate goes red.

**Measured.** Chain 116, A/B against chain 115's 1 682.
