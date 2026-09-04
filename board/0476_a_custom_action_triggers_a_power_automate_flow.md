Type:     task
Status:   open
Parent:   0067
Area:     gen
Source:   developer/properties/devenv-customactiontype-property.md, developer/properties/devenv-flowid-property.md, developer/properties/devenv-flowtemplateid-property.md, developer/properties/devenv-flowtemplatecategoryname-property.md, developer/properties/devenv-flowenvironmentid-property.md
Verdict:  fehlt
Class:    silent-wrong-data

# A custom action triggers a Power Automate flow

**Five pages, one item**: the custom action's type and the four identifiers that address the flow it
runs. They are one integration, and none of the four ids means anything without the type.

> **CustomActionType** (Page Custom Action, runtime 10.0): `Flow` -- **"an action that can trigger a
> Power Automate Flow"**; `FlowTemplate` (runtime 11.0) -- opens the template editor;
> `FlowTemplateGallery` -- opens the gallery.
>
> **FlowId**, **FlowTemplateId**, **FlowTemplateCategoryName**, **FlowEnvironmentId**: the
> identifiers. `FlowCaption` (board:0394) is the fifth and measures 0.

**Power Automate is a cloud service agiru has no connection to**, and unlike board:0364's `CRM`
tables -- where a local table for a Dataverse entity would return rows nobody wrote -- an unreachable
flow action is inert: it renders a button that cannot work.

**So the decision is REFUSE, and the reason is different from the zero-population refusals**: 150
declarations exist, the feature is well-defined, and it targets a service outside this system. A
button that silently does nothing is the worst of the three options; a refusal names the gap.

## Population, measured 2026-09-04 over `~/Git/BCApps/src`

`CustomActionType =` **150** · `FlowCaption =` **0** (board:0394).

The four id properties are measured with this item when it is pulled; `CustomActionType` at 150 is the
gate on all of them, since a custom action without it is not a flow action.

## The IST-state

`src/gen/PageWriter.cpp` consumes `SourceTable` alone; there is no outbound HTTP integration of any
kind beyond board:0035's declared surface.

## The choice

**Refuse a `customaction` whose `CustomActionType` names a flow**, naming the property and the action,
with this item as the reason: the target service is outside agiru and a rendered button that cannot
run is worse than a translation error somebody can read.

**What would change it**: an outbound HTTP path and a decision that agiru talks to Power Automate,
which is a scope question CLAUDE.md's "complete BC business functionality" sentence does not settle --
the flows are a bridge to a cloud service, and `scope.json` already excludes the cloud bridges.

That last point is the argument and it belongs here rather than in a commit message.

## Ordering

With board:0067's census. No runtime work.

## Gate, and its negative control

A page declaring a custom action of type `Flow` fails to transpile with a message naming the action.

**The negative control is a page customization with no custom actions** -- it must transpile, and the
refusal must not catch the `customaction` keyword itself, only a flow-typed one.
