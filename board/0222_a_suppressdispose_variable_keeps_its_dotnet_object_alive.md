Type:     task
Status:   open
Parent:   0035
Area:     gen
Source:   developer/attributes/devenv-suppressdispose-attribute.md
Verdict:  fehlt
Class:    activation

# A `[SuppressDispose]` variable keeps its .NET object alive past its scope

`[SuppressDispose]` on a VARIABLE -- "Specifies if a Microsoft .NET Framework object that is defined
by a DotNet variable is disposed when it goes out of scope." On-premises only.

It is a LIFETIME instruction, and the C++ equivalent is exact: the default is a destructor at end of
scope, and this attribute suppresses it.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**0 occurrences** (2026-09-04). The read roots do not use it.

## The IST-state

The attribute parses into the raw list and is dropped, along with the `DotNet` variable it sits on.

## The choice

**A REFUSAL, named**, for the same reason as board:0215 and with the opposite risk. Ignoring it
means the object IS disposed at end of scope, so anything that held a reference past that point
reads freed memory -- in C++ that is undefined behaviour rather than a wrong answer, which is worse.

If it ever needs to be honoured, the shape is a `DotNet` variable emitted as a
`std::shared_ptr` rather than a value, and the attribute is what selects between them. That is
recorded so the decision is not re-derived, but with zero call sites it is not built.

## Ordering

Last, with 0214: zero population and a dependency on board:0035.

## Gate, and its negative control

A `DotNet` variable marked `[SuppressDispose]` must FAIL the translation, naming it.
**The negative control is the unmarked variable** -- it must translate once board:0035 rebuilds its
class.
