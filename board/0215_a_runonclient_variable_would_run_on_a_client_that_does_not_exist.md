Type:     task
Status:   open
Parent:   0035
Area:     gen
Source:   developer/attributes/devenv-runonclient-attribute.md
Verdict:  fehlt
Class:    activation

# A `[RunOnClient]` variable would run on a client, and agiru refuses it rather than pretending

`[RunOnClient]` on a VARIABLE -- "Sets whether a .NET object that is defined by a variable is run on
the Business Central Web client or Business Central service." On-premises only.

It is one of three attributes that only mean something for a `DotNet` variable, with
`[SuppressDispose]` (0222) and `[WithEvents]` (0227). All three describe the lifetime or the
location of a .NET Framework object.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**91 `[RunOnClient` declarations.**

## The IST-state

The attribute parses into the raw list and is dropped, and so is the `DotNet` variable it sits on --
`.NET interop is not translated at all` (board:0035, board:0034's object-kind table).

## The choice

**A REFUSAL, named.** CLAUDE.md's route for .NET is that the types are REBUILT -- "one C++ class per
.NET class" -- not bridged, and a rebuilt class runs in the process. There is no client, so
"run on the client" has no meaning that could be honoured, and honouring it as "run here" would
silently change where 91 objects execute.

So the generator refuses a `[RunOnClient]` variable with a diagnostic naming the variable and the
object -- which is exactly board:0190's default and the reason that default is the right one: an
attribute whose meaning cannot be reproduced must stop the translation rather than evaporate.

**Why not carry it inert like 0207.** `[NonDebuggable]` changes no answer if ignored; this one does.
A client-side .NET object and a server-side one differ in what they can reach.

## Ordering

Behind board:0035, which decides which .NET classes are rebuilt at all. Until then the 91 variables
do not translate for a larger reason and this refusal is unreachable.

## Gate, and its negative control

A `DotNet` variable marked `[RunOnClient]` must FAIL the translation, naming it.
**The negative control is the same variable unmarked** -- it must translate, or the refusal is
catching the variable rather than the attribute.
