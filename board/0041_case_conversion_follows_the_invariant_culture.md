Type: leaf
State: open
Area: net

# Case conversion covers every cased script, not only ASCII

`Text.ToLower`, `Text.ToUpper` and the free `LowerCase`/`UpperCase` change `A`..`Z` and `a`..`z` and
leave everything else alone. A German BaseApp caption is full of `Ä`, `Ö`, `Ü` and `ß`; a Nordic one
of `Æ` and `Ø`. Every one of them passes through unchanged today, so a comparison that lowers both
sides still separates `STRASSE` from `Straße` -- and a filter or a lookup built on that finds
nothing while looking correct.

## Reference

`text-tolower-method.md` says only "Returns a copy of this string converted to lowercase". The
platform is .NET and .NET's `ToLower()` is culture-aware, `ToLowerInvariant()` culture-independent;
BC's own `UPPERCASE`/`LOWERCASE` are documented as locale-independent in
`methods-auto/text/text-uppercase-method.md`. So the target is the INVARIANT mapping: Unicode's
simple case folding, one code point to one code point, no locale table and no Turkish dotless `i`.

`~/Git/openerp/` got this for free -- Python's `str.lower()` is Unicode-aware -- which is why the
predecessor's backlog says nothing about it. Here it is a table.

## The choice

The simple case mapping is a `constexpr` table derived from `UnicodeData.txt`: fewer than 1 500
entries with a non-empty simple lowercase or uppercase mapping, most of them in contiguous runs
(`0x0041`..`0x005A`, the Latin Extended blocks, Greek, Cyrillic). Emitted as sorted ranges with a
delta, it is a few kilobytes of `.rodata` and a binary search -- which is what "static const data"
means here, and what keeps ICU out of the dependency list for a job that is one table lookup.

**Not a locale-aware `ToLower(culture)`.** AL exposes no culture on these methods, and the two the
platform documents as locale-independent are the ones the BaseApp calls.

## Gate

`TextMethodGate` claims ASCII today. It gains: a Latin-1 letter lowers, a Greek letter lowers, the
final sigma stays out of it (it is a special mapping, not a simple one), and a character with no
case mapping survives untouched.
