Type:     task
Status:   open
Parent:   0054
Area:     rt
Source:   developer/attributes/devenv-httpclienthandler-attribute.md
Verdict:  fehlt
Class:    activation

# An `[HttpClientHandler]` mocks the outbound request, and its default is to mock

```al
[HttpClientHandler]
procedure HttpClientHandler(Request: TestHttpRequestMessage;
                            var Response: TestHttpResponseMessage) IssueOriginalRequest: Boolean
```

**The return value decides whether the network is touched**: `true` issues the real request, `false`
uses the mocked `Response`. And the DEFAULT return value is `false` -- "an empty handler would still
intercept the outbound request and mock a default response"
(`devenv-httpclient-mock-outbound-calls.md`).

**That is the determinism lever the milestone needs.** CLAUDE.md makes determinism compulsory, and a
suite that can reach the network is not deterministic. The page is marked on-premises only, which
agiru is.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**101 `[HttpClientHandler` declarations.**

## The IST-state

`include/type/TestHttpRequestMessage.h` and `TestHttpResponseMessage.h` exist as door headers with
refusing bodies -- the two types the handler needs are already declared. `HttpClient` is a door
refusal. The attribute parses into the raw list and is dropped.

## The choice

A table entry with kind `HttpClient`, no object id. `HttpClient.Send` consults it: with a handler
registered it builds a `TestHttpRequestMessage`, calls the handler, and either issues the real
request or returns the handler's `TestHttpResponseMessage`. **The Boolean's default is `false`
because the AL default is `false`** -- a handler that sets nothing mocks.

`TestHttpRequestPolicy` on the codeunit decides what an UNHANDLED request does:
`AllowAllOutboundRequests` (the AL default), `AllowOutboundFromHandler`, `BlockOutboundRequests`.
**`agiru run-tests` sets `BlockOutboundRequests`**, which is a deviation from the AL default in the
direction the determinism rule requires, and it is stated here so it is not read as an accident.

## Ordering

Needs 0199's table. Needs no page runtime. It should come EARLY regardless of its 101 sites: until
it exists, any test that reaches the network is nondeterministic and its result means nothing.

## Gate, and its negative control

A test whose code calls `HttpClient.Send` with a handler that sets a canned response and returns
nothing: the caller must see the canned response and no socket must open. A second test with no
handler must FAIL under `BlockOutboundRequests`.

**The negative control is the empty handler** -- it must still intercept. A runtime that treats "the
handler set no return value" as "issue the request" inverts the whole feature.
