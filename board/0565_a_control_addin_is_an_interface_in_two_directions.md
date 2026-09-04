Type:     task
Status:   open
Parent:   0034
Area:     al, gen, rt
Source:   developer/devenv-control-addin-object.md
Verdict:  fehlt
Class:    activation

# A control add-in is an interface in two directions

board:0479 filed the add-in's PROPERTIES -- scripts, stylesheets, the three lifecycle hooks -- and
board:0424 its ten sizing properties. **Neither carries the object's actual surface**, which is two
lists of signatures and nothing else:

```al
controladdin SampleAddIn
{
    Scripts = 'https://.../knockout-debug.js', 'main.js';
    StartupScript = 'startup.js';

    // what AL may call in JavaScript -- DECLARATIONS, no bodies
    procedure CallJavaScript(i: integer; s: text; d: decimal; c: char);

    // what JavaScript may raise in AL -- DECLARATIONS, no bodies
    event Callback(i: integer; s: text; d: decimal; c: char);
}
```

**A `controladdin`'s `procedure` has NO BODY.** It is a declaration of what the JavaScript provides,
so the object is an INTERFACE and not an implementation -- the same shape as an AL `interface`
(board:0033's kind), with the implementation on the other side of a language boundary instead of in
another AL object.

## The two directions, and each has one call shape

**AL calls into the add-in through the PAGE, not through the object:**

```al
CurrPage.ControlName.CallJavaScript(5, 'text', 6.3, 'c');
```

So `CurrPage` carries a member per `usercontrol` control, and that member carries the add-in's
declared procedures. **That is a THIRD thing `CurrPage` has to be** -- board:0553's tree gives it
controls, board:0554's parts give it sub-pages, and this gives it a typed proxy per user control.

**JavaScript raises into AL through a trigger on the CONTROL:**

```al
usercontrol(ControlName; SampleAddIn)
{
    trigger Callback(i: integer; s: text; d: decimal; c: char)
    begin ... end;
}
```

matched by name to the add-in's `event`. From the JavaScript side the call is
`Microsoft.Dynamics.NAV.InvokeExtensibilityMethod('CallBack', [42, 'some text', 5.8, 'c'])` -- **a
string method name and a positional argument array**, so the binding is by NAME and the arity and
types are checked at the boundary or not at all.

**Note the casing in Microsoft's own example**: the AL declares `event Callback` and the JavaScript
invokes `'CallBack'`. AL is case-insensitive, so both are the same symbol -- and it is CLAUDE.md's
`identifier casing` trap in its natural habitat, on a name that crosses a language boundary where the
other language is NOT case-insensitive. **The collapse has to happen on the AL side of the boundary,
once, in the generator.**

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

| | count |
|---|---:|
| `usercontrol(` placements | **225** |
| `CurrPage.<control>.<method>(` calls | 57 |
| `event <name>(` declarations | **51** |
| `controladdin` objects | **20** |

**The `event` count is EXACT and separable**, which is unusual in this sweep: every file containing
`^\s*event\s+<name>\s*\(` is one of the 20 `controladdin` files, checked by listing both sets. 51
events across 19 add-ins -- `EarlyAccessPreviewBanner` declares none.

Properties over the 20: `Scripts` 19, `VerticalStretch` 17, `HorizontalStretch` 17,
`RequestedHeight` 16, `StartupScript` 15, `StyleSheets` 14, `Images` 6, `MinimumHeight` 6,
`RecreateScript` 5, `RefreshScript` 5.

**Sixteen of the twenty are in `System Application/App/ControlAddIns/`**, and the remaining four are
one each in `Apps/W1/ClientAddIns`, `Apps/W1/INTaxEngine`, `Apps/W1/EDocument` and
`Apps/GB/UKMakingTaxDigital`. **So this object kind is a closed set in one directory**, which is
different from every other kind in this board and changes what "implement it" means: the twenty are
enumerable and each can be decided on its own.

They are, by what they do:

| reachable in a browser UI | needs something agiru does not have |
|---|---|
| `BusinessChart`, `WebPageViewer`, `PageReady`, `WaitSpinner`, `VideoPlayer`, `RoleCenterSelector` | `CameraBarcodeScannerProviderAddIn`, `BarcodeScannerProviderAddIn` (device hardware) |
| `OAuthControlAddIn`, `OAuthIntegration`, `OAuthAddIn` | `PowerBIManagement` (an external service) |
| `WelcomeWizard`, `EarlyAccessPreviewBanner`, `PDFViewer` | `SatisfactionSurvey`, `SatisfactionSurveyAsync`, `CustomerExperienceSurvey` (Microsoft telemetry) |

**That split is a judgement and it is labelled as one.** It is not measured; what is measured is that
there are twenty and where they live.

## The IST-state

**`ControlAddIn` is not parsed at all.** `grep -n ControlAddIn src/al/Ast.h src/al/Parser.cpp` is
empty -- no AST node, no parse path. The only occurrence in the tree is
`src/gen/CodeunitWriter.cpp:129`, where `ControlAddIn` is recognised as a VARIABLE TYPE, the same
half-state board:0556 found for `Query`: the name is known and the object is not.

**board:0297 owns the page field's `OnControlAddIn` trigger** -- the legacy event path -- and
board:0479 the properties. Nothing owns the signatures.

## The choice

**The `controladdin` object generates a header with two `constexpr` signature lists and NO code**,
because there is no code in it:

```cpp
struct AddInMethod { std::string_view name; std::span<const ParameterDef> parameters; };
struct AddInDef {
  std::string_view name;
  std::span<const std::string_view> scripts;      // board:0479
  std::span<const AddInMethod> procedures;        // AL -> JavaScript
  std::span<const AddInMethod> events;            // JavaScript -> AL
};
```

**Why signature lists and not generated C++ member functions:** the procedures have no bodies and
never will -- their implementation is a `.js` file. A generated C++ method would have to be a
forwarder to a name-and-array call, which is what the list already is, written once in the runtime
instead of 51 times by the generator.

**But the `usercontrol` control DOES get a typed proxy**, because `CurrPage.ControlName.Method(...)`
is AL that the transpiler has to turn into C++ that compiles. The proxy is generated per usercontrol
from the add-in's procedure list, and each method marshals its arguments into the boundary call.
**That is where the type checking lives** -- 57 call sites whose arity and types the C++ compiler can
check, which the JavaScript side cannot.

**The event triggers are ordinary control triggers**, so board:0553's tree already carries them; what
this adds is that their names must MATCH the add-in's `event` list. **A `static_assert`**, because
both lists are `constexpr`: a `trigger` on a `usercontrol` whose name is not one of the add-in's
events is a translation error, and it is the kind of error that otherwise shows up as a callback that
silently never fires.

**Casing collapses once, in the generator**, per CLAUDE.md's guard for that trap -- and here it must
also be recorded in the emitted metadata, because the JavaScript boundary compares strings.

**Scripts are files, not code.** They are copied into the app's output and referenced by the page;
CLAUDE.md's `Artefacts go to build/` applies, and an external `https://` script (the example's
knockout CDN) is a URL passed through unchanged.

## Ordering

**Inside board:0034's object-kind census, and it is the SMALLEST of the twelve** -- 20 objects, 51
events, 225 placements. **After board:0553's tree**, which is what a `usercontrol` control sits in,
and after board:0479's properties.

**`usercontrol(` at 225 against 20 objects** says the placements outnumber the definitions eleven to
one, so the proxy on `CurrPage` is what most AL actually touches -- and it, rather than the
`controladdin` object, is what to build first.

## Gate, and its negative control

A `controladdin` with one procedure and one event, placed on a page:

1. the add-in generates a header with one entry in each list and no function bodies
2. `CurrPage.Ctl.CallJavaScript(5, 'text', 6.3, 'c')` COMPILES, and passing a `Date` for the
   `integer` does not
3. a `trigger` on the usercontrol whose name is not an add-in event **fails to transpile**
4. `trigger CallBack` matches `event Callback` -- the casing collapses
5. the add-in's scripts land in the app's output in declaration order

**The negative control is case 3.** Drop the `static_assert` and cases 1, 2, 4 and 5 stay green while
a misnamed trigger becomes a callback that never fires -- silent, and indistinguishable from
JavaScript that never called.

**Case 2's second half is the other control**: accept anything and the 57 call sites lose the only
type check the boundary has, because the JavaScript side has none.

## Class

`activation`. No add-in exists today, so nothing regresses. What the item cannot deliver on its own is
BEHAVIOUR: a control add-in's function is in its JavaScript, and for sixteen of the twenty that
JavaScript ships with the System Application. **Transpiling the object makes the surface exist; making
the add-in WORK is per-add-in and belongs to phase 2**, which is why the split table above is in this
item rather than a later one.

## EVERY ADD-IN METHOD IS `void`, AND THAT IS A REFUSAL RATHER THAN A CONVENTION

`devenv-control-addin-asynchronous-considerations.md` (read 2026-09-04, routed here) states the rule
that makes the two lists above the ONLY channel:

> "**All calls between the AL code running on the server and the script method running in the Web
> browser are ASYNCHRONOUS. This means that methods in the control add-in interface MUST BE OF TYPE
> VOID and property methods shouldn't be used.**"
>
> "To transfer a result FROM an AL trigger TO the calling script method, just add a METHOD to the
> control add-in interface ... To transfer a result FROM a script method TO an AL trigger, just add an
> EVENT."

**So a `controladdin` procedure returning anything is a translation error**, and it is a
`static_assert` this item can make with no risk: the 51 documented events and the procedures beside
them are the whole population, and every one of them is `void` by the platform's own rule.

**And the asymmetry is now explained.** board:0565 records that AL calls in through `CurrPage` and
JavaScript raises back through a trigger; this says WHY it cannot be one call with a return -- the
boundary is asynchronous, so a round trip has to be two one-way messages. **A future implementation
that made the proxy synchronous because C++ can would be faithful to neither side.**
