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

**Resolved 2026-09-10, and the decision is recorded because it was the user's to make.** The
blocker was `sudo apt install`, which this loop cannot do and which the user had not answered
after two askings. The library is therefore VENDORED rather than installed: `nlohmann/json`
3.11.3, one header, MIT, under `third_party/nlohmann/json.hpp` and on the include path of
`agiru_net` ALONE -- so the door never sees it, no generated unit includes it, and removing it is
one file and one CMake line. Header-only is why this needs no root and is reachable on every
architecture the tree builds for, which is the requirement CLAUDE.md sets for a dependency.

The engine is `src/net/Json.cpp` behind an intrusive handle, the way the XML types hold libxml2
(board:0646). What the generated tree actually calls decides what is implemented first, measured
over `apps/`: `AsValue` 268, `Add` 2 979 (52 on a JsonObject directly), `Get`, `AsInteger`,
`AsDecimal`, `Contains`, `AsText`, `Keys`, `ReadFrom` 91, `WriteTo` 60, `SelectToken`. The rest of
the 166 declared methods keep refusing until a case asks for them.

**Chain 110 measured the first cut at +4 and said why in one line, 106 times:** `this JSON value
refers to nothing`. An AL `JsonObject` variable is DECLARED and used --
`JsonObject.Add('key', X)` with nothing read into it -- because in AL a declared `JsonObject` IS
an empty object and a declared `JsonArray` an empty array. The handle is therefore made in the
member's own initialiser, per type, and a gate case declares one and adds to it. That is the
difference between a JSON engine and a JSON engine anyone can use.
