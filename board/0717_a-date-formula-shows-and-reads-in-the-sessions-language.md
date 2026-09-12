Type:     task
Status:   open
Parent:   0035
Area:     net, rt
Source:   developer/methods-auto/dateformula/dateformula-data-type.md
Class:    silent-wrong-data

# A date formula shows and reads in the session's language

**What the platform guarantees** (`dateformula-data-type.md`): a DateFormula is stored
language-independently -- `<1Y>` -- and "the letters in the formula are translated" to the
session's language when shown and when read: `1J` in a German session, `1Y` in an English one,
`1A` in a French one; `Evaluate(DateFormula, '1J')` under `GlobalLanguage(1031)` is `<1Y>`.

**What this tree does (2026-09-12):** `DateFormula::ToText()` renders the invariant letters in every
language, `FromText` reads only them, and `Format(DateFormula)` is that text -- so a German session
sees `1Y`. One UT case measures it (ERM General Journal UT `RecurringFrequencyDisplaysLocalized
GermanDateFormula`, Layers/CH), and every non-English page of the UI phase will.

**The shape:** the formatting language is the value layer's own `thread_local` (`type/Language.h`,
set by `GlobalLanguage` in the runtime -- `src/net` reaches nothing and must not read the session),
`DateFormula::DisplayText(language)` and `FromText(text, language)` translate through a table of
letter sets -- current, day, week, month, quarter, year -- for the languages BC ships: EN `C D W M
Q Y`, DE/AT/CH `L T W M Q J`, DA `L D U M K Å`, NL `H D W M K J`, FR `C J S M T A`, IT `C G S M T A`,
SV `L D V M K Å`, NB `L D U M K Å`; `Variant::Rendered` and `Evaluate` go through them, the column
keeps `<1Y>`. A language the table lacks reads and shows the invariant letters, which is what BC
does for a language without a translation.
