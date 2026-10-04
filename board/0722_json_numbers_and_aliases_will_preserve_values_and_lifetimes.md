# 0722 — JSON numbers and aliases will preserve values and lifetimes

Status: open | Priority: P0 | Stage: UT; prerequisite for client codecs | Reviewed: 2026-10-03
Depends on: 0066 CLR Decimal calculation precision.

## Evidence

- System yyjson 0.10.0 replaces the vendored header. Parsing/serialization use yyjson; the private C++ ownership layer retains stable nodes independently of ordered container membership. Raw number lexemes never pass through double. No backend header or `<memory>` reaches `JsonHandle.h`.
- Current JsonGate: 231 checks/zero red; JObjectGate: 43/zero red. Both pass ASan/UBSan with leak detection. Receipts: `/tmp/agiru-yyjson-asan-{al-retry,dotnet}.log`, `/tmp/agiru-yyjson-{sanitizer,negative}-compile-commands.json` and `{positive,negative}-inputs.sha256`. Disposable sanitized/negative source copies and binaries are removed; logs/hashes/commands remain.
- Original HEAD `108944f` with the new public-API fixture fails exact large-number conversion; running its retained-node case first reproduces ASan heap-use-after-free. `/tmp/agiru-yyjson-negative-{old,alias}.log`. Negative fixtures are outside the repository, not production source.
- The first expanded contract run has 227 checks/four red: malformed numeric string, SetValue disconnect/alias retention and Decimal/BigInteger setter representation. The corrected run is green. `/tmp/agiru-yyjson-contract-{before,after}.log`.
- AL SetValue disconnects its variable; Decimal/BigInteger setters store strings, while Add Decimal emits a number. Integer/Int64 refuse overflow or inexact conversion; Boolean requires its declared type. Both adapters share checked path indices. Object rename preserves unaffected node identities.
- JSON parsing/serialization have an explicit 256-level bound; the number adapter checks 28-place scale/29-digit magnitude and Int64 limits. `Json.cpp`/`DotNetJson.cpp` then call the scale-20 Decimal core (0066): retained lexemes do not prove exact high-scale typed conversion/equality. Programmatic insertion/copy bounds, performance and WASM remain unproved.
- Full replay `20261003T172807Z-701995`: 2,160/2,314 passed, 154 failed, zero incomplete/crashed; unchanged identities, one XML gain and zero losses. `/tmp/agiru-json-xpath-ut-comparison.json`. This does not close the remaining JSON contracts or sealed-seed A/B proof.

## Implementation

1. Complete distinct AL/.NET insertion and replacement contracts. Existing Add still copies supplied trees; unparented Newtonsoft tokens instead retain identity, while already-parented/self/ancestor tokens clone. Existing JToken.Replace mutates the referenced value rather than replacing/detaching the parent's child. Do not call either behaviour proven parity.
2. Add owned parent/membership tracking for correct detached Root/Path and replacement semantics, without cycles or retained-node use-after-free. Root currently returns the original document; Path is still an empty successful result and must become correct or explicitly refuse. Keep AL SetValue disconnection separate from .NET token mutation.
3. Implement remaining AL token/value ReadFrom/WriteTo/AsToken/Clone refusals through the same engine. Prove depth boundaries, Unicode/escaping, complete numeric limits and alias/clone contracts; close the 0066 scale-20 core dependency instead of describing lexical validation as typed Decimal parity. Measure document/mutation allocation cost and prove WASM dependency integration.
4. Regenerate and replay the unchanged full source-counted UT population after activation; investigate every loss. Private engine targeted analysis is green; wrapper/header findings remain unclosed, not suppressed.

## Acceptance

- ASan/UBSan probes pass; alias writes remain visible where required, Clone is independent, removed-node behavior is explicit and memory-safe.
- Exact large/high-scale/signed/exponent number round trips; invalid, overflow and inexact inputs refuse. Reintroducing double parsing or unstable child pointers fails dedicated gates.
- Run JSON/.NET gates and unchanged full UT population. Client codecs in 0720 use these exact values, never JS Number for Decimal/BigInteger.

## References

Code: `include/type/JsonHandle.h`, `src/net/{JsonEngine,Json,DotNetJson}.cpp`; gates: `test/gate/{Json,JObject}Gate.cpp`, `test/tooling/header-dependencies.sh`. Platform revision `ff5939a46e`: `methods-auto/jsonvalue/jsonvalue-{asdecimal,asinteger,asbiginteger,asboolean,setvalue-decimal,setvalue-biginteger,setvalue-integer,setvalue-string}-method.md`; JSON data types and Clone/Remove/Add overloads. BCApps `bb7111877f`: `src/System Application/Test/Json/src/JsonTest.Codeunit.al::TestGetDecimalPropertyValueFromJObjectByName`. User import intent adds no JSON alias guarantee. Predecessor 1347 distinguishes setter strings from Add numbers; 1376 rejects silent numeric loss; 1346/1358 cover refusal boundaries. Newtonsoft 13.0.3 original `Linq/JContainer.cs::EnsureParentToken` and `Linq/JToken.cs`; yyjson 0.10.0 original `doc/API.md` and installed header. Split from 0035; no separate .NET-only engine.
