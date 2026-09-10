# The .NET regex family runs over `std::regex`

**Finding (2026-09-10).** `Regex Impl.` (System Application) is AL over `DotNet Regex`, `Match`,
`Group`, `Capture`, their collections, `RegexOptions` and `TimeSpan`; every member was an absent
stub, so the first `Regex(Pattern, Options, Timeout)` refused ("RegexOptions.=", 8 UT cases in
Country/Region UT, Currency UT, Data Exch. Def UT), and 16 BaseApp objects reach the same codeunit.

**Reference.** .NET `System.Text.RegularExpressions`; the members are the ones the implementation
names (measured 2026-09-10: `Matches`, `Replace` with count and start, `Split`, `IsMatch`,
`Escape`/`Unescape`, group names and numbers, `Result`, `Groups`, `Captures`, `CacheSize`).

**Choice.** `include/dotnet/Regex.h` over `std::regex` in ECMAScript grammar, which is .NET's
grammar for the patterns the BaseApp writes; named groups are rewritten to plain groups and the
names kept beside the numbers, since `std::regex` has none. `IgnoreCase` and `Multiline` map to
flags; `RightToLeft`, `ECMAScript`, `CultureInvariant` change nothing; `IgnorePatternWhitespace`
refuses; the timeout is carried. `Array` (what `Split` and the group names answer) and `TimeSpan`
come with it. A C++ library is allowed where the standard library is not enough, and here it is.
Gate `RegexGate`.
