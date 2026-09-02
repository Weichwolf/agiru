Type: arc
State: open
Area: gen, rt, net

# The AL builtins are a counted population

The generator translates no builtin at all. `CurrentDateTime`, `StrSubstNo`, `CopyStr`, `Format`,
`Evaluate`, `CreateGuid` -- AL writes them as free calls and the generator emits the name unchanged,
which is an undeclared identifier in every file that uses one.

## The population is measured, not guessed

`~/Git/openerp/openerp/runtime/builtins/` holds **165 `_al_*` functions**, and that is the same
question answered once already on the same BaseApp:

| file | builtins |
|---|---|
| `_string.py` | 41 |
| `_system.py` | 26 |
| `_datetime.py` | 22 |
| `_format.py` | 14 |
| `_math.py`, `_clear.py` | 10 each |
| `_error.py` | 9 |
| `_enum.py` | 8 |
| `_recordref.py`, `_array.py` | 5 each |
| `_guid.py` | 4 |
| `_core.py` | 3 |

That list is the WORKLIST and the DENOMINATOR both, and it is a measured one: it is what a 97 %-green
run over the UT subset actually needed. It is not the specification -- `methods-auto/` is -- but a
builtin that is not in it is one no BaseApp path reached.

## Two answers taken from the predecessor rather than re-derived

- **`UserSecurityId()` returns the blank GUID.** `_system.py:409` returns
  `'00000000-0000-0000-0000-000000000000'` and 97 % of the UT subset went green over it. So the
  audit fields `SystemCreatedBy` and `SystemModifiedBy` do not block on an authentication story
  (board:0013). It becomes a value the SESSION carries, defaulting to blank, rather than a constant
  in a function -- a session field is honest, a hardcoded GUID is not.
- **`CurrentDateTime` is the wall clock**, `_datetime.py:386`. No work date, no session offset.

## The choice

**A builtin is a free function in `agiru::`, named as AL names it**, and it lands in the tier its
type lives in -- `StrSubstNo` beside the string types in `src/net`, `CurrentDateTime` beside
DateTime, `CurrFieldNo` in `src/rt` because it needs the session. The generator emits the call
unqualified and the door's `using` makes it resolve, so a generated line reads exactly as the AL
line does.

`_format.py` is the one that is not a function but a LANGUAGE: `Format(Value, Length, FormatStr)`
carries BC's own format specifiers, and openerp needed 14 functions and a spec parser for it.
It gets its own item when it is reached; board:0007 already holds the decimal half of it.

## What is true when this closes

- A counter beside the AL surface baseline: builtins reachable against 165.
- The generator resolves a builtin call by NAME and refuses an unknown one loudly, rather than
  emitting an identifier that the C++ compiler will refuse two steps later with no AL name in the
  message.
