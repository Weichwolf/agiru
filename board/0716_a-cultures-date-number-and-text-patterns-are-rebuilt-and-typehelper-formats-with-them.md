Type:     task
Status:   open
Parent:   0035
Area:     net
Class:    activation

# A culture's date, number and text patterns are rebuilt, and TypeHelper formats with them

**The population (2026-09-12, over `apps/`):** `DotNet CultureInfo` is named by 15 units, 10 in
the slice. Its NAME half is rebuilt (`include/dotnet/CultureInfo.h`: tag, LCID, ISO and Windows
names, parent, invariant and current culture, from a table of 52 cultures) because `Business
Chart Impl.` sets a `DataTable`'s `Locale` and `Language.GetCultureName` reads the tag. Its
FORMAT half is refused: `DateTimeFormat`, `NumberFormat` and `TextInfo` are `Refused` members,
and the chains behind them -- `DateTimeFormat.ShortDatePattern`, `NumberFormat.
NumberDecimalSeparator`, `TextInfo.ToTitleCase` -- are what `Type Helper` (`FormatDate`,
`FormatDecimal`, `Evaluate` with a culture), `Transformation Rule` (`Titlecase`) and the
`DotNet_CultureInfo` / `DotNet_DateTimeFormatInfo` wrappers reach.

**What the platform guarantees:** nothing -- these are .NET's globalization data, and what BC
does with them is `Format(Value, 0, FormatString)` with a culture-specific pattern.

**The choice:** `DateTimeFormatInfo` and `NumberFormatInfo` as small rebuilt classes carrying the
CLDR patterns for the same 52 cultures (short date pattern, date separator, decimal and group
separators, AM/PM designators), `TextInfo.ToTitleCase` over the runtime's own case folding, and
the `Refused` members replaced. No UT case fails on the refusal today (0 in run 129), which is
why the name half went first; `Type Helper.FormatDate` with a culture is where the first one
will.
