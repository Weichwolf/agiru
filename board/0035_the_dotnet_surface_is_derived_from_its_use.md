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

## What is true when this closes

- Every `DotNet` variable the corpus declares has a type, and every method it calls has a member.
- An unimplemented member refuses by NAME, and the count of refusing members is a baseline that may
  only fall.
- The XML family is real rather than refusing, built on AL's own Xml types.
- No `_NilValue` anywhere: a .NET call either works or says which one did not.
