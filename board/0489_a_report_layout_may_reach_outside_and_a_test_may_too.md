Type:     task
Status:   open
Parent:   0063
Area:     gen, rt
Source:   developer/properties/devenv-enableexternalimages-property.md, developer/properties/devenv-enablehyperlinks-property.md, developer/properties/devenv-testhttprequestpolicy-property.md, developer/properties/devenv-tableno-property.md, developer/properties/devenv-formatevaluate-property.md
Verdict:  teilweise
Class:    activation

# A report layout may reach outside, and by default a test may too

**Five pages, one item**: three about whether code or a layout may reach OUTSIDE the process, plus two
remainders. The first three are one security question asked on two object kinds.

> **EnableExternalImages** (Report, default **false**): whether external images are allowed. **"It
> applies to RDLC report layouts."** With an embedded image the source is compiled into the report;
> **"if a report only uses embedded images, you can set this to false."**
>
> **EnableHyperlinks** (Report, default false): whether links to other URLs are allowed.
>
> **TestHttpRequestPolicy** (Codeunit, runtime 15.0): **`BlockOutboundRequests`** -- any HTTP request
> not caught by a handler **raises**; **`AllowOutboundFromHandler`** -- all requests must be caught,
> and the handler may explicitly fall through to the real endpoint; **`AllowAllOutboundRequests`** --
> **"By default, all outbound requests are allowed."**
>
> **TableNo** (Codeunit): the source table. **"Setting `TableNo` changes the SIGNATURE of the `OnRun`
> trigger to include a `var Record` parameter named `Rec`."**
>
> **FormatEvaluate** (XmlPort): `Legacy` (standard AL data types) or `Xml` (standard XML data types)
> -- board:0442's format question from the value side, deciding whether an imported value is evaluated
> as AL or converted from XML.

**Three properties, one boundary, and the defaults disagree.** A report fetching an external image and
a test issuing a real HTTP request are the same class of thing -- the process reaching a network it
did not have to. Reports default to REFUSING; tests default to ALLOWING.

**That asymmetry is the finding**: an agiru test suite would issue real outbound requests by default,
which is a hermeticity hole in the 2 291 and a reason a test run could depend on a network.
board:0200's `[HttpClientHandler]` is the mechanism this property gates.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`TableNo =` **1 341** · `FormatEvaluate =` **144** · `TestHttpRequestPolicy =` **45** ·
`EnableHyperlinks =` **23** · `EnableExternalImages =` **2**.

## The IST-state, and it is why this is `teilweise`

`src/gen/CodeunitWriter.cpp` consumes **`TableNo`** -- one of the nine properties the generator knows
(board:0067) -- and uses it to decide `OnRun`'s signature, which is exactly what the page describes.
**So one of the five works.**

The other four are unread; reports and XMLports have no generator.

## The choice

`TestHttpRequestPolicy` is an enumerator on the test codeunit read by board:0200's handler mechanism,
**with the documented default of allowing everything** -- and this item records that `agiru run-tests`
should consider `BlockOutboundRequests` as its own default, which is a deliberate deviation from BC
and has to be argued rather than assumed.

The two report bits gate the layout renderer's outbound fetches. `FormatEvaluate` goes with
board:0442's format.

## Ordering

`TableNo` is done. `TestHttpRequestPolicy` with board:0200 and board:0039; the report bits inside
board:0063; `FormatEvaluate` inside board:0065.

## Gate, and its negative control

A test declaring `BlockOutboundRequests` raises on an unhandled HTTP request; a report declaring
`EnableExternalImages = false` does not fetch a remote image its layout references.

**The negative control is a test declaring NOTHING** -- under BC's default it may reach the network,
and whichever default agiru picks, the gate must assert the chosen one explicitly rather than assert
BC's.
