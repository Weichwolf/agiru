# 0722 — JSON numbers and aliases will preserve values and lifetimes

Status: open | Priority: P0 | Stage: UT; prerequisite for client codecs | Reviewed: 2026-09-28
Depends on: none.

## Evidence

- `JsonHandle::node` points into ordered_json vector storage. Retain a child, append 128 siblings, read child: ASan heap-use-after-free in `JsonValue::AsInteger`.
- Decimal 999999999999999.99 serializes as 1e+15 and AsDecimal refuses; 0.1234567890123456789012345678 silently becomes 0.12345678901234568. Positive control 1.25 survives.
- Reproduction: `build/review-20260928/JsonProbe.cpp`, `json-{alias,decimal}.log`; source is the frozen 20260928T142909Z-7923 lane.

## Implementation

1. Replace raw pointers into relocating containers with stable owned node identities. Keep member order separate from node storage; aliases retain the referenced node. Define detach/replace/remove behavior from AL/.NET contracts, not vector invalidation. A path/index alone is not stable under edits.
2. Parse JSON numbers without a double intermediate; preserve an exact number token and checked integer/Decimal conversions. Serialize Decimal directly as an exact JSON number, not a quoted workaround. Reject out-of-range/inexact conversion explicitly.
3. Use the engine through distinct AL Json* and .NET JToken/JObject contracts. Cover Add/ReadFrom/WriteTo/Clone/Replace/Remove and retained children across parent mutation/destruction; never deep-copy silently to hide alias bugs.

## Acceptance

- ASan/UBSan probes pass; alias writes remain visible where required, Clone is independent, removed-node behavior is explicit and memory-safe.
- Exact large/high-scale/signed/exponent number round trips; invalid, overflow and inexact inputs refuse. Reintroducing double parsing or unstable child pointers fails dedicated gates.
- Run JSON/.NET gates and unchanged full UT population. Client codecs in 0720 use these exact values, never JS Number for Decimal/BigInteger.

## References

Code: `include/type/JsonHandle.h`, `src/net/{Json,DotNetJson}.cpp`, `src/net/JsonEngine.h`. Platform: `methods-auto/jsonvalue/jsonvalue-asdecimal-method.md`, `methods-auto/jsontoken/jsontoken-data-type.md`, JsonObject.Add Decimal overload, Decimal data type. BCApps main: `src/System Application/Test/Json/src/JsonTest.Codeunit.al::TestGetDecimalPropertyValueFromJObjectByName`. Predecessor: WI-1376 rejects silent numeric loss; WI-1346/1358 cover JSON refusal boundaries. Split from 0035; no separate .NET-only JSON engine.
