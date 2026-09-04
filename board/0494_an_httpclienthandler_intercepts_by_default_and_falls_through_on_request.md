Type:     task
Status:   open
Parent:   0200
Area:     rt, gen, net
Source:   developer/devenv-httpclient-mock-outbound-calls.md
Verdict:  fehlt
Class:    activation

# An `HttpClientHandler` intercepts by default and falls through only on request

board:0200 filed `[HttpClientHandler]` from the attribute page and board:0489 filed
`TestHttpRequestPolicy` from the property page. **This page is the mechanism both of them describe**,
and it carries the one fact neither page states.

> The handler procedure **"receives a `TestHttpRequestMessage` ... and a `TestHttpResponseMessage`
> ... The boolean return value indicates whether to ISSUE THE ORIGINAL HTTP REQUEST -- `true` -- or
> USE THE MOCKED RESPONSE -- `false`."**
>
> **"NOTE: The DEFAULT RETURN VALUE of the `HttpClientHandler` procedure is `false`, ensuring that
> external service calls are only made INTENTIONALLY. Therefore, AN EMPTY HANDLER WOULD STILL
> INTERCEPT the outbound request and mock a default response."**
>
> "When a handler is attached to a test then **ALL** `HttpClient` calls that occur during the
> execution of that test are routed to the handler instead of the actual endpoint."
>
> **"NOTE: This feature is only supported in Business Central ON-PREMISES."**

**The default return value is the whole safety property**, and it is the opposite of what a C++
implementation gets for free: an AL procedure with no `exit` returns the type's default, which for
`Boolean` is `false`, which here means "use the mock". So a handler somebody wrote and left empty
blocks the network -- and a generated procedure whose return value is uninitialised, or which
defaults to `true` for any reason, silently opens it.

**That inverts board:0489's finding.** `TestHttpRequestPolicy` defaults to `AllowAllOutboundRequests`,
so a test with NO handler may reach the network; but a test WITH a handler intercepts everything by
default. The two defaults pull in opposite directions and both are documented.

**And the whole feature is on-premises only**, which for agiru is the only mode -- so unlike
board:0476's Power Automate this is a documented non-target in BC's cloud and a full target here.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

board:0199 measured `[HandlerFunctions]` at **47 994** -- the attribute that attaches a handler to a
test. board:0489 measured `TestHttpRequestPolicy` at **45**. The `[HttpClientHandler]` attribute's own
count belongs to board:0200 and is an attribute, not a property, so this sweep's declaration pattern
does not apply -- **stated rather than guessed.**

## The IST-state

board:0200 records the attribute's state; board:0035 records the `HttpClient` surface as declared and
refusing. `TestHttpRequestMessage` and `TestHttpResponseMessage` are two AL types -- whether they
exist in `include/type/` is board:0051's per-type door question and is not measured here.

## The choice

A per-session handler stack that `HttpClient` consults before issuing anything, with the handler's
**return value defaulting to `false`** -- so the generated procedure's return variable is initialised
to `false` and an AL body with no `exit` yields interception, exactly as AL does.

**Not an interceptor installed by the test framework only.** The policy property (board:0489) also
gates unhandled calls, so the check sits in `HttpClient` itself and the runner supplies the handler,
which keeps one code path for both.

## Ordering

Behind board:0035's `HttpClient` and board:0054's handler mechanism. With board:0489, whose policy
this reads.

## Gate, and its negative control

A test with an EMPTY handler attached makes no outbound request and receives a default response.

**The negative control is the empty handler** -- a handler that does nothing must still intercept, and
an implementation whose default return is `true` passes every gate whose handler explicitly returns
`false` and reaches the network from every gate that does not.
