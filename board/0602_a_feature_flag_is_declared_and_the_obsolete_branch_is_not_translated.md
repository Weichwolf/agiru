Type:     arc
Status:   open
Area:     al, gen
Source:   -Winfinite-recursion on a generated body, 2026-09-07
Class:    activation

# The preprocessor symbols are a DECLARED input, and which set is translated is a scope decision

**The first version of this item claimed the transpiler translates code BC's own build removes.
That is wrong and the settings file says so.** `~/Git/BCApps/.github/AL-Go-Settings.json` declares

```json
{ "buildModes": ["Clean"],
  "settings": { "preprocessorSymbols": ["CLEAN25", "CLEAN26", "CLEAN27", "CLEAN28",
                                        "CLEAN29", "CLEAN30"] } }
```

and that is the ONLY declaration in the repository. The symbols belong to the `Clean` build mode --
a CI mode that proves the code still builds once the obsolete parts are gone. **The default build
defines none of them**, so `#if not CLEAN27` is true there, and `ConditionReader`'s "an unknown
symbol is false" reproduces BC's default exactly. The transpiler is right.

## What the finding actually is

The AL that came out of it is this, and it is obsolete code that BC ships:

```al
#if not CLEAN27
    [Obsolete('Replaced by procedure in codeunit AsmCarryOutAction', '27.0')]
    procedure AsmOrderChgAndReshedule(RequisitionLine: Record "Requisition Line"): Boolean
    begin
        exit(AsmOrderChgAndReshedule(RequisitionLine));
    end;
#endif
```

A faithful translation of it recurses for ever, and `-Winfinite-recursion` under `-Werror` refuses
to build it. So the tree has to answer a question it has not asked yet: **which of BC's two build
modes is agiru translating?**

| | default build | `Clean` build |
|---|---|---|
| what it carries | every obsolete member still shipped, this stub among them | only what survives the cleanup |
| what the UT suite expects | the tests are written against the shipped app | some tests would lose their subject |
| what it costs us | a body the C++ compiler refuses, per case | 10 127 `#if` blocks change side, and 990 `#else` swap |

**The default is the right target and it is what the tree already does** -- the milestone is the
shipped BaseApp and its tests. So the symbols stay undefined BY DECISION rather than by accident,
and `apps.json` gains the list as a DECLARED and empty input, so that the day a case wants the Clean
side it is a setting and not a patch.

## Population, measured over `~/Git/BCApps/src` 2026-09-07

10 127 `#if` and 990 `#else` in 3 462 files. `not CLEAN28` is 3 306 of them, `not CLEAN27` 3 228,
`not CLEAN29` 1 654. **That is how much AL changes side if the flag set ever moves**, which is why
it belongs in a declaration with a number beside it rather than in a default nobody stated.

## What to do with the one body that will not build

It is one obsolete procedure and its own AL is the defect. The generic answer is NOT to special-case
it: a body whose every path calls itself is a body that cannot run, so the generator may emit the
refusal it already emits elsewhere -- the procedure exists, it is declared, and calling it raises.
That keeps the member (which an obsolete-but-shipped API needs) without asking the compiler to
accept a loop.
