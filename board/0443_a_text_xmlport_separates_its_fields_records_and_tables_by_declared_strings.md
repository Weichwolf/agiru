Type:     task
Status:   open
Parent:   0065
Area:     rt, gen
Source:   developer/properties/devenv-fieldseparator-property.md, developer/properties/devenv-fielddelimiter-property.md, developer/properties/devenv-recordseparator-property.md, developer/properties/devenv-tableseparator-property.md
Verdict:  fehlt
Class:    activation

# A text XMLport separates its fields, records and tables by declared strings

**Four pages, one item**: the three separators and the delimiter are one text-serialisation model,
all on the XMLport, all conditional on board:0442's `Format`, and all sharing one value vocabulary.

> | value | means |
> |---|---|
> | `<None>` | no separator |
> | `<NewLine>` | **any combination of CR and LF** |
> | `<CR/LF>` | CR followed by LF |
> | `<CR>` / `<LF>` | one of them alone -- ASCII 13 and 10 |
> | `<TAB>` | tabulator |
> | other strings | the literal string |
>
> **The strings must be entered literally, that is, the `<` and `>` characters must be entered.**
> These special strings **can be combined and can be mixed with other characters.**
>
> `FieldDelimiter` default is `""`, an empty string. `RecordSeparator` default is `<NewLine>`.
>
> **You can set the separator IN AL CODE so that the XMLport can import and export records with
> different separators** -- change the property at run time.
>
> **NOTE: Do not use a semicolon in a quoted argument as another property's value when you import a
> file through an XMLport. Even in a quoted argument, THE SEMICOLON IS INTERPRETED AS A RECORD
> SEPARATOR.**

Four things follow, each one an implementation would otherwise get wrong:

1. **`<NewLine>` is a SET, not a string** -- "any combination of CR and LF" -- so reading it as `\n`
   splits a CRLF file into empty rows.
2. **The values compose**: `'<CR><LF>x'` is legal, so the value is parsed into a byte sequence rather
   than matched against a table of six.
3. **The properties are ASSIGNABLE at run time**, so they are not `constexpr` metadata like everything
   else in this sweep -- they are the XMLport object's mutable state with the declaration as the
   initial value.
4. **The semicolon note is a documented defect in BC's own parser** and it is recorded rather than
   reproduced: an agiru XMLport that handled a quoted semicolon correctly would read a file BC
   cannot, which is a deviation and has to be argued for.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`FieldSeparator =` **189** · `RecordSeparator =` **140** · `FieldDelimiter =` **47** ·
`TableSeparator =` **10**.

Against board:0442's 272 text-format XMLports, so most of them declare at least the field separator.

## The IST-state

XMLports have no generator (board:0065, board:0034).

## The choice

The declared value is parsed by the GENERATOR into a byte sequence and a match mode -- `<NewLine>`
becomes "one or more of CR, LF" and the rest become literals -- and stored as the XMLport member's
initial value, since AL may reassign it.

## Ordering

Inside board:0065, behind board:0442's format.

## Gate, and its negative control

A `VariableText` import of a CRLF file with `RecordSeparator = '<NewLine>'` yields one record per
line, and the same file with lone LFs yields the same records.

**The negative control is the CRLF file** -- treating `<NewLine>` as a single `\n` produces an empty
record between every pair, and an LF-only fixture never shows it.
