Type: root
State: active
Area: net
Tags: dotnet

# The Encoding family is rebuilt for four encodings

**Finding (2026-09-10, chain 93).** 27 UT cases refuse on `UTF8Encoding.UTF8Encoding`, and
`DotNet Encoding` is declared 124 times in the AL source with `GetEncoding` (34), `GetBytes`
(11), `GetString` (7), `Convert` (7), `GetPreamble` (3) behind it -- every one an absent stub.

**Choice.** `include/dotnet/Encoding.h`: `Encoding`, `UTF8Encoding`, `UnicodeEncoding` and
`ASCIIEncoding`, over four encodings -- UTF-8 (65001), UTF-16 LE (1200), UTF-32 LE (12000),
ASCII (20127) -- and the single-byte pages read and written as Latin-1. A byte array is a
`dotnet::Array` of Integers 0..255, the way AL reads a `byte[]` back. `GetEncoding(0)` is UTF-8,
which is .NET Core's default too. Every factory goes through `Encoding::Made`, because a class
whose constructor AL spells as a member (`E.Encoding()`) may declare no constructor of its own
(the rebuilt-.NET-type rules).

**Not done.** Real code-page tables (1252 and its siblings map to Latin-1 here); a character
above 255 in a single-byte page becomes `?`, as .NET's replacement fallback does.

**Measurement.** Chain 95 A/B against chain 93.
