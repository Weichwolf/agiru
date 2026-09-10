# A part naming an out-of-scope page refuses its `.PAGE` methods

**Finding (2026-09-10).** `Item List` and the role centres carry `part(PowerBIEmbeddedReportPart;
"Power BI Embedded Report Part")`; the part's page lives in `System.Integration.PowerBI`, which
`scope.json` EXCLUDES (the cloud glue carve-out the predecessor's WI-990 recorded), so the
generator types the part as `PartRef<Page<>>` and `PowerBIEmbeddedReportPart.PAGE.SetPageContext`
refuses with "has no object behind it" (9 UT cases; `Intelligent Cloud` under `System.AI` is the
same shape). The transpiler prints no failure for it because the file never enters the run: it
is filtered by scope before parsing, which is correct and was checked by parsing the five files
directly.

**Reference.** `scope.json` and its `_doc`; the pages parse and would generate.

**Choice (the user's).** Widening the scope to `System.Integration.PowerBI` for the embedding
part alone would carry the whole Power BI namespace in; the alternative is a part over an absent
page that answers its `.PAGE` methods as no-ops on premises, the way `Office Management` answers
"not running". Nothing is built until the scope owner decides; the 9 cases stay counted here.
