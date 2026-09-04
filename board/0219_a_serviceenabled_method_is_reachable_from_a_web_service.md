Type:     task
Status:   open
Parent:   0190
Area:     gen
Source:   developer/attributes/devenv-serviceenabled-attribute.md
Verdict:  fehlt
Class:    activation

# A `[ServiceEnabled]` method is reachable from a web service, and nothing else in the object is

`[ServiceEnabled]` -- "Exposes a method to the service." A codeunit published as a web service
exposes ONLY its `[ServiceEnabled]` methods; every other procedure, global or not, stays internal.

So the attribute is an ACCESS decision, and its absence is the default: not exposed.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**131 `[ServiceEnabled` declarations.**

## The IST-state

The attribute parses into the raw list and is dropped.

## The choice

`constexpr` metadata beside the method -- one flag -- so the exposed set is decidable when the tree
is compiled rather than by walking the object at publish time.

**The consumer does not exist.** SOAP and OData are phase 3 and unclaimed; `devenv-restapi-overview`
and the eight web-service root pages have no epic. So the flag is carried and nothing reads it, and
that is stated rather than hidden: this item closes when the flag is emitted, not when a web service
can call it.

**Why it is still worth doing early.** The exposed set is a SECURITY surface, and computing it from
declarations is free now and archaeology later. A runtime that decided exposure at publish time
would have to re-derive from AL what the generator already knew.

## Ordering

Low, and it does not block anything. It is one of the four attributes whose consumer is phase 3
(with 0192, 0214 and the OData half of 0197).

## Gate, and its negative control

An annotated method carries the flag in the metadata and an unannotated one does not.
**The negative control is the unannotated method** -- a generator that marks everything exposed
turns a closed surface into an open one, which is the direction that matters.
