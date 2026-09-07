Type:     task
Status:   open
Area:     gen
Source:   nine sources refused by -Werror while the slice grew, 2026-09-07
Class:    silent-wrong-data

**STANDING: nothing is emitted differently yet; the nine sources are out of `test/slice`.**

# A faithful translation of AL is one the C++ compiler accepts

**AL permits three things that clang refuses under `-Werror`, and all three are FAITHFUL
translations.** They surfaced together when the slice grew by the sources a sweep found compiling:
nine of them build alone and fail the gate.

| what clang says | what the AL says | count in this batch |
|---|---|---:|
| `-Wunused-but-set-parameter` | a BY-VALUE parameter assigned in the body -- legal in AL, and just as dead there | 4 |
| `-Winfinite-recursion` | an `[Obsolete]` procedure whose own body calls itself (board:0602) | 2 |
| `comparison of integers of different signs` | an AL `Integer` compared against a `.Count()` | 2 |
| `-Wunused-parameter` | a subscriber parameter the body never reads | 1 |

The clearest is the first:

```al
[EventSubscriber(ObjectType::Codeunit, Codeunit::"Mail Management", 'OnBeforeIsEnabled', '', false, false)]
local procedure OnBeforeIsEnabled(OutlookSupported: Boolean; var Result: Boolean; var IsHandled: Boolean)
begin
    OutlookSupported := false;   // by value: the write goes nowhere, in AL as in C++
    Result := true;
```

The subscriber's signature is fixed by the publisher, so the parameter must exist; the body assigns
to it and nothing reads it. That is BC's own code, in a UT codeunit the milestone counts.

## The rule this needs

**`-Werror` over `apps/` is deliberate and stays** -- CLAUDE.md keeps generated code out of the
ANALYSER and inside the COMPILER on purpose. So the answer is not to switch a warning off; it is
that the generator emits what the construct means:

- a by-value parameter that AL may assign and never read is `[[maybe_unused]]`, which the generator
  already writes on locals it declares for a body that may not use them;
- an `Integer` against a size is a comparison the door should carry, so the size becomes the AL type
  rather than the comparison becoming a cast;
- the recursion is board:0602's obsolete body, and a refusal is the shape for it.

**The measure is the slice**: 6 894 today, and these nine come back the moment each shape is
emitted. The sweep that found them runs over every source outside the slice on all cores in about
three minutes and it now carries the gate's own flags -- it reported 130 as compiling while it was
still missing `-Werror`, and six of those failed the build.
