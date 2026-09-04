Type: root
State: open
Area: gen, net

# The .NET surface is derived from its use

Measured over the generated tree on 2026-09-02, after the subtype stopped being thrown away:

| | |
|---|---|
| distinct `dotnet::` types in `apps/system` alone | **263** |
| distinct across the whole tree | **499** |
| declarations | 7 117 |
| the ten most named | `XmlNode` 683, `XmlDocument` 293, `JObject` 235, `Array` 202, `UserInfo` 199, `XmlNodeList` 139, `CrmHelper` 93, `Encoding` 83, `XmlNamespaceManager` 65, `String` 51 |

Four have been written by hand -- `ALConfigSettings`, `NavTenantSettingsHelper`, `UserInfo`,
`GenericList1`/`GenericDictionary2` -- and each cost a turn. **259 more in `system` before `base`
begins is not a sequence anybody finishes.**

## What the predecessor did, and why it cannot be copied

`~/Git/openerp` built TWELVE modules, 2 124 lines: config, convert, data, datetime, encoding, enum,
http_utility, lasterror, math, path, regex, uri. Everything else fell to `_NilValue`, whose
`__getattr__` and `__call__` turn every operation into a silent no-op -- the module comments say so
outright and record what it cost: "jeder Aufruf lieferte NilValue, jede Abfrage las falsch -- und
zwar STILL".

That is the one thing this tree may not do. It is also why 97 % green is compatible with being
wrong: a layer that returns nothing quietly passes every test that does not look at it.

## The choice

**THE SURFACE COMES FROM THE CALL SITES, and the generator writes it.** The transpiler already parses
procedure bodies; a `DotNet` variable's type is known and every `X.Method(...)` on it is in the AST.
So the generator can emit, per distinct .NET type, a class carrying exactly the members the AL corpus
calls -- and each one REFUSES at run time, naming the type and the method.

That gives three things at once:

- **It compiles.** The whole `DotNet` class of failures closes with one mechanism rather than 499
  hand-written files, and the tree can be measured past it.
- **It is loud.** A call that is not implemented raises with `System.Xml.XmlDocument.SelectNodes` in
  the text, which is a defect anybody can act on. `_NilValue` returns a value that looks like an
  answer.
- **It is a WORKLIST with a denominator.** The emitted members are the exact set the BaseApp uses --
  not .NET's full API, which nobody needs -- and each family replaced by a real implementation
  removes its stubs. `XmlNode`, `XmlDocument`, `XmlNodeList` and `XmlNamespaceManager` together are
  1 180 of the declarations and go FIRST, and the predecessor already answers how: it has no
  `dotnet_xml.py` at all, because AL's own `XmlDocument` covers it -- `al_xml.py`, 4 004 lines.

## WHAT THIS ITEM IS NOT, and 1 429 references say otherwise

Measured 2026-09-04: `board:0035` is cited **1 441 times in the tree, 1 429 of them in `include/`**
-- by an order of magnitude the most-cited item in the repository. It is not cited for this subject.
`scripts/door.py:200` and `scripts/gen_builtins.py:194` write it into EVERY generated refusal in the
AL door, so `Record.ChangeCompany`, `Record.TransferFields`, `Label.*`, `File.*` and `KeyRef.*` all
point here, and none of them is a .NET type.

**This item owns the `dotnet::` surface derived from call sites. The AL door's own refusals are
board:0059**, which gives them a counted baseline and takes the label. Nothing about the design
below changes; what changes is that a reader following a refusal arrives at the item that owns it.

The 28 `dotnet.al` files in the read roots -- the alias declarations this item's generator reads --
are also the largest of the three kinds the run summary silently fails to count (board:0034).

## What is true when this closes

- Every `DotNet` variable the corpus declares has a type, and every method it calls has a member.
- An unimplemented member refuses by NAME, and the count of refusing members is a baseline that may
  only fall.
- The XML family is real rather than refusing, built on AL's own Xml types.
- No `_NilValue` anywhere: a .NET call either works or says which one did not.

## THE SURFACE, MEASURED -- AND HALF OF IT IS ONE LIBRARY

`devenv-create-a-wrapper-module.md` (read 2026-09-04, routed here) shows how the System Application
reaches .NET, and it is not what an agiru reader expects: **the BaseApp WRAPS a .NET class in a pair
of AL codeunits** -- a public facade with no logic and an `Access = Internal` implementation holding
the `DotNet` variable.

```al
codeunit 3960 Regex          { Access = Public;   procedure IsMatch(...) begin exit(RegexImpl.IsMatch(...)) end; }
codeunit 3961 "Regex Impl."  { Access = Internal; var DotNetRegex: DotNet Regex; }
```

**So `Regex` is an AL OBJECT the transpiler translates like any other**, and what agiru owes is not a
`Regex` module but the `DotNet Regex` TYPE underneath it. The same holds for every wrapper module in
the System Application. That is the right way round for this tree and it is worth stating, because the
instinct on meeting a `Regex` codeunit is to implement `Regex`.

**Population, measured 2026-09-04 over `~/Git/BCApps/src`**, by `:\s*DotNet\s+<name>`:

**7 050 `DotNet` variable declarations across 847 files.** By type, the top of the distribution:

| .NET type | declarations |
|---|---:|
| `XmlNode` | **1 756** |
| `XmlDocument` | **957** |
| `XmlNodeList` | 347 |
| `JObject` | 226 |
| `UserInfo` | 157 |
| `Array` | 155 |
| `Encoding` | 118 |
| `Char` | 92 |
| `GenericList1` | 78 |
| `CrmHelper` | 72 |
| `Convert` | 69 |
| `XmlNamespaceManager` | 68 |
| `String` | 61 |
| `XmlAttribute` | 56 |
| `MemoryStream` | 54 |
| `HttpStatusCode` | 52 |
| `Regex` | 49 |

**`XmlNode`, `XmlDocument`, `XmlNodeList`, `XmlNamespaceManager`, `XmlAttribute` and `XmlElement` sum
to 3 228 of the 7 050 -- 46 % of the whole DotNet surface is the .NET XML DOM.** And agiru already
has an AL-native equivalent for all six (`include/type/XmlNode.h`, `XmlDocument.h`, `XmlNodeList.h`,
`XmlNamespaceManager.h`, `XmlAttribute.h`, `XmlElement.h`), which the methods-auto pass measured at
27, 25, 2, 8, 18 and 33 documented methods with almost no gaps.

**That is the single most consequential number in this item.** It does NOT mean the AL types can be
substituted -- `DotNet XmlDocument` is `System.Xml.XmlDocument` and AL's `XmlDocument` is a different
class with a different surface, and the BaseApp chose the .NET one deliberately in 957 places. What it
means is that **46 % of the DotNet work is ONE library with a known shape**, and the C++ side of it is
the same XML library the AL types already sit on. The remaining 54 % is a long tail.
