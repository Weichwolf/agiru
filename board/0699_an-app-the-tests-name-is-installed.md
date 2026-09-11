# 0699 An app the tests name is installed

**The finding.** 18 UT cases failed with `the .NET member PEPPOL30Setup.GetSetup is named by AL and
not rebuilt here (board:0035)`. `PEPPOL30Setup` is not a .NET type at all: it is a TABLE in
`src/Apps/W1/PEPPOL`, an app `apps.json` did not install, so every name from it fell through to the
absent-.NET surface -- which is what that surface is for, and it names the wrong cause.

**The choice.** The app is installed: `apps.json` gains `peppol` (source `Apps/W1/PEPPOL/App`,
depends on system/foundation/base) and `tests` depends on it. `Microsoft.Peppol` was already inside
`scope.json`, so nothing about the whitelist moved -- the app was simply never read. 50 AL files in,
58 generated files out, 34 of them sources.

**Four generic defects it exposed, all fixed in the same round:**

| defect | the fix |
|---|---|
| `this.Held.Member(...)` -- AL's `this` qualifier -- lost the handle, so a codeunit held by `Instance` got `.` where it needed `->` | the member chain treats `this` as transparent and asks `IsHandle` of the member behind it |
| an option declared in a CONTROL or XMLPORT-ELEMENT trigger got no synthetic type: the sweep read only object-level and procedure-level declarations | the sweep walks `layout`, `actions`, `views` and `dataset` recursively |
| a record parameter passed BY VALUE was forward-declared in the callee's header, so a caller converting a `Variant` to it saw an incomplete type | a header that declares a by-value record parameter INCLUDES that table |
| `XmlDeclaration.Create` was a non-static member; AL calls it on the TYPE | `static`, the way every other `Create` on the Xml types already is |

**What is still out:** nothing. All 34 PEPPOL sources are in the slice.

**The cost to watch:** the by-value include rule widens generated headers (the sample codeunit went
from 10 includes to 13). It is an APP header and not the door, so it costs the units that include
it and not all 7 885; `build/times.log` carries the full build either side.

**Measured: 1 775 -> 1 775. NO CHANGE, twice over.** Installing the app moved the 18 cases from
`PEPPOL30Setup.GetSetup is named by AL and not rebuilt here` to `this value of PEPPOL 3.0 Format
names no implementation of PEPPOL Monetary Info Provider`; reading `DefaultImplementation` and
`UnknownValueImplementation` (the enum-level fallbacks, which the generator parsed by taking
everything after the FIRST `=` in a LIST of `Interface = Codeunit` pairs, so it found nothing) moved
them again, to `Format: a Variant holding Blob has no text form yet`; giving a Blob the empty text
moved them to `Assert.IsTrue failed. Queue underflow` -- a handler that never ran.

**It is kept, and the reason is not the counter.** Four rounds of cause, each one a generic defect
with its own fix, and the app is one the W1 tests name. What the counter says is that this codeunit
family is blocked on a CHAIN and not on one gap; the next link is a test handler that does not run,
which belongs to board:0030's UI shape and not here.

**The lesson for the ranked list:** a failure text names the FIRST refusal on a path, not the
distance to green. 18 cases that all say one thing can be 18 cases four gaps deep.
