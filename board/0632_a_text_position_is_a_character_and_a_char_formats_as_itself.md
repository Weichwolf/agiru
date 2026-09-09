Type:     task
Status:   open
Parent:   0041
Area:     net
Source:   devenv-text-data-type.md; char-data-type.md; text-strlen-method.md
Verdict:  measured
Class:    silent-wrong-data

# A text position is a character and a `Char` formats as itself

**Two findings from one run, 2026-09-09, both about what a `Text` holds.**

- **A `Char` above 127 appended as ONE BYTE.** `'A' + Char(133)` wrote the byte 0x85 into a
  `Payment Export Data` row and PostgreSQL refused the insert as invalid UTF-8, which aborted the
  transaction and turned every later `Find` of that test into `false`. Fixed at the two append
  operators and at `Text[Index] := Char` with `Encoded(Char)`, gated in `TextBuiltinGate`.
- **A `Variant` holds a `Char` AS ITS CODE POINT**, so `Format(C)` renders `65` where AL renders
  `A`. `char-data-type.md`: "A Char variable represents Unicode characters in the same way as the
  .NET Framework Char structure", and .NET's `Char.ToString()` is the character. Not yet fixed.

**And the one behind both: `Text[Index]`, `StrLen`, `CopyStr` and `StrPos` count BYTES here and
CHARACTERS in AL.** `text-data-type.md` counts a Text in characters and every position method with
it; this runtime keeps UTF-8 and indexes the byte array, so `StrLen('Zürich')` answers 7 and
`CopyStr('Zürich', 1, 3)` cuts the `ü` in half. Every W1 test with a Latin character on a position
is wrong by the number of multi-byte characters before it.

## References

- `~/Git/openerp` kept Python `str`, which indexes code points, so it never met this; its board
  has nothing to say and the documentation has everything.
- `.NET Char` is a UTF-16 code unit; AL's `Char` is that. A code point above U+FFFF is two of
  them in AL and four bytes here -- the one place where "character" and "code unit" part, and
  the BaseApp does not reach it (measured: no surrogate literal in `Layers/W1`).

## The choice

The Text keeps UTF-8 -- it is what the database, the JSON and the XML layers speak -- and every
POSITION converts: a character index walks the encoding to its byte offset, once per call, at the
cost of one linear pass per position method on a text that contains a multi-byte character and no
cost on one that does not (a byte count equal to the character count is the common case and is
detected in the same pass). `StrLen` counts code points. A `Variant(Char)` holds the character's
text and answers `IsChar` true.

## Gate, and its negative control

`StrLen('Zürich') = 6`, `CopyStr('Zürich', 1, 3) = 'Zür'`, `StrPos('Zürich', 'r') = 3`,
`Format(Char(228)) = 'ä'`. The negative control is the ASCII text beside each, which a byte
implementation answers the same way -- the case proves nothing unless the multi-byte one is red
before the change.
