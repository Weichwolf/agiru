Type:     task
Status:   open
Parent:   0035
Area:     al, gen
Source:   developer/devenv-dotnet-subscribe-to-events.md
Verdict:  fehlt
Class:    silent-wrong-data

# A `DotNet` variable with `[WithEvents]` gets triggers, and that syntax must still parse

> "You can configure a DotNet variable to **subscribe to events published by a .NET Framework type**.
> Events are handled by triggers in the AL code."
>
> **"You can only subscribe to events emitted by GLOBAL VARIABLES of the .NET type marked with the
> `[WithEvents]` attribute. For all the global variables marked with this attribute, THE COMPILER
> WILL EXPOSE THE EVENTS AVAILABLE ON THE TYPE AS TRIGGERS ON THE VARIABLE."**
>
> **"The syntax for declaring these triggers is `{VariableName}::{EventName}(...ParameterList)`."**
>
> ```AL
> var
>     [WithEvents]
>     timer: DotNet MyTimer;
> trigger timer::Elapsed(sender: Variant; e: DotNet MyElapsedEventArgs)
> ```

**This is a SYNTAX item before it is a runtime one.** `trigger timer::Elapsed(...)` is a trigger
declaration whose name contains `::` and is scoped to a variable -- a shape that appears nowhere else
in AL. board:0028's parser must accept it or the file does not parse, and a file that does not parse
is a lost object, not a refused feature.

**And the events come from the .NET type**, not from a declaration -- "the compiler will expose the
events available on the type" -- so the set of legal trigger names is not in the AL source at all. A
transpiler cannot validate the name without the assembly, which is the same problem board:0035 has
for every DotNet member.

## The decision, and it is a refusal with a parse

**Parse it, refuse it, and say which.** CLAUDE.md's rule is that a kind with no generator is a hole
with a count and never a decision, and that a failure is loud. So:

- the parser accepts `[WithEvents]` and the `Var::Event` trigger form;
- the generator refuses the object, naming the variable and the event;
- the count goes into board:0035's refusing-door surface, which board:0059 already counts.

**Not silently dropping the trigger.** A `DotNet` object whose event trigger vanished would compile
and never fire, which is the shape this tree calls a finding.

**.NET Framework interop is out of scope by construction**: CLAUDE.md says the .NET types are REBUILT
here, one C++ class per .NET class, precisely because bridging them failed in the predecessor. A
`System.Timers.Timer` is not among the classes an ERP needs rebuilt, and an AL object that subscribes
to one is asking for a CLR.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`[WithEvents]` is an attribute, not a property; board:0190's attribute family owns its count and
board:0035 the `dotnet` declaration blocks. **Stated rather than guessed** -- and it is worth taking,
because if the count is zero the parser work is still required (the syntax must not break the parse)
while the refusal costs nothing.

## The IST-state

`src/al/Parser.cpp:545` reads every attribute into `ProcedureDecl::attributes` as raw text, so
`[WithEvents]` on a VARIABLE is a different position and is not covered by that. Whether the parser
accepts a `Var::Event` trigger name is not measured here and is the item's first task.

## The choice

Parser support for both forms, and a generator refusal naming the object. The refusal is one entry in
board:0035's counted surface.

## Ordering

The parser half first and independently -- it protects the parse of any file that uses the syntax.
The refusal with board:0035's census.

## Gate, and its negative control

An AL file declaring `[WithEvents]` and a `timer::Elapsed` trigger PARSES, and the object is refused
by the generator with a message naming both.

**The negative control is the parse** -- a parser that rejects the syntax loses the whole file, and a
gate that only checks "the object is refused" cannot tell a refusal from a parse failure.
