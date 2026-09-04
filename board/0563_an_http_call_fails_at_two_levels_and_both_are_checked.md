Type:     task
Status:   open
Parent:   0028
Area:     net
Source:   developer/devenv-httpclient.md
Verdict:  deklariert
Class:    activation

# An HTTP call fails at two levels, and both are checked

`HttpClient` is fully declared in the door and every method refuses (`src/rt/Door.cpp:892`-`:999`),
which is the honest state CLAUDE.md asks for. **This item is what has to be true when the refusals are
replaced**, and the one thing the page insists on that an implementation gets wrong for free.

## Two levels of failure and they are different questions

```AL
IsSuccessful := Client.Get('https://...', Response);
if not IsSuccessful then
    // the TRANSPORT failed -- no response exists
if not Response.IsSuccessStatusCode() then
    // the SERVER answered, with 4xx or 5xx
Response.Content().ReadAs(ResponseText);
```

**`Get` returning `true` means a response arrived, not that it was a good one.** A 500 is a successful
call. The page states it as the thing to watch: *"The service call itself might not succeed, so make
sure that you check the HTTP status code in your AL code."*

An implementation that mapped the HTTP status onto the return value would be more convenient and would
break every one of the 122 `IsSuccessStatusCode` call sites, because the second `if` would then never
fire. **The Boolean is the transport; `IsSuccessStatusCode` is the status; `HttpStatusCode` and
`ReasonPhrase` are the detail.** The door already has all four with the right signatures --
`Boolean Get(Text, HttpResponseMessage&)` (`include/type/HttpClient.h:77`),
`Boolean IsSuccessStatusCode()` (`HttpResponseMessage.h:75`) -- so this is a rule about the BODY, not
about the shape.

## Headers are not a map, and they arrive by `var`

```AL
Content.GetHeaders(ContentHeaders);
if ContentHeaders.Contains('Content-Type') then ContentHeaders.Remove('Content-Type');
ContentHeaders.Add('Content-Type', 'multipart/form-data;boundary=boundary');
```

**`Add` does not replace**, so the documented idiom is `Contains` then `Remove` then `Add` -- BC's own
example does exactly that for `Content-Type`, which the platform sets by default. So `HttpHeaders` is a
MULTI-MAP: one name may carry several values, which is what HTTP itself allows.

**`GetHeaders` is an out parameter**, on both `HttpContent` and `HttpRequestMessage`, and so is
`ReadAs`. That is CLAUDE.md's first named trap -- *"a builtin with a `var` parameter that sets the
value only locally"* -- and it is closed in C++ only as long as the generator never copies the
argument. **200 `GetHeaders` and 162 `ReadAs` call sites depend on that**, and the same mechanism
board:0516 needs for `IsHandled`.

**The headers a `GetHeaders` hands back are a VIEW, not a copy**: the example mutates
`ContentHeaders` and expects the change to reach `Content`. So the out parameter is a reference into
the owning object and its lifetime is the owner's.

## The policy agiru has to take a position on

**Server certificate validation is on by default and, from version 27, cannot be turned off except per
call**: `HttpClient.UseServerCertificateValidation(Boolean)`. Before 27 a feature key controlled it.
**Measured: ONE call site in the whole tree**, so the escape hatch exists and BC barely uses it.
agiru validates by default and implements the method; the pre-27 feature key is not a target
(board:0490's reading of a superseded scheme).

**Anti-SSRF blocks internal IP addresses by default.** Online it cannot be disabled; on premises two
server settings control it -- `NavHttpClientAntiSSRFEnabled` and
`NavHttpClientAntiSSRFAllowedAddresses`, a JSON array of permitted addresses.

**agiru is on premises by construction** -- one process and one PostgreSQL on the user's own machine
-- **so the on-premises behaviour is the one to implement**, and it is a CONFIGURED default rather
than a fixed one. That places it with the other externalised configuration CLAUDE.md names, not in the
code. **The default stays `true`**, because the documentation calls disabling it "discouraged" and a
default that is safe is the one to inherit.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

| declaration | count |
|---|---:|
| `: HttpRequestMessage;` | **323** |
| `: HttpResponseMessage;` | 234 |
| `: HttpHeaders;` | 205 |
| `: HttpClient;` | 148 |
| `: HttpContent;` | 136 |

| call | count |
|---|---:|
| `.WriteFrom(` | 259 |
| `.HttpStatusCode` | 215 |
| `.GetHeaders(` | 200 |
| `.ReadAs(` | 162 |
| `.SetRequestUri` | 128 |
| `.IsSuccessStatusCode` | 122 |
| `.ReasonPhrase` | 56 |
| `.UseServerCertificateValidation` | **1** |

**`HttpRequestMessage` outnumbers `HttpClient` more than two to one**, which says the
`Send(Request, Response)` form is the normal one and `Get`/`Post`/`Put` are the exceptions -- so
`Send` is what to build first.

**The per-verb counts are NOT usable and are excluded rather than printed as facts.** The only pattern
available is `Client\.<Verb>\(`, which matches a variable literally NAMED `Client`; it returns `Send`
99, `Get` 39, `Post` 11, `Put` 6, `Delete` 5, `Patch` 2, and every one of those is a lower bound of
unknown tightness. The ratio between them is the only thing worth keeping, and it agrees with the
declaration counts above.

**`HttpStatusCode` at 215 against `IsSuccessStatusCode` at 122** says BC's own code more often reads
the code than asks the Boolean. Both must work; neither is the primary.

## The IST-state

- **Every `HttpClient` method refuses loudly**: `src/rt/Door.cpp:892` through `:999`, eighteen
  `RefuseDoor` calls covering `AddCertificate` (both overloads), `Clear`, `DefaultRequestHeaders`,
  `Delete`, `Get`, `GetBaseAddress`, `Patch`, `Post`, `Put`, `Send`, `SetBaseAddress`, `Timeout`,
  `UseDefaultNetworkWindowsAuthentication`, `UseResponseCookies`,
  `UseServerCertificateValidation` and `UseWindowsAuthentication` (both).
- **`HttpContent` refuses from `:1003`.**
- **The signatures are already right** -- `Boolean` returns on all six verbs, `HttpResponseMessage &`
  out parameters. So the two-level model is expressible today and only the bodies are missing.
- **board:0200 and board:0494 own the test handler** (`HttpClientHandler`), which intercepts by
  default. **So the UT suite can be green over an HTTP surface that never calls out**, and that is the
  reason this is `activation` rather than urgent.

## The choice

**One dependency, named with what it replaces.** CLAUDE.md permits a library where the standard
library is not enough and requires it to be justified and reachable on every architecture this builds
for. There is no HTTP client in C++23. **The candidate is named when the item is pulled, against three
requirements**: TLS with certificate validation, a header multi-map, and no dependency unreachable on
`aarch64`. Writing one is not on the table -- CLAUDE.md lists HTTP among the four things not written
from scratch.

**`HttpHeaders` is a multi-map with `Contains`, `Remove` and an appending `Add`**, and `GetHeaders`
returns a reference into its owner. Not a `Dictionary` -- board:0078's `Dictionary` is single-valued
and would silently drop the second `Set-Cookie`.

**The transport Boolean never carries the status.** One sentence, and it is the whole gate below.

**Anti-SSRF and certificate validation are configuration with safe defaults**, read once at startup,
never per call.

## Ordering

**After board:0200 and board:0494's handler**, which is what the UT suite actually reaches. **`Send`
first** by the 323-to-148 ratio, the five verb shortcuts after it, and `UseWindowsAuthentication` last
-- it names an authentication scheme this tree has no other part of.

## Gate, and its negative control

Against a local test server:

1. a 200 gives `Get` = `true` and `IsSuccessStatusCode` = `true`
2. **a 500 gives `Get` = `true` and `IsSuccessStatusCode` = `false`**
3. an unreachable host gives `Get` = `false`
4. `Content.GetHeaders(H)`; `H.Add('X', '1')`; the request carries `X: 1` -- the out parameter is a
   view
5. `Add` twice under one name yields two header values, and `Remove` then `Add` yields one
6. a request to an internal address is refused with anti-SSRF enabled and passes with it disabled

**The negative control is case 2, and nothing else catches it.** Fold the status into the return value
-- the convenient implementation -- and cases 1, 3, 4, 5 and 6 all stay green while every AL
`if not Response.IsSuccessStatusCode()` becomes unreachable. It is the one failure that turns a
server error into a success in 122 places.

**Case 4 is the second control**, for the `var` trap: return a COPY from `GetHeaders` and case 4 goes
red while 1, 2, 3, 5 and 6 stay green -- and case 5 must be checked on the same object to be sure it
is testing the view rather than the copy.

## Class

`activation`. Every call refuses today, so a UT case that reaches HTTP fails loudly and is not silently
wrong. Turning the refusals into calls means AL code that has never run starts running -- and, unlike
the rest of this board, it starts talking to the network, which is why board:0494's handler
intercepting BY DEFAULT is the precondition and not an afterthought.
