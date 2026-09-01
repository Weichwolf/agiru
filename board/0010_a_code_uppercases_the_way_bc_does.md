Type: bug
State: open
Area: net

# A `Code` uppercases the way BC does, including outside ASCII

`Code<N>` uppercases ASCII letters and leaves everything else alone. `code-data-type.md` says
"converted to uppercase" and, two paragraphs later, "The Code data type supports Unicode". Those two
sentences together say more than the implementation currently does.

## Reference

**Platform documentation**: `methods-auto/code/code-data-type.md` states the conversion and the
Unicode support, and does not say which casing rules apply. `methods-auto/text/text-uppercase-method.md`
documents `UpperCase(Text)` as "Converts all letters in a string to uppercase" -- also without a
culture.

**The .NET side**: BC strings are .NET strings, and .NET has two different answers --
`ToUpper()` uses the current culture and `ToUpperInvariant()` does not. They disagree on real
letters: Turkish `i` uppercases to `İ` under `tr-TR` and to `I` under the invariant culture. A
Turkish installation with a Code field containing `i` therefore stores a different key depending on
which one BC calls.

**AL source**: not decisive -- the BaseApp never uppercases a Code itself, it relies on the platform
doing it on assignment.

**Predecessor**: openerp used Python's `str.upper()`, which is full Unicode and locale-independent
-- a third answer again, and one nobody there measured against BC.

**Why it is not urgent and not ignorable:** Code fields in the BaseApp hold document numbers, item
numbers, posting groups and dimension codes -- ASCII in every demo dataset and nearly every
installation. But a Code field is a PRIMARY KEY. Getting its normalisation wrong does not produce a
wrong display, it produces a record that cannot be found.

## What will be true

- [ ] The casing rule is settled from a document or from a real BC, not chosen.
- [ ] `Code<N>` follows it, and the header says which of the three answers it is and on what
      evidence.
- [ ] Proof: gate cases over a non-ASCII letter whose uppercase differs between the invariant and a
      named culture.
- [ ] **Negative control**: switch to the other rule and require those cases to go red. If nothing
      falls, the cases are not testing the distinction.
