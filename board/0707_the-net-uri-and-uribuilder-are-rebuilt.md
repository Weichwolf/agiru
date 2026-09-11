Type:     task
Status:   open
Area:     net
Source:   developer/devenv-dotnet-types.md
Verdict:  fehlt
Class:    silent-wrong-data

# 0707 The .NET Uri and UriBuilder are rebuilt

**The finding (2026-09-11, ranked list).** `System.Uri`, `System.UriKind` and `System.UriBuilder`
are absent stubs, so every call refuses: `the .NET member UriKind.Absolute is named by AL and not
rebuilt here` (3 UT cases: `Test OAuth 2.0 UT` SecureServiceURL, `Workflow Engine UT`
TestCustomLinkWorkflowStepArgument*), and behind them the whole `URI` module of the System
Application (`Uri.Codeunit.al`, `UriBuilderImpl.Codeunit.al`) and `Web Request Helper` -- 56
member calls over the generated tree.

**The member set (counted, the rule from memory `rebuilt-dotnet-type-rules`):** `Uri`: the
class-named constructor `Uri(Text)`, `Scheme`, `Host`, `Port`, `AbsolutePath`, `Query`,
`Fragment`, `AbsoluteUri`, `Authority`, `Segments`, `IsBaseOf`, `ToString`, and the statics
`IsWellFormedUriString(Text, UriKind)`, `TryCreate(Text, UriKind, var Uri)`,
`EscapeDataString(Text)`, `UnescapeDataString(Text)`. `UriKind`: `Absolute`, `Relative`,
`RelativeOrAbsolute`. `UriBuilder`: constructor `UriBuilder(Text)`, `Scheme`, `Host`, `Port`,
`Path`, `Query`, `Fragment` as read AND write properties, `Uri`.

**The shape.** One header `include/dotnet/Uri.h` carrying the three classes (each needs its own
`kElsewhere` entry in `src/gen/Door.cpp`), a parser for RFC 3986 in `src/net/Uri.cpp` -- scheme,
authority (userinfo, host, port), path, query, fragment -- and .NET's escaping rules for
`EscapeDataString` (RFC 3986 unreserved set kept, everything else percent-encoded as UTF-8).
`IsWellFormedUriString` is stricter than a parse: an absolute URI needs a scheme and, for
`http(s)`, a host.

**Before the chain:** `-fsyntax-only` the wrapper codeunits `apps/system/system/uri/codeunit/*.cpp`
and `apps/base/system/integration/codeunit/WebRequestHelper.cpp`.
