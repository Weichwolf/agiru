Type:     task
Status:   open
Parent:   0035
Area:     gen
Source:   developer/attributes/devenv-withevents-attribute.md
Verdict:  fehlt
Class:    activation

# A `[WithEvents]` variable subscribes to a .NET type's events, and the triggers are AL

`[WithEvents]` on a VARIABLE -- "Sets whether a DotNet variable subscribes to the events published
by a .NET Framework type." On-premises only.

`devenv-dotnet-subscribe-to-events.md` gives the shape: declare a `DotNet` variable of, say,
`System.Timers.Timer` with `[WithEvents]`, and the .NET type's `Elapsed` event is then handled by a
TRIGGER in the AL object. So it is a third event mechanism, beside AL's own publishers
(board:0057) and the platform's trigger events -- and this one crosses into a runtime agiru does not
have.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**27 `[WithEvents` declarations.**

## The IST-state

The attribute parses into the raw list and is dropped, along with the `DotNet` variable and the
trigger that would handle the event.

## The choice

**A REFUSAL, named.** The three .NET-variable attributes divide by what ignoring them costs, and
this is the most expensive: dropping it means the AL trigger that handles the event is emitted and
never called, so a timer never fires and the object waiting on it waits forever -- a hang rather
than a wrong answer.

The shape if it is ever honoured: the rebuilt C++ class exposes a callback slot, `[WithEvents]`
makes the generator bind the AL trigger to it, and the trigger becomes a member function pointer.
That is a small mechanism, and it is not built because CLAUDE.md's .NET route rebuilds the classes
this tree NEEDS -- `StringBuilder`, `MemoryStream`, `XmlDocument` -- and a `System.Timers.Timer` is
not among them.

## Ordering

Behind board:0035, which decides which .NET classes exist here at all. 27 sites.

## Gate, and its negative control

A `DotNet` variable marked `[WithEvents]` must FAIL the translation, naming it and the trigger that
would have handled the event.

**The negative control is naming the trigger.** A refusal that names only the variable leaves the
reader to find the dead trigger, which is the part that would otherwise hang.
