Type: root
State: open
Area: gen, rt

# A codeunit variable is a handle, not a value

`AzureOpenAI` holds `AzureOpenAIImpl` as a member, and the member is an incomplete type: the header
that defines it is already open higher in the include stack, so `#pragma once` makes the second
include a no-op and the class is half-written when the layout is needed.

**Forward declarations fixed the PARAMETERS and cannot fix the MEMBERS.** A parameter taken by
reference needs the name; a member needs the layout, and no declaration supplies it. Where the
include graph has a cycle, no ordering exists in which both classes are complete before the other --
that is a property of C++ value semantics and not of this generator.

## What AL actually says

`var AzureOpenAIImpl: Codeunit "Azure OpenAI Impl."` declares an INSTANCE the platform provides. It
is not laid out inside the owner: AL objects are reached through the platform's own object table,
and `SingleInstance` decides whether a session shares one or gets a fresh one
(`devenv-singleinstance-property.md`, default false). C++ made it a member by value because that is
what reads like AL -- and it is the one place where reading like AL and BEING like AL disagree.

## The candidates, none free

| way | costs |
|---|---|
| a member by value | cannot compile where the include graph has a cycle, which it does |
| a pointer built in the constructor | one allocation per codeunit variable, on the path every call takes |
| a pointer built on first use | the allocation moves but does not go, and every access grows a branch |
| the SESSION holds the instances, the member holds a reference | no allocation per call, one lookup, and `SingleInstance` falls out of it -- but the session then knows something about objects |
| the generator breaks cycles: value where it can, indirect where it cannot | keeps the cheap case cheap and needs the include GRAPH, which the transpiler is the only thing that has |

The last two are the ones worth measuring against each other. The fourth is what BC itself does --
a codeunit is reached through the platform, not embedded in its caller -- and the fifth is what
keeps `no allocation on the hot path` true for the 95 % of members that are in no cycle at all.

## What has to be measured before choosing

- How many codeunit members sit in an include cycle. `make tree` counts `field has incomplete type`
  as a first diagnostic, which is that number.
- What a codeunit instance costs to construct, because if it is nothing then indirection buys
  nothing either.

## What is true when this closes

- A codeunit variable of any type compiles, cycle or not.
- `SingleInstance = true` shares one instance per session and the default does not, which is what
  the property says and what board:0027 already needs for interface implementations.
- The choice is recorded with the measurement that decided it, not with the argument that sounded
  best.
