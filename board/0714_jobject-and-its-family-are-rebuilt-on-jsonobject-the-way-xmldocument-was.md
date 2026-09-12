Type:     task
Status:   open
Parent:   0035
Area:     net, gen
Class:    activation

# `JObject` and its family are rebuilt on `JsonObject`, the way `XmlDocument` was

**The population, measured 2026-09-12 over `~/Git/BCApps/src` and run 127:** `DotNet JObject` is
the third most named .NET type in the corpus (235 declarations, board:0035), and the hub is
`codeunit 5459 "JSON Management"` (753 lines) which the Graph tests, the workflow webhook tests,
`Image Analysis Result`, `SOAP Web Service Request Mgt.` and `Workflow Webhook Subscription` go
through. It names `JObject`, `JArray`, `JProperty`, `JToken`, `JValue`, `JsonConvert` and the
`GenericIEnumerable1`/`GenericIEnumerator1` walk over them. In the UT suite the family costs 10
cases of `Graph Collect Mgt Item UT` outright ("the .NET member JObject.JObject is named by AL and
not rebuilt here") and stands behind `ERM General Journal UT.TestCreateGenJnlBatchPostWebService`.

**What the platform guarantees:** nothing -- these are Newtonsoft classes, and the AL that names
them is a BC 14 relic kept alive beside the native `JsonObject`. The reference is Newtonsoft's own
surface: `JObject.Parse`, `Add`, `Remove`, `Property`, `GetValue`, `SelectToken`, `ToString`;
`JArray.Add`, `Count`, `Item`; `JProperty.Name`, `Value`; `JToken.Type`, `Value`; `JValue.Value`.

**The predecessor** mapped them onto Python dicts and lists and "bled on the semantic difference"
(CLAUDE.md); its `al_json.py` is 1 200 lines and was measured green on the Graph set.

**The choice:** one C++ class per .NET class under `include/dotnet/` (`JObject.h`, `JArray.h`,
`JToken.h`, `JProperty.h`, `JValue.h`, `JsonConvert.h`), each a thin face over `agiru::JsonObject`
/ `JsonArray` / `JsonValue` / `JsonToken` -- the rebuilt AL JSON types already hold the tree and
the parser -- with the members the corpus calls (`grep -oh "JObject\.[A-Za-z]*" apps -r | sort |
uniq -c` names them), the class-named binder member (`JObject.JObject()` is the constructor call
AL writes), and `Door.cpp`'s `kElsewhere` entries so the generated units find the headers. The
rule set is in memory `rebuilt-dotnet-type-rules.md`: full member set, door-spelling scan, check
the wrapper codeunits before the chain.

**Order:** after the UI-half of the `DirectedPutAway` summary and the SaaS testability flag
(batch190, 2026-09-12); before `DataTable`/`BusinessChartData` (5 cases) and
`WorkbookWriter`/`WorkbookReader` (6 cases), which are the next two families by count.
