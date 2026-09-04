Type:     task
Status:   open
Parent:   0057
Area:     gen
Source:   developer/attributes/devenv-externalbusinessevent-attribute.md
Verdict:  fehlt
Class:    activation

# An `[ExternalBusinessEvent]` carries a name, a display name, a description, a category and a version

`[ExternalBusinessEvent(Name: Text, DisplayName: Text, Description: Text, Category: enum [, Version: Text])]`

Unlike the three internal publisher kinds, this one is METADATA before it is dispatch: the four
required arguments describe the event to an external subscriber that is not an AL object at all.
`devenv-business-events-overview.md` names the consumer -- Dataverse and Power Automate through the
`Business Central Virtual Table` app.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

**25 `[ExternalBusinessEvent` declarations**, and `devenv-deprecate-external-business-events.md`
documents how they are retired -- so the population is small and already has a deprecation path.

## The IST-state

The attribute is NOT in `IsPublisher` (`src/gen/CodeunitWriter.cpp:29` lists only `IntegrationEvent`,
`BusinessEvent`, `InternalEvent`), so a procedure carrying it is emitted as an ordinary method whose
body is translated. That is a different defect from the other three: they are recognised and empty,
this one is not recognised at all.

## The choice

**The declaration is carried and the transport is not.** The generator adds the attribute to
`IsPublisher` so the body is empty like every other publisher's, and emits the five arguments as
`constexpr` metadata beside the codeunit. The event is then raiseable through board:0057's
dispatcher for any in-process subscriber.

**What is deliberately NOT built**: the external transport. It ends at Dataverse, which `scope.json`
excludes entry for entry. An external business event with no external subscriber is a publisher
nobody subscribed to -- which board:0057 already says should SAY SO rather than be silent.

## Ordering

After 0196 and the dispatcher. Low: 25 sites, no in-tree consumer.

## Gate, and its negative control

A procedure carrying the attribute is emitted with an EMPTY body and its five arguments appear in
the metadata. **The negative control is the body**: today it is translated, so a gate asserting the
body is empty goes red before the change and green after -- which is the only way to see that
`IsPublisher` learned the attribute.
