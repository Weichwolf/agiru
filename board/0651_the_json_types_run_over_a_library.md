# The JSON types run over a library

**Finding (2026-09-10).** With posting reaching the end of Sales-Post, `JsonObject.Add(Text,
Text)` refuses 84 UT cases (ERM Sales Invoice Aggregate, Sales/Purch. Cr. Memo Aggr., Credit
Transfer Register UT): the AL JSON door is declared and refuses -- `JsonObject` 64 of 65
methods, `JsonArray` 86 of 86, `JsonValue` 36 of 36, `JsonToken` 14 of 14 -- with no engine
behind it. The generated tree calls `.Add(` 68 times in the UT codeunits alone and reads
through `AsValue`/`AsText`/`AsDecimal`/`Get`/`SelectToken`.

**Reference.** `methods-auto/jsonobject/` .. `jsontoken/`: `ReadFrom(Text | InStream)`,
`WriteTo(Text | OutStream)`, `Add(Key, Value: Any)`, `Get(Key, var Token)`, `Contains`,
`Replace`, `Remove`, `Keys`, `Values`, `SelectToken(Path, var Token)` (a JSONPath subset),
`AsValue().AsText()/AsInteger()/AsDecimal()/AsBoolean()/AsDate()/AsDateTime()/IsNull()`,
`JsonArray.Add/Get/Insert/Set/Count`. A JSON value is a reference, like the XML types.

**Choice, pending one decision.** CLAUDE.md names JSON among the four things not written from
scratch and asks for a dependency reachable on every architecture: `libjsoncpp-dev` is in the
Debian archive (candidate 1.9.6-3) but not installed, and `nlohmann-json3-dev` neither. The
engine goes in `src/net` behind an intrusive handle the way the XML types do (board:0646), with
the .NET-free AL surface only. **Installing the package is the user's call**; until it is
there this item waits, and the milestone's ceiling carries the 84.
