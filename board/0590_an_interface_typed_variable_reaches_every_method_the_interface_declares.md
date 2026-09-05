Type:     task
Status:   open
Parent:   0035
Area:     gen, rt
Verdict:  fehlt
Class:    compile root

# An interface-typed variable reaches every method the interface declares

`NoSeriesImpl` (Business Foundation) holds a variable of type `Interface "No. Series - Single"`
and calls `GetLastNoUsed` on it; the generated `agiru::Implementation<...>` wrapper has no such
member (settle run, 2026-09-06). The wrapper is what an interface-typed variable becomes, and it
must carry the interface's whole surface, dispatching to the implementation the enum value chose
(`ImplementationOf`).

## The choice

`Implementation<I>` is generated PER INTERFACE from the interface object: one forwarding member
per declared method, each calling through the held implementation pointer. The interface writer
(`WriteInterface`) already emits the abstract class; the wrapper is the same list with bodies.

## Population

The whole No. Series subsystem of Business Foundation stands behind this one, and every test that
posts a document numbers it through No. Series. It blocks the `no_series/` sources from the slice
until it lands.
